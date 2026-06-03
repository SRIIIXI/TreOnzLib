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

#include "environment.h"
#include "stringex.h"
#include "directory.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

// Global lock file descriptor - module static
static int g_lock_file_descriptor = -1;

static const char* env_internal_get_user_name()
{
    const char* user = getenv("USER");
    if(user == NULL || user[0] == 0)
    {
        return "root";
    }

    return user;
}

string_t* env_get_current_process_name()
{
    string_t* retval = NULL;
    size_t ln = 0;
    bool fnd = false;
    int ctr = 0;

    pid_t proc_id = getpid();
    char buffer[1025] = {0};

    char* procfilename = (char*)calloc(1, 32);
    if(procfilename == NULL)
    {
        return NULL;
    }

    sprintf(procfilename, "/proc/%d/cmdline", proc_id);
    FILE* fp = fopen(procfilename, "r");
    free(procfilename);
    procfilename = NULL;

    if(fp)
    {
        if(fgets(buffer, 1024, fp))
        {
            ln = strlen(buffer);
            fnd = false;

            for(ctr = 0; ctr < ln; ctr++)
            {

                if(fnd)
                {
                    buffer[ctr] = 0;
                }
                else
                {
                    if(isspace(buffer[ctr]))
                    {
                        fnd = true;
                        buffer[ctr] = 0;
                    }                    
                }
            }

            ln = strlen(buffer);
            fnd = false;

            for(ctr = ln -1 ; ctr > -1; ctr--)
            {

                if(fnd)
                {
                    buffer[ctr] = 32;
                }
                else
                {
                    if(buffer[ctr] == '/')
                    {
                        fnd = true;
                        buffer[ctr] = 32;
                    }                    
                }
            }

            retval = string_allocate(buffer);
            if(retval != NULL)
            {
                retval = string_all_trim(retval);
            }
        }

        fclose(fp);
    }

    return retval;
}

string_t* env_get_current_user_name()
{
    string_t* ptr = string_allocate(env_internal_get_user_name());
    return ptr;
}

string_t* env_get_lock_filename()
{
    string_t* lock_filename = string_allocate_default();
    string_t* temp = NULL;

    if (lock_filename == NULL)
    {
        return NULL;
    }
    
    temp = dir_get_temp_directory();
    if (temp == NULL)
    {
        string_free(&lock_filename);
        return NULL;
    }

    string_append_string(lock_filename, temp);
    string_free(&temp);
    string_append_char(lock_filename, '/');

    temp = env_get_current_process_name();
    if (temp == NULL)
    {
        string_free(&lock_filename);
        return NULL;
    }

    string_append_string(lock_filename, temp);
    string_free(&temp);
    string_append_char(lock_filename, '.');

    temp = env_get_current_user_name();
    if (temp == NULL)
    {
        string_free(&lock_filename);
        return NULL;
    }

    string_append_string(lock_filename, temp);
    string_free(&temp);

    string_append(lock_filename, ".lock");

    return  lock_filename;
}

bool env_lock_process()
{
    string_t* lock_filename = NULL;
    char* lock_path = NULL;
    
    // Already locked
    if (g_lock_file_descriptor != -1)
    {
        return false;
    }

    lock_filename = env_get_lock_filename();
    if (lock_filename == NULL)
    {
        return false;
    }

    lock_path = strdup(string_c_str(lock_filename));
    string_free(&lock_filename);

    if (lock_path == NULL)
    {
        return false;
    }

    // Create lock file with exclusive access
    g_lock_file_descriptor = open(lock_path, O_CREAT | O_RDWR | O_EXCL, 0666);
    
    if (g_lock_file_descriptor == -1)
    {
        // File might already exist, try to open it
        g_lock_file_descriptor = open(lock_path, O_RDWR);
        
        if (g_lock_file_descriptor == -1)
        {
            free(lock_path);
            return false;
        }
    }

    // Try to acquire exclusive lock
    if (flock(g_lock_file_descriptor, LOCK_EX | LOCK_NB) == -1)
    {
        close(g_lock_file_descriptor);
        g_lock_file_descriptor = -1;
        free(lock_path);
        return false;
    }

    // Write our PID to the lock file
    char pid_buffer[32] = {0};
    sprintf(pid_buffer, "%d\n", getpid());
    
    if (write(g_lock_file_descriptor, pid_buffer, strlen(pid_buffer)) == -1)
    {
        // Writing failed, but we still have the lock
        // This is not critical, continue
    }

    free(lock_path);

    return true;
}

bool env_unlock_process()
{
    string_t* lock_filename = NULL;
    char* lock_path = NULL;
    
    // Not locked
    if (g_lock_file_descriptor == -1)
    {
        return false;
    }

    // Release the flock
    flock(g_lock_file_descriptor, LOCK_UN);
    
    // Close the file descriptor
    close(g_lock_file_descriptor);
    g_lock_file_descriptor = -1;

    // Remove the lock file
    lock_filename = env_get_lock_filename();
    if (lock_filename != NULL)
    {
        lock_path = strdup(string_c_str(lock_filename));
        string_free(&lock_filename);
        
        if (lock_path != NULL)
        {
            unlink(lock_path);
            free(lock_path);
        }
    }

    return true;
}

bool env_is_process_locked()
{
    return (g_lock_file_descriptor != -1);
}

pid_t env_get_lock_file_pid()
{
    string_t* lock_filename = NULL;
    char* lock_path = NULL;
    FILE* fp = NULL;
    char buffer[32] = {0};
    pid_t locked_pid = 0;
    
    lock_filename = env_get_lock_filename();
    if (lock_filename == NULL)
    {
        return 0;
    }

    lock_path = strdup(string_c_str(lock_filename));
    string_free(&lock_filename);
    
    if (lock_path == NULL)
    {
        return 0;
    }

    fp = fopen(lock_path, "r");
    free(lock_path);
    
    if (fp == NULL)
    {
        return 0;
    }

    if (fgets(buffer, sizeof(buffer) - 1, fp) != NULL)
    {
        locked_pid = (pid_t)atoi(buffer);
    }

    fclose(fp);
    return locked_pid;
}
