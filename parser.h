#ifndef _PARSER_H
#define _PARSER_H

#include "msgcfg.h"
#include "converters.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*=============================== Parsing =============================================================*/
// strips data from data array corresponding to bitmask in mask array. Whend controls endianness.
uint64_t strip_bits(const uint8_t data_array[], const uint8_t mask_array[], uint32_t num_bytes, bool whend);
// parse a single message.
int32_t parse_single_msg(const uint8_t bytes[], struct msg_cfg* msgcfg);
/*=============================== Output Writing ===========================================================*/
// parse data from text file.
int32_t parse_from_file(FILE* ftoparse, FILE* fparsed, struct msg_cfg* cfg, bool read_type);
// open and parse file. 0 = readbinary, 1 = readhexascii
int32_t open_and_parse_file(const char filetoparse[], const char outputfile[], struct msg_cfg* cfg, bool readmethod);
// write parsed message to a file
int32_t parsed_msg_to_file(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast);
// write message headers to file
int32_t write_msg_headers(FILE* output_file, struct msg_cfg* cfg, uint8_t newlineforlast);

#endif