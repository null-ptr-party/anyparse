#include "converters.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*=============================== Converters ===========================================================*/

// converters for signed types
union parsed_result read_twoscomp(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf)
{	// twos complement
	union parsed_result result = { .int_result = 0 };
	raw_bits = (raw_bits & MASK((num_bits - 1))); // ensures num doesnt exceed size.
	uint64_t sign = raw_bits & (uint64_t)(0x01 << (num_bits - 1));
	int64_t word = (int64_t)((raw_bits & ~sign) - (sign));

	if (dtype_out == DTYPE_OUT_FLOAT)
	{
		result.float_result = word * sf;
	}
	else
	{
		result.int_result = word * (int64_t)sf;
	}
	
	return result;
}

union parsed_result read_ob(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf)
{	// offset binary
	union parsed_result result = { .int_result = 0 };

	raw_bits = (raw_bits & MASK((num_bits - 1))); // ensures num doesnt exceed size.
	uint64_t offset = (uint64_t)(0x01LLU << (num_bits - 1));
	int64_t word = raw_bits - offset;

	if (dtype_out == DTYPE_OUT_FLOAT)
	{
		result.float_result = word * sf;
	}
	else
	{
		result.int_result = word * (int64_t)sf;
	}

	return result;
}

union parsed_result read_cob(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf)
{	// complementary offest binary
	union parsed_result result = { .int_result = 0 };

	raw_bits = (~raw_bits & MASK((num_bits - 1))); // ensures num doesnt exceed size.
	int64_t offset = (0x01LLU << (num_bits - 1));
	int64_t word = (int64_t)(raw_bits - offset);

	if (dtype_out == DTYPE_OUT_FLOAT)
	{
		result.float_result = word * sf;
	}
	else
	{
		result.int_result = word * (int64_t)sf;
	}

	return result;
}

union parsed_result read_uns(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf)
{	// unsigned integer
	union parsed_result result = { .int_result = 0 };

	int64_t word = (int64_t)(raw_bits & MASK((num_bits - 1))); // ensures num doesnt exceed size.

	if (dtype_out == DTYPE_OUT_FLOAT)
	{
		result.float_result = word * sf;
	}
	else
	{
		result.int_result = word * (int64_t)sf;
	}

	return result;
}

union parsed_result read_ieee_fp(uint64_t raw_bits, uint8_t num_bits, uint8_t dtype_out, double sf)
{	// ieee fp types
	/* this is a weird one. Since typecasting a uint as a double changes the value of the bits
	what we have to do here is "fool" C into looking at the raw bits as ieee type. We can do this
	by typcasting a pointer. Pretty cool*/
	(void)dtype_out; // param not used in this function. only kept to maintain call signature. 

	union parsed_result result = { .int_result = 0 };

	raw_bits = (uint64_t)(raw_bits & MASK((num_bits - 1LLU))); // ensures num doesnt exceed size.

	if (num_bits == 32)
	{
		float* float_ptr = (float*)&raw_bits; // typcast pointer to raw bits as pointer to float
		result.float_result = (double)(*float_ptr) * sf; // dereference float pointer, then typcast to double for storage.
	}
	else
	{
		double* double_ptr = (double*)&raw_bits; // typecast double float as 
		result.float_result = *double_ptr * sf; // double type
	}

	if (DTYPE_OUT_INT)
	{
		result.int_result = (int64_t)result.float_result; // truncate to int. not sure why you would need this.
	}

	return result;
}

union parsed_result read_char(uint64_t raw_bits)
{
	union parsed_result result = { .int_result = 0 };

	char* char_ptr = (char*)&raw_bits;

	for (uint8_t idx = 0; idx < 8; idx++, char_ptr++)
	{
		result.char_result[idx] = *char_ptr;
	}

	return result;
}

union parsed_result call_parser(uint64_t raw_bits, uint8_t parser_select, uint8_t num_bits, uint8_t dtype_out, double sf)
{	//wrapper function to call whatever parser is needed.
	switch (parser_select)
	{
		case CONVERTER_TWOS_COMP:
			return read_twoscomp(raw_bits, num_bits, dtype_out, sf);

		case CONVERTER_READ_COB:
			return read_cob(raw_bits, num_bits, dtype_out, sf);

		case CONVERTER_READ_OB:
			return read_ob(raw_bits, num_bits, dtype_out, sf);

		case CONVERTER_READ_UNS:
			return read_uns(raw_bits, num_bits, dtype_out, sf);

		case CONVERTER_READ_IEEE_FP:
			return read_ieee_fp(raw_bits, num_bits, dtype_out, sf);

		case CONVERTER_READ_CHAR:
			(void)num_bits; // discard
			(void)dtype_out;
			(void)sf;
			return read_char(raw_bits);

		default:
			return read_uns(raw_bits, num_bits, dtype_out, sf);
	}
}