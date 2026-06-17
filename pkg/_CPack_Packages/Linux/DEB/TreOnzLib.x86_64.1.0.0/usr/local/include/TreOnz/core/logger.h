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

#ifndef LOGGER_C
#define LOGGER_C

#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum LogLevel
{
	LOG_INFO = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_CRITICAL = 3,
	LOG_PANIC = 4
}LogLevel;

#if defined(_WIN32) || defined(WIN32)
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

typedef struct logger_t logger_t;

extern LIBRARY_EXPORT logger_t*  logger_allocate_default();
extern LIBRARY_EXPORT logger_t*  logger_allocate(size_t flszmb, const char* dirpath);
extern LIBRARY_EXPORT logger_t*  logger_allocate_file(size_t flszmb, const char* filename);
extern LIBRARY_EXPORT const char* logger_filename(logger_t* loggerptr);
extern LIBRARY_EXPORT void    logger_release(logger_t* loggerptr);
extern LIBRARY_EXPORT bool    logger_write(logger_t* loggerptr, const char* logentry, LogLevel llevel, const char* func, const char* file, int line);
extern LIBRARY_EXPORT void logger_enable_console_out(logger_t* loggerptr, bool consoleout);
extern LIBRARY_EXPORT void logger_set_log_level(logger_t* loggerptr, LogLevel llevel);

#define WriteLog(lptr, str, level) \
    logger_write(lptr, str, level, (char*)__PRETTY_FUNCTION__, (char*)__FILE__, __LINE__)

#define WriteInformation(lptr, str) \
    logger_write(lptr, str, LOG_INFO, (char*)__PRETTY_FUNCTION__, (char*)__FILE__, __LINE__);

#ifdef __cplusplus
}
#endif

#endif

