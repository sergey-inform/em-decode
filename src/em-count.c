/**
 * Parse EuroMISS raw data and count timestamps.
 *
 * Autor: Sergey Ryzhikov <sergey.ryzhikov@ihep.ru>, May 2018
 * Made for SPASCHARM experiment.
 */

#define _GNU_SOURCE

#include <argp.h>
#include <error.h>
#include <string.h> //strcmp
#include <stdlib.h> //strtol
#include <unistd.h> //dup
#include <sysexits.h>
#include <assert.h>

#include "parser-em5.h"
#include "uDAQ.h"
#include "gzopen.h"


const char *argp_program_version = "em-ts 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey.ryzhikov@ihep.ru>";
static char doc[] = "\nParse EuroMISS raw data file, count events.\n";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{ 0 } 
};

struct args {
	char *infile;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct args *args = state->input;
	switch (key) {
		case ARGP_KEY_NO_ARGS:
			// stdin by default;
			args->infile = "-";
			break;

		case ARGP_KEY_ARG: 
			if (state->arg_num == 0) {  // FILENAME
				args->infile = arg;
			}
			else {
				argp_usage(state); // catch a typo
			}
			break;

		default: return ARGP_ERR_UNKNOWN;
	}

	return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc };


int em_count( FILE * infile, FILE * outfile, FILE * errfile, struct args * args)
{
	emword wrd;
	struct parser_em5 parser = {{0}};
	enum parser_em5_ret ret;
	unsigned bytes;

	unsigned count = 0;

	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = parser_em5_next(&parser, wrd);
		if (ret == RET_EVENT)
			count += 1;

	}
	printf("%d \n", count);
	return 0; 
}


int main(int argc, char *argv[])
{
	struct args args = {0};
	FILE * infile = NULL;
	int err;

	argp_parse(&argp, argc, argv, 0, 0, &args);
	
	infile = gzopen(args.infile);
	if (!infile) 
		error(EX_IOERR, errno, "can't open file '%s'", args.infile);	

	if (infile)
		err = em_count(infile, NULL, stderr, &args);
	
	return err;
}
