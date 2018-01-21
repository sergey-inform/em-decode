/**
 * Decode EuroMISS raw data and print it word by word.
 *
 * Autor: Sergey Ryzhikov <sergey.ryzhikov@ihep.ru>, Nov 2017
 * Made for SPASCHARM experiment.
 */

#include <argp.h>
#include <stdbool.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <stdlib.h> //malloc()
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <sysexits.h>

#include <assert.h>

#include "em5-parser.h"
#include "uDAQ.h"


const char *argp_program_version = "em-debug 1.0";
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
	{ "quiet", 'q', 0, 0, "Print only errors"},
	{ 0 } 
};

struct args {
	bool events, quiet;
	char *infile;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct args *args = state->input;
	switch (key) {
		case 'q': args->quiet = true; break;
		case 'e': args->events = true; break;

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


int em_debug( FILE * infile, FILE * outfile, struct args * args)
/** Process data with em5 state machine. 
Prints errors/debug info to outfile.
*/
{
	size_t wofft = 0;
	size_t bytes = 0;
	emword wrd;

	struct em5_parser parser = {0};
	enum em5_parser_ret ret;
	int len_diff;


	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = em5_parser_next(&parser, wrd);

		if (!args->quiet || (ret > RET_ERROR) ) {
			
/*			if (parser.state == DATA) 
				fprintf(outfile, "%06lx  %02d %02d  %04x  %-6s %s\n"
					,wofft
					,EM_ADDR_MOD(wrd.addr)
					,EM_ADDR_CHAN(wrd.addr)
					,wrd.data
					,fsm.state == DATA ? "." :em5_fsm_statestr[fsm.state]
					,em5_fsm_retstr[ret]
					);
		
			else 
*/
				fprintf(outfile, "%06lx  0x%04x %04x  %s %s\n"
					,wofft
					,wrd.addr
					,wrd.data
					, em5_protocol_state_str[parser.state]
					//, emword_class_str[emword_classify(wrd, &parser)] //FIXME DELME
					,ret == RET_OK?parser.evt.corrupt? "X":".":em5_parser_retstr[ret]
					);
		}
	
		if (ret == RET_EVENT && args->events) {
			len_diff = (int)(parser.evt.len & EM_STATUS_COUNTER) - parser.evt.len_1f;

			fprintf(outfile, "# Event %d\tts: %u\tlen: %d (%d)\t \n"
			, parser.ret_cnt[RET_EVENT]
			, parser.evt.ts
			, parser.evt.len
			, len_diff
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

	err = em_debug(infile, stdout, &args);

	if (infile)	fclose(infile);

	return err;
}

