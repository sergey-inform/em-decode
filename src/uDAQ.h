#ifndef uDAQ_H
#define uDAQ_H


#include <inttypes.h>

#define DAQ_EVT_IDX_PACKED_SZ  2

enum daq_evt_info_flags {
	DAQ_PACKED_FIELD_OVERFLOW	= 1 << 0, /** if true, 
				the word has a special meaning:
				all field values are a number 
				of field sizes to add to it's value.
				
				Example: for 'uint_16 field:10;'
				we add '1024 * field' to it's value 
				and skip this word.
				*/
	DAQ_EVENT_DIRTY 	= 1 << 1,
	DAQ_ERR_OVERFLOW	= 1 << 2,
	DAQ_ERR_TIMEOUT 	= 1 << 3,
	DAQ_ERR_HW      	= 1 << 4,
};


struct daq_ts_idx {  // timestamp index files (.idx) for raw data (.dat)
	uint16_t dts;  // timestamp delta
	uint8_t flags;
	uint16_t dwoff;  // offset delta (in 32-bit words)
};


struct daq_evt_idx {  // event index files (.evt) for raw data (.dat)
	uint8_t flags;
	uint16_t dwoff;
};

/** 
	We use deltas instead of values to produce smaller files.
	
	If the field is overflowed, we insert a special word
	with DAQ_PACKED_FIELD_OVERFLOW
	before to keep the field arithmetics.

#define DAQ_DWOFF_BITS=10;

struct daq_ts_idx {  // timestamp index files (.idx) for raw data (.dat)
	uint16_t dts;  // timestamp delta
	uint8_t flags:6;
	uint16_t dwoff:10;  // offset delta (in 32-bit words)
}__attribute__((packed));


struct daq_evt_idx {  // event index files (.evt) for raw data (.dat)
	uint8_t flags:6;
	uint16_t dwoff:10;
}__attribute__((packed));

**/

//static_assert(sizeof(struct daq_event_info) == DAQ_EVENT_INFO_SZ,
//		"daq_event_header gets wrong size in your compiler. fix it or nothing will work!" );

// Static assert
//char event_size_assertion[sizeof(struct daq_evt_idx) == DAQ_EVT_IDX_PACKED_SZ ? 1 : -1];
//char event_size_assertion[sizeof(struct daq_raw_idx) == DAQ_RAW_IDX_SZ ? 1 : -1];
//char event_size_assertion[sizeof(struct daq_raw_idx) == DAQ_FILE_IDX_SZ ? 1 : -1];

#endif /* uDAQ_H */ 
