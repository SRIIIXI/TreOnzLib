#include <treonzlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "stringex.h"

void test_list(void);
void test_string_list(void);
void test_string(void);
void test_logger(void);
void test_configuration(void);
void test_dictionary(void);
void test_base64(void);
void test_email(void);
void test_queue(void);

int main(int argc, char* argv[])
{
    string_t* user_name = env_get_current_user_name();
    string_t* process_name = env_get_current_process_name();
    string_t* lock_file_name = env_get_lock_filename();
    string_t* tmpdirname = dir_get_temp_directory();
    string_t* logdirname = dir_get_log_directory();
    string_t* cfgdirname = dir_get_config_directory();

    if (argc == 2)
    {
        switch (argv[1][0])
        {
        case 'b':
        {
            //Base64
            test_base64();
            break;
        }
        case 'f':
        {
            //Buffer
            break;
        }
        case 'c':
        {
            //Configuration
            test_configuration();
            break;
        }
        case 'd':
        {
            //Dictionary
            test_dictionary();
            break;
        }
        case 't':
        {
            //DateTime
            break;
        }
        case 'y':
        {
            //Directory
            break;
        }
        case 'e':
        {
            //Environmemt
            break;
        }
        case 'k':
        {
            //KeyValue
            break;
        }
        case 'l':
        {
            //List
            test_list();
            break;
        }
        case 'g':
        {
            //Logger
            test_logger();
            break;
        }
        case 'q':
        {
            //Queue
            test_queue();
            break;
        }
        case 'r':
        {
            //Responder
            break;
        }
        case 'i':
        {
            //SignalHandler
            break;
        }
        case 's':
        {
            //Stack
            break;
        }
        case 'x':
        {
            //StringEx
            test_string();
            break;
        }
        case 'n':
        {
            //StringList
            test_string_list();
            break;
        }
        case 'v':
        {
            //Variant
            break;
        }
        case 'm':
        {
            //Email
            test_email();
            break;
        }
        default:
        {
            break;
        }
        }
    }
    else
    {
        printf("Usage : coretest <option>\nOptions are b, f, c, d, t, y, e, k, l, g, q, r, i, s, x, n, v\n");
    }

    return 0;
}

void test_list(void)
{
    list_t* mylist = NULL;

    mylist = list_allocate(mylist);

    list_add_to_tail(mylist, "Hello", strlen("Hello"));
    list_add_to_tail(mylist, "World", strlen("World"));
    list_add_to_tail(mylist, "Linux", strlen("Linux"));

    void* item = NULL;

    size_t size = 0;

    item = list_get_first(mylist, &size);

    while(item)
    {
        printf("%s\n", (char*)item);
        item = list_get_next(mylist, &size);
    }

    list_clear(mylist);
    list_free(mylist);
}

void test_string_list(void)
{
    string_t* mystr = NULL;
    string_list_t* mylist = NULL;
    string_t* item = NULL;

    mystr = string_allocate_default();

    string_append(mystr, "Hello");
    string_append(mystr, "|");
    string_append(mystr, "World");
    string_append(mystr, "|");
    string_append(mystr, "Linux");

    mylist = string_split_by_substr(mystr, "|");

    free(mystr);

    item = string_get_first_from_list(mylist);

    while(item)
    {
        printf("%s\n", string_c_str(item));
        item = string_get_next_from_list(mylist);
    }

    string_free_list(mylist);
}

void test_string(void)
{
    /*
    char* str = "aaxxbbxxccxxddxxeexx";

    char** sub_str_list = NULL;
    sub_str_list = strsplitsubstr(str, "xx");

    for(int index = 0; sub_str_list[index] != 0; index++)
    {
        char* sub_str = NULL;
        sub_str = sub_str_list[index];
        printf("%s\n", sub_str);
    }

    strfreelist(sub_str_list);

    char process_name[64] = {0};
    char buffer[1025] = {0};
    pid_t proc_id = getpid();
    char** cmd_args = NULL;
    char** dir_tokens = NULL;

    sprintf(buffer, "/proc/%d/cmdline", proc_id);

    FILE* fp = fopen(buffer, "r");

    if(fp)
    {
        memset(buffer, 0, 1025);

        if(fgets(buffer, 1024, fp))
        {
            cmd_args = str_list_allocate_from_string(cmd_args, buffer, " ");

            if(cmd_args && str_list_item_count(cmd_args) > 0)
            {
                str_list_allocate_from_string(dir_tokens, str_list_get_first(cmd_args), "/");

                if(dir_tokens && str_list_item_count(dir_tokens) > 0)
                {
                    strcpy(process_name, str_list_get_last(dir_tokens));
                }
            }
            else
            {
                dir_tokens = str_list_allocate_from_string(dir_tokens, buffer, "/");

                if(dir_tokens && str_list_item_count(dir_tokens) > 0)
                {
                    strcpy(process_name, str_list_get_last(dir_tokens));
                }
            }
        }

        fclose(fp);
    }

    if(cmd_args)
    {
        str_list_clear(cmd_args);
        str_list_free(cmd_args);
    }

    if(dir_tokens)
    {
        str_list_clear(dir_tokens);
        str_list_free(dir_tokens);
    }
    */
}

void test_logger(void)
{
    logger_t* logger = logger_allocate_default();

    WriteInformation(logger, "test");

    logger_release(logger);
}

void test_configuration(void)
{
    configuration_t* conf = configuration_allocate_default();

    string_list_t* sections = configuration_get_all_sections(conf);

    string_t* sec_str = NULL;
    sec_str = string_get_first_from_list(sections);

    while(sec_str)
    {
        printf("Section %s\n", string_c_str(sec_str));
        
        string_list_t* keys = NULL;
        keys = configuration_get_all_keys(conf, string_c_str(sec_str));

        string_t* key_str = NULL;
        key_str = string_get_first_from_list(keys);

        while(key_str)
        {
            printf("Key %s Value %s\n", string_c_str(key_str), configuration_get_value_as_string(conf, string_c_str(sec_str), string_c_str(key_str)));
            key_str = string_get_next_from_list(keys);
        }

        string_free_list(keys);

        sec_str = string_get_next_from_list(sections);
    }

    string_free_list(sections);

    configuration_release(conf);
}

void test_dictionary(void)
{
    dictionary_t* dict = dictionary_allocate();

    dictionary_set_value(dict, "123", strlen("123"), "ABCDEFGHIJKLMN", strlen("ABCDEFGHIJKLMN"));
    dictionary_set_value(dict, "ABC", strlen("ABC"), "0123456789", strlen("0123456789"));

    char* val1 = (char*)dictionary_get_value(dict, "123", strlen("123"));
    char* val2 = (char*)dictionary_get_value(dict, "ABC", strlen("ABC"));

    dictionary_set_value(dict, "123", strlen("123"), "opqrstuvwxyz", strlen("opqrstuvwxyz"));
    char* val3 = (char*)dictionary_get_value(dict, "123", strlen("123"));

    char** all_keys = dictionary_get_all_keys(dict);

    for(int kindex = 0; all_keys[kindex] != 0; kindex++)
    {
        char* key_str = NULL;
        key_str = all_keys[kindex];
        printf("%s\n", key_str);
    }

    dictionary_free_key_list(dict, all_keys);

    dictionary_free(dict);
}

void test_base64(void)
{
    const char* old_fname = "/home/subrato/Pictures/testimage.jpg";
    const char* new_fname = "/home/subrato/Pictures/testimage_new.jpg";

    FILE* fp = fopen(old_fname, "rb");

    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        rewind(fp);

        unsigned char* old_image = NULL;
        old_image = (unsigned char*)calloc(1, sz);

        if(!old_image)
        {
            fclose(fp);
            return;
        }

        fread(old_image, sz, 1, fp);
        fclose(fp);

        char* encoded_image = NULL;
        unsigned long encoded_image_len = 0;

        encoded_image = base64_encode(old_image, sz, encoded_image, &encoded_image_len);

        unsigned char* new_image = NULL;
        unsigned long new_image_len = 0;

        new_image = base64_decode(encoded_image, encoded_image_len, new_image, &new_image_len);

        fp = fopen(new_fname, "wb");

        if(fp)
        {
            fwrite(new_image, new_image_len, 1, fp);
            fflush(fp);
            fclose(fp);
        }

        free(encoded_image);
        free(new_image);
    }
}

void test_email(void)
{

}

void test_queue(void)
{

}
