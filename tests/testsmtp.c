#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "securitytypes.h"
#include "smtp.h"

static int env_to_int(const char* value, int fallback)
{
    if (value == NULL || value[0] == 0)
    {
        return fallback;
    }

    char* end = NULL;
    long parsed = strtol(value, &end, 10);
    if (end == value || *end != 0)
    {
        return fallback;
    }

    return (int)parsed;
}

static security_type_t env_to_security_type(const char* value)
{
    if (value == NULL || value[0] == 0)
    {
        return None;
    }

    if (strcmp(value, "ssl") == 0 || strcmp(value, "SSL") == 0)
    {
        return Ssl;
    }

    if (strcmp(value, "tls") == 0 || strcmp(value, "TLS") == 0)
    {
        return Tls;
    }

    return None;
}

int main(int argc, char* argv[])
{
    /* Offline safety checks (always run). */
    smtp_t* tmp = smtp_allocate();
    assert(tmp != NULL);
    assert(smtp_connect(NULL) == false);
    assert(smtp_disconnect(NULL) == false);
    smtp_start_tls(NULL);
    assert(smtp_need_tls(NULL) == false);
    assert(smtp_login(NULL) == false);
    assert(smtp_logout(NULL) == false);
    assert(smtp_sendmail_basic(NULL, NULL, NULL, NULL) == false);
    assert(smtp_is_connected(NULL) == false);
    smtp_set_account_information(NULL, NULL, 0, NULL, NULL, NULL, None);
    smtp_free(tmp);

    /* Optional real broker integration block.
     * Enabled only when SMTP_HOST is provided in environment. */
    const char* smtp_host = getenv("SMTP_HOST");
    const char* smtp_port_str = getenv("SMTP_PORT");
    const char* smtp_username = getenv("SMTP_USERNAME");
    const char* smtp_password = getenv("SMTP_PASSWORD");
    const char* smtp_from = getenv("SMTP_FROM");
    const char* smtp_to = getenv("SMTP_TO");
    const char* smtp_subject = getenv("SMTP_SUBJECT");
    const char* smtp_body = getenv("SMTP_BODY");
    const char* smtp_security = getenv("SMTP_SECURITY");
    const char* smtp_do_sendmail_str = getenv("SMTP_DO_SENDMAIL");

    if (smtp_host == NULL || smtp_host[0] == 0)
    {
        printf("SMTP integration skipped (set SMTP_HOST to enable)\n");
        return 0;
    }

    int smtp_port = env_to_int(smtp_port_str, 25);
    int smtp_do_sendmail = env_to_int(smtp_do_sendmail_str, 0);
    security_type_t sec_type = env_to_security_type(smtp_security);

    if (smtp_username == NULL || smtp_username[0] == 0 ||
        smtp_password == NULL || smtp_password[0] == 0 ||
        smtp_from == NULL || smtp_from[0] == 0)
    {
        fprintf(stderr, "SMTP integration config invalid: set SMTP_USERNAME, SMTP_PASSWORD, SMTP_FROM\n");
        return 2;
    }

    smtp_t* smtp = smtp_allocate();
    if(smtp == NULL)
    {
        printf("Failed to allocate SMTP instance\n");
        return 1;
    }

    smtp_set_account_information(smtp, smtp_host, (uint16_t)smtp_port, smtp_username, smtp_password, smtp_from, sec_type);

    /* Best effort; test continues even if this fails. */
    smtp_resolve_public_ip_address();

    if(!smtp_connect(smtp))
    {
        printf("Failed to connect\n");
        smtp_free(smtp);
        return 1;
    } 
    printf("Connected successfully\n");  

    if(!smtp_send_helo(smtp))
    {
        printf("Failed to send HELO\n");
        smtp_free(smtp);
        return 1;
    }
    
    if(smtp_need_tls(smtp))
    {
        printf("Server requires STARTTLS\n");
        if(!smtp_start_tls(smtp))
        {
            printf("Failed to start TLS\n");
        }
        else
        {
            printf("TLS started successfully\n");
        }
    }
    else
    {
        printf("Server does not require STARTTLS\n");
    }
    
    if(!smtp_login(smtp))
    {
        printf("Failed to login\n");
        smtp_free(smtp);
        return 1;
    }
    printf("Logged in successfully\n");

    if(smtp_do_sendmail != 0)
    {
        if (smtp_to == NULL || smtp_to[0] == 0)
        {
            fprintf(stderr, "SMTP_DO_SENDMAIL=1 requires SMTP_TO\n");
            smtp_free(smtp);
            return 3;
        }

        if (smtp_subject == NULL || smtp_subject[0] == 0)
        {
            smtp_subject = "TreOnzLib SMTP integration";
        }

        if (smtp_body == NULL || smtp_body[0] == 0)
        {
            smtp_body = "This is an automated SMTP integration test.";
        }

        if(!smtp_sendmail_basic(smtp, smtp_to, smtp_subject, smtp_body))
        {
            printf("Failed to send email\n");
            smtp_free(smtp);
            return 1;
        }
        printf("Email sent successfully\n");
    }

    if(!smtp_logout(smtp))
    {
        printf("Failed to logout\n");
        smtp_free(smtp);
        return 1;
    }
    printf("Logged out successfully\n");

    if(!smtp_disconnect(smtp))
    {
        printf("Failed to disconnect\n");
        smtp_free(smtp);
        return 1;
    }
    printf("Disconnected successfully\n");  


    smtp_free(smtp);

    return 0;
}