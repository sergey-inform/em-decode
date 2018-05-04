#ifndef uDAQ_H
#define uDAQ_H


#include <inttypes.h>

#define DAQ_EVENT_INFO_SZ	12  //FIXME

enum daq_event_flags {
	DAQ_EVENT_DATA	= 1 << 0,  // if true, offset is actually a data
	DAQ_EVENT_DIRTY	= 1 << 1,
	DAQ_EVENT_SYNC	= 1 << 2,
};


struct daq_event_info {
	uint32_t ts;  // timestamp
	uint8_t flags; //
	uint8_t unit; // module num for EuroMISS;
	uint16_t pad; // reserverd for xaddress
	union {
		uint32_t woffset; // offset in 4-byte words
		uint32_t data;
	};
};

//static_assert(sizeof(struct daq_event_info) == DAQ_EVENT_INFO_SZ,
//		"daq_event_header gets wrong size in your compiler. fix it or nothing will work!" );

// Static assert
char event_size_assertion[sizeof(struct daq_event_info) == DAQ_EVENT_INFO_SZ ? 1 : -1];



#endif /* uDAQ_H */ 
