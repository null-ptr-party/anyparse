#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "reader.h"


int32_t read_bytes(FILE* input_file, uint8_t outbuff[], uint32_t buffsize, uint32_t bytes_to_read)
{
	if ((input_file == NULL) || (bytes_to_read >= buffsize)) return -1; // check file pointer validity and num bytes.
	
	int32_t bytes_read = (uint32_t)fread(outbuff, 1, (size_t)bytes_to_read, input_file);
	return bytes_read;
}

int32_t read_hexascii_bytes(FILE* input_file, uint8_t outbuff[], uint32_t buffsize, uint32_t bytes_to_read)
{
	if ((input_file == NULL) || (bytes_to_read > buffsize)) return -1; // check file pointer validity and num bytes.
	
	uint32_t byte = 0;
	int32_t bytes_read = 0;

	for (uint32_t idx = 0; idx < bytes_to_read; idx++)
	{
		if ((fscanf(input_file, "%2X", &byte) != 1))
		{
			break;
		}
		else
		{
			bytes_read++;
		}
		outbuff[idx] = (uint8_t)byte;
	}

	return bytes_read;
}

int32_t get_file_len(FILE* file)
{
	if (file == NULL) return -1;

	int32_t fsize = 0;
	int32_t fcur = ftell(file); // store current location

	if (fseek(file, 0, SEEK_END) != 0)
	{
		return -1; // seek error
	}

	fsize = ftell(file);
	fseek(file, fcur, SEEK_SET); // return fpointer to current location.
	return fsize;
}

int32_t shift_file_ptr(FILE* file, int32_t bytes_to_shift, bool read_type)
{
	if (file == NULL) return -1;

	int32_t max_shift_neg = -ftell(file);
	int32_t max_shift_pos = get_file_len(file) - max_shift_neg;

	if ((bytes_to_shift > max_shift_pos) && (bytes_to_shift < max_shift_neg)) return -1;
	
	if (fseek(file, bytes_to_shift, SEEK_CUR) != 0) return -1;

	return 0;
}

