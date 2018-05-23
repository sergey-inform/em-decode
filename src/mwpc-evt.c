/**
 * Parse EuroMISS raw data, output binary event_raw_idx structs for further processing.
 *
 * Autor: Sergey Ryzhikov <sergey.ryzhikov@ihep.ru>, Nov 2017
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

#include "em5-parser.h"
#include "uDAQ.h"
#include "gzopen.h"


const char *argp_program_version = "em-idx 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey.ryzhikov@ihep.ru>";
static char doc[] = "\nParse EuroMISS raw data file, " \
	"output valid em_raw_idx structures to stdout, print error counts and stats to stderr.\n";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
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
	unsigned mod_cnt_ok[EM_MAX_MODULE_NUM] = {0};  // valid events per module
	unsigned mod_cnt_words_ok[EM_MAX_MODULE_NUM] = {0};  // valid events per module

	struct em5_parser parser = {{0}};
	enum em5_parser_ret ret;

	struct daq_event_info event = {0};
	
	struct daq_ts_info dts = {{0}};	
	unsigned ts_prev = 0;	


	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = em5_parser_next(&parser, wrd);
		
		if (ret == RET_EVENT) {
			// output struct event_info to outfile
			//
			if (outfile) {
				// fill struct
				//event.ts = parser.evt.ts;
				dts.dts = parser.evt.ts - ts_prev;
				ts_prev = parser.evt.ts;
				// write to file
				//
				fwrite(&dts, sizeof(dts), 1, outfile);
				printf("%d %d\n", dts.dts, parser.evt.ts);
				// clean up
				memset(&dts, 0, sizeof(dts));
			}

			// add word counters per module
			if (!parser.evt.dirty) {
				for (int i = 0; i<EM_MAX_MODULE_NUM; i++) {
					if (parser.evt.mod_cnt[i]) {
						mod_cnt_words_ok[i] += parser.evt.mod_cnt[i];
						mod_cnt_ok[i] +=1;
					}
				}
			}

		}

		wofft += 1;
	}

	/// Print event counters
	for (int i = RET_EVENT; i<RET_WARNING; i++) {
		if(em5_parser_retstr[i] && parser.ret_cnt[i])
			fprintf(errfile, "%s\t %d \n"
				, em5_parser_retstr[i]
				, parser.ret_cnt[i]
				);
	}

	fprintf(errfile, "%s\t %d \n"	
		,"CNT_EM_EVENT_DIRTY"
		, parser.dirty_cnt
		);

	/// Print module numbers
	fprintf(errfile, "MODULES: ");
	for (int i = 0; i<EM_MAX_MODULE_NUM; i++) {
		if(mod_cnt_ok[i]) {
			fprintf(errfile, "%d ", i);
		}
	}
	fprintf(errfile, "\n");
		
	
	/// Print error counters
	fprintf(errfile, "-- Errors:\n");

	word_count = 0;
	for (int i = 0; i< MAX_EM5_PARSER_RET; i++) 
		word_count += parser.ret_cnt[i];  //FIXME: use parser.word_cnt
	
	if (word_count != parser.word_cnt) {
		fprintf(errfile, "WRONG WORD COUNT! %+d \n",
				word_count - parser.word_cnt);
	}

	fprintf(errfile, "%-25s\t %d \n"
		,"WORDS_TOTAL"
		,parser.word_cnt
		);

	for (int i = RET_WARNING + 1; i<MAX_EM5_PARSER_RET; i++) {
		if(em5_parser_retstr[i] && parser.ret_cnt[i])
			fprintf(errfile, "%-25s\t %d \n"
				, em5_parser_retstr[i]
				, parser.ret_cnt[i]
				);
	}

	/// Print module stats
	if (args->stats) {
		fprintf(errfile, "-- Stats:\n");
		
		for (int i = 0; i<EM_MAX_MODULE_NUM; i++) {
			if(mod_cnt_ok[i]) {
				fprintf(errfile, "MOD_%d: %d %d\n"
					,i
					,mod_cnt_ok[i]
					,mod_cnt_words_ok[i]
					);
			}
		}
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

	args.outfile = "-";  // default
	argp_parse(&argp, argc, argv, 0, 0, &args);
	//dump_args(&args);
	
	// outfile
	if ( !strcmp(args.outfile, "-") ) {  // output to stdout
		if (isatty(fileno(stdout))) {  // stdout printed on terminal
			errno = EPIPE;
			fprintf(stderr, "%s: Attempt to send binary output to terminal; %s\n",
                                        program_invocation_short_name, strerror (errno));
			exit(EX_USAGE);
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
	
	infile = gzopen(args.infile);
	if (!infile) 
		error(EX_IOERR, errno, "can't open file '%s'", args.infile);	

	if (infile && outfile)
		err = em_parse(infile, outfile, stderr, &args);
	

	return err;
}
