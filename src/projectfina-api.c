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

    if (!pgconn)
    {
        pgconn = PQconnectdb(conninfo);
        uo_http_sess_set_user_data(http_sess, pgconn);
    }

    if (PQstatus(pgconn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pgconn));
        http_response_with_500(http_response);
        uo_http_sess_next_close(http_sess);
        return;
    }

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

    if (!pgconn)
    {
        pgconn = PQconnectdb(conninfo);
        uo_http_sess_set_user_data(http_sess, pgconn);
    }

    if (PQstatus(pgconn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pgconn));
        http_response_with_500(http_response);
        uo_http_sess_next_close(http_sess);
        return;
    }

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

static bool http_request_get_user_uuid(
    uo_http_request *http_request,
    char user_uuid[37])
{
    char *hdr_authorization = uo_http_msg_get_header(http_request, "authorization");
    char jwt[0x400];

    if (!hdr_authorization || sscanf(hdr_authorization, "Bearer %1023s", jwt) != 1)
        return false;

    char *jwt_payload = uo_jwt_decode_payload(NULL, jwt, strlen(jwt));
    if (!jwt_payload)
        return false;

    char *jwt_claim_sub = uo_json_find_value(jwt_payload, "sub");
    if (!jwt_claim_sub)
        return false;

    memcpy(user_uuid, jwt_claim_sub + 1, 36);
    user_uuid[36] = '\0';

    return true;
}

static void http_response_handler_get_user_groups(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_request = http_sess->http_request;
    uo_http_msg *http_response = http_sess->http_response;

    char user_uuid[37];

    if (http_request_get_user_uuid(http_request, user_uuid))
        http_sess_get_groups(http_sess, user_uuid);
    else
        http_response_with_401(http_response);
    
    uo_cb_invoke(cb);
}

static void http_response_handler_get_user_watchlists(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_request = http_sess->http_request;
    uo_http_msg *http_response = http_sess->http_response;

    char user_uuid[37];

    if (http_request_get_user_uuid(http_request, user_uuid))
        http_sess_get_watchlists(http_sess, user_uuid);
    else
        http_response_with_401(http_response);

    uo_cb_invoke(cb);
}

static void http_server_after_recv_request(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_response = http_sess->http_response;

    uo_http_msg_set_header(http_response, "server", "libuo http");
    uo_http_msg_set_header(http_response, "access-control-allow-origin", "*");

    uo_cb_invoke(cb);
}

static void http_server_after_close(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);

    PGconn *pgconn = uo_http_sess_get_user_data(http_sess);

    if (pgconn)
        PQfinish(pgconn);

    uo_cb_invoke(cb);
}

int main(
    int argc,
    char **argv)
{
    uo_http_init();

    uo_conf *conf = uo_conf_create("projectfina-api.conf");

    conninfo             = uo_conf_get(conf, "pg.conninfo");
    const char *port     = uo_conf_get(conf, "http_server.port");
    const char *root_dir = uo_conf_get(conf, "http_server.root_dir");

    uo_http_server *http_server = uo_http_server_create(port);

    uo_cb *cb_get_user_groups = uo_cb_create();
    uo_cb *cb_get_user_watchlists = uo_cb_create();

    uo_cb_append(cb_get_user_groups, http_response_handler_get_user_groups);
    uo_cb_append(cb_get_user_watchlists, http_response_handler_get_user_watchlists);

    uo_http_server_add_request_handler(http_server, UO_HTTP_GET, "/user/groups", cb_get_user_groups);
    uo_http_server_add_request_handler(http_server, UO_HTTP_GET, "/user/watchlists", cb_get_user_watchlists);

    uo_cb_append(http_server->evt_handlers.after_recv_msg, http_server_after_recv_request);
    uo_cb_append(http_server->evt_handlers.after_close, http_server_after_close);

    if (!uo_http_server_set_opt_serve_static_files(http_server, root_dir))
        uo_err_exit("Error while setting root directory.");

    uo_http_server_start(http_server);

    uo_prog_init();
    uo_prog_wait_for_sigint();

    uo_http_server_destroy(http_server);

    uo_cb_destroy(cb_get_user_groups);
    uo_cb_destroy(cb_get_user_watchlists);

    uo_conf_destroy(conf);

    return 0;
}