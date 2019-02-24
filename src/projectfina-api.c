#include "uo_conf.h"
#include "uo_http.h"
#include "uo_http_server.h"
#include "uo_jwt.h"
#include "uo_base64.h"
#include "uo_err.h"
#include "uo_prog.h"

#include <libpq-fe.h>

#include <stdio.h>

#define HTTP_401_JSON \
    "{ \"message\": \"unauthorized\" }"

static PGconn *pgconn;

static void http_response_with_401(
    uo_http_msg *http_response)
{
    uo_http_msg_set_status_line(http_response, UO_HTTP_401, UO_HTTP_1_1);
    uo_http_msg_set_content(http_response, HTTP_401_JSON, "application/json", UO_STRLEN(HTTP_401_JSON));
}

void http_server_before_send_response(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_request = http_sess->http_request;
    uo_http_msg *http_response = http_sess->http_response;

    const char *uri = uo_http_request_get_uri(http_request);

    uo_http_msg_set_header(http_response, "server", "libuo http");
    uo_http_msg_set_header(http_response, "access-control-allow-origin", "*");

    if (strncmp(uri, "/user/", UO_STRLEN("/user/")) == 0)
    {
        char *hdr_authorization = uo_http_msg_get_header(http_request, "authorization");
        char jwt[0x400];

        if (hdr_authorization && sscanf(hdr_authorization, "Bearer %1023s", jwt) == 1)
        {
            char *jwt_payload = strchr(jwt, '.') + 1;
            char *jwt_payload_end = strchr(jwt_payload, '.');

            jwt_payload_end = uo_base64url_decode(jwt_payload, jwt_payload, jwt_payload_end - jwt_payload);
            *jwt_payload_end = '\0';

            if (strcmp(uri, "/user/groups") == 0)
            {
                if (uo_http_request_get_method(http_request) == UO_HTTP_GET)
                {
                }
            }
        }
        else
            http_response_with_401(http_response);
    }

    uo_cb_invoke(cb);
}

int main(
    int argc,
    char **argv)
{
    uo_http_init();

    uo_conf *conf = uo_conf_create("projectfina-api.conf");

    const char *conninfo = uo_conf_get(conf, "pg.conninfo");

    pgconn = PQconnectdb(conninfo);
    if (!pgconn)
        uo_err_exit("Unable to connect to database.");

    const char *port = uo_conf_get(conf, "http_server.port");
    const char *root_dir = uo_conf_get(conf, "http_server.root_dir");

    uo_http_server *http_server = uo_http_server_create(port);

    uo_cb_append(http_server->evt_handlers.before_send_msg, http_server_before_send_response);

    if (!uo_http_server_set_opt_serve_static_files(http_server, root_dir))
        uo_err_exit("Error while setting root directory.");

    uo_http_server_start(http_server);

    uo_prog_init();
    uo_prog_wait_for_sigint();

    uo_http_server_destroy(http_server);

    PQfinish(pgconn);

    uo_conf_destroy(conf);

    return 0;
}