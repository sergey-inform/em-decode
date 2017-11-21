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
#include <stdlib.h> //malloc()
#include <assert.h>
#include <inttypes.h>
#include <sysexits.h>

const char *argp_program_version = "em-parse 2.0";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey-inform@ya.ru>";
static char doc[] = "Parse EuroMISS raw data, output valid event data.\n" \
	"Use --verbose and --debug flags to understand what is going on." \
	"\v Text after opts.";
static char args_doc[] = "CrateID [FILENAMES...]";

static struct argp_option options[] = { 
	{0,0,0,0, "CrateID is an integer number." },
	{ "debug", 'd', 0, 0, "Interprete input word by word."},
	{ "verbose", 'v', 0, 0, "Trace all events to stderr."},
	{ "quiet", 'q', 0, 0, "Print to stderr only errors."},
	{ "stats", 's', 0, 0, "Print statistics."},
	{ "output", 'o', "OUTFILE", 0, "Output events to OUTFILE instead of stdout."},
	{ 0 } 
};

struct args {
	bool debug, verbose, quiet, stats;
	char **infiles;
	char *outfile;
	unsigned crate_id;
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
			if (state->arg_num >= 2){
				argp_usage(state);
			}
			crate_id = strtoul(arg, NULL, 0 /*base*/);
			if (crate_id < 0) {
				argp_failure(state, EX_USAGE, EINVAL,
					"CrateID should be unsigned integer"\
					", but '%s' is given.",
					arg);

			}
			// consume the rest of arguments as filenames
			args->infiles = &state->argv[state->next];
			state->next = state->argc;
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
		"CrateId %d \n" \
		"flags: %s%s%s%s \n",
		args->outfile,
		args->crate_id,
		args->verbose ? "verbose ": "",
		args->stats ? "stats ": "",
		args->debug ? "debug ": "",
		args->quiet ? "quiet ": ""
		);

	printf("infiles: ");
	for (j = 0; args->infiles[j]; j++)
		printf(j==0 ? "%s" : ", %s", args->infiles[j]);
	printf("\n");
	fflush(stdout);

}


int check_infiles( char ** infiles)
/** Check input files exists and readable */
{
	return 0;
}


int main(int argc, char *argv[])
{
	struct args args = {0};
	// Defaults
	args.outfile = "-";
	args.infiles = (char *[]) {"-", NULL};	
	
	argp_parse(&argp, argc, argv, 0, 0, &args);
	
	//dump_args(&args);
	
	if ( check_infiles(args.infiles)) {
		exit(1);
	}
		

	exit(0);
}

