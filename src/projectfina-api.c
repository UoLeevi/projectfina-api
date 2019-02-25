#include "uo_conf.h"
#include "uo_http.h"
#include "uo_http_server.h"
#include "uo_jwt.h"
#include "uo_err.h"
#include "uo_prog.h"

#include <libpq-fe.h>

#include <stdio.h>

#define HTTP_400_JSON \
    "{ \"message\": \"bad request\" }"
#define HTTP_401_JSON \
    "{ \"message\": \"unauthorized\" }"
#define HTTP_500_JSON \
    "{ \"message\": \"server error\" }"

static char *conninfo;

static void http_response_with_400(
    uo_http_msg *http_response)
{
    uo_http_msg_set_status_line(http_response, UO_HTTP_400, UO_HTTP_1_1);
    uo_http_msg_set_content(http_response, HTTP_400_JSON, "application/json", UO_STRLEN(HTTP_400_JSON));
}

static void http_response_with_401(
    uo_http_msg *http_response)
{
    uo_http_msg_set_status_line(http_response, UO_HTTP_401, UO_HTTP_1_1);
    uo_http_msg_set_content(http_response, HTTP_401_JSON, "application/json", UO_STRLEN(HTTP_401_JSON));
}

static void http_response_with_500(
    uo_http_msg *http_response)
{
    uo_http_msg_set_status_line(http_response, UO_HTTP_500, UO_HTTP_1_1);
    uo_http_msg_set_content(http_response, HTTP_500_JSON, "application/json", UO_STRLEN(HTTP_500_JSON));
}

static void http_sess_get_groups(
    uo_http_sess *http_sess,
    const char *user_uuid)
{
    uo_http_msg *http_response = http_sess->http_response;
    PGconn *pgconn = uo_http_sess_get_user_data(http_sess);
    
    const char *paramValues_get_groups[1] = { user_uuid };

    PGresult *groups_res = PQexecParams(pgconn,
        "SELECT get_groups_for_user_as_json($1::uuid) json;",
        1, NULL, paramValues_get_groups, NULL, NULL, 0);

    if (PQresultStatus(groups_res) != PGRES_TUPLES_OK)
        http_response_with_500(http_response);
    else
    {
        char *json = PQgetvalue(groups_res, 0, 0);
        size_t json_len = PQgetlength(groups_res, 0, 0);

        uo_http_msg_set_status_line(http_response, UO_HTTP_200, UO_HTTP_1_1);
        uo_http_msg_set_content(http_response, json, "application/json", json_len);
    }

    PQclear(groups_res);
}

static void http_sess_get_watchlists(
    uo_http_sess *http_sess,
    const char *user_uuid)
{
    uo_http_msg *http_response = http_sess->http_response;
    PGconn *pgconn = uo_http_sess_get_user_data(http_sess);

    const char *paramValues_get_watchlists[1] = { user_uuid };

    PGresult *watchlists_res = PQexecParams(pgconn,
        "SELECT get_watchlists_for_user_as_json($1::uuid) json;",
        1, NULL, paramValues_get_watchlists, NULL, NULL, 0);

    if (PQresultStatus(watchlists_res) != PGRES_TUPLES_OK)
        http_response_with_500(http_response);
    else
    {
        char *json = PQgetvalue(watchlists_res, 0, 0);
        size_t json_len = PQgetlength(watchlists_res, 0, 0);

        uo_http_msg_set_status_line(http_response, UO_HTTP_200, UO_HTTP_1_1);
        uo_http_msg_set_content(http_response, json, "application/json", json_len);
    }

    PQclear(watchlists_res);
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
            char *jwt_payload = uo_jwt_decode_payload(NULL, jwt, strlen(jwt));
            if (!jwt_payload)
                goto response_401;

            char *jwt_claim_sub = uo_json_find_value(jwt_payload, "sub");
            if (!jwt_claim_sub)
                goto response_401;

            char user_uuid[37];
            memcpy(user_uuid, jwt_claim_sub + 1, 36);
            user_uuid[36] = '\0';

            switch (uo_http_request_get_method(http_request))
            {
                case UO_HTTP_GET:
                {
                    if (strcmp(uri, "/user/groups") == 0)
                        http_sess_get_groups(http_sess, user_uuid);
                    else if (strcmp(uri, "/user/watchlists") == 0)
                        http_sess_get_watchlists(http_sess, user_uuid);

                    break;
                }
            
                default:
                    http_response_with_400(http_response);
            }
        }
        else
response_401:
            http_response_with_401(http_response);
    }

    uo_cb_invoke(cb);
}

void http_server_after_open(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);

    PGconn *pgconn = PQconnectdb(conninfo);
    if (!pgconn)
        uo_http_sess_next_close(http_sess);

    uo_http_sess_set_user_data(http_sess, pgconn);

    uo_cb_invoke(cb);
}

void http_server_after_close(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);
    PGconn *pgconn = uo_http_sess_get_user_data(http_sess);
    PQfinish(pgconn);

    uo_cb_invoke(cb);
}


int main(
    int argc,
    char **argv)
{
    uo_http_init();

    uo_conf *conf = uo_conf_create("projectfina-api.conf");

    conninfo = uo_conf_get(conf, "pg.conninfo");

    const char *port = uo_conf_get(conf, "http_server.port");
    const char *root_dir = uo_conf_get(conf, "http_server.root_dir");

    uo_http_server *http_server = uo_http_server_create(port);

    uo_cb_append(http_server->evt_handlers.after_open, http_server_after_open);
    uo_cb_append(http_server->evt_handlers.before_send_msg, http_server_before_send_response);
    uo_cb_append(http_server->evt_handlers.after_close, http_server_after_close);

    if (!uo_http_server_set_opt_serve_static_files(http_server, root_dir))
        uo_err_exit("Error while setting root directory.");

    uo_http_server_start(http_server);

    uo_prog_init();
    uo_prog_wait_for_sigint();

    uo_http_server_destroy(http_server);

    uo_conf_destroy(conf);

    return 0;
}