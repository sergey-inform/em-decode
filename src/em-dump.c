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
#include "gzopen.h"


const char *argp_program_version = "em-dump 1.2";
const char *argp_program_bug_address = "\"Sergey Ryzhikov\" <sergey.ryzhikov@ihep.ru>";
static char doc[] = "\n" \
	"Parse EuroMISS raw data file, print decoded data with errors to stdout.\n" \
	"Use it to understand what's wrong in data." \
	"\v" \
	" Columns: \n" \
	"  offset  data addr [ mod(dec) chan(dec) data(dec) | state ] errstr\n" \
	"\n" \
	" Example: \n" \
	"  000000  7331 00be  PCHI          - \n" \
	"  000001  0c80 0001 | 01 00  3200  - \n" \
	"  000002  0300 0001 | 01 00   768  - \n" \
	"  ...\n" \
	"  003576  1313 0001 | 01 00  4883  - \n" \
	"  003577  0313 0002 | 02 00   787  - \n" \
	"  003578  0313 0002 | 02 00   787  ERR_EM_DUPWORD \n" \
	"  ...\n" \
	"  00359a  1313 0007 | 07 00  4883  - \n" \
	"  00359b  002b 001f  END           X \n" \
	"  00359c  07cf 00fe  -             X \n" \
	"\n" \
	"  `X` in errstr means the event is corrupted\n" \
	"";

static char args_doc[] = "[FILENAME]";

static struct argp_option options[] = { 
	{0,0,0,0, "If no FILENAME, waits for data in stdin." },
	{0,0,0,0, "Options:" },
	{ "events", 'e', 0, 0, "Print event data"},
	{ "nodec", 'd', 0, 0, "Do not show data in decimal"},
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
			args->infile = "-";  // default
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
	unsigned errcnt = 0;

	int prwidth = 12;

	if (args->nodec)
		prwidth = 5;

	
	while ((bytes = fread(&wrd, 1 /*count*/, sizeof(emword), infile)))
	{
		if (bytes != sizeof(emword)) {
			//ERR FILE_LEN_ODD 
			break;
		}
		
		ret = em5_parser_next(&parser, wrd);
		
		// Print dump
		if ((ret > RET_WARNING) || ! args->quiet ) {
			errcnt +=1; 	
			if (parser.state == PCH_DATA)  // instead of parser state decode address and data words 
				if (args->nodec)  // no decode
					fprintf(outfile, "%06lx  %04x %04x  %-*s  %s \n"
						,(long unsigned int)wofft
						,wrd.data
						,wrd.addr
						,prwidth  // field width
						,"."
						,em5_parser_retstr[ret]
						);
				else  // decode hex as dec
					fprintf(outfile, "%06lx  %04x %04x | %02d %02d %5d  %s \n"
						,(long unsigned int)wofft
						,wrd.data
						,wrd.addr
						,EM_ADDR_MOD(wrd.addr)
						,EM_ADDR_CHAN(wrd.addr)
						,wrd.data
						,em5_parser_retstr[ret]
						);
		
			else 
				fprintf(outfile, "%06lx  %04x %04x  %-*s  %s \n"
					,(long unsigned int)wofft
					,wrd.data
					,wrd.addr
					,prwidth  // field width
					,em5_protocol_state_str[parser.state]
					,ret != RET_OK && ret != RET_EVENT ? em5_parser_retstr[ret] : parser.evt.corrupt ? "X" : "-"
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

	if (errcnt) return 1; 
	return 0; 
}


int main(int argc, char *argv[])
{
	struct args args = {0};
	FILE * infile = NULL;
	int err;
	
	argp_parse(&argp, argc, argv, 0, 0, &args);
	infile = gzopen(args.infile);

	if (infile)
		err = em_dump(infile, stdout, &args);

	return err;
}
