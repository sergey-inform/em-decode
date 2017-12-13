#ifndef uDAQ_H
#define uDAQ_H


#include <inttypes.h>

#define DAQ_EVENT_HDR_SZ	12

struct daq_event_header {
	uint32_t ts;  // timestamp
	struct {
		uint8_t crate;
		uint8_t module;
	} addr;
	union {
		uint16_t length;
		uint16_t flags;  //
	};
	uint32_t offt;  //file offset
};

static_assert(sizeof(struct daq_event_header) == DAQ_EVENT_HDR_SZ,
		"daq_event_header gets wrong size in your compiler. fix it!" );

#endif /* uDAQ_H */ 
