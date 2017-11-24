
//#include <stdio.h>

#include "em5-fsm.h"

enum em5_fsm_err em5_fsm_next(struct em5_fsm * fsm, emword wrd)

{
	enum em5_fsm_err err = FSM_OK;
	enum em5_fsm_state new_state, cur_state;
	
	new_state = BUG;	
	cur_state = fsm->state;

	if (wrd.whole == fsm->prev.whole) 
		err = DUP;  //duplicate word

	else if (wrd.whole == 0x0U)
		err = ZEROES;

	else if (wrd.whole == ~0x0U)
		err = ONES;

	else {
		switch (wrd.byte[0])
		{
		case 0xBE:  //begin event
			if (cur_state == END || cur_state == INIT || cur_state == SPOIL)
				new_state = PCHI;	
			else {
				err = NO_FE;
				new_state = SPOIL;
			}
			break;

		case 0xDE:  //begin pchn (crate enumeration event)
			if (cur_state == END || cur_state == INIT || cur_state == SPOIL)
				new_state = PCHN;
			else {
				err = NO_FE;
				new_state = SPOIL;
			}
			break;

		case 0x1F:  //miss status
			if (cur_state == PCHI || cur_state == PCHN) {
				new_state = END;
				//FIXME: check length
			}
			else {
				err = NO_BE;
				new_state = SPOIL;
			}
			break;


		case 0xFE:  //end event
			break;
		
		default:
			if (wrd.byte[0] < 0x1F) { // data word

			}
			else {
			 	err = UNKNOWN_WORD;
			}

		}
	}
	

	fsm->state = new_state;
	fsm->prev = wrd;

	return err;
} 
