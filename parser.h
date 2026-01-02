#ifndef _PARSER_H
#define _PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "converters.h"

// macro defs
#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define MAX_NUM_FIELDS 10
#define MAX_BITMASK_LEN_BYTES 20
#define MAX_FIELDNAME_LEN 30
#define RESULT_IS_INT 0
#define RESULT_IS_FLOAT 1
#define MSGBUFF_SIZE 1024
#define BINDATABUFF_SIZE 1024
#define MAX_BITMASK_STR_LEN 500

// struct defs
struct msg_cfg {
	char messagename[MAX_FIELDNAME_LEN];
	uint8_t num_bytes;
	uint8_t num_fields;
	struct field_cfg* first_field; // used for storing field configs.
	struct parsed_field* first_pfield; // used for storing parsed fields.
};

// field cfg struct
struct field_cfg {
	char fieldname[MAX_FIELDNAME_LEN];
	uint8_t bitmask[MAX_BITMASK_LEN_BYTES]; // bitmask for removing bits
	uint8_t num_bits;
	uint8_t converter;
	uint8_t dtype; // result will be typcast if not
	double sf;
	bool whend; // Endianness, which end.
	struct field_cfg* next_field;
};

// parsed field struct
struct parsed_field {
	char fieldname[MAX_FIELDNAME_LEN];
	uint8_t dtype;
	union parsed_result parsed_val;
	struct parsed_field* next_field;
};

/*=============================== Parsing =============================================================*/
// strips data from data array corresponding to bitmask in mask array. Whend controls endianness.
uint64_t strip_bits(const uint8_t data_array[], const uint8_t mask_array[], uint32_t num_bytes, bool whend);
// parse a single message.
int32_t parse_single_msg(const uint8_t bytes[], struct msg_cfg* msgcfg);
// parse data from text file.
int32_t parse_from_file(FILE* ftoparse, FILE* fparsed, struct msg_cfg* cfg, bool read_type);
// open and parse file. 0 = readbinary, 1 = readhexascii
int32_t open_and_parse_file(const char filetoparse[], const char outputfile[], struct msg_cfg* cfg, bool readmethod);
/*=============================== Setup/Config Functions ======================================================*/
// initialize message config
int32_t init_msgcfg(struct msg_cfg* cfg, char fieldname[], uint8_t num_bytes);
// add field to message config
int32_t add_field_to_msgcfg(struct msg_cfg* cfg, const uint8_t bitmask[], const char fieldname[], uint8_t converter_select, uint8_t dtype, double sf, bool whend);
// get field config by index.
struct field_cfg* field_cfg_by_idx(struct msg_cfg* cfg, uint32_t field_idx);
// append field to end of config.
int32_t append_field(struct msg_cfg* cfg,
	const uint8_t bitmask[], const char fieldname[],
	uint8_t converter_select, uint8_t dtype,
	double sf, bool whend);
// add field at index
int32_t add_field_at_idx(struct msg_cfg* cfg, uint32_t field_idx,
						const uint8_t bitmask[], const char fieldname[],
						uint8_t converter_select, uint8_t dtype,
						double sf, bool whend);
// remove specific field by index.
int32_t rm_field_by_idx(struct msg_cfg* cfg, uint32_t field_idx);
// get parsed field by index
struct parsed_field* pfield_by_idx(struct msg_cfg* cfg, uint32_t field_idx);
// get field config by name
struct field_cfg* get_field_cfg_by_name(struct msg_cfg* cfg, const char fieldname[]);
// get parsed field by name
struct parsed_field* get_pfield_by_name(struct msg_cfg* cfg, const char fieldname[]);
// Remove the first field of message config.
int32_t rm_first_field(struct msg_cfg* cfg);
// Remove all fields from message config.
int32_t rm_all_msg_fields(struct msg_cfg* cfg);
/*=============================== Output Writing ===========================================================*/
// write headers to file.
int32_t write_msg_headers(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast);
// write parsed results to file.
int32_t parsed_msg_to_file(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast);
/*=============================== Utilities ===========================================================*/
// counts the number of bits in a bitmask array
uint32_t bits_in_bitmask(const uint8_t bitmask[], uint32_t num_bytes);
// create bitmask from string in fmt: [byte0,byte1,byten] [startbit0,startbit1,startbitn] [stopbit0,stopbit1,stopbitn]
int32_t bitmask_from_cfgstr(char cfg_str[], uint8_t bitmask[MAX_BITMASK_LEN_BYTES]);

#endif