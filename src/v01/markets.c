#include "v01/markets.h"
#include "uo_http_util.h"
#include "uo_pg.h"

// GET /v01/markets/
void v01_get_markets(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        PGresult *markets_res = PQexecParams(pg_conn,
            "SELECT get_markets_as_json() json;",
            0, NULL, NULL, NULL, NULL, 0);

        uo_pg_http_res_json_from_pg_res(&http_conn->http_res, markets_res);
    }

    uo_cb_invoke(cb);
}

// GET /v01/markets/{market_uuid}/instruments/
void v01_get_markets_instruments(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *mic = uo_http_conn_get_req_data(http_conn, uo_nameof(mic));

        PGresult *instruments_res = PQexecParams(pg_conn,
            "SELECT get_instruments_for_market_by_mic_as_json($1::text) json;",
            1, NULL, (const char *[1]) { mic }, NULL, NULL, 0);

        uo_pg_http_res_json_from_pg_res(&http_conn->http_res, instruments_res);
    }

    uo_cb_invoke(cb);
}
