/**
 * Parse Epicur MWPC (multi wire proportional chambers)
 * raw data, output binary daq_evt_idx structs.
 *
 * In present Epicur format is based on EuroMISS data format
 * (which is a subject of further discussion).
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

//TODO: common main function for all small programms

const char *argp_program_version = "mwpc-evt 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey.ryzhikov@ihep.ru>";
static char doc[] = "\nParse raw data for Epicur MWPC (multiwire proportional chambers), " \
	"output valid daq_evt_idx structures to stdout, print error counts and stats to stderr.\n";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "output", 'o', "OUTFILE", 0, "Instead of stdout, output events to OUTFILE."},
//	{ "stats", 's', 0, 0, "Print some statistics per channel."}, //TODO
	{ 0 } 
};

struct args {
	char *infile;
	char *outfile;
//	bool stats;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct args *args = state->input;
	switch (key) {
//		case 's': args->stats = true; break;
		case 'o': args->outfile = arg; break;

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


int mwpc_parse( FILE * infile, FILE * outfile, FILE * errfile, struct args * args)
/** Since the data format is based on EuroMISS,
 * process data with em5 state machine. 
 *
 * Generates daq_evt_idx structures and put them to outfile.
 * Prints error counts and stats to errfile.
*/
{
	size_t bytes;
	emword wrd;
	unsigned wofft_prev = 0;

	struct parser_em5 parser = {{0}};
	enum parser_em5_ret ret;

	struct daq_evt_idx evt_idx = {0};

	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = parser_em5_next(&parser, wrd);
		
		if (ret == RET_EVENT) {

			// fix bug with LEN_1F
			if (parser.evt.len == parser.evt.len_1f + 1) {
				parser.evt.dirty = false;
			}

			if (outfile) {
				// fill struct
				evt_idx.flags |= parser.evt.dirty ? DAQ_EVENT_DIRTY : 0;
				//TODO: evt_idx.flags |= parser.evt.err...

				evt_idx.dwoff = parser.evt.woff - wofft_prev;
				wofft_prev = parser.evt.woff;

///				fprintf(stdout, "%d\t%d\n", parser.evt.ts, evt_idx.dwoff);
				// write to file
				fwrite(&evt_idx, sizeof(evt_idx), 1, outfile);
				// clean up
				memset(&evt_idx, 0, sizeof(evt_idx));
			}
		}
	}

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
		err = mwpc_parse(infile, outfile, stderr, &args);
	

	return err;
}
