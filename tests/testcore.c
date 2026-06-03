#include <treonzlib.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "listdoublelinked.h"
#include "stringex.h"
#include "variant.h"
#include "datetime.h"
#include "signalhandler.h"
#include "json.h"
#include "xml.h"

extern int raise(int sig);

void test_list(void);
void test_string_list(void);
void test_string(void);
void test_buffer(void);
void test_logger(void);
void test_configuration(void);
void test_dictionary(void);
void test_variant(void);
void test_keyvalue(void);
void test_datetime(void);
void test_signalhandler(void);
void test_base64(void);
void test_json(void);
void test_xml(void);
void test_file(void);
void test_directory(void);
void test_environment(void);
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
            test_buffer();
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
            test_datetime();
            break;
        }
        case 'y':
        {
            //Json
            test_json();
            break;
        }
        case 'u':
        {
            //Directory
            test_directory();
            break;
        }
        case 'w':
        {
            //Environment
            test_environment();
            break;
        }
        case 'e':
        {
            //File
            test_file();
            break;
        }
        case 'k':
        {
            //KeyValue
            test_keyvalue();
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
            //Xml
            test_xml();
            break;
        }
        case 'i':
        {
            //SignalHandler
            test_signalhandler();
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
            test_variant();
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
        printf("Usage : coretest <option>\nOptions are b, f, c, d, t, y(json), u(directory), w(environment), e, k, l, g, q, r, i, s, x, n, v\n");
    }

    return 0;
}

void test_list(void)
{
    list_t* mylist = NULL;
    list_t* emptylist = NULL;
    list_t* sizelist = NULL;
    list_double_linked_t* mydoublelist = NULL;
    list_double_linked_t* emptydoublelist = NULL;
    const char hello_value[] = "Hello";
    const char world_value[] = "World";
    const char linux_value[] = "Linux";
    const char short_value[] = "A";
    const char longer_value[] = "Longer";
    size_t size = 0;
    void* item = NULL;

    emptylist = list_allocate(emptylist);
    assert(emptylist != NULL);
    assert(list_get_first(emptylist, &size) == NULL);
    assert(list_get_next(emptylist, &size) == NULL);
    assert(list_get_last(emptylist, &size) == NULL);
    assert(list_index_of_value(emptylist, (void*)hello_value, sizeof(hello_value)) == -1);
    list_free(emptylist);

    sizelist = list_allocate(sizelist);
    assert(sizelist != NULL);
    list_add_to_tail(sizelist, (void*)short_value, sizeof(short_value));
    list_add_to_tail(sizelist, (void*)longer_value, sizeof(longer_value));
    item = list_get_first(sizelist, &size);
    assert(item != NULL);
    assert(size == sizeof(short_value));
    item = list_get_next(sizelist, &size);
    assert(item != NULL);
    assert(size == sizeof(longer_value));
    list_free(sizelist);

    mylist = list_allocate(mylist);
    assert(mylist != NULL);

    list_add_to_tail(mylist, (void*)hello_value, sizeof(hello_value));
    list_add_to_tail(mylist, (void*)world_value, sizeof(world_value));
    list_add_to_tail(mylist, (void*)linux_value, sizeof(linux_value));

    assert(list_item_count(mylist) == 3);
    assert(memcmp(list_get_at(mylist, 0), hello_value, sizeof(hello_value)) == 0);
    assert(memcmp(list_get_at(mylist, 1), world_value, sizeof(world_value)) == 0);
    assert(memcmp(list_get_at(mylist, 2), linux_value, sizeof(linux_value)) == 0);

    list_remove_from_tail(mylist);
    assert(list_item_count(mylist) == 2);
    assert(memcmp(list_get_at(mylist, 1), world_value, sizeof(world_value)) == 0);

    list_add_to_tail(mylist, (void*)linux_value, sizeof(linux_value));
    assert(list_item_count(mylist) == 3);
    assert(memcmp(list_get_at(mylist, 2), linux_value, sizeof(linux_value)) == 0);

    list_remove_value(mylist, (void*)world_value, sizeof(world_value));
    assert(list_item_count(mylist) == 2);
    assert(memcmp(list_get_at(mylist, 0), hello_value, sizeof(hello_value)) == 0);
    assert(memcmp(list_get_at(mylist, 1), linux_value, sizeof(linux_value)) == 0);

    list_add_to_tail(mylist, (void*)world_value, sizeof(world_value));
    assert(list_item_count(mylist) == 3);

    item = list_get_first(mylist, &size);

    while(item)
    {
        printf("%s\n", (char*)item);
        item = list_get_next(mylist, &size);
    }

    list_clear(mylist);
    list_free(mylist);

    emptydoublelist = list_double_linked_allocate(emptydoublelist);
    assert(emptydoublelist != NULL);
    assert(list_double_linked_get_first(emptydoublelist) == NULL);
    assert(list_double_linked_get_last(emptydoublelist) == NULL);
    assert(list_double_linked_get_previous(emptydoublelist) == NULL);
    assert(list_double_linked_index_of(emptydoublelist, hello_value) == -1);
    assert(list_double_linked_index_of_value(emptydoublelist, (void*)hello_value, sizeof(hello_value)) == -1);
    list_double_linked_free(emptydoublelist);

    mydoublelist = list_double_linked_allocate(mydoublelist);
    assert(mydoublelist != NULL);

    list_double_linked_add_to_tail(mydoublelist, (void*)hello_value, sizeof(hello_value));
    list_double_linked_add_to_tail(mydoublelist, (void*)world_value, sizeof(world_value));
    list_double_linked_add_to_tail(mydoublelist, (void*)linux_value, sizeof(linux_value));
    assert(list_double_linked_item_count(mydoublelist) == 3);

    list_double_linked_remove_from_tail(mydoublelist);
    assert(list_double_linked_item_count(mydoublelist) == 2);
    assert(memcmp(list_double_linked_get_at(mydoublelist, 1), world_value, sizeof(world_value)) == 0);

    list_double_linked_add_to_tail(mydoublelist, (void*)linux_value, sizeof(linux_value));
    assert(list_double_linked_item_count(mydoublelist) == 3);
    list_double_linked_remove_value(mydoublelist, (void*)world_value, sizeof(world_value));
    assert(list_double_linked_item_count(mydoublelist) == 2);
    assert(memcmp(list_double_linked_get_at(mydoublelist, 0), hello_value, sizeof(hello_value)) == 0);
    assert(memcmp(list_double_linked_get_at(mydoublelist, 1), linux_value, sizeof(linux_value)) == 0);

    list_double_linked_clear(mydoublelist);
    list_double_linked_free(mydoublelist);
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
    const char* logger_file = NULL;

    assert(logger != NULL);

    logger_file = logger_filename(logger);
    assert(logger_file != NULL);
    assert(strlen(logger_file) > 0);

    WriteInformation(logger, "test");

    /* Hardened null-safety paths should fail cleanly without crashing. */
    assert(logger_write(logger, NULL, LOG_INFO, __PRETTY_FUNCTION__, __FILE__, __LINE__) == false);
    assert(logger_write(logger, "test", LOG_INFO, NULL, __FILE__, __LINE__) == false);
    assert(logger_write(logger, "test", LOG_INFO, __PRETTY_FUNCTION__, NULL, __LINE__) == false);
    assert(logger_write(logger, "test", (LogLevel)99, __PRETTY_FUNCTION__, __FILE__, __LINE__) == false);

    logger_release(logger);
}

void test_json(void)
{
    const char* js = "{\"name\":\"treonz\",\"num\":123,\"flag\":true,\"arr\":[\"a\",\"b\"]}";
    json_document_t* doc = NULL;
    json_document_t* doc_file = NULL;
    FILE* fp = NULL;
    const char* tmp_json = "/tmp/treonz_json_test.json";

    doc = json_parse_string(js);
    assert(doc != NULL);

    assert(json_parse_string("{\"bad\": }") == NULL);

    fp = fopen(tmp_json, "wb");
    assert(fp != NULL);
    assert(fwrite(js, 1, strlen(js), fp) == strlen(js));
    fclose(fp);
    fp = NULL;

    doc_file = json_load_file(tmp_json);
    assert(doc_file != NULL);

    remove(tmp_json);

    json_free_document(doc_file);

    json_free_document(doc);
}

void test_xml(void)
{
    const char* xs = "<?xml version=\"1.0\"?><root><item id=\"42\">hello</item><item>world</item></root>";
    const char* malformed = "<root><item></root>";
    xml_document_t* doc = NULL;
    xml_document_t* malformed_doc = NULL;
    xml_document_t* doc_file = NULL;
    FILE* fp = NULL;
    const char* tmp_xml = "/tmp/treonz_xml_test.xml";

    doc = xml_parse_string(xs);
    assert(doc != NULL);

    malformed_doc = xml_parse_string(malformed);

    fp = fopen(tmp_xml, "wb");
    assert(fp != NULL);
    assert(fwrite(xs, 1, strlen(xs), fp) == strlen(xs));
    fclose(fp);
    fp = NULL;

    doc_file = xml_load_file(tmp_xml);
    assert(doc_file != NULL);

    remove(tmp_xml);

    xml_free_document(malformed_doc);
    xml_free_document(doc_file);
    xml_free_document(doc);
}

void test_file(void)
{
    char* parent = NULL;
    char* base = NULL;
    char* ext = NULL;

    assert(file_is_exists(NULL) == false);
    assert(file_get_parent_directory(NULL) == NULL);
    assert(file_get_basename(NULL) == NULL);
    assert(file_get_extension(NULL) == NULL);

    parent = file_get_parent_directory("/tmp/abc.txt");
    assert(parent != NULL);
    assert(strcmp(parent, "/tmp") == 0);
    free(parent);

    parent = file_get_parent_directory("abc.txt");
    assert(parent == NULL);

    base = file_get_basename("stdin");
    assert(base != NULL);
    assert(strcmp(base, "stdin") == 0);
    free(base);

    base = file_get_basename("/tmp/abc.txt");
    assert(base != NULL);
    assert(strcmp(base, "abc.txt") == 0);
    free(base);

    ext = file_get_extension("noext");
    assert(ext != NULL);
    assert(strcmp(ext, "") == 0);
    free(ext);

    ext = file_get_extension("/tmp/abc.txt");
    assert(ext != NULL);
    assert(strcmp(ext, "txt") == 0);
    free(ext);
}

void test_directory(void)
{
    string_t* parent = NULL;
    string_t* tmp = NULL;
    string_t* logd = NULL;
    string_t* cfg = NULL;
    string_t* missing = NULL;
    const char* old_user = getenv("USER");

    assert(dir_is_exists(NULL) == false);
    assert(dir_get_parent_directory(NULL) == NULL);

    parent = string_allocate("/tmp/treonz/sub");
    assert(parent != NULL);
    string_t* parent_dir = dir_get_parent_directory(parent);
    assert(parent_dir != NULL);
    assert(strcmp(string_c_str(parent_dir), "/tmp/treonz") == 0);
    string_free(&parent_dir);
    string_free(&parent);

    missing = string_allocate("/path/that/does/not/exist");
    assert(missing != NULL);
    assert(dir_is_exists(missing) == false);
    string_free(&missing);

    unsetenv("USER");
    tmp = dir_get_temp_directory();
    logd = dir_get_log_directory();
    cfg = dir_get_config_directory();
    assert(tmp != NULL && logd != NULL && cfg != NULL);
    assert(strcmp(string_c_str(tmp), "/tmp/") == 0);
    assert(strcmp(string_c_str(logd), "/var/log/") == 0);
    assert(strcmp(string_c_str(cfg), "/etc/") == 0);
    string_free(&tmp);
    string_free(&logd);
    string_free(&cfg);

    if(old_user != NULL)
    {
        setenv("USER", old_user, 1);
    }
}

void test_environment(void)
{
    string_t* user = NULL;
    string_t* proc = NULL;
    string_t* lock_name = NULL;
    const char* old_user = getenv("USER");
    char* old_user_copy = old_user ? strdup(old_user) : NULL;

    unsetenv("USER");

    user = env_get_current_user_name();
    assert(user != NULL);
    assert(strcmp(string_c_str(user), "root") == 0);
    string_free(&user);

    proc = env_get_current_process_name();
    assert(proc != NULL);
    assert(string_get_length(proc) > 0);
    string_free(&proc);

    lock_name = env_get_lock_filename();
    assert(lock_name != NULL);
    assert(string_get_length(lock_name) > 0);
    string_free(&lock_name);

    if (env_lock_process())
    {
        assert(env_is_process_locked() == true);
        assert(env_unlock_process() == true);
    }

    assert(env_is_process_locked() == false);

    if(old_user_copy != NULL)
    {
        setenv("USER", old_user_copy, 1);
        free(old_user_copy);
    }
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
    int ref_value = 99;

    assert(dict != NULL);

    dictionary_set_value(dict, "123", strlen("123"), "ABCDEFGHIJKLMN", strlen("ABCDEFGHIJKLMN"));
    dictionary_set_value(dict, "ABC", strlen("ABC"), "0123456789", strlen("0123456789"));

    char* val1 = (char*)dictionary_get_value(dict, "123", strlen("123"));
    char* val2 = (char*)dictionary_get_value(dict, "ABC", strlen("ABC"));

    assert(val1 != NULL);
    assert(val2 != NULL);
    assert(memcmp(val1, "ABCDEFGHIJKLMN", strlen("ABCDEFGHIJKLMN")) == 0);
    assert(memcmp(val2, "0123456789", strlen("0123456789")) == 0);

    dictionary_set_value(dict, "123", strlen("123"), "opqrstuvwxyz", strlen("opqrstuvwxyz"));
    char* val3 = (char*)dictionary_get_value(dict, "123", strlen("123"));
    assert(val3 != NULL);
    assert(memcmp(val3, "opqrstuvwxyz", strlen("opqrstuvwxyz")) == 0);

    // Mismatched key size should be a safe miss and must not read past key buffer.
    assert(dictionary_get_value(dict, "123", 64) == NULL);

    dictionary_set_reference(dict, "ABC", strlen("ABC"), &ref_value);
    assert(dictionary_get_value(dict, "ABC", strlen("ABC")) == &ref_value);

    dictionary_set_value(dict, "ABC", strlen("ABC"), "reset", strlen("reset"));
    char* val4 = (char*)dictionary_get_value(dict, "ABC", strlen("ABC"));
    assert(val4 != NULL);
    assert(memcmp(val4, "reset", strlen("reset")) == 0);

    char** all_keys = dictionary_get_all_keys(dict);
    int key_count = 0;

    assert(all_keys != NULL);

    for(int kindex = 0; all_keys[kindex] != 0; kindex++)
    {
        char* key_str = NULL;
        key_str = all_keys[kindex];
        printf("%s\n", key_str);
        key_count++;
    }

    // Replacing existing keys should not duplicate entries.
    assert(key_count == 2);

    dictionary_free_key_list(dict, all_keys);

    dictionary_free(dict);
}

void test_base64(void)
{
    const unsigned char sample[] = "Hello, Base64!";
    const char expected_encoded[] = "SGVsbG8sIEJhc2U2NCE=";
    char* encoded = NULL;
    unsigned char* decoded = NULL;
    unsigned char* empty_decoded = NULL;
    unsigned long encoded_len = 0;
    unsigned long decoded_len = 0;
    unsigned long invalid_len = 123;

    encoded = base64_encode(sample, sizeof(sample) - 1, NULL, &encoded_len);
    assert(encoded != NULL);
    assert(encoded_len == strlen(expected_encoded));
    assert(memcmp(encoded, expected_encoded, encoded_len) == 0);

    decoded = base64_decode(encoded, encoded_len, NULL, &decoded_len);
    assert(decoded != NULL);
    assert(decoded_len == sizeof(sample) - 1);
    assert(memcmp(decoded, sample, decoded_len) == 0);

    assert(base64_encode(sample, sizeof(sample) - 1, NULL, NULL) == NULL);
    assert(base64_decode("abc", 3, NULL, &invalid_len) == NULL);
    assert(invalid_len == 0);

    empty_decoded = base64_decode("", 0, NULL, &decoded_len);
    assert(empty_decoded != NULL);
    assert(decoded_len == 0);

    free(empty_decoded);
    free(decoded);
    free(encoded);
}

void test_email(void)
{

}

void test_buffer(void)
{
    buffer_t* b = NULL;
    buffer_t* cpy = NULL;
    string_t* str = NULL;
    const char one[] = "one";
    const char two[] = "two";
    char big[9000] = {0};
    memset(big, 'x', sizeof(big));

    b = buffer_allocate_default();
    assert(b != NULL);
    assert(buffer_get_size(b) == 0);

    b = buffer_append(b, one, sizeof(one));
    assert(b != NULL);
    assert(buffer_get_size(b) == sizeof(one));

    b = buffer_append(b, two, sizeof(two));
    assert(b != NULL);
    assert(buffer_get_size(b) == sizeof(one) + sizeof(two));

    // Oversized remove should be ignored safely.
    buffer_remove(b, 2, 1024);
    assert(buffer_get_size(b) == sizeof(one) + sizeof(two));

    buffer_remove_start(b, sizeof(one));
    assert(buffer_get_size(b) == sizeof(two));

    str = buffer_convert_to_string(b);
    assert(str != NULL);
    assert(strcmp(string_c_str(str), two) == 0);
    string_free(&str);

    cpy = buffer_allocate_length(16);
    assert(cpy != NULL);
    cpy = buffer_copy(cpy, b);
    assert(cpy != NULL);
    assert(buffer_is_equal(cpy, b));

    cpy = buffer_append(cpy, big, sizeof(big));
    assert(cpy != NULL);
    assert(buffer_get_size(cpy) == sizeof(two) + sizeof(big));

    buffer_free(&cpy);
    buffer_free(&b);
}

void test_queue(void)
{
    queue_t* qptr = NULL;
    const char one[] = "one";
    const char two[] = "two";
    size_t out_size = 123;
    void* item = NULL;

    qptr = queue_allocate(qptr);
    assert(qptr != NULL);

    item = queue_dequeue(qptr, &out_size);
    assert(item == NULL);
    assert(out_size == 0);

    item = queue_peek(qptr, &out_size);
    assert(item == NULL);
    assert(out_size == 0);

    queue_enqueue(qptr, (void*)one, sizeof(one));
    queue_enqueue(qptr, (void*)two, sizeof(two));
    assert(queue_item_count(qptr) == 2);

    item = queue_peek(qptr, &out_size);
    assert(item != NULL);
    assert(out_size == sizeof(one));
    assert(memcmp(item, one, sizeof(one)) == 0);
    assert(queue_item_count(qptr) == 2);

    item = queue_dequeue(qptr, &out_size);
    assert(item != NULL);
    assert(out_size == sizeof(one));
    assert(memcmp(item, one, sizeof(one)) == 0);
    free(item);
    assert(queue_item_count(qptr) == 1);

    item = queue_dequeue(qptr, &out_size);
    assert(item != NULL);
    assert(out_size == sizeof(two));
    assert(memcmp(item, two, sizeof(two)) == 0);
    free(item);
    assert(queue_item_count(qptr) == 0);

    queue_free(qptr);
}

static volatile int signalhandler_called = 0;
static volatile int signalhandler_last_type = -1;

static void test_signalhandler_callback(SignalType stype)
{
    signalhandler_called++;
    signalhandler_last_type = (int)stype;
}

void test_signalhandler(void)
{
    signalhandler_called = 0;
    signalhandler_last_type = -1;

    /* Must not crash when callback is not registered. */
    signals_register_callback(NULL);
    signals_initialize_handlers();
    raise(10); /* SIGUSR1 on Linux */
    assert(signalhandler_called == 0);

    signals_register_callback(test_signalhandler_callback);

    raise(10); /* SIGUSR1 on Linux */
    assert(signalhandler_called > 0);
    assert(signalhandler_last_type == Userdefined1);

    raise(28); /* SIGWINCH on Linux */
    assert(signalhandler_last_type == WindowResized);
}

void test_datetime(void)
{
    date_time_t* now = NULL;
    date_time_t* later = NULL;
    date_time_t* from_epoch = NULL;
    char default_ts[65] = {0};
    char long_format[256] = {0};
    char* text = NULL;
    char* formatted = NULL;

    now = date_time_allocate_default();
    assert(now != NULL);

    assert(date_time_get_default_string(default_ts) != NULL);
    assert(strlen(default_ts) == 14);

    text = date_time_get_string(now);
    assert(text != NULL);
    assert(strlen(text) == 14);
    free(text);

    from_epoch = date_time_from_epoch_ll(1700000000ULL);
    assert(from_epoch != NULL);
    assert(date_time_to_epoch_ll(from_epoch) > 0);

    later = date_time_add_seconds(now, 30);
    assert(later != NULL);
    assert(date_time_is_first_greater(later, now) || date_time_are_equal(later, now));

    for (int i = 0; i < 254; i += 2)
    {
        long_format[i] = 'h';
        long_format[i + 1] = 'X';
    }
    long_format[254] = 'h';
    long_format[255] = 0;

    formatted = date_time_get_formatted_string(now, long_format);
    if (formatted != NULL)
    {
        free(formatted);
    }

    date_time_release(later);
    date_time_release(from_epoch);
    date_time_release(now);
}

void test_keyvalue(void)
{
    key_value_t src = {0};
    key_value_t dst = {0};
    key_value_t cmp = {0};
    const char key_a[] = "A";
    const char key_b[] = "B";
    const char value_x[] = "x";
    const char value_y[] = "y";

    src.Key = buffer_allocate(key_b, sizeof(key_b));
    src.Value = buffer_allocate(value_y, sizeof(value_y));
    dst.Key = buffer_allocate(key_a, sizeof(key_a));
    dst.Value = buffer_allocate(value_x, sizeof(value_x));
    cmp.Key = buffer_allocate(key_b, sizeof(key_b));
    cmp.Value = buffer_allocate(value_y, sizeof(value_y));

    assert(src.Key != NULL && src.Value != NULL);
    assert(dst.Key != NULL && dst.Value != NULL);
    assert(cmp.Key != NULL && cmp.Value != NULL);

    keyvalue_copy(&dst, &src);
    assert(keyvalue_is_equal(&dst, &src) == true);
    assert(keyvalue_is_equal(&dst, &cmp) == true);
    assert(keyvalue_is_greater(&dst, &src) == false);
    assert(keyvalue_is_less(&dst, &src) == false);

    /* Null safety checks: these must not crash and must return false for comparisons. */
    keyvalue_copy(NULL, &src);
    keyvalue_copy(&dst, NULL);
    assert(keyvalue_is_equal(NULL, &src) == false);
    assert(keyvalue_is_equal(&src, NULL) == false);
    assert(keyvalue_is_equal(NULL, NULL) == false);
    assert(keyvalue_is_greater(NULL, &src) == false);
    assert(keyvalue_is_less(&src, NULL) == false);

    buffer_free(&src.Key);
    buffer_free(&src.Value);
    buffer_free(&dst.Key);
    buffer_free(&dst.Value);
    buffer_free(&cmp.Key);
    buffer_free(&cmp.Value);
}

void test_variant(void)
{
    variant_t* v = NULL;
    variant_t* src = NULL;
    const char long_text[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    v = variant_allocate_default();
    assert(v != NULL);

    variant_set_char(v, 'A');
    assert(variant_get_data_type(v) == Char);
    assert(variant_get_char(v) == 'A');

    variant_set_bool(v, true);
    assert(variant_get_data_type(v) == Boolean);
    assert(variant_get_bool(v) == true);

    variant_set_string(v, "hello", 5);
    assert(variant_get_data_type(v) == String);
    assert(strcmp(variant_get_string(v), "hello") == 0);

    variant_set_string(v, long_text, strlen(long_text));
    assert(variant_get_data_type(v) == String);
    assert(variant_get_data_size(v) <= 255);
    assert(strlen(variant_get_string(v)) == variant_get_data_size(v));

    src = variant_allocate_unsigned_long(123456UL);
    assert(src != NULL);
    variant_set_variant(v, src);
    assert(variant_get_data_type(v) == UnsignedNumber);
    assert(variant_get_unsigned_long(v) == 123456UL);

    variant_set_variant(v, NULL);
    assert(variant_get_data_type(v) == UnsignedNumber);
    assert(variant_get_unsigned_long(v) == 123456UL);

    variant_release(src);
    variant_release(v);
}
