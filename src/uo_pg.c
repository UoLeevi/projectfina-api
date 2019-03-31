#include "uo_pg.h"
#include "uo_http_util.h"

#include <stdio.h>

void uo_pg_http_server_after_close(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_http_conn_get_user_data(http_conn, uo_nameof(pg_conn));

    if (pg_conn)
        PQfinish(pg_conn);

    uo_cb_invoke(cb);
}

PGconn *uo_pg_http_conn_get_pg_conn(
    uo_http_conn *http_conn)
{
    PGconn *pg_conn = uo_http_conn_get_user_data(http_conn, uo_nameof(pg_conn));

    if (!pg_conn)
    {
        char *pg_conninfo = uo_http_conn_get_user_data(http_conn, uo_nameof(pg_conninfo));
        pg_conn = PQconnectdb(pg_conninfo);
        uo_http_conn_set_user_data(http_conn, uo_nameof(pg_conn), pg_conn);
    }

    if (PQstatus(pg_conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(pg_conn));
        http_res_with_500(&http_conn->http_res);
        uo_http_conn_next_close(http_conn);
        return NULL;
    }

    return pg_conn;
}

void uo_pg_http_res_json_from_pg_res(
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
