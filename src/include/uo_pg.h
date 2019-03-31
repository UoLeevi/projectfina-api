#ifndef UO_PG_H
#define UO_PG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http.h"
#include "uo_http_server.h"

#include <libpq-fe.h>

void uo_pg_http_server_after_close(
    uo_cb *cb);

PGconn *uo_pg_http_conn_get_pg_conn(
    uo_http_conn *http_conn);

void uo_pg_http_res_json_from_pg_res(
    uo_http_res *http_res,
    PGresult *pg_res);

#ifdef __cplusplus
}
#endif

#endif
