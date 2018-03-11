/**
 * Decode EuroMISS raw data and print it word by word.
 *
 * Autor: Sergey Ryzhikov <sergey.ryzhikov@ihep.ru>, Nov 2017
 * Made for SPASCHARM experiment.
 */

#define _GNU_SOURCE

#include <stdio.h>  //fdopen, fileno
#include <argp.h>
#include <stdbool.h>
#include <error.h>
#include <string.h>
#include <stdlib.h> //malloc()
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <sysexits.h>

#include "em5-parser.h"
#include "uDAQ.h"


const char *argp_program_version = "em-dump 1.2";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey.ryzhikov@ihep.ru>";
static char doc[] = "\n" \
	"Parse EuroMISS raw data file, print decoded data with errors to stdout.\n" \
	"\v" \
	"Use it to understand what's wrong in data.";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "events", 'e', 0, 0, "Print event data."},
	{ "nodec", 'd', 0, 0, "Do not show data in decimal."},
	{ "quiet", 'q', 0, 0, "Print only errors"},
	{ 0 } 
};

struct args {
	bool events, quiet, nodec;
	char *infile;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct args *args = state->input;
	switch (key) {
		case 'q': args->quiet = true; break;
		case 'e': args->events = true; break;
		case 'd': args->nodec = true; break;

		case ARGP_KEY_NO_ARGS:
			argp_usage(state);

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


int em_dump( FILE * infile, FILE * outfile, struct args * args)
/** Process data with em5 state machine. 
Prints errors/debug info to outfile.
*/
{
	size_t wofft = 0;
	size_t bytes = 0;
	emword wrd;

	struct em5_parser parser = {{0}};
	enum em5_parser_ret ret;
	int len_diff;



	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = em5_parser_next(&parser, wrd);
		
		// Print dump
		if ((ret > RET_ERROR) || ! args->quiet ) {
			
			if (parser.state == PCH_DATA)  // instead of parser state decode address and data words 
				if (args->nodec)  // no decode
					fprintf(outfile, "%06lx  %04x %04x  %s \n"
						,(long unsigned int)wofft
						,wrd.data
						,wrd.addr
						,em5_parser_retstr[ret]
						);
				else
					fprintf(outfile, "%06lx  %04x %04x | %5d %02d %02d | %s \n"
						,(long unsigned int)wofft
						,wrd.data
						,wrd.addr
						,wrd.data
						,EM_ADDR_MOD(wrd.addr)
						,EM_ADDR_CHAN(wrd.addr)
						,em5_parser_retstr[ret]
						);
		
			else 
				fprintf(outfile, "%06lx  %04x %04x  %5s  %s \n"
					,(long unsigned int)wofft
					,wrd.data
					,wrd.addr
					,em5_protocol_state_str[parser.state]
					,ret != RET_OK && ret != RET_EVENT ? em5_parser_retstr[ret] : parser.evt.corrupt ? "X" : "."
					);
		}
		
		// Print events
		if (ret == RET_EVENT && args->events) {
			len_diff = (int)(parser.evt.len & EM_STATUS_COUNTER) - parser.evt.len_1f;

			fprintf(outfile, "# Event %-5d  ts: %-10u  len: %4d "
			, parser.ret_cnt[RET_EVENT]
			, parser.evt.ts
			, parser.evt.len
			);

			// print len_diff
			if (len_diff)
				fprintf(outfile, "(%d)", len_diff);

			fprintf(outfile, "  %s \n",
				parser.evt.corrupt? "CORRUPT" : "OK"
				);

		}
		wofft += 1;
	}

	return 0; 
}


int main(int argc, char *argv[])
{
	struct args args = {0};
	FILE * infile = NULL;
	int err;

	// Defaults
	args.infile = "-";	
	
	argp_parse(&argp, argc, argv, 0, 0, &args);
	
	// infile
	if ( !strcmp(args.infile, "-") ) {  // get input from stdin
		if (isatty(fileno(stdin))) {  // stdin is a terminal
			error(EX_USAGE, errno, "No input file specified, stdin is a terminal. RTFM.\n");
		}
		else {
			infile = fdopen(dup(fileno(stdin)), "rb");
		}
	} else {
		infile = fopen( args.infile, "rbm");
		if (infile == NULL) {
			error(EX_NOINPUT, errno, "can't open file '%s'", args.infile);
		}
	}

	err = em_dump(infile, stdout, &args);

	if (infile)	fclose(infile);

	return err;
}
