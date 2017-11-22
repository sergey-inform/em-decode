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
#include <string.h>
#include <stdlib.h> //malloc()
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <sysexits.h>

const char *argp_program_version = "em-parse 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey-inform@ya.ru>";
static char doc[] = "Parse EuroMISS raw data file, output valid em-event structures.\n" \
	"\v How to use:\n" \
	"Use --verbose and --debug flags to understand what's going on." \
	"...\n.";  // FIXME
static char args_doc[] = "CrateID [FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "CrateID is an integer number, decimal or hex with '0x' prefix." },
	{0,0,0,0, "If no FILENAME, wait for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "debug", 'd', 0, 0, "Interprete input word by word."},
	{ "verbose", 'v', 0, 0, "Trace all events to stderr."},
	{ "quiet", 'q', 0, 0, "Print to stderr only errors."},
	{ "stats", 's', 0, 0, "Print statistics."},
	{ "output", 'o', "OUTFILE", 0, "Instead of stdout output events to OUTFILE."},
	{ 0 } 
};

struct args {
	bool debug, verbose, quiet, stats;
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
		case 'q': args->quiet = true; break;
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
	int j;
	printf(	"outfile %s \n" \
		"infile %s \n" \
		"CrateId %d \n" \
		"flags: %s%s%s%s \n",
		args->outfile,
		args->infile,
		args->crate_id,
		args->verbose ? "verbose ": "",
		args->stats ? "stats ": "",
		args->debug ? "debug ": "",
		args->quiet ? "quiet ": ""
		);

	printf("\n");
	fflush(stdout);

}


int check_infile( char * infile)
/** Check input files exists and readable */
{

	return 0;
}


int main(int argc, char *argv[])
{
	struct args args = {0};
	FILE * outfile;
	FILE * const infile;

	// Defaults
	args.outfile = "-";
	args.infile = "-";	
	
	argp_parse(&argp, argc, argv, 0, 0, &args);
	dump_args(&args);
	
	if ( !strcmp(args.outfile, "-") ) { // output to stdout
		if (isatty(fileno(stdout))) { // stdout is a tty
			args.no_output = true;
			fprintf(stderr, "Binary output to terminal, srsly? Output suppressed.\n");
		}
		else {
			outfile = fdopen(dup(fileno(stdout)), "wb"); // force binary output ...
					/*... for compatibility with lame systems */
		}
	}

	if ( check_infile(args.infile)) {
		
	}
		
	
	fclose(outfile);

	exit(0);
}

