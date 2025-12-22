#include <stdio.h>
#include "parser.h"
#include "converters.h"
#include "reader.h"
#include <time.h>


int main(void)
{
	// why dont we just elminate the secondary pmsg struct, and only have the
	// msgconfig struct. Within each field could be a pointer to the result.
	uint8_t bitmask[MAX_BITMASK_LEN_BYTES] = { 0xff, 0xff, 0xff, 0xff };
	char fieldname[MAX_FIELDNAME_LEN] = { "field1" };
	uint8_t converter = CONVERTER_READ_UNS;
	uint8_t dtype = DTYPE_OUT_INT;
	bool whend = LITTLE_ENDIAN;
	double sf = 1;

	struct msg_cfg msg_cfg = { 0 };
	init_msgcfg(&msg_cfg, "message1", 4);
	add_field_to_msgcfg(&msg_cfg, bitmask, fieldname, converter, dtype, sf, whend);

	uint8_t bitmask_two[MAX_BITMASK_LEN_BYTES] = { 0xff, 0xff, 0xff, 0xff };
	char fieldname_two[MAX_FIELDNAME_LEN] = { "field2" };
	converter = CONVERTER_READ_UNS;
	dtype = DTYPE_OUT_FLOAT;
	whend = LITTLE_ENDIAN;
	sf = 2.3124;
	add_field_to_msgcfg(&msg_cfg, bitmask_two, fieldname_two, converter, dtype, sf, whend);

	FILE* outfile = fopen("test.txt", "w");
	FILE* infile = fopen("testfile.txt", "r");
	if (outfile != NULL)
	{
		write_msg_headers(outfile, &msg_cfg, 1);
		parse_from_file(infile, outfile, &msg_cfg, 1);
	}


	rm_all_msg_fields(&msg_cfg);


	return 0;
}