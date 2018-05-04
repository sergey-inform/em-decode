

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#include "gzopen.h"


FILE * gzopen(const char * filename)
/** 
 *  Fork a gunzip process.
 *  Return pipe to gunzip process or NULL on error.
 */
{
	int fd;
	int fd_stdin = 0;
	FILE * file;

	if ( !strcmp(filename, "-") ) {  // get input from stdin
		if (isatty(fileno(stdin))) {  // stdin is a terminal
			fprintf(stderr, "%s: No input file specified, stdin is a terminal. RTFM! \n",
                                        program_invocation_short_name);
			return NULL;
		}
	}
	else {
		fd = open(filename, O_RDONLY );
		if (fd == -1) {
			fprintf(stderr, "%s: Couldn't open file %s; %s\n",
					program_invocation_short_name, filename, strerror (errno));
			return NULL;
		}
		fd_stdin = dup(STDIN_FILENO);
		dup2(fd, STDIN_FILENO);
	}

	file = popen("gzip -dcfq -", "r");
	
	dup2(fd_stdin, STDIN_FILENO);  // restore stdin	
	return file;	
}
