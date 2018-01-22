/**
 * Parse EuroMISS raw data, output binary event-info structs for further processing.
 *
 * Autor: Sergey Ryzhikov <sergey.ryzhikov@ihep.ru>, Nov 2017
 * Made for SPASCHARM experiment.
 */

#include <argp.h>
#include <error.h>
#include <string.h> //strcmp
#include <stdlib.h> //strtol
#include <unistd.h> //dup
#include <sysexits.h>
#include <assert.h>

#include "em5-parser.h"
#include "uDAQ.h"


const char *argp_program_version = "em-parse 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey.ryzhikov@ihep.ru>";
static char doc[] = "\nParse EuroMISS raw data file, " \
	"output valid em-event structures to stdout, print error counts and stats to stderr.\n";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "crate" , 'c', "NUM", 0, "CrateID for struct em-event, is an integer number (decimal or hex with '0x' prefix)." },
	{ "output", 'o', "OUTFILE", 0, "Instead of stdout, output events to OUTFILE."},
	{ "stats", 's', 0, 0, "Print error statistics per module."}, //TODO
	{ 0 } 
};

struct args {
	char *infile;
	char *outfile;
	unsigned crate_id;
	bool stats;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	long crate_id;

	struct args *args = state->input;
	switch (key) {
		case 's': args->stats = true; break;
		case 'o': args->outfile = arg; break;
		case 'c': 
			crate_id = strtol(arg, NULL, 0 /*base*/);
			// check CrateID is valid
			if (crate_id < 0) {
				argp_failure(state, EX_USAGE, EINVAL,
					"CrateID should be unsigned integer"\
					", but '%s' is given.",	arg);
				break;
			}
			args->crate_id = (unsigned int)crate_id;
			break;

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


void dump_args( struct args * args) 
/** Print command line args */
{
	fprintf(stderr,
		"outfile %s \n" \
		"infile %s \n" \
		"CrateId %d \n" \
		"flags: %s \n" 
		, args->outfile
		, args->infile
		, args->crate_id
		, args->stats ? "stats ": ""
		);

	printf("\n");
	fflush(stderr);
}


int em_parse( FILE * infile, FILE * outfile, FILE * errfile, struct args * args)
/** Process data with em5 state machine. 
Generates daq_event_info structures and put them to outfile.
Prints error counts and stats to errfile.
*/
{
	size_t wofft = 0;
	size_t bytes = 0;
	emword wrd;
	unsigned word_count;

	struct em5_parser parser = {0};
	enum em5_parser_ret ret;


	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = em5_parser_next(&parser, wrd);
		
		if (ret == RET_EVENT) {
			//FIXME: output struct event_info to outfile
		}

		wofft += 1;
	}

	/// Print event counters
	for (int i = RET_EVENT; i<RET_ERROR; i++) {
		if(em5_parser_retstr[i] && parser.ret_cnt[i])
			fprintf(errfile, "%d\t %s \n"
				, parser.ret_cnt[i]
				, em5_parser_retstr[i]
				);
	}

	fprintf(errfile, "%d\t %s \n"	
		, parser.corrupted_cnt
		,"CNT_EM_EVENT_CORRUPTED"
		);
	
	/// Print word counters
	fprintf(errfile, "--\n");

	word_count = 0;
	for (int i = 0; i< MAX_EM5_PARSER_RET; i++) 
		word_count += parser.ret_cnt[i];
	
	fprintf(errfile, "%-25s\t %d \n"
		,"WORDS_TOTAL"
		,word_count
		);

	for (int i = RET_ERROR + 1; i<MAX_EM5_PARSER_RET; i++) {
		if(em5_parser_retstr[i] && parser.ret_cnt[i])
			fprintf(errfile, "%-25s\t %d \n"
				, em5_parser_retstr[i]
				, parser.ret_cnt[i]
				);
	}

	fprintf(errfile, "\n");

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
	//dump_args(&args);
	
	// outfile
	if ( !strcmp(args.outfile, "-") ) {  // output to stdout
		if (isatty(fileno(stdout))) {  // stdout printed on terminal
			error(EX_USAGE, 0, "Binary output to terminal, srsly? Pipe it to em-dump for printable output!");
		}
		else {
			outfile = fdopen(dup(fileno(stdout)), "wb"); // force binary output ...
					/*... for compatibility with lame operating systems */
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

	err = em_parse(infile, outfile, stderr, &args);

	if (infile)	fclose(infile);
	if (outfile)	fclose(outfile);

	return err;
}
