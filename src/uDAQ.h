#ifndef uDAQ_H
#define uDAQ_H


#include <inttypes.h>

#define DAQ_EVENT_INFO_SZ	12

struct daq_event_info {
	uint32_t ts;  // timestamp
	struct {
		uint8_t crate;
		uint8_t module;
	} addr;
	union {
		uint16_t length; // if not length & 0b1
		uint16_t flags;  //
	};
	uint32_t offt;  //file offs et
};

static_assert(sizeof(struct daq_event_info) == DAQ_EVENT_INFO_SZ,
		"daq_event_header gets wrong size in your compiler. fix it or nothing will work!" );

#endif /* uDAQ_H */ 
