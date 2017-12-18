/**
 * Parse EuroMISS raw data.
 * Check the file syntax and optionally print some statistics.
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

#include "em5-fsm.h"
#include "uDAQ.h"


const char *argp_program_version = "em-parse 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey-inform@ya.ru>";
static char doc[] = "\nParse EuroMISS raw data file, output valid em-event structures.\n" \
	"\v How to use:\n" \
	"Use --verbose and --debug flags to understand what's going on." \
	"...\n.";  // FIXME
static char args_doc[] = "CrateID [FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "CrateID is an integer number, decimal or hex with '0x' prefix." },
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "debug", 'd', 0, 0, "Interprete input word by word."},
	{ "verbose", 'v', 0, 0, "Trace all events to stderr."},
	{ "stats", 's', 0, 0, "Print statistics."},
	{ "output", 'o', "OUTFILE", 0, "Instead of stdout output events to OUTFILE."},
	{ 0 } 
};

struct args {
	bool debug, verbose, stats;
	char *infile;
	char *outfile;
	unsigned crate_id;
	bool no_output;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	long crate_id;

	struct args *args = state->input;
	switch (key) {
		case 's': args->stats = true; break;
		case 'v': args->verbose = true; break;
		case 'd': args->debug = true; break;
		case 'o': args->outfile = arg; break;

		case ARGP_KEY_NO_ARGS:
			argp_usage(state);

		case ARGP_KEY_ARG: 
			if (state->arg_num == 0) {  // CrateID
				crate_id = strtoul(arg, NULL, 0 /*base*/);

				// check CrateID is valid
				if (crate_id < 0) {
					argp_failure(state, EX_USAGE, EINVAL,
						"CrateID should be unsigned integer"\
						", but '%s' is given.",	arg);
				}
				args->crate_id = (unsigned int)crate_id;
			
			}
			else if (state->arg_num == 1) {  // FILENAME
				args->infile = arg;
			}
			else {
				argp_usage(state); // more then two args is a typo
			}
			break;

		default: return ARGP_ERR_UNKNOWN;
	}

	return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc };


void dump_args( struct args * args) 
/** Print command line args */
{
	printf(	"outfile %s \n" \
		"infile %s \n" \
		"CrateId %d \n" \
		"flags: %s%s%s \n" 
		, args->outfile
		, args->infile
		, args->crate_id
		, args->verbose ? "verbose ": ""
		, args->stats ? "stats ": ""
		, args->debug ? "debug ": ""
		);

	printf("\n");
	fflush(stdout);
}


int em_parse( FILE * infile, FILE * outfile, struct args * args)
/** Process data with em5 state machine. 
Generates daq_event_info structures and put them to outfile.
Prints errors/debug info to stderr.
*/
{
	size_t wofft = 0;
	size_t bytes = 0;
	emword wrd;

	struct em5_fsm fsm = {0};
	enum em5_fsm_ret ret;
	int len_diff;


	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = em5_fsm_next(&fsm, wrd);

		if (args->debug || ( args->verbose && ret > FSM_ERROR) ) {
			if (fsm.state == DATA) 
				fprintf(stderr, "%06lx  %02d %02d  %04x  %-6s %s\n"
					,wofft
					,EM_ADDR_MOD(wrd.addr)
					,EM_ADDR_CHAN(wrd.addr)
					,wrd.data
					,fsm.state == DATA ? "." :em5_fsm_statestr[fsm.state]
					,em5_fsm_retstr[ret]
					);
		
			else 
				fprintf(stderr, "%06lx  0x%04x %04x  %-6s %s\n"
					,wofft
					,wrd.addr
					,wrd.data
					,fsm.state == DATA ? "." :em5_fsm_statestr[fsm.state]
					,em5_fsm_retstr[ret]
					);
		}
		
		if (ret == FSM_EVENT) {
			//FIXME: output event_info to outfile
			


			len_diff = (int)(fsm.evt.len & EM_STATUS_COUNTER) - fsm.evt.len_1f;

			if (args->verbose) {
				fprintf(stderr, "# Event %d\tts: %u\tlen: %d (%d)\t \n"
				, fsm.ret_cnt[FSM_EVENT]
				, fsm.evt.ts
				, fsm.evt.len
				, len_diff
				);
			}

		}

		wofft += 1;
	}


	for (int i = FSM_EVENT; i<MAX_EM5_FSM_RET; i++) {
		if(em5_fsm_retstr[i] && fsm.ret_cnt[i])
			fprintf(stderr, "%d\t %s \n"
				, fsm.ret_cnt[i]
				, em5_fsm_retstr[i]
				);
	}

	return 0; 
}


int main(int argc, char *argv[])
{
	struct args args = {0};
	FILE * outfile = NULL;
	FILE * infile = NULL;
	int err;

	// Defaults
	args.outfile = "-";
	args.infile = "-";	
	
	argp_parse(&argp, argc, argv, 0, 0, &args);
//	dump_args(&args);
	
	// outfile
	if ( !strcmp(args.outfile, "-") ) {  // output to stdout
		if (isatty(fileno(stdout))) {  // stdout printed on terminal
			fprintf(stderr, "Binary output to terminal, srsly? Output suppressed.\n");
		}
		else {
			outfile = fdopen(dup(fileno(stdout)), "wb"); // force binary output ...
					/*... for compatibility with lame systems */
		}
	} else {
		outfile = fopen( args.outfile, "wb");  // rewrite outfile if exists
		if (outfile == NULL) {
			error(EX_IOERR, errno, "can't open file '%s'", args.outfile);
		}
	}

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



	err = em_parse(infile, outfile, &args);

	if (infile)	fclose(infile);
	if (outfile)	fclose(outfile);

	return err;
}

