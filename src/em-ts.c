/**
 * Parse EuroMISS raw data for timestamps.
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
static char doc[] = "\nParse EuroMISS raw data file, " \
	"extract timestamps or timestamp diffs in binary or text form.\n" \
	"timestamps are 32-bit unsigned ints\n";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "outfile", 'o', "FILENAME", 0, "Direct output to OUTFILE instead of stdout."},
	{ "diff", 'd', 0, 0, "Timestamp diffs (current minus previous) instead of timestamp."},
	{ "text", 't', 0, 0, "Text output instead of binary."},
	{ 0 } 
};

struct args {
	char *infile;
	char *outfile;
	bool diff;
	bool text;
};


static error_t 
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct args *args = state->input;
	switch (key) {
		case 'd': args->diff = true; break;
		case 't': args->text = true; break;
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


int em_ts( FILE * infile, FILE * outfile, FILE * errfile, struct args * args)
{
	emword wrd;
	struct parser_em5 parser = {{0}};
	enum parser_em5_ret ret;

	unsigned bytes;
	unsigned count = 0;
	unsigned ts_prev = 0;
	unsigned outval;
	unsigned long long diff_summ=0;

	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = parser_em5_next(&parser, wrd);
		if (ret != RET_EVENT)
			continue;

		count += 1;

		if (!outfile) 
			continue;

		if (args->diff) {
			outval = parser.evt.ts - ts_prev;
			diff_summ += outval;
			ts_prev = parser.evt.ts;
		} else {
			outval = parser.evt.ts;	
		}

		if (args->text) {
			fprintf(outfile, "%u\n", outval);
		} else {
			fwrite(&outval, sizeof(outval), 1, outfile); 
		}
	}

		printf(". %lld %d\n", diff_summ , parser.evt.ts);
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
		if (isatty(fileno(stdout)) && ! args.text) {  // binary printed on terminal
			errno = EPIPE;
			fprintf(stderr, "%s: Attempt to send binary output to terminal; %s\n",
                                        program_invocation_short_name, strerror (errno));
			exit(EX_USAGE);
		}
		else {
			outfile = fdopen(dup(fileno(stdout)), "wb"); // force binary output ...
					/*... for compatibility with lame operating systems */
		}
	} else if (!strcmp(args.outfile, "/dev/null")) {
		outfile = NULL;
	} else {
		outfile = fopen( args.outfile, "wb");  // rewrite outfile if exists
		if (outfile == NULL) {
			error(EX_IOERR, errno, "can't open file '%s'", args.outfile);
		}
	}
	
	infile = gzopen(args.infile);
	if (!infile) 
		error(EX_IOERR, errno, "can't open file '%s'", args.infile);	

	if (infile)
		err = em_ts(infile, outfile, stderr, &args);
	
	return err;
}
