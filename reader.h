#ifndef _READER_H
#define _READER_H

#include <stdint.h>

int32_t read_bytes(FILE* input_file, uint8_t outbuff[], uint32_t buffsize, uint32_t bytes_to_read);
int32_t read_hexascii_bytes(FILE* input_file, uint8_t outbuff[], uint32_t buffsize, uint32_t bytes_to_read);
int32_t shift_file_ptr(FILE* file, int32_t bytes_to_shift, bool read_type);
int32_t get_file_len(FILE* file);

#endif