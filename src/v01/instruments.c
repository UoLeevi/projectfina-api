#include "v01/watchlists.h"
#include "uo_http_util.h"
#include "uo_pg.h"

// GET /v01/instruments/{instrument_uuid}/
void v01_get_instruments(
    uo_cb *cb);

// GET /v01/instruments/{instrument_uuid}/notes/
void v01_get_instruments_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *instrument_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(instrument_uuid));

        PGresult *notes_res = PQexecParams(pg_conn,
            "SELECT get_notes_for_instrument_by_user_uuid_as_json($1::uuid, $2::uuid) json;",
            2, NULL, (const char *[2]) { instrument_uuid, jwt_user_uuid }, NULL, NULL, 0);

        uo_pg_http_res_json_from_pg_res(&http_conn->http_res, notes_res);
    }

    uo_cb_invoke(cb);
}

// PUT /v01/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_put_instruments_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
    char *note_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(note_uuid));
    char *instrument_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(instrument_uuid));

    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS(SELECT 1 FROM notes WHERE uuid = $1::uuid AND created_by_user_uuid = $2::uuid);",
            2, NULL, (const char *[2]) { note_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *add_note_res = PQexecParams(pg_conn,
                "SELECT add_note_to_instrument($1::uuid, $2::uuid) note_x_instrument_uuid;",
                2, NULL, (const char *[2]) { note_uuid, instrument_uuid }, NULL, NULL, 0);

            if (PQresultStatus(add_note_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(add_note_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// DELETE /v01/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_delete_instruments_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
    char *note_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(note_uuid));
    char *instrument_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(instrument_uuid));

    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS(SELECT 1 FROM notes WHERE uuid = $1::uuid AND created_by_user_uuid = $2::uuid);",
            2, NULL, (const char *[2]) { note_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *remove_note_res = PQexecParams(pg_conn,
                "SELECT remove_note_from_instrument($1::uuid, $2::uuid) note_x_instrument_uuid;",
                2, NULL, (const char *[2]) { note_uuid, instrument_uuid }, NULL, NULL, 0);

            if (PQresultStatus(remove_note_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(remove_note_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}
