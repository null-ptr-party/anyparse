#ifndef CONVERTERS_H
#define CONVERTERS_H
#include <stdint.h>

// dtype macros
#define DTYPE_OUT_INT 0U
#define DTYPE_OUT_FLOAT 1U
#define DTYPE_OUT_CHAR 2U
#define DTYPE_OUT_UINT 3U
#define NUM_DTYPES 4U

// converter macros
#define CONVERTER_TWOS_COMP 0U
#define CONVERTER_READ_OB 1U
#define CONVERTER_READ_COB 2U
#define CONVERTER_READ_UNS 3U
#define CONVERTER_READ_IEEE_FP 4U
#define CONVERTER_READ_CHAR 5U
#define NUM_CONVERTERS 6U

// mask off bits up to bit x (starts at bit 0)
#define MASK(x) ((x < 63) ?  ((0x01LLU << (x + 1LLU)) - 1LLU) : 0xffffffffffffffff)

// union used to store parsed result which can be integer our float.
union parsed_result {
	int64_t int_result;
	uint64_t uint_result;
	double float_result;
	char char_result[8];
};

/*=============================== Converters ===========================================================*/
// convert twos complement of length num bits to signed integer
union parsed_result read_twoscomp(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf);
// convert offset binary of length num bits to signed integer
union parsed_result read_ob(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf);
// convert binary offset complementary of length num bits to signed integer.
union parsed_result read_cob(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf);
// Read unsigned
union parsed_result read_uns(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf);
// Read floating point. Can be double(64 bit) or float (32 bit)
union parsed_result read_ieee_fp(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf);
// read char. returns 
union parsed_result read_char(uint64_t raw_bits);
// wrapper to call parser
union parsed_result call_parser(uint64_t raw_bits, uint8_t parser_select, uint8_t num_bits, uint8_t dtype_out, double sf);

#endif
