#include "msgcfg.h"
#include "converters.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*=============================== Setup Functions ======================================================*/
int32_t init_msgcfg(struct msg_cfg* cfg)
{
	// Note that number of fields is incremented as fields are added.
	if (cfg == NULL) return 1;
	cfg->num_fields = 0;
	cfg->first_field = NULL;
	return 0;
}

int32_t update_msgcfg(struct msg_cfg* cfg, char fieldname[], uint8_t num_bytes, bool whend)
{
	// Note that number of fields is incremented as fields are added.
	if (cfg == NULL) return 1;
	strncpy(cfg->messagename, fieldname, MAX_FIELDNAME_LEN);
	cfg->num_bytes = num_bytes;
	cfg->whend = whend;

	return 0;
}

// update field configuration
int32_t update_fieldcfg_by_idx(struct msg_cfg* cfg, uint32_t field_idx,
	const uint8_t bitmask[MAX_BITMASK_LEN_BYTES], const char fieldname[],
	uint8_t converter_select, uint8_t dtype, double sf)
{
	if (cfg == NULL) return 1;
	struct field_cfg* field = field_cfg_by_idx(cfg, field_idx);
	if (field == NULL) return 1;
	strncpy(field->fieldname, fieldname, MAX_FIELDNAME_LEN);
	field->converter = converter_select;
	field->dtype = dtype;
	field->sf = sf;
	memcpy(field->bitmask, bitmask, MAX_BITMASK_LEN_BYTES);
	field->num_bits = bits_in_bitmask(field->bitmask, cfg->num_bytes);

	return 0;
}
// add field to message in position 0.
int32_t add_field_to_msgcfg(struct msg_cfg* cfg, const uint8_t bitmask[MAX_BITMASK_LEN_BYTES], const char fieldname[], uint8_t converter_select, uint8_t dtype, double sf)
{
	if (cfg == NULL) return 1;
	// check for max num fields.
	if (cfg->num_fields >= MAX_NUM_FIELDS)
	{
		printf("Too many fields\n");
		return 1;
	}

	// first in linked list is the message
	struct field_cfg* new_field = (struct field_cfg*)malloc(sizeof(struct field_cfg)); // allocation memory for new struct.
	if (new_field == NULL)
	{
		free(new_field);
		printf("Memory allocation failed\n");
		return 1;
	}

	if (cfg->num_fields == 0)
	{
		cfg->first_field = new_field;
	}
	else
	{
		struct field_cfg* temp_field = cfg->first_field; // save the current pointer.
		cfg->first_field = new_field; // new field is now first.
		new_field->next_field = temp_field; // relink after adding new field.
	}

	strncpy(new_field->fieldname, fieldname, MAX_FIELDNAME_LEN);
	new_field->converter = converter_select;
	new_field->dtype = dtype;
	new_field->sf = sf;
	memcpy(new_field->bitmask, bitmask, MAX_BITMASK_LEN_BYTES);
	new_field->num_bits = bits_in_bitmask(new_field->bitmask, cfg->num_bytes);

	cfg->num_fields++; // increment number of fields.

	return 0;
}

struct field_cfg* field_cfg_by_idx(struct msg_cfg* cfg, uint32_t field_idx)
{
	if (cfg == NULL) return NULL;
	if (field_idx >= cfg->num_fields) return NULL;

	struct field_cfg* field_cfg_ptr = cfg->first_field;

	for (uint32_t idx = 0; idx < field_idx; idx++)
	{
		field_cfg_ptr = field_cfg_ptr->next_field;
		if (field_cfg_ptr == NULL) return NULL;
	}
	return field_cfg_ptr;
}

int32_t append_field(struct msg_cfg* cfg,
	const uint8_t bitmask[], const char fieldname[],
	uint8_t converter_select, uint8_t dtype,
	double sf)
{
	if (cfg == NULL) return 1;
	struct field_cfg* field = NULL;

	if (cfg->num_fields != 0)
	{	// if more than 0 fields, get last field.
		field = field_cfg_by_idx(cfg, cfg->num_fields - 1);

		if (field == NULL) return 1;
	}

	struct field_cfg* new_field = (struct field_cfg*)malloc(sizeof(struct field_cfg)); // allocate memory for new struct.

	if (new_field == NULL)
	{	// free memory just in case one was allocated successfully.
		free(new_field);
		printf("Memory allocation failed\n");
		// return error
		return 1;
	}

	if (cfg->num_fields == 0)
	{
		cfg->first_field = new_field;
	}
	else
	{
		// link last field to new field.
		field->next_field = new_field;
	}


	// copy data.
	strncpy(new_field->fieldname, fieldname, MAX_FIELDNAME_LEN);
	new_field->converter = converter_select;
	new_field->dtype = dtype;
	new_field->sf = sf;
	memcpy(new_field->bitmask, bitmask, MAX_BITMASK_LEN_BYTES);
	new_field->num_bits = bits_in_bitmask(new_field->bitmask, cfg->num_bytes);

	// increment number of fields.
	cfg->num_fields++;

	return 0;
}

int32_t add_field_at_idx(struct msg_cfg* cfg, uint32_t field_idx,
	const uint8_t bitmask[MAX_BITMASK_LEN_BYTES], const char fieldname[],
	uint8_t converter_select, uint8_t dtype,
	double sf)
{
	if (cfg == NULL) return 1;
	// Highest index a field can be added is n = num_fields-1
	// anything higher should return error.
	if (field_idx >= cfg->num_fields) return 1;

	struct field_cfg* field_cfg_ptr = cfg->first_field;
	struct field_cfg* prev_field_cfg_ptr = NULL;

	for (uint32_t idx = 0; idx < field_idx; idx++)
	{
		prev_field_cfg_ptr = field_cfg_ptr;
		field_cfg_ptr = field_cfg_ptr->next_field;

		// if any field but the last field is null, return error.
		if (field_cfg_ptr == NULL) return 1;
	}

	struct field_cfg* new_field = (struct field_cfg*)malloc(sizeof(struct field_cfg)); // allocate memory for new struct.

	if (new_field == NULL)
	{	// free memory just in case one was allocated successfully.
		free(new_field);
		printf("Memory allocation failed\n");
		// return error
		return 1;
	}
	if (field_idx == 0)
	{	// case for adding field in idx = 0
		// cfg points to new fields
		cfg->first_field = new_field;
		// new field points to what was previously idx = field_idx.
		new_field->next_field = field_cfg_ptr;
	}
	else
	{	// all other cases
		// previous field points to new field.
		prev_field_cfg_ptr->next_field = new_field;
		// new field points to what was previously idx = field_idx
		new_field->next_field = field_cfg_ptr;
	}

	strncpy(new_field->fieldname, fieldname, MAX_FIELDNAME_LEN);
	new_field->converter = converter_select;
	new_field->dtype = dtype;
	new_field->sf = sf;
	memcpy(new_field->bitmask, bitmask, MAX_BITMASK_LEN_BYTES);
	new_field->num_bits = bits_in_bitmask(new_field->bitmask, cfg->num_bytes);

	cfg->num_fields++;

	return 0;
}

int32_t rm_field_by_idx(struct msg_cfg* cfg, uint32_t field_idx)
{
	if (cfg == NULL) return 1;
	if (field_idx >= cfg->num_fields) return 1;

	struct field_cfg* field_cfg_ptr = cfg->first_field;
	struct field_cfg* prev_field_cfg_ptr = NULL;

	for (uint32_t idx = 0; idx < field_idx; idx++)
	{

		prev_field_cfg_ptr = field_cfg_ptr;
		field_cfg_ptr = field_cfg_ptr->next_field;

		if (field_cfg_ptr == NULL) return 1;
	}

	if (cfg->num_fields > 1)
	{
		// case for more than one field
		if (field_idx == 0)
		{	// if first field, need to re-link using 
			// config pointer to first field.
			cfg->first_field = field_cfg_ptr->next_field;
		}
		else if (field_idx > 0)
		{	// if fields greater than 0, we need to re-link
			// previous field with next.
			prev_field_cfg_ptr->next_field = field_cfg_ptr->next_field;
		}
	}
	else
	{	// if only one field to delete. NULL out
		// first field.
		cfg->first_field = NULL;
	}

	// free allocated memory for field.
	free(field_cfg_ptr);

	// decrement number of fields.
	cfg->num_fields--;

	return 0;
}

struct field_cfg* get_field_cfg_by_name(struct msg_cfg* cfg, const char fieldname[])
{
	if (cfg == NULL) return NULL;
	struct field_cfg* field_cfg = cfg->first_field;

	for (uint32_t idx = 0; idx < cfg->num_fields; idx++)
	{
		if (field_cfg == NULL) return NULL;
		if ((strcmp(field_cfg->fieldname, fieldname) == 0)) return field_cfg;
		// iterate and get next field_cfg
		field_cfg = field_cfg->next_field;
	}

	printf("No field cfg %s found\n", fieldname);
	return NULL;
}

int32_t rm_first_field(struct msg_cfg* cfg)
{
	// function needs rework.
	if (cfg == NULL) return 1; // check for NULL cfg ptr.
	if (cfg->num_fields == 0) return 1; // nothing to delete

	struct field_cfg* first_field_ptr = cfg->first_field;

	if (cfg->num_fields > 1)
	{
		struct field_cfg* next_field_ptr = first_field_ptr->next_field;
		cfg->first_field = next_field_ptr; // re-link to what was 2nd field.
	}
	else
	{
		cfg->first_field = NULL;
	}

	if (first_field_ptr == NULL) return 1;
	free(first_field_ptr); // free memory allocated for deleted field.

	cfg->num_fields--; // decrement number of fields.

	return 0;
}

int32_t rm_all_msg_fields(struct msg_cfg* cfg)
{
	// removes both parsed fields and field configs.
	uint32_t num_fields = cfg->num_fields; // get num fields

	for (uint32_t idx = 0; idx < num_fields; idx++)
	{
		if (rm_first_field(cfg) == 1) // delete failure occured if 1.
		{
			printf("Failed to delete message in position %d\n", idx);
			return 1;
		}
	}
	return 0;
}

/*=============================== Utilities ===========================================================*/
uint32_t bits_in_bitmask(const uint8_t bitmask[], uint32_t num_bytes)
{
	uint32_t bitct = 0, byte = 0;

	for (uint32_t byte_idx = 0; byte_idx < num_bytes; byte_idx++)
	{
		byte = bitmask[byte_idx];

		for (uint32_t bit_idx = 0; bit_idx < 8; bit_idx++)
		{
			bitct += (0x01 & byte);
			byte >>= 1;
		}
	}

	return bitct;
}

int32_t bitmask_from_cfgstr(char cfg_str[], uint8_t bitmask[MAX_BITMASK_LEN_BYTES])
{	// cfgstring format is [byte0,byte1,byten] [startbit0,startbit1,startbitn] [stopbit0,stopbit1,stopbitn]
	uint32_t len = strlen(cfg_str);
	if (strlen(cfg_str) >= MAX_BITMASK_STR_LEN) return 1;

	uint32_t parsed_num_buff[3 * MAX_BITMASK_LEN_BYTES] = { 0 };
	uint32_t buff_idx = 0;
	char* const firstchar_ptr = cfg_str; // get pointer to first char of string.
	char* tknptr = NULL;
	tknptr = strtok(cfg_str, "[,]");

	while ((tknptr != NULL) || (tknptr - firstchar_ptr) >= len) // this makes sure we dont overrun
	{	// this loop gets all entries
		if (isdigit(*tknptr) != 0)
		{	// if digit found after delimiter, scanstring.
			sscanf(tknptr, "%u", &parsed_num_buff[buff_idx]);
			buff_idx++;
		}
		tknptr = strtok(NULL, "[,]"); // get next delimiter.
	}

	if (((buff_idx % 3) != 0) || (buff_idx == 0)) return 1; // need sets of 3 to build bitmask
	uint32_t byte_idx = 0, strtbit = 0, stpbit = 0;
	uint32_t num_bytes = buff_idx / 3; // num bytes to parse.

	for (uint32_t idx = 0; idx < num_bytes; idx++)
	{
		byte_idx = parsed_num_buff[idx];
		if (byte_idx >= MAX_BITMASK_LEN_BYTES) return 1;

		strtbit = parsed_num_buff[idx + num_bytes];
		stpbit = parsed_num_buff[idx + 2 * num_bytes];
		if ((strtbit > 7) || (stpbit > 7)) return 1; // cant exceed num bits in byte

		if (strtbit == 0)
		{
			bitmask[byte_idx] = (uint8_t)MASK(stpbit);
		}
		else
		{
			bitmask[byte_idx] = (uint8_t)(MASK(stpbit) - MASK((strtbit - 1)));
		}
	}

	return 0;
}