#ifndef EM_H
#define EM_H


#define EM_MAX_MODULE_NUM	20
#define EM_MAX_EVENT_LENGTH	0x1FFF
#define EM_MAX_PCHI_DELAY	0xFFF

/* EuroMISS special word (0x1F word in the end of readout) */

#define EM_STATUS_ERR	(1 << 15)  // catch all errors
#define EM_STATUS_OVERFLOW  (1 << 14)  // more words than expected 
/* for pchn -- more than 20 words
   for pchi -- more than a number written in event_lenght_register (12 bit)
*/
#define EM_STATUS_TIMEOUT  (1 << 13)  // 300 ns timeout exceeded
#define EM_STATUS_FALURE   (1 << 12)  // falure state of at least one crate modules
#define EM_STATUS_COUNTER	0x7FF      // words counter (11 bit, not 12 for some reason)

typedef union {
        unsigned int whole;
        struct {
                unsigned short addr;
                unsigned short data;
        };
        unsigned char byte[4];
} emword;

#define EM_ADDR_CHAN(x)	(((x) & 0xFE0) >> 5)
#define EM_ADDR_MOD(x)	((x) & 0x1F)

#endif /* EM_H */
