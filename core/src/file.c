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

#include "file.h"

#define DIRECTORY_SEPARATOR '/'

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

bool file_is_exists(const char* filename)
{
	if(filename == NULL || filename[0] == 0)
	{
		return false;
	}

	FILE* fp = fopen(filename, "r");

	if(fp)
	{
		fclose(fp);
		return true;
	}

	return false;
}

char* file_get_parent_directory(const char* filename)
{
	if(filename == NULL || filename[0] == 0)
	{
		return NULL;
	}

	size_t origlen = strlen(filename);
	if(origlen < 2)
	{
		return NULL;
	}

	char* parent_dir = (char*)calloc(1, sizeof(char) * (origlen + 1));

	if(parent_dir == NULL)
	{
		return NULL;
	}

	memcpy(parent_dir, filename, origlen);

	char* last_sep = strrchr(parent_dir, '/');
	char* last_bsep = strrchr(parent_dir, '\\');
	char* sep = last_sep;

	if(sep == NULL || (last_bsep != NULL && last_bsep > sep))
	{
		sep = last_bsep;
	}

	if(sep == NULL)
	{
		free(parent_dir);
		return NULL;
	}

	*sep = 0;

	return parent_dir;
}

char* file_get_basename(const char* filename)
{
	if(filename == NULL || filename[0] == 0)
	{
		return NULL;
	}

	size_t origlen = strlen(filename);
	if(origlen == 0)
	{
		return NULL;
	}

	char* basename = (char*)calloc(1, sizeof(char) * (origlen + 1));

	if(basename == NULL)
	{
		return NULL;
	}

	const char* last_sep = strrchr(filename, '/');
	const char* last_bsep = strrchr(filename, '\\');
	const char* start = filename;

	if(last_sep != NULL || last_bsep != NULL)
	{
		const char* sep = last_sep;
		if(sep == NULL || (last_bsep != NULL && last_bsep > sep))
		{
			sep = last_bsep;
		}
		start = sep + 1;
	}

	size_t base_len = strlen(start);
	memcpy(basename, start, base_len);
	basename[base_len] = 0;

	return basename;
}

char* file_get_extension(const char* filename)
{
	if(filename == NULL || filename[0] == 0)
	{
		return NULL;
	}

    size_t origlen = strlen(filename);
    if(origlen == 0)
    {
        return NULL;
    }

    char* extension = (char*)calloc(1, sizeof(char) * (origlen + 1));

    if(extension == NULL)
    {
        return NULL;
    }

	const char* last_dot = strrchr(filename, '.');
	const char* last_sep = strrchr(filename, '/');
	const char* last_bsep = strrchr(filename, '\\');
	const char* last_path_sep = last_sep;

	if(last_path_sep == NULL || (last_bsep != NULL && last_bsep > last_path_sep))
	{
		last_path_sep = last_bsep;
	}

	if(last_dot == NULL || (last_path_sep != NULL && last_dot < last_path_sep) || *(last_dot + 1) == 0)
	{
		extension[0] = 0;
		return extension;
	}

	size_t ext_len = strlen(last_dot + 1);
	memcpy(extension, last_dot + 1, ext_len);
	extension[ext_len] = 0;

    return extension;
}
