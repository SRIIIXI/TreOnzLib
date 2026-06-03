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

#include "logger.h"
#include "directory.h"
#include "file.h"
#include "stringex.h"
#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define END_OF_LINE "\n"
#define MAX_LOGGERS 512
#define MAX_PATHLEN 1024

static char log_level_names[5][16] = {"Information", "Error", "Warning", "Critical", "Panic"};

void normalize_function_name(char* func_name);

#pragma pack(1)
typedef struct logger_t
{
    size_t LogFileSizeMB;
    char FileName[MAX_PATHLEN+1];
    FILE* FileHandle;
    bool console_out;
    LogLevel log_level;
}logger_t;

logger_t*	logger_allocate_default()
{
    return logger_allocate(10, NULL);
}

logger_t*  logger_allocate_file(size_t flszmb, const char* filename)
{
    if(!filename)
    {
        return NULL;
    }

    logger_t* logger_ptr = (logger_t*)calloc(1, sizeof (logger_t));

    if(!logger_ptr)
    {
        return NULL;
    }

    logger_ptr->FileHandle = NULL;

    if(flszmb < 1 || flszmb > 10)
    {
        flszmb = 10;
    }

    logger_ptr->LogFileSizeMB = flszmb;
    strncpy(logger_ptr->FileName, filename, MAX_PATHLEN);
    logger_ptr->FileName[MAX_PATHLEN] = 0;

    logger_ptr->log_level = LOG_INFO;
    logger_ptr->console_out = false;

    return logger_ptr;
}


logger_t*	logger_allocate(size_t flszmb, const char* dirpath)
{
    logger_t* logger_ptr = (logger_t*)calloc(1, sizeof (logger_t));

    if(!logger_ptr)
    {
        return NULL;
    }

    logger_ptr->FileHandle = NULL;

    if(flszmb < 1 || flszmb > 10)
    {
        flszmb = 10;
    }

    logger_ptr->LogFileSizeMB = flszmb;

    if(dirpath != NULL && dirpath[0] != 0)
    {
        strncat(logger_ptr->FileName, dirpath, MAX_PATHLEN - strlen(logger_ptr->FileName));

        size_t dn_len = strlen(logger_ptr->FileName);

        if(dn_len > 0 && logger_ptr->FileName[dn_len - 1] != '/')
        {
            strncat(logger_ptr->FileName, "/", MAX_PATHLEN - strlen(logger_ptr->FileName));
        }
    }
    else
    {
        string_t* logdir = dir_get_log_directory();
        if(logdir == NULL)
        {
            free(logger_ptr);
            return NULL;
        }

        memset(&logger_ptr->FileName[0], 0, MAX_PATHLEN + 1);
        strncpy(&logger_ptr->FileName[0], string_c_str(logdir), MAX_PATHLEN);
        string_free(&logdir);
    }

    string_t* dir_t_str = string_allocate(logger_ptr->FileName);

    if(!dir_is_exists(dir_t_str))
    {
        dir_create_directory(dir_t_str);
    }

    string_free(&dir_t_str);
    
    string_t* process_name = env_get_current_process_name();
    if(process_name != NULL)
    {
        strncat(logger_ptr->FileName, string_c_str(process_name), MAX_PATHLEN - strlen(logger_ptr->FileName));
        string_free(&process_name);
    }
    else
    {
        strncat(logger_ptr->FileName, "process", MAX_PATHLEN - strlen(logger_ptr->FileName));
    }

    strncat(logger_ptr->FileName, ".log", MAX_PATHLEN - strlen(logger_ptr->FileName));

    logger_ptr->log_level = LOG_INFO;
    logger_ptr->console_out = false;

    return logger_ptr;
}

const char* logger_filename(logger_t* loggerptr)
{
    if(!loggerptr)
    {
        return NULL;
    }

    return &loggerptr->FileName[0];
}

void logger_release(logger_t* loggerptr)
{
    if(!loggerptr)
    {
        return;
    }

    if(loggerptr->FileHandle)
    {
        fflush(loggerptr->FileHandle);
        fclose(loggerptr->FileHandle);
    }

    free(loggerptr);
}

bool logger_write(logger_t* loggerptr, const char* logentry, LogLevel llevel, const char* func, const char* file, int line)
{
    if(!loggerptr || logentry == NULL || func == NULL || file == NULL)
    {
        return false;
    }

    if(llevel < LOG_INFO || llevel > LOG_PANIC)
    {
        return false;
    }

    if(llevel < loggerptr->log_level)
    {
        return false;
    }

    if(loggerptr->FileHandle == NULL)
    {
        loggerptr->FileHandle = fopen(loggerptr->FileName, "w");

        if(loggerptr->FileHandle == NULL)
        {
            return false;
        }
    }

    // Check the file size
    size_t sz = (size_t)ftell(loggerptr->FileHandle);

    // If it exceeds the set size
    if(sz >= loggerptr->LogFileSizeMB*1024*1024)
    {
        // Stop logging
        fflush(loggerptr->FileHandle);
        fclose(loggerptr->FileHandle);

        // Rename the file
        char old_log_filename[1025] = {0};
        strcat(old_log_filename, loggerptr->FileName);
        strcat(old_log_filename, ".old");

        rename(loggerptr->FileName, old_log_filename);

        // Reopen the log file with original name
        loggerptr->FileHandle = fopen(loggerptr->FileName, "w");

        if(loggerptr->FileHandle == NULL)
        {
            return false;
        }
    }

    time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

    if(tmp == NULL)
    {
        return false;
    }

    // Timestamp
    fprintf(loggerptr->FileHandle, "%02d-%02d-%04d %02d:%02d:%02d\t",
             tmp->tm_mday, (tmp->tm_mon+1), (tmp->tm_year+1900),
             tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    // Level
    fprintf(loggerptr->FileHandle, "%s\t", log_level_names[llevel]);

    // File
    const char* slash = strrchr(file, '/');
    const char* bslash = strrchr(file, '\\');
    const char* base_file_name = file;

    if (slash != NULL || bslash != NULL)
    {
        const char* last_sep = slash;
        if (last_sep == NULL || (bslash != NULL && bslash > last_sep))
        {
            last_sep = bslash;
        }

        if (last_sep != NULL && *(last_sep + 1) != 0)
        {
            base_file_name = last_sep + 1;
        }
    }

    const char* file_name_to_write = base_file_name;
    fprintf(loggerptr->FileHandle, "%s\t", file_name_to_write);

    // Line
    fprintf(loggerptr->FileHandle, "%d\t", line);

    // Function
    char* func_name = (char*)calloc(1, strlen(func)+1);
    if(func_name == NULL)
    {
        return false;
    }

    strcpy(func_name, func);
    normalize_function_name(func_name);
    fprintf(loggerptr->FileHandle, "%s\t", func_name);

    // Message
    fprintf(loggerptr->FileHandle, "%s", logentry);

    // End of line
    fprintf(loggerptr->FileHandle, END_OF_LINE);

    // Flush th contents
    fflush(loggerptr->FileHandle);

    if(loggerptr->console_out)
    {
        printf("%s %d %s %s\n", file_name_to_write, line, func, logentry);
        fflush(stdout);
    }

    free(func_name);

    return true;
}

void logger_enable_console_out(logger_t* loggerptr, bool consoleout)
{
    if(!loggerptr)
    {
        return;
    }

    loggerptr->console_out = consoleout;
}

void logger_set_log_level(logger_t* loggerptr, LogLevel llevel)
{
    if(!loggerptr)
    {
        return;
    }

    loggerptr->log_level = llevel;
}

void normalize_function_name(char* func_name)
{
    if(func_name == NULL)
    {
        return;
    }

    string_t* func_name_str = string_allocate(func_name);
    if(func_name_str == NULL)
    {
        return;
    }

    int len = (int)strlen(func_name);

    if(len < 2)
    {
        string_free(&func_name_str);
        return;
    }

    long ctr = len - 1;

    long pos = 0;

    pos = string_index_of_char(func_name_str, '(');

    if(pos > -1)
    {
        while(true)
        {
            func_name[ctr] = 0;
            ctr--;
            if(func_name[ctr] == '(')
            {
                func_name[ctr] = 0;
                break;
            }
        }
    }

    pos = string_index_of_char(func_name_str, ' ');

    if(pos > -1)
    {
        ctr = pos;
        while(ctr > -1)
        {
            func_name[ctr] = 32;
            ctr--;
        }
    }

    string_left_trim(func_name_str);

    memset(func_name, 0, len + 1);
    strcpy(func_name, string_c_str(func_name_str));
    string_free(&func_name_str);
}
