

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

	if ( !strcmp(filename, "-") ) {  // get input from stdin
		if (isatty(fileno(stdin))) {  // stdin is a terminal
			perror("No input file specified, stdin is a terminal. RTFM\n");
			return NULL;
		}
	}
	else {
		fd = open(filename, O_RDONLY | O_NOATIME);
		if (fd == -1) {
			fprintf(stderr, "%s: Couldn't open file %s; %s\n",
					program_invocation_short_name, filename, strerror (errno));
			return NULL;
		}
		dup2(fd, STDIN_FILENO);
	}

	return popen("gzip -dcfq -", "r");

}
