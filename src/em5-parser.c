#include <stdio.h>

#include "em.h"
#include "em5-parser.h"

#include <string.h>  // memset


enum em5_parser_ret em5_parser_next(struct em5_parser * parser, emword wrd)

{
	enum em5_parser_ret ret = RET_OK;
	enum emword_class wrd_class = WORD_UNKNOWN;
	enum em5_protocol_state next_state = NO_STATE;
	bool append_data = false;  // if true, data is valid

	struct em5_parser_event_info evt = parser->evt;	

	// Classify the word, but our judgement is not final (class and ret could be adjusted later)
	if (wrd.whole == 0x0U) {  // check it first
		wrd_class = WORD_ZERO;
		ret = ERR_ZEROES;
	}
	else if (wrd.whole == parser->prev.whole) {
		wrd_class = WORD_DUP;
		ret = ERR_DUP;
	}
	else if (wrd.whole == ~0x0U) {
		wrd_class = WORD_ONES;
		ret = ERR_ONES;
	}
	else if ((wrd.byte[0] & 0x1F) <= EM_MAX_MODULE_NUM) {
		wrd_class = WORD_DATA;
	}
	else {
		switch(wrd.byte[0])
		{
		case 0xBE: wrd_class = WORD_BEGIN_EVENT; break;
		case 0xDE: wrd_class = WORD_BEGIN_ENUM;  break;
		case 0x1F: wrd_class = WORD_STAT_1F;     break;
		case 0xFE: wrd_class = WORD_END_EVENT;   break;

		default:
			wrd_class = WORD_UNKNOWN;
			ret = ERR_UNKNOWN_WORD;
		}
	}

	// Take some action depending on word class and previous protocol state
	switch (parser->state) 
	{
	case NO_STATE:
		switch (wrd_class)
		{
		case WORD_BEGIN_EVENT:
		case WORD_BEGIN_ENUM:
			memset(&evt, 0, sizeof(struct em5_parser_event_info)); // flush
			evt.ts = wrd.data; //save timestamp low
			
			if (wrd_class == WORD_BEGIN_EVENT) {
				next_state = PCHI_BEGIN;
				//TODO: event_type = pchi
			}
			else if (wrd_class == WORD_BEGIN_ENUM) {
				//TODO: event_type = pchn
				next_state = PCHN_BEGIN;
			}
			ret = RET_OK;
			
			break;

		case WORD_SYNC:
			next_state = parser->state; //no_change
			//TODO
			ret = RET_SYNC;
			break;

		case WORD_END_SPILL:
			next_state = parser->state; 
			//TODO
			ret = RET_END_SPILL;
			break;

		case WORD_DUP:
			ret = WARN_DMA_OVERREAD;  // known hardware bug
			break;

		default:
			break;
		}

		break;  //NO_STATE

	case PCHI_BEGIN:
	case PCHN_BEGIN:
		switch (wrd_class)
		{
		case WORD_ZERO:	// the first word in PCHI (& PCHN?) could be 0x0U
			wrd_class = WORD_DATA;
			ret = RET_OK;
			//nobreak

		case WORD_DATA:
			next_state = PCH_DATA;
			append_data = true;
			break;

		case WORD_STAT_1F:	// empty event
			// TODO: count empty events
			next_state = PCH_END;
			break;
		
		case WORD_DUP:
			next_state = parser->state; //no change
			ret = WARN_DMA_OVERREAD;
			break;

		default:
			break;
		}
		break;  // PCHI_BEGIN

	
	case PCH_DATA:
		switch (wrd_class)
		{
		case WORD_DATA:
			next_state = PCH_DATA;
			append_data = true;
			break;

		case WORD_STAT_1F:
			next_state = PCH_END;

			evt.len += 1;
			evt.len_1f = (wrd.data & EM_STATUS_COUNTER);

			if( (evt.len & EM_STATUS_COUNTER) != evt.len_1f) {
				ret = ERR_MISS_LEN;
			}
			break;

		case WORD_DUP:
			next_state = PCH_DATA;
			append_data = true;
			ret = ERR_DUP;
			break;

		default:
			break;

		}
		break;  // PCH_DATA

	case PCH_END:
		switch (wrd_class)
		{
		case WORD_END_EVENT:
			evt.ts += wrd.data << 16;  //save timestamp high
			next_state = NO_STATE;
			ret = RET_EVENT;
			break;
		
		default:
			break;
		}

		break;  // PCH_END
	}


	if (append_data) {
		evt.len += 1;
		evt.mod_cnt[EM_ADDR_MOD(wrd.addr)] += 1;


		// check MISS addresses are ascending
		if (evt.prev_mod > EM_ADDR_MOD(wrd.addr)) {
			ret = WARN_MISS_ADDR_ORDER;
		}

		evt.prev_mod = EM_ADDR_MOD(wrd.addr);
	}


	if (ret >= RET_ERROR && !evt.dirty) {  // if not dirty already
		parser->dirty_cnt += 1;
		evt.dirty = true;
	}

	parser->prev = wrd;
	parser->state = next_state;	
	parser->ret_cnt[ret] += 1;

	return ret;
} 
