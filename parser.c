#include "parser.h"
#include "converters.h"
#include "reader.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

uint64_t strip_bits(const uint8_t data_array[], const uint8_t mask_array[], uint32_t num_bytes, bool whend)
{
	// whend "which end" controls which end is read from.
	uint64_t bitsum = 0;
	uint32_t shift_counter = 0, idx = 0;
	uint8_t mask_byte = 0, byte_stripped = 0;
	int8_t incr = 0;

	idx = (whend == 0) ? (num_bytes - 1) : 0; // set starting index based on endianness
	incr = (whend == 0) ? -1 : 1; // increment in positive or negative direction based on selection
	
	for (; (idx < num_bytes) && (idx >= 0); idx += incr)
	{	// iterate over bytes
		mask_byte = mask_array[idx];
		byte_stripped = mask_byte & data_array[idx]; // mask off data for this byte

		if (mask_byte > 0)
		{
			// only iterate over bytes that have maskbits
			for (uint32_t idy = 0; idy < 8; idy++, byte_stripped >>= 1, mask_byte >>= 1)
			{
				bitsum += ((uint64_t)(byte_stripped & 0x01LLU) << shift_counter); // the typecast is important here otherwise weird stuff happens.
				shift_counter += (mask_byte & 0x01); // extra shifts after last bitsum
			}
		}
	}
	return bitsum;
}

int32_t parse_single_msg(const uint8_t bytes[], struct msg_cfg* cfg)
{
	if (cfg == NULL) return 1;

	// top level function with be responsible for framing and ensuring no buffer overrun
	// copy message name to output
	uint64_t field_data = 0;
	struct field_cfg* field_cfg_ptr = NULL;
	struct parsed_field* pfield_ptr = NULL;

	for (int field_num = 0; field_num < cfg->num_fields; field_num++)
	{
		pfield_ptr = pfield_by_idx(cfg, field_num);
		field_cfg_ptr = field_cfg_by_idx(cfg, field_num);
		if ((field_cfg_ptr == NULL) || (pfield_ptr == NULL)) return 1;

		field_data = strip_bits(bytes, field_cfg_ptr->bitmask, cfg->num_bytes, field_cfg_ptr->whend); // extract bits
		pfield_ptr->parsed_val = call_parser(field_data, field_cfg_ptr->converter, field_cfg_ptr->num_bits, field_cfg_ptr->dtype, field_cfg_ptr->sf); // call converter
	}
	return 0;
}

int32_t parse_from_file(FILE* ftoparse, FILE* fparsed, struct msg_cfg* cfg, bool read_type)
{
	if ((ftoparse == NULL) || (fparsed == NULL)) return 1;

	int32_t file_len = get_file_len(ftoparse);
	uint8_t bindata_buff[BINDATABUFF_SIZE] = { 0 };
	uint8_t bytes_per_message = cfg->num_bytes;
	while (ftell(ftoparse) <= file_len) // parse up to EOF
	{
		prog_bar((uint32_t)ftell(ftoparse), (uint32_t)file_len, 100000);
		if (read_type == 1)
		{
			if (read_hexascii_bytes(ftoparse, bindata_buff, BINDATABUFF_SIZE, bytes_per_message) != bytes_per_message) break;
		}
		else
		{
			if (read_bytes(ftoparse, bindata_buff, BINDATABUFF_SIZE, bytes_per_message) != bytes_per_message) break;
		}
		parse_single_msg(bindata_buff, cfg);
		parsed_msg_to_file(fparsed, cfg, 1);
	}

	return 0;
}
/*=============================== Setup Functions ======================================================*/

int32_t init_msgcfg(struct msg_cfg* cfg, char fieldname[], uint8_t num_bytes)
{
	// Note that number of fields is incremented as fields are added.
	if (cfg == NULL) return 1;
	strncpy(cfg->messagename, fieldname, MAX_FIELDNAME_LEN);
	cfg->num_bytes = num_bytes;
	cfg->num_fields = 0;
	cfg->first_pfield = NULL;
	cfg->first_field = NULL;
	return 0;
}
// add field to message 
int32_t add_field_to_msgcfg(struct msg_cfg* cfg, const uint8_t bitmask[], const char fieldname[], uint8_t converter_select, uint8_t dtype, double sf, bool whend)
{
	// check for max num fields.
	if (cfg->num_fields >= MAX_NUM_FIELDS)
	{
		printf("Too many fields\n");
		return 1;
	}
	
	// first in linked list is the message
	struct field_cfg* new_field = (struct field_cfg*)malloc(sizeof(struct field_cfg)); // allocation memory for new struct.
	struct parsed_field* new_pfield = (struct parsed_field*)malloc(sizeof(struct parsed_field)); // 
	if ((new_field == NULL) || (new_pfield == NULL))
	{
		free(new_field);
		free(new_pfield);
		printf("Memory allocation failed\n");
		return 1;
	}

	if (cfg->num_fields == 0)
	{	
		cfg->first_field = new_field;
		cfg->first_pfield = new_pfield;
	}
	else
	{
		struct field_cfg* temp_field = cfg->first_field; // save the current pointer.
		struct parsed_field* temp_pfield = cfg->first_pfield; // save the current pointer.
		cfg->first_field = new_field; // new field is now first.
		cfg->first_pfield = new_pfield;
		new_field->next_field = temp_field; // relink after adding new field.
		new_pfield->next_field = temp_pfield;
	}

	strncpy(new_field->fieldname, fieldname, MAX_FIELDNAME_LEN);
	strncpy(new_pfield->fieldname, fieldname, MAX_FIELDNAME_LEN);
	new_field->converter = converter_select;
	new_field->dtype = dtype;
	new_pfield->dtype = dtype;
	new_field->sf = sf;
	new_field->whend = whend;
	memcpy(new_field->bitmask, bitmask, MAX_BITMASK_LEN_BYTES);
	new_field->num_bits = bits_in_bitmask(new_field->bitmask, cfg->num_bytes);

	cfg->num_fields++; // increment number of fields.

	return 0;
}

struct field_cfg* field_cfg_by_idx(struct msg_cfg* cfg, uint32_t field_idx)
{
	if (field_idx >= cfg->num_fields) return NULL;
	struct field_cfg* field_cfg_ptr = cfg->first_field;

	for (uint32_t idx = 0; idx < field_idx; idx++)
	{
		field_cfg_ptr = field_cfg_ptr->next_field;
		if (field_cfg_ptr == NULL) return NULL;
	}
	return field_cfg_ptr;
}

struct parsed_field* pfield_by_idx(struct msg_cfg* cfg, uint32_t field_idx)
{
	if (field_idx >= cfg->num_fields) return NULL;
	struct parsed_field* pfield_ptr = cfg->first_pfield;

	for (uint32_t idx = 0; idx < field_idx; idx++)
	{
		pfield_ptr = pfield_ptr->next_field;
		if (pfield_ptr == NULL) return NULL;
	}
	return pfield_ptr;
}

int32_t add_field_at_idx(struct msg_cfg* cfg, uint32_t field_idx, 
						const uint8_t bitmask[], const char fieldname[],
						uint8_t converter_select, uint8_t dtype,
						double sf, bool whend)
{
	// Highest index a field can be added is n = num_fields (append)
	// anything higher should return error.
	if (field_idx > cfg->num_fields) return 1;

	struct parsed_field* pfield_ptr = cfg->first_pfield;
	struct parsed_field* prev_pfield_ptr = NULL;
	struct field_cfg* field_cfg_ptr = cfg->first_field;
	struct field_cfg* prev_field_cfg_ptr = NULL;

	for (uint32_t idx = 0; idx < field_idx; idx++)
	{
		prev_pfield_ptr = pfield_ptr;
		pfield_ptr = pfield_ptr->next_field;

		prev_field_cfg_ptr = field_cfg_ptr;
		field_cfg_ptr = field_cfg_ptr->next_field;

		if ((pfield_ptr == NULL) || (field_cfg_ptr == NULL)) return 1;
	}

	struct field_cfg* new_field = (struct field_cfg*)malloc(sizeof(struct field_cfg)); // allocate memory for new struct.
	struct parsed_field* new_pfield = (struct parsed_field*)malloc(sizeof(struct parsed_field)); // allocate memory new pfield.

	if ((new_field == NULL) || (new_pfield == NULL))
	{	// free memory just in case one was allocated successfully.
		free(new_field);
		free(new_pfield);
		printf("Memory allocation failed\n");
		// return error
		return 1;
	}

	if (field_idx == 0)
	{	// case for adding field in idx = 0
		// cfg points to new fields
		cfg->first_field = new_field;
		cfg->first_pfield = new_pfield;
		// new field points to what was previously idx = field_idx.
		new_field->next_field = field_cfg_ptr;
		new_pfield->next_field = pfield_ptr;
	}
	else if (field_idx == cfg->num_fields)
	{ // case for field being appended to end.
		field_cfg_ptr->next_field = new_field;
		pfield_ptr->next_field = new_pfield;
	}
	else
	{	// all other cases
		// previous field points to new field.
		prev_field_cfg_ptr->next_field = new_field;
		prev_pfield_ptr->next_field = new_pfield;
		// new field points to what was previously idx = field_idx
		new_field->next_field = field_cfg_ptr;
		new_pfield->next_field = pfield_ptr;
	}
	cfg->num_fields++;
	
	return 0;
}

int32_t rm_field_by_idx(struct msg_cfg* cfg, uint32_t field_idx)
{
	if (field_idx >= cfg->num_fields) return 1;

	struct parsed_field* pfield_ptr = cfg->first_pfield;
	struct parsed_field* prev_pfield_ptr = NULL;
	struct field_cfg* field_cfg_ptr = cfg->first_field;
	struct field_cfg* prev_field_cfg_ptr = NULL;


	for (uint32_t idx = 0; idx < field_idx; idx++)
	{
		prev_pfield_ptr = pfield_ptr;
		pfield_ptr = pfield_ptr->next_field;

		prev_field_cfg_ptr = field_cfg_ptr;
		field_cfg_ptr = field_cfg_ptr->next_field;

		if ((pfield_ptr == NULL) || (field_cfg_ptr == NULL)) return 1;
	}

	if (cfg->num_fields > 1)
	{
		// case for more than one field
		if (field_idx == 0)
		{	// if first field, need to re-link using 
			// config pointer to first field.
			cfg->first_field = field_cfg_ptr->next_field;
			cfg->first_pfield = pfield_ptr->next_field;
		}
		else if (field_idx > 0)
		{	// if fields greater than 0, we need to re-link
			// previous field with next.
			prev_field_cfg_ptr = field_cfg_ptr->next_field;
			prev_pfield_ptr = pfield_ptr->next_field;
		}
	}
	else
	{	// if only one field to delete. NULL out
		// first field.
		cfg->first_field = NULL;
		cfg->first_pfield = NULL;
	}

	// free allocated memory for field.
	free(field_cfg_ptr);
	free(pfield_ptr);

	// decrement number of fields.
	cfg->num_fields--;

	return 0;
}

struct field_cfg* get_field_cfg_by_name(struct msg_cfg* cfg, const char fieldname[])
{
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

struct parsed_field* get_pfield_by_name(struct msg_cfg* cfg, const char fieldname[])
{
	struct parsed_field* pfield = cfg->first_pfield;

	for (uint32_t idx = 0; idx < cfg->num_fields; idx++)
	{
		if (pfield == NULL) return NULL;
		if ((strcmp(pfield->fieldname, fieldname) == 0)) return pfield;
		// iterate and get next pfield
		pfield = pfield->next_field;
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
	struct parsed_field* first_pfield_ptr = cfg->first_pfield; // this field used to save pointer to 2nd element

	if (cfg->num_fields > 1)
	{
		struct field_cfg* next_field_ptr = first_field_ptr->next_field;
		struct parsed_field* next_pfield_ptr = first_pfield_ptr->next_field;
		cfg->first_field = next_field_ptr; // re-link to what was 2nd field.
		cfg->first_pfield = next_pfield_ptr;
	}
	else
	{
		cfg->first_field = NULL;
		cfg->first_pfield = NULL;
	}

	if ((first_field_ptr == NULL) || (first_pfield_ptr == NULL)) return 1;
	free(first_field_ptr); // free memory allocated for deleted field.
	free(first_pfield_ptr); // free memory for first pfield.

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


/*=============================== Output Writing ===========================================================*/

int32_t write_msg_headers(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast)
{
	if ((output_file == NULL) || (cfg == NULL)) return 1;
	struct field_cfg* field_cfg = NULL;

	for (uint32_t idx = 0; idx < cfg->num_fields; idx++)
	{
		field_cfg = field_cfg_by_idx(cfg, idx);
		if (field_cfg == NULL) return 1;

		fprintf(output_file, "%s.%s", cfg->messagename, field_cfg->fieldname);
		if ((idx < cfg->num_fields - 1))
		{
			fprintf(output_file, ", ");
		}
	}

	if (newlineforlast)
	{
		fprintf(output_file, "\n");
	}
	else
	{
		fprintf(output_file, ", ");
	}

	return 0;
}

int32_t parsed_msg_to_file(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast)
{
	if ((output_file == NULL) || (cfg == NULL)) return 1;

	uint8_t dtype = 0;
	struct parsed_field* field_ptr = cfg->first_pfield;
	if (field_ptr == NULL) return 1;

	for (uint32_t idx = 0; (field_ptr != NULL) && (idx < cfg->num_fields); idx++, field_ptr = field_ptr->next_field)
	{
		dtype = field_ptr->dtype;

		switch (dtype)
		{
			case DTYPE_OUT_INT:
				fprintf(output_file, "%lld", field_ptr->parsed_val.int_result);
				break;

			case DTYPE_OUT_FLOAT:
				fprintf(output_file, "%lf", field_ptr->parsed_val.float_result);
				break;

			case DTYPE_OUT_CHAR:
				for (uint32_t idx = 0; idx < 4; idx++)
				{
					fprintf(output_file, "%c", field_ptr->parsed_val.char_result[idx]);
				}

				break;
			
			default:
				;
		}

		if (idx < (cfg->num_fields - 1))
		{
			fprintf(output_file, ", "); // comma only between fields.
		}
	}

	if (newlineforlast)
	{
		fprintf(output_file, "\n");
	}
	else
	{
		fprintf(output_file, ", ");
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