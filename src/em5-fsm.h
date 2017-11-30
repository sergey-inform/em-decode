#ifndef EM5_FSM_H
#define EM5_FSM_H

/* Workaround for gcc -Wunused-variable */
#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#include "em.h"

enum em5_fsm_state {
	INIT
	, PCHI	/* begin readout */
	, PCHN	/* begin enumerate modules */
	, DATA  /* regular data word for PCHI or PCHN */
	, STAT  /* miss status word */
	, END	/* end of readout */
	//, XX 	/* synchronisation data */
	, CORRUPT /* corrupted event data */
	, BUG /* fsm bug */
	};

static const char UNUSED *em5_fsm_statestr[] = {
	[INIT] = "INIT"
	, [PCHI] = "PCHI"
	, [PCHN] = "PCHN"
	, [DATA] = "-"
	, [STAT] = "STATS"
	, [END] = "END"
	//
	, [CORRUPT] = "XXX"
	, [BUG] = "FSM_BUG"
	};


enum em5_fsm_ret{
	FSM_OK
	, FSM_EVENT
//	, SYNC_EVENT
	, ERROR
	, DUP
	, ZEROES, ONES
	, NO_FE, NO_BE, NO_1F
	, WRONG_LEN_1F
	, UNKNOWN_WORD
	, ADDR_ORDER
	, MAX_EM5_FSM_RET // the last element
	};

static const char UNUSED *em5_fsm_retstr[] = {
	[FSM_OK] = "-"
	, [FSM_EVENT] = "Event"
//	, [SYNC_EVENT] = "Sync"
	, [DUP] = "Duplicate word."
	, [ZEROES] = "A zero word."
	, [ONES] = "A word with all ones."
	, [NO_FE]= "Sudden new event (0xBE)."
	, [NO_BE]= "Sudden event tail (0xFE)."
	, [NO_1F] = "Missing stats word (0x1F)."
	, [WRONG_LEN_1F] = "MISS event len counter != actual lengh."
	, [UNKNOWN_WORD] = "Unknown word type."
	, [ADDR_ORDER] = "Miss addresses not ascending."
	};


struct em5_fsm {
	enum em5_fsm_state state;
	emword prev;  // previous word
	unsigned ret_cnt; //[MAX_EM5_FSM_RET];  // error counters
	struct em5_fsm_event{
		unsigned cnt;  // event word counter
		unsigned prev_mod;  // previous module address
		unsigned mod_cnt[EM_MAX_MODULE_NUM]; // word counter per module
		unsigned data[EM_MAX_MODULE_NUM];  // event data offset //FIXME: use 32-bit unsigned int
		} evt; 
	};

enum em5_fsm_ret em5_fsm_next(struct em5_fsm *, emword);

#endif /* EM5_FSM_H */
