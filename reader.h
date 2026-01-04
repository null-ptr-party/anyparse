#ifndef _READER_H
#define _READER_H

#include <stdint.h>
#define PROGBAR_LEN 40

int32_t read_bytes(FILE* input_file, uint8_t outbuff[], uint32_t buffsize, uint32_t bytes_to_read);
int32_t read_hexascii_bytes(FILE* input_file, uint8_t outbuff[], uint32_t buffsize, uint32_t bytes_to_read);
int32_t shift_file_ptr(FILE* file, int32_t bytes_to_shift);
int32_t get_file_len(FILE* file);
int32_t prog_bar(uint32_t current, uint32_t max, uint32_t every);
int32_t read_single_hexchar(char byte);

#endif