#include "v01/watchlists.h"
#include "uo_http_util.h"
#include "uo_pg.h"

// GET /v01/watchlists/
void v01_get_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));

        PGresult *watchlists_res = PQexecParams(pg_conn,
            "SELECT get_watchlists_for_user_as_json($1::uuid) json;",
            1, NULL, (const char *[1]) { jwt_user_uuid }, NULL, NULL, 0);

        uo_pg_http_res_json_from_pg_res(&http_conn->http_res, watchlists_res);
    }

    uo_cb_invoke(cb);
}

// GET /v01/watchlists/{watchlist_uuid}/instruments/
void v01_get_watchlists_instruments(
    uo_cb *cb);

// GET /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/
void v01_get_watchlists_instruments_notes(
    uo_cb *cb);

// POST /v01/watchlists/
void v01_post_watchlists(
    uo_cb *cb);

// PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/
void v01_put_watchlists_instruments(
    uo_cb *cb);

// PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_put_watchlists_instruments_notes(
    uo_cb *cb);

// DELETE /v01/watchlists/{watchlist_uuid}/
void v01_delete_watchlists(
    uo_cb *cb);

// DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/
void v01_delete_watchlists_instruments(
    uo_cb *cb);

// DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_delete_watchlists_instruments_notes(
    uo_cb *cb);