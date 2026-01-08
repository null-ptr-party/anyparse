#include "parser.h"
#include "converters.h"
#include "reader.h"
#include "msgcfg.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdbool.h>

uint64_t strip_bits(const uint8_t data_array[], const uint8_t mask_array[], uint32_t num_bytes, bool whend)
{
	// whend "which end" controls which end is read from.
	uint64_t bitsum = 0;
	uint32_t shift_counter = 0, idx = 0;
	uint8_t mask_byte = 0, byte_stripped = 0;
	int8_t incr = 0;

	idx = (whend == 0) ? (num_bytes - 1) : 0; // set starting index based on endianness
	incr = (whend == 0) ? -1 : 1; // increment in positive or negative direction based on selection
	
	for (; (idx < num_bytes); idx += incr)
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
	struct field_cfg* field_ptr = NULL;

	for (int field_num = 0; field_num < cfg->num_fields; field_num++)
	{
		field_ptr = field_cfg_by_idx(cfg, field_num);
		if (field_ptr == NULL) return 1;

		field_data = strip_bits(bytes, field_ptr->bitmask, cfg->num_bytes, cfg->whend); // extract bits
		field_ptr->parsed_val = call_parser(field_data, field_ptr->converter, field_ptr->num_bits, field_ptr->dtype, field_ptr->sf); // call converter
	}
	return 0;
}

int32_t open_and_parse_file(const char filetoparse[], const char outputfile[], struct msg_cfg* cfg, bool readmethod)
{
	FILE* ftoparse = NULL;
	FILE* fout = NULL;

	if (readmethod == 1)
	{
		ftoparse = fopen(filetoparse, "r");
	}
	else
	{
		ftoparse = fopen(filetoparse, "rb");
	}
	fout = fopen(outputfile, "w");
	
	if (write_msg_headers(fout, cfg, 1) == 1)
	{
		return 1;
	}
	// parse but return error if issue.
	if (parse_from_file(ftoparse, fout, cfg, readmethod) == 1)
	{
		return 1;
	}

	fclose(ftoparse);
	fclose(fout);
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

/*=============================== Output Writing ===========================================================*/

int32_t write_msg_headers(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast)
{
	if ((output_file == NULL) || (cfg == NULL)) return 1;
	struct field_cfg* field_ptr = NULL;

	for (uint32_t idx = 0; idx < cfg->num_fields; idx++)
	{
		field_ptr = field_cfg_by_idx(cfg, idx);
		if (field_ptr == NULL) return 1;

		fprintf(output_file, "%s.%s", cfg->messagename, field_ptr->fieldname);
		if ((idx < (uint32_t)cfg->num_fields - 1))
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
	struct field_cfg* field_ptr = cfg->first_field;
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
				fprintf(output_file, "%c", field_ptr->parsed_val.char_result[0]);
				break;

			case DTYPE_OUT_UINT:
				fprintf(output_file, "%llu", field_ptr->parsed_val.uint_result);
				break;

			default:
				;
		}

		if (idx < (uint32_t)(cfg->num_fields - 1))
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