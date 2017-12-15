//#include <stdio.h>

#include "em.h"
#include "em5-fsm.h"

#include <string.h>  // memset


enum em5_fsm_ret em5_fsm_next(struct em5_fsm * fsm, emword wrd)

{
	enum em5_fsm_ret  ret = FSM_OK;
	enum em5_fsm_state  new_state = BUG;
	enum em5_fsm_state  cur_state = fsm->state;
	
		
	switch (wrd.byte[0])
	{

	case 0xBE:  // begin readout event (pchi)
	case 0xDE:  // begin enumeration event (pchn)
		if (cur_state == END || cur_state == INIT || cur_state == CORRUPT) {
			memset(&fsm->evt, 0, sizeof(struct em5_fsm_event));  // flush previous event
			if (wrd.byte[0] == 0xBE) {
				new_state = PCHI;
			}
			else if (wrd.byte[0] == 0xDE) {
				new_state = PCHN;
			}
		}
		else {
			ret = NO_FE;  // previous event was not ended correctly
			break;
		}
		
		fsm->evt.ts = wrd.data; //save timestamp low
		break;

	case 0x1F:  //MISS status word (in the end of event)
		if (cur_state == PCHI || cur_state == PCHN || cur_state == DATA || cur_state == CORRUPT) {
			new_state = STAT;
		}
		else {
			ret = NO_BE;
			break;
		}
		fsm->evt.len += 1;
		fsm->evt.len_1f = (wrd.data & EM_STATUS_COUNTER);

		if( (fsm->evt.len & EM_STATUS_COUNTER) != fsm->evt.len_1f) {
			ret = WRONG_LEN_1F;
		}
		break;

	case 0xFE:  //end event
		if (cur_state == STAT || cur_state == CORRUPT) {
			ret = FSM_EVENT;	
			new_state = END;
		}
		else {
			ret = NO_BE;
			break;
		}

		fsm->evt.ts += wrd.data << 16; //save timestamp high
		break;

//TODO
//		case 0xXX:  //sync event
//			new_state = cur_state;  //invisible
//			fsm->sync_ts = ...
//			ret = SYNC_EVENT;
//			break;

	default:
		if ((wrd.byte[0] & 0x1F) <= EM_MAX_MODULE_NUM) {  // data word
			if (cur_state == PCHI || cur_state == PCHN || cur_state == DATA) {
				new_state = DATA;
				fsm->evt.cnt += 1;

				fsm->evt.mod_cnt[EM_ADDR_MOD(wrd.addr)] += 1;
				
				// check MISS addresses are ascending
				if (fsm->evt.prev_mod > EM_ADDR_MOD(wrd.addr))
					ret = ADDR_ORDER;

				fsm->evt.prev_mod = EM_ADDR_MOD(wrd.addr);

			}
			else if ( cur_state == CORRUPT) {
				new_state = CORRUPT;
			}
			else {
				ret = UNKNOWN_WORD;
				new_state = CORRUPT;
			}

		}
		else {  // some unknown service word
			ret = UNKNOWN_WORD;
		}

		fsm->evt.len += 1;
	
	}

	if (wrd.whole == fsm->prev.whole)  
		ret = DUP;  // duplicate word (should not be possible)

	if (wrd.whole == 0x0U) 
		ret = ZEROES;
	
	if (wrd.whole == ~0x0U) 
		ret = ONES;
	
	if (ret >= FSM_ERROR) {
		new_state = CORRUPT;
		fsm->evt.corrupt = true;
	}

	
	fsm->state = new_state;
	fsm->prev = wrd;
	if (ret) fsm->ret_cnt[ret] += 1;

	return ret;
} 
