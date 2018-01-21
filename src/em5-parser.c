//#include <stdio.h>

#include "em.h"
#include "em5-parser.h"

#include <string.h>  // memset



enum emword_class emword_classify(emword wrd, struct em5_parser * parser ) {
	
	if (wrd.whole == parser->prev.whole)
		return WORD_DUP;

	if (wrd.whole == 0x0U && parser->state != PCHI_BEGIN)	// the first word in PCHI could be 0x0U
		return WORD_ZERO;
	
	if (wrd.whole == ~0x0U)
		return WORD_ONES;

	if ((wrd.byte[0] & 0x1F) <= EM_MAX_MODULE_NUM)
		return WORD_DATA;

	switch(wrd.byte[0])
	{
	case 0xBE: return WORD_BEGIN_EVENT;
	case 0xDE: return WORD_BEGIN_ENUM;
	case 0x1F: return WORD_STAT_1F;
	case 0xFE: return WORD_END_EVENT;

	default:
		return WORD_UNKNOWN;
	}
}


enum em5_parser_ret em5_parser_next(struct em5_parser * parser, emword wrd)

{
	enum em5_parser_ret ret = RET_OK;
//	enum emword_class wrd_class = emword_classify(wrd, parser);

	// word type?
	
	return ret;
} 
