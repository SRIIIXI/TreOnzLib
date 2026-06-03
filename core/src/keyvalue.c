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

#include "keyvalue.h"

#include <string.h>
#include <memory.h>
#include <stdlib.h>

void keyvalue_copy(key_value_t* dest, key_value_t* orig)
{
	if (dest == NULL || orig == NULL)
	{
		return;
	}

	if (dest->Key != NULL && orig->Key != NULL)
	{
		buffer_copy(dest->Key, orig->Key);
	}

	if (dest->Value != NULL && orig->Value != NULL)
	{
		buffer_copy(dest->Value, orig->Value);
	}
}

bool keyvalue_is_equal(key_value_t* first, key_value_t* second)
{
    if (first == NULL || second == NULL || first->Key == NULL || second->Key == NULL)
    {
        return false;
    }

    return buffer_is_equal(first->Key, second->Key);
}

bool keyvalue_is_greater(key_value_t* first, key_value_t* second)
{
    if (first == NULL || second == NULL || first->Key == NULL || second->Key == NULL)
    {
        return false;
    }

    return buffer_is_greater(first->Key, second->Key);
}

bool keyvalue_is_less(key_value_t* first, key_value_t* second)
{
    if (first == NULL || second == NULL || first->Key == NULL || second->Key == NULL)
    {
        return false;
    }

    return buffer_is_less(first->Key, second->Key);
}
