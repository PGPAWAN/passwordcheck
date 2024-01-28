/* -------------------------------------------------------------------------
 *
 * auth_delay.c
 *
 * Copyright (c) 2010-2024, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *      contrib/auth_delay/auth_delay.c
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"

#include <limits.h>
#include "libpq/auth.h"
#include "port.h"
#include "utils/guc.h"
#include "utils/timestamp.h"

PG_MODULE_MAGIC;

/* GUC Variables */
static int auth_delay_milliseconds = 0;
static char *api_url = "http://example.com/webhook";  // Specify your API URL

/* Original Hook */
static ClientAuthentication_hook_type original_client_auth_hook = NULL;

/*
 * Check authentication
 */
static void
auth_delay_checks(Port *port, int status)
{
    /*
     * Any other plugins which use ClientAuthentication_hook.
     */
    if (original_client_auth_hook)
        original_client_auth_hook(port, status);

    /*
     * Inject a short delay if authentication failed.
     */
    if (status != STATUS_OK)
    {
        pg_usleep(1000L * auth_delay_milliseconds);

        /* Call an external API using curl with username and client IP as parameters */
        char curl_command[1024];
        snprintf(curl_command, sizeof(curl_command), "curl -X POST %s -d 'username=%s&client_ip=%s'",
                 api_url, port->user_name, port->remote_host);
        system(curl_command);
    }
}

/*
 * Module Load Callback
 */
void
_PG_init(void)
{
    /* Define custom GUC variables */
    DefineCustomIntVariable("auth_delay.milliseconds",
                            "Milliseconds to delay before reporting authentication failure",
                            NULL,
                            &auth_delay_milliseconds,
                            0,
                            0, INT_MAX / 1000,
                            PGC_SIGHUP,
                            GUC_UNIT_MS,
                            NULL,
                            NULL,
                            NULL);

    DefineCustomStringVariable("auth_delay.api_url",
                               "API URL to call on authentication failure",
                               NULL,
                               &api_url,
                               "http://example.com/webhook",
                               PGC_SIGHUP,
                               0,
                               NULL,
                               NULL,
                               NULL);

    MarkGUCPrefixReserved("auth_delay");

    /* Install Hooks */
    original_client_auth_hook = ClientAuthentication_hook;
    ClientAuthentication_hook = auth_delay_checks;
}
