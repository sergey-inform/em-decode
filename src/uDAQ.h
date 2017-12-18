#ifndef uDAQ_H
#define uDAQ_H


#include <inttypes.h>

#define DAQ_EVENT_INFO_SZ	8  //FIXME

struct daq_event_info {
	uint32_t ts;  // timestamp
	struct {
		uint8_t crate;
		uint8_t module;
	} addr;
	uint16_t flags;  //
/*
	union {
		struct {
			uint16_t length;
			uint16_t xxx;
			uint32_t offset;
		};
		uint64_t data;
	};
*/
};

static_assert(sizeof(struct daq_event_info) == DAQ_EVENT_INFO_SZ,
		"daq_event_header gets wrong size in your compiler. fix it or nothing will work!" );



#endif /* uDAQ_H */ 
