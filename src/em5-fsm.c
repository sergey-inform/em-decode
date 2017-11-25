
//#include <stdio.h>

#include "em5-fsm.h"


enum em5_fsm_err em5_fsm_next(struct em5_fsm * fsm, emword wrd)

{
	enum em5_fsm_err  err = FSM_OK;
	enum em5_fsm_state  new_state = BUG;
	enum em5_fsm_state  cur_state = fsm->state;
	
	if (wrd.whole == fsm->prev.whole) 
		err = DUP;  // duplicate word (should not be possible)
		
	if (wrd.whole == 0x0U) 
		err = ZEROES;
	
	if (wrd.whole == ~0x0U) 
		err = ONES;
	
	if (err) {
		new_state = CORRUPT;
	}
	else {
		switch (wrd.byte[0])
		{
		case 0xBE:  //begin event
			if (cur_state == END || cur_state == INIT || cur_state == CORRUPT)
				new_state = PCHI;
			else {
				err = NO_FE;  // previous event was not ended correctly
				new_state = CORRUPT;
			}
			break;

		case 0xDE:  //begin pchn (crate enumeration event)
			if (cur_state == END || cur_state == INIT || cur_state == CORRUPT)
				new_state = PCHN;
			else {
				err = NO_FE;
				new_state = CORRUPT;
			}
			break;

		case 0x1F:  //miss status word (in the end of event)
			if (cur_state == PCHI || cur_state == PCHN || cur_state == DATA || cur_state == CORRUPT) {
				new_state = STAT;
				//FIXME: check event length
			}
			else {
				err = NO_BE;
				new_state = CORRUPT;
			}
			break;

		case 0xFE:  //end event
			if (cur_state == STAT) {
				new_state = END;
				// FIXME:get timestamp, 
				// FIXME: send data to stdout
			}
			else {
				err = NO_BE;
				new_state = CORRUPT;
			}
			break;

//		case 0xXX:  //sync event
//			new_state = cur_state;  //invisible
//			break;

		default:
			if (wrd.byte[0] < 0x1F) {  // data word
				if (cur_state == PCHI || cur_state == PCHN || cur_state == DATA) {
					new_state = DATA;
				
					// check [ADDR_ORDER}
					// FIXME: save data

				}
				else if ( cur_state == CORRUPT) {
					new_state = CORRUPT;
				}
				else {
					err = UNKNOWN_WORD;
					new_state = CORRUPT;
				}

			}
			else {  // some unknown service word
			 	err = UNKNOWN_WORD;
				new_state = CORRUPT;
			}
		
		}
	}
	

	fsm->state = new_state;
	fsm->prev = wrd;

	return err;
} 
