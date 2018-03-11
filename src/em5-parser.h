#ifndef EM5_PARSER_H
#define EM5_PARSER_H

/* Workaround for gcc -Wunused-variable */
#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#include "em.h"
#include <stdbool.h>


enum em5_parser_ret{
	RET_OK
	, RET_EVENT
	, RET_SYNC
	, RET_END_SPILL
	, RET_ERROR
	, ERR_DMA_OVERREAD
	, ERR_DUP
	, ERR_ZEROES
	, ERR_ONES
	, ERR_UNKNOWN_WORD
	, ERR_PROTOCOL
	, ERR_MISS_LEN
	, ERR_MISS_ADDR_ORDER
	, MAX_EM5_PARSER_RET  // the last element
	};


static const char UNUSED *em5_parser_retstr[] = {
	[RET_OK] = "-"
	, [RET_EVENT] = "CNT_EM_EVENT"
	, [RET_SYNC] = "CNT_EM_SYNC_EVENT"
	, [RET_END_SPILL] = "CNT_END_SPILL"
	, [ERR_DMA_OVERREAD] = "ERR_KNOWN_DMA_OVERREAD"  // DMA read full burst when no more data in buffer (always after 0xFE)
	, [ERR_DUP] = "ERR_EM_DUPWORD"	// Duplicate word.
	, [ERR_ZEROES] = "ERR_EM_ZERO_WORD"	// A zero word.
	, [ERR_ONES] = "ERR_EM_ONES_WORD"	// A word with all ones.
	, [ERR_UNKNOWN_WORD] = "ERR_EM_UNKNOWN_WORD"	// Unknown word type.
	, [ERR_PROTOCOL] = "ERR_PROTOCOL"  // EM5 protocol error
//	, [ERR_NO_FE]= "ERR_EM_NO_FE"	// Sudden new event (0xBE).
//	, [ERR_NO_BE]= "ERR_EM_NO_BE"	// Sudden event tail (0xFE).
//	, [ERR_NO_1F] = "ERR_EM_NO_1F"	// Missing stats word (0x1F).
	, [ERR_MISS_LEN] = "ERR_MISS_LEN"	// MISS event len counter != actual lengh.
	, [ERR_MISS_ADDR_ORDER] = "ERR_MISS_ADDR_ORDER"	// MISS addresses not ascending during sequential readout.
	};


enum em5_protocol_state {
	NO_STATE
	, PCHI_BEGIN  // sequential data readout 
	, PCHN_BEGIN  // sequential module position numbers readout
	, PCH_DATA
	, PCH_END  // end of pchi or pchn
	};

static const char UNUSED * em5_protocol_state_str[] = {
	[NO_STATE] = "-"
	, [PCHI_BEGIN] = "PCHI"
	, [PCHN_BEGIN] = "PCHN"
	, [PCH_DATA] = "data"
	, [PCH_END] = "END"
	};

//enum event_epoch {
//	PRESPILL, 
//	SPILL,  // readout during beam spill
//	POSTSPILL  // readout after beam spill (LED or PED events)
//	};


enum emword_class {
	WORD_END_SPILL
	, WORD_BEGIN_EVENT
	, WORD_BEGIN_ENUM
	, WORD_STAT_1F
	, WORD_END_EVENT
	, WORD_DATA
	, WORD_SYNC
	, WORD_UNKNOWN
	, WORD_ZERO
	, WORD_ONES
	, WORD_DUP	
	};

static const char UNUSED * emword_class_str[] = {
	[WORD_END_SPILL] = "ES"
	, [WORD_BEGIN_EVENT] = "EVENT"
	, [WORD_BEGIN_ENUM] = "ENUM"
	, [WORD_STAT_1F] = "STAT"
	, [WORD_END_EVENT] = "END"
	, [WORD_DATA] = "DATA"
	, [WORD_SYNC] = "SYNC"
	, [WORD_UNKNOWN] = "UNKNOWN"
	, [WORD_ZERO] = "ZERO"
	, [WORD_ONES] = "ONES"
	, [WORD_DUP] = "DUPLICATE"
};

		



struct em5_parser {
	emword prev;  // previous word
	unsigned last_sync_ts;  // last sync event timestamp
	enum em5_protocol_state state;  // current readout protocol
//	enum event_epoch epoch;
	struct em5_parser_event_info {
		unsigned ts;  // timestamp
		unsigned len; // length in words
		unsigned len_1f; // length according to MISS
		unsigned prev_mod;  // previous module position number
		unsigned cnt;  // event word counter
		bool corrupt; // event is corrupted
		unsigned mod_offt[EM_MAX_MODULE_NUM];  // event data offset
		unsigned mod_cnt[EM_MAX_MODULE_NUM]; // word counter per module
		} evt; 

	unsigned ret_cnt[MAX_EM5_PARSER_RET];  // return value counters
	unsigned corrupted_cnt;  // corrupted events couner
	};

enum em5_parser_ret em5_parser_next(struct em5_parser *, emword);

#endif /* EM5_PARSER_H */
