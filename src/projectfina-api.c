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

static void http_res_with_400(
    uo_http_msg *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_400, UO_HTTP_VER_1_1);
    uo_http_res_set_content(http_res, HTTP_400_JSON, "application/json", UO_STRLEN(HTTP_400_JSON));
}

static void http_res_with_401(
    uo_http_msg *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_401, UO_HTTP_VER_1_1);
    uo_http_res_set_content(http_res, HTTP_401_JSON, "application/json", UO_STRLEN(HTTP_401_JSON));
}

static void http_res_with_500(
    uo_http_msg *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_500, UO_HTTP_VER_1_1);
    uo_http_res_set_content(http_res, HTTP_500_JSON, "application/json", UO_STRLEN(HTTP_500_JSON));
}

static PGconn *http_conn_get_pg_conn(
    uo_http_conn *http_conn)
{
    PGconn *pg_conn = uo_http_conn_get_user_data(http_conn, uo_nameof(pg_conn));

    if (!pg_conn)
    {
        char *pg_conninfo = uo_http_conn_get_user_data(http_conn, uo_nameof(pg_conninfo));
        pg_conn = PQconnectdb(pg_conninfo);
        uo_http_conn_set_user_data(http_conn, uo_nameof(pg_conn), pg_conn);
    }

    return pg_conn;
}

static void http_res_json_from_pg_res(
    uo_http_res *http_res,
    PGresult *pg_res)
{
    if (PQresultStatus(pg_res) != PGRES_TUPLES_OK)
        http_res_with_500(http_res);
    else
    {
        char *json = PQgetvalue(pg_res, 0, 0);
        size_t json_len = PQgetlength(pg_res, 0, 0);

        uo_http_res_set_status_line(http_res, UO_HTTP_200, UO_HTTP_VER_1_1);
        uo_http_res_set_content(http_res, json, "application/json", json_len);
    }

    PQclear(pg_res);
}

static void http_conn_get_groups(
    uo_http_conn *http_conn,
    const char *user_uuid)
{
    uo_http_msg *http_res = &http_conn->http_res;
    PGconn *pg_conn = http_conn_get_pg_conn(http_conn);

    if (PQstatus(pg_conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pg_conn));
        http_res_with_500(http_res);
        uo_http_conn_next_close(http_conn);
        return;
    }

    const char *paramValues_get_groups[1] = { user_uuid };

    PGresult *groups_res = PQexecParams(pg_conn,
        "SELECT get_groups_for_user_as_json($1::uuid) json;",
        1, NULL, paramValues_get_groups, NULL, NULL, 0);

    http_res_json_from_pg_res(http_res, groups_res);
}

static void http_conn_get_watchlists(
    uo_http_conn *http_conn,
    const char *user_uuid)
{
    uo_http_msg *http_res = &http_conn->http_res;
    PGconn *pg_conn = http_conn_get_pg_conn(http_conn);

    if (PQstatus(pg_conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pg_conn));
        http_res_with_500(http_res);
        uo_http_conn_next_close(http_conn);
        return;
    }

    const char *paramValues_get_watchlists[1] = { user_uuid };

    PGresult *watchlists_res = PQexecParams(pg_conn,
        "SELECT get_watchlists_for_user_as_json($1::uuid) json;",
        1, NULL, paramValues_get_watchlists, NULL, NULL, 0);

    http_res_json_from_pg_res(http_res, watchlists_res);
}

static void http_conn_create_note(
    uo_http_conn *http_conn,
    const char *user_uuid,
    const char *body)
{
    uo_http_msg *http_res = &http_conn->http_res;
    PGconn *pg_conn = http_conn_get_pg_conn(http_conn);

    if (PQstatus(pg_conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pg_conn));
        http_res_with_500(http_res);
        uo_http_conn_next_close(http_conn);
        return;
    }

    const char *paramValues_create_note[2] = { user_uuid, body };

    PGresult *notes_res = PQexecParams(pg_conn,
        "SELECT create_note($1::uuid, $2::text) note_uuid;",
        2, NULL, paramValues_create_note, NULL, NULL, 0);

    if (PQresultStatus(notes_res) != PGRES_TUPLES_OK || PQgetlength(notes_res, 0, 0) != 36)
        http_res_with_500(http_res);
    else
    {
        char *note_uuid = PQgetvalue(notes_res, 0, 0);

        uo_http_res_set_status_line(http_res, UO_HTTP_200, UO_HTTP_VER_1_1);
        char json[0x40];
        size_t json_len = sprintf(json, "{ \"note_uuid\": \"%s\" }", note_uuid);

        uo_http_res_set_content(http_res, json, "application/json", json_len);
    }

    PQclear(notes_res);
}

static bool http_req_get_user_uuid(
    uo_http_req *http_req,
    char user_uuid[37])
{
    char *hdr_authorization = uo_http_msg_get_header(http_req, "authorization");
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

static void http_req_handler_user(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_req = &http_conn->http_req;

    char buf[37];

    if (http_req_get_user_uuid(http_req, buf))
    {
        char *user_uuid = strdup(buf);
        uo_finstack_add(http_req->finstack, user_uuid, free);
        uo_http_conn_set_user_data(http_conn, uo_nameof(user_uuid), user_uuid);
    }
    else
        http_res_with_401(&http_conn->http_res);
    
    uo_cb_invoke(cb);
}

static void http_req_handler_get_user_groups(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    char *user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(user_uuid));
    http_conn_get_groups(http_conn, user_uuid);

    uo_cb_invoke(cb);
}

static void http_req_handler_get_user_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    char *user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(user_uuid));
    http_conn_get_watchlists(http_conn, user_uuid);

    uo_cb_invoke(cb);
}

static void http_req_handler_post_user_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    char *user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(user_uuid));
    char *note_body = uo_json_find_value(http_conn->http_req.body, "body");

    if (note_body)
    {
        char *note_body_end = uo_json_find_end(note_body);

        if (note_body_end && 
            (note_body_end = uo_json_decode_utf8(note_body, note_body, note_body_end - note_body)))
        {
            *note_body_end = '\0';
            http_conn_create_note(http_conn, user_uuid, note_body);
        }
        else
            http_res_with_400(&http_conn->http_res);
    }

    uo_cb_invoke(cb);
}

static void http_req_handler_get_markets(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_res = &http_conn->http_res;
    PGconn *pg_conn = http_conn_get_pg_conn(http_conn);

    if (PQstatus(pg_conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pg_conn));
        http_res_with_500(http_res);
        uo_http_conn_next_close(http_conn);
    }
    else
    {
        PGresult *instruments_res = PQexecParams(pg_conn,
            "SELECT get_markets_as_json() json;",
            0, NULL, NULL, NULL, NULL, 0);

        http_res_json_from_pg_res(http_res, instruments_res);
    }

    uo_cb_invoke(cb);
}

static void http_conn_get_instruments(
    uo_http_conn *http_conn,
    const char *mic)
{
    uo_http_msg *http_res = &http_conn->http_res;
    PGconn *pg_conn = http_conn_get_pg_conn(http_conn);

    if (PQstatus(pg_conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pg_conn));
        http_res_with_500(http_res);
        uo_http_conn_next_close(http_conn);
        return;
    }

    const char *paramValues_get_instruments[1] = { mic };

    PGresult *instruments_res = PQexecParams(pg_conn,
        "SELECT get_instruments_for_market_by_mic_as_json($1::text) json;",
        1, NULL, paramValues_get_instruments, NULL, NULL, 0);

    http_res_json_from_pg_res(http_res, instruments_res);
}

static void http_req_handler_get_markets_instruments(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    char *mic = uo_http_conn_get_req_data(http_conn, uo_nameof(mic));
    http_conn_get_instruments(http_conn, mic);

    uo_cb_invoke(cb);
}

static void http_server_after_recv_request(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_res = &http_conn->http_res;

    uo_http_msg_set_header(http_res, "server", "libuo http");
    uo_http_msg_set_header(http_res, "access-control-allow-origin", "*");

    uo_cb_invoke(cb);
}

static void http_server_after_close(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    PGconn *pg_conn = uo_http_conn_get_user_data(http_conn, uo_nameof(pg_conn));

    if (pg_conn)
        PQfinish(pg_conn);

    uo_cb_invoke(cb);
}

int main(
    int argc,
    char **argv)
{
    uo_http_init();

    uo_conf *conf = uo_conf_create("projectfina-api.conf");

    const char *pg_conninfo = uo_conf_get(conf, "pg.conninfo");
    const char *port        = uo_conf_get(conf, "http_server.port");
    const char *root_dir    = uo_conf_get(conf, "http_server.root_dir");

    uo_http_server *http_server = uo_http_server_create(port);

    uo_http_server_set_user_data(http_server, uo_nameof(pg_conninfo), pg_conninfo);

    // request handlers for /user/
    uo_http_server_add_req_handler(http_server, "GET /user/*", http_req_handler_user);
    uo_http_server_add_req_handler(http_server, "POST /user/*", http_req_handler_user);

    uo_http_server_add_req_handler(http_server, "GET /user/groups", http_req_handler_get_user_groups);
    uo_http_server_add_req_handler(http_server, "GET /user/watchlists", http_req_handler_get_user_watchlists);
    uo_http_server_add_req_handler(http_server, "POST /user/notes", http_req_handler_post_user_notes);

    //request handlers for /v01/markets/
    uo_http_server_add_req_handler(http_server, "GET /v01/markets", http_req_handler_get_markets);
    uo_http_server_add_req_handler(http_server, "GET /v01/markets/{mic}/instruments", http_req_handler_get_markets_instruments);

    uo_cb_append(http_server->evt_handlers.after_recv_msg, http_server_after_recv_request);
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