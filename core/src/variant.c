/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "variant.h"
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>

typedef struct variant_t
{
    VariantType DataType;
    unsigned char RawBuffer[256];
    size_t DataSize;
}variant_t;

static size_t variant_internal_string_len(size_t ln)
{
	/* Keep one byte for null terminator when storing string payload. */
	return ln > (sizeof(((variant_t*)0)->RawBuffer) - 1) ? (sizeof(((variant_t*)0)->RawBuffer) - 1) : ln;
}

variant_t*  variant_allocate_default()
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	return retval;
}

void variant_release(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return;
	}

	free(varptr);
	varptr = NULL;
}

variant_t* variant_allocate(variant_t* varptr)
{
	varptr = (variant_t*)calloc(1, sizeof(variant_t));

	return varptr;
}

variant_t* variant_allocate_char(char ch)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(char);
	retval->DataType = Char;
	retval->RawBuffer[0] = ch;

	return retval;
}

variant_t* variant_allocate_unsigned_char(unsigned char ch)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(unsigned char);
	retval->DataType = UnsignedChar;
	retval->RawBuffer[0] = ch;

	return retval;
}

variant_t* variant_allocate_string(const char* str, size_t ln)
{
    if (str == NULL)
    {
        return NULL;
    }

	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = variant_internal_string_len(ln);
	retval->DataType = String;
	if (retval->DataSize > 0)
	{
		memcpy(&retval->RawBuffer[0], str, retval->DataSize);
	}
	retval->RawBuffer[retval->DataSize] = 0;

	return retval;
}

variant_t* variant_allocate_bool(bool fl)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(char);
	retval->DataType = Boolean;
	retval->RawBuffer[0] = fl;

	return retval;
}

variant_t* variant_allocate_long(long val)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(long);
	retval->DataType = Number;
	memcpy(&retval->RawBuffer[0], &val, sizeof(long));

	return retval;
}

variant_t* variant_allocate_unsigned_long(unsigned long val)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(unsigned long);
	retval->DataType = UnsignedNumber;
	memcpy(&retval->RawBuffer[0], &val, sizeof(unsigned long));

	return retval;
}

variant_t* variant_allocate_double(double val)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(double);
	retval->DataType = Decimal;
	memcpy(&retval->RawBuffer[0], &val, sizeof(double));

	return retval;
}

variant_t* variant_allocate_time_value(unsigned long val)
{
	variant_t* retval = (variant_t*)calloc(1, sizeof(variant_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->DataSize = sizeof(unsigned long);
	retval->DataType = DateTimeStamp;
	memcpy(&retval->RawBuffer[0], &val, sizeof(unsigned long));

	return retval;
}


void variant_set_variant(variant_t* varptr, variant_t* val)
{
	if(varptr == NULL || val == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	varptr->DataSize = val->DataSize > sizeof(varptr->RawBuffer) ? sizeof(varptr->RawBuffer) : val->DataSize;
	if (varptr->DataSize > 0)
	{
		memcpy(&varptr->RawBuffer[0], &val->RawBuffer[0], varptr->DataSize);
	}
	varptr->DataType = val->DataType;
}

void variant_set_char(variant_t* varptr, char ch)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	varptr->RawBuffer[0] = ch;
	varptr->DataSize = sizeof(char);
	varptr->DataType = Char;
}

void variant_set_unsigned_char(variant_t* varptr, unsigned char ch)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	varptr->RawBuffer[0] = ch;
	varptr->DataSize = sizeof(unsigned char);
	varptr->DataType = UnsignedChar;
}

void variant_set_string(variant_t* varptr, const char* str, size_t ln)
{
	if(varptr == NULL || str == NULL || ln < 1)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	varptr->DataSize = variant_internal_string_len(ln);
	if (varptr->DataSize > 0)
	{
		memcpy(&varptr->RawBuffer[0], str, varptr->DataSize);
	}
	varptr->RawBuffer[varptr->DataSize] = 0;
	varptr->DataType = String;
}

void variant_set_bool(variant_t* varptr, bool fl)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	varptr->RawBuffer[0] = fl;
	varptr->DataSize = sizeof(bool);
	varptr->DataType = Boolean;
}

void variant_set_long(variant_t* varptr, long val)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	memcpy(&varptr->RawBuffer[0], &val, sizeof(long));
	varptr->DataSize = sizeof(long);
	varptr->DataType = Number;
}

void variant_set_unsigned_long(variant_t* varptr, unsigned long val)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	memcpy(&varptr->RawBuffer[0], &val, sizeof(unsigned long));
	varptr->DataSize = sizeof(unsigned long);
	varptr->DataType = UnsignedNumber;
}

void variant_set_double(variant_t* varptr, double val)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	memcpy(&varptr->RawBuffer[0], &val, sizeof(double));
	varptr->DataSize = sizeof(double);
	varptr->DataType = Decimal;
}

void variant_set_time_value(variant_t* varptr, unsigned long val)
{
	if(varptr == NULL)
	{
		return;
	}

	memset(&varptr->RawBuffer[0], 0, sizeof(varptr->RawBuffer));
	memcpy(&varptr->RawBuffer[0], &val, sizeof(unsigned long));
	varptr->DataSize = sizeof(unsigned long);
	varptr->DataType = DateTimeStamp;
}


VariantType variant_get_data_type(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return Void;
	}

	return varptr->DataType;
}

size_t variant_get_data_size(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return 0;
	}

	return varptr->DataSize;
}

char variant_get_char(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != Char)
	{
		return 0;
	}

	return (char)varptr->RawBuffer[0];
}

unsigned char variant_get_unsigned_char(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != UnsignedChar)
	{
		return 0;
	}

	return (unsigned char)varptr->RawBuffer[0];
}

const char* variant_get_string(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return NULL;
	}

	if(varptr->DataType != String)
	{
		return NULL;
	}

	return (const char*)&varptr->RawBuffer[0];
}

bool variant_get_bool(variant_t* varptr)
{
	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != Boolean)
	{
		return false;
	}

	return (bool)varptr->RawBuffer[0];
}

long variant_get_long(variant_t* varptr)
{
	long retval = 0;

	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != Number)
	{
		return 0;
	}

	memcpy(&retval, &varptr->RawBuffer[0], sizeof(long));

	return retval;
}

unsigned long variant_get_unsigned_long(variant_t* varptr)
{
	unsigned long retval = 0;

	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != UnsignedNumber)
	{
		return 0;
	}

	memcpy(&retval, &varptr->RawBuffer[0], sizeof(unsigned long));

	return retval;
}

double variant_get_double(variant_t* varptr)
{
	double retval = 0;

	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != Decimal)
	{
		return 0;
	}

	memcpy(&retval, &varptr->RawBuffer[0], sizeof(double));

	return retval;
}

unsigned long variant_get_time_value(variant_t* varptr)
{
	unsigned long retval = 0;

	if(varptr == NULL)
	{
		return 0;
	}

	if(varptr->DataType != DateTimeStamp)
	{
		return 0;
	}

	memcpy(&retval, &varptr->RawBuffer[0], sizeof(unsigned long));

	return retval;
}
