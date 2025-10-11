#include <stdio.h>
#include <malloc.h>
#include "securitytypes.h"
#include "smtp.h"

int main(int argc, char* argv[])
{
    //Before we start a SMTP session we need to resolve our owm public IP Address or FQDN
    smtp_resolve_public_ip_address();

    smtp_t* smtp = smtp_allocate();
    if(smtp == NULL)
    {
        printf("Failed to allocate SMTP instance\n");
        return 1;
    }

    const char* host = "sandbox.smtp.mailtrap.io";
    uint16_t port = 2525;
    const char* username = "c0b55e0c9f5568";
    const char* password = "05a61dc37c019c";
    security_type_t secType = None;

    smtp_set_account_information(smtp, host, port, username, password, "sriiixi@gmail.com", secType);

    if(!smtp_connect(smtp))
    {
        printf("Failed to connect: %s\n", smtp_get_error(smtp));
        smtp_free(smtp);
        return 1;
    } 
    printf("Connected successfully\n");  

    if(!smtp_send_helo(smtp))
    {
        printf("Failed to send HELO: %s\n", smtp_get_error(smtp));
        smtp_free(smtp);
        return 1;
    }
    
    if(smtp_need_tls(smtp))
    {
        printf("Server requires STARTTLS\n");
        if(!smtp_start_tls(smtp))
        {
            printf("Failed to start TLS: %s\n", smtp_get_error(smtp));  
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
        printf("Failed to login: %s\n", smtp_get_error(smtp));
        smtp_free(smtp);
        return 1;
    }
    printf("Logged in successfully\n");

    // Here you would typically prepare a mail_t object and send an email
    // For this example, we will skip that part

    if(!smtp_sendmail_basic(smtp, "iot.edge.2021@gmail.com", "Test Subject", "This is a test email body."))
    {
        printf("Failed to send email: %s\n", smtp_get_error(smtp));
        smtp_free(smtp);
        return 1;
    }
    printf("Email sent successfully\n");
    

    if(!smtp_logout(smtp))
    {
        printf("Failed to logout: %s\n", smtp_get_error(smtp));
        smtp_free(smtp);
        return 1;
    }
    printf("Logged out successfully\n");

    if(!smtp_disconnect(smtp))
    {
        printf("Failed to disconnect: %s\n", smtp_get_error(smtp));
        smtp_free(smtp);
        return 1;
    }
    printf("Disconnected successfully\n");  


    smtp_free(smtp);

    return 0;
}