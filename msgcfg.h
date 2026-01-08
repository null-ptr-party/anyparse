#ifndef _MSGCFG_H
#define _MSGCFG_H

#include <stdint.h>
#include <stdbool.h>

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
	bool whend; // Endianness, which end.
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
	struct field_cfg* next_field;
};

// parsed field struct
struct parsed_field {
	char fieldname[MAX_FIELDNAME_LEN];
	uint8_t dtype;
	union parsed_result parsed_val;
	struct parsed_field* next_field;
};

/*=============================== Setup/Config Functions ======================================================*/
// initialize message config
int32_t init_msgcfg(struct msg_cfg* cfg, char fieldname[], uint8_t num_bytes, bool whend);
// update message configuration
int32_t update_msgcfg(struct msg_cfg* cfg, char fieldname[], uint8_t num_bytes, bool whend);
// add field to message config
int32_t add_field_to_msgcfg(struct msg_cfg* cfg, const uint8_t bitmask[MAX_BITMASK_LEN_BYTES], const char fieldname[], uint8_t converter_select, uint8_t dtype, double sf);
// get field config by index.
struct field_cfg* field_cfg_by_idx(struct msg_cfg* cfg, uint32_t field_idx);
// append field to end of config.
int32_t append_field(struct msg_cfg* cfg,
	const uint8_t bitmask[], const char fieldname[],
	uint8_t converter_select, uint8_t dtype,
	double sf);
// add field at index
int32_t add_field_at_idx(struct msg_cfg* cfg, uint32_t field_idx,
	const uint8_t bitmask[MAX_BITMASK_LEN_BYTES], const char fieldname[],
	uint8_t converter_select, uint8_t dtype,
	double sf);
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

#endif