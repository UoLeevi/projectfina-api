#include "uo_conf.h"
#include "uo_http.h"
#include "uo_http_server.h"
#include "uo_err.h"
#include "uo_prog.h"

#include "uo_http_util.h"
#include "uo_pg.h"

#include "v01/groups.h"
#include "v01/instruments.h"
#include "v01/markets.h"
#include "v01/notes.h"
#include "v01/watchlists.h"

#include <stdio.h>

static void http_server_after_recv_request(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_res = &http_conn->http_res;

    uo_http_msg_set_header(http_res, "server", "libuo http");
    uo_http_msg_set_header(http_res, "access-control-allow-origin", "*");

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

    // markets
    uo_http_server_add_req_handler(http_server, "GET /v01/markets/", v01_get_markets);
    uo_http_server_add_req_handler(http_server, "GET /v01/markets/{market_uuid}/instruments/", v01_get_markets_instruments);

    // instruments
    uo_http_server_add_req_handler(http_server, "GET /v01/instruments/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "PUT /v01/instruments/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "DELETE /v01/instruments/*", uo_http_req_handler_parse_user_uuid_from_jwt);

    //uo_http_server_add_req_handler(http_server, "GET /v01/instruments/{instrument_uuid}/", v01_get_instruments);
    uo_http_server_add_req_handler(http_server, "GET /v01/instruments/{instrument_uuid}/notes/", v01_get_instruments_notes);
    uo_http_server_add_req_handler(http_server, "PUT /v01/instruments/{instrument_uuid}/notes/{note_uuid}/", v01_put_instruments_notes);
    uo_http_server_add_req_handler(http_server, "DELETE /v01/instruments/{instrument_uuid}/notes/{note_uuid}/", v01_delete_instruments_notes);

    // notes
    uo_http_server_add_req_handler(http_server, "GET /v01/notes/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "POST /v01/notes/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "DELETE /v01/notes/*", uo_http_req_handler_parse_user_uuid_from_jwt);

    //uo_http_server_add_req_handler(http_server, "GET /v01/notes/", v01_get_notes);
    uo_http_server_add_req_handler(http_server, "POST /v01/notes/", v01_post_notes);
    uo_http_server_add_req_handler(http_server, "DELETE /v01/notes/{note_uuid}/", v01_delete_notes);

    // watchlists
    uo_http_server_add_req_handler(http_server, "GET /v01/watchlists/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "POST /v01/watchlists/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "PUT /v01/watchlists/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "DELETE /v01/watchlists/*", uo_http_req_handler_parse_user_uuid_from_jwt);

    uo_http_server_add_req_handler(http_server, "GET /v01/watchlists/", v01_get_watchlists);
    uo_http_server_add_req_handler(http_server, "GET /v01/watchlists/{watchlist_uuid}/instruments/", v01_get_watchlists_instruments);
    //uo_http_server_add_req_handler(http_server, "GET /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/", v01_get_watchlists_instruments_notes);
    uo_http_server_add_req_handler(http_server, "POST /v01/watchlists/", v01_post_watchlists);
    //uo_http_server_add_req_handler(http_server, "PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/", v01_put_watchlists_instruments);
    //uo_http_server_add_req_handler(http_server, "PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/", v01_put_watchlists_instruments_notes);
    //uo_http_server_add_req_handler(http_server, "DELETE /v01/watchlists/{watchlist_uuid}/", v01_delete_watchlists);
    //uo_http_server_add_req_handler(http_server, "DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/", v01_delete_watchlists_instruments);
    //uo_http_server_add_req_handler(http_server, "DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/", v01_delete_watchlists_instruments_notes);

    // groups
    uo_http_server_add_req_handler(http_server, "GET /v01/groups/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "POST /v01/groups/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "PUT /v01/groups/*", uo_http_req_handler_parse_user_uuid_from_jwt);
    uo_http_server_add_req_handler(http_server, "DELETE /v01/groups/*", uo_http_req_handler_parse_user_uuid_from_jwt);

    uo_http_server_add_req_handler(http_server, "GET /v01/groups/", v01_get_groups);
    uo_http_server_add_req_handler(http_server, "GET /v01/groups/{group_uuid}/users/", v01_get_groups_users);
    uo_http_server_add_req_handler(http_server, "GET /v01/groups/{group_uuid}/watchlists/", v01_get_groups_watchlists);
    uo_http_server_add_req_handler(http_server, "POST /v01/groups/", v01_post_groups);
    //uo_http_server_add_req_handler(http_server, "PUT /v01/groups/{group_uuid}/users/{user_uuid}/", v01_put_groups_users);
    //uo_http_server_add_req_handler(http_server, "PUT /v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/", v01_put_groups_watchlists);
    //uo_http_server_add_req_handler(http_server, "DELETE /v01/groups/{group_uuid}/", v01_delete_groups);
    //uo_http_server_add_req_handler(http_server, "DELETE /v01/groups/{group_uuid}/users/{user_uuid}/", v01_delete_groups_users);
    //uo_http_server_add_req_handler(http_server, "DELETE /v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/", v01_delete_groups_watchlists);

    uo_cb_append(http_server->evt_handlers.after_recv_msg, http_server_after_recv_request);
    uo_cb_append(http_server->evt_handlers.after_close, uo_pg_http_server_after_close);

    if (!uo_http_server_set_opt_serve_static_files(http_server, root_dir, 0xA00000))
        uo_err_exit("Error while setting root directory.");

    uo_http_server_start(http_server);

    uo_prog_init();
    uo_prog_wait_for_sigint();

    uo_http_server_destroy(http_server);

    uo_conf_destroy(conf);

    return 0;
}
