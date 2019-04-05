#include "v01/notes.h"
#include "uo_http_util.h"
#include "uo_pg.h"

// GET /v01/notes/
void v01_get_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));

        PGresult *notes_res = PQexecParams(pg_conn,
            "SELECT get_notes_created_by_user_uuid_as_json($1::uuid) json;",
            1, NULL, (const char *[1]) { jwt_user_uuid }, NULL, NULL, 0);

        uo_pg_http_res_json_from_pg_res(&http_conn->http_res, notes_res);
    }

    uo_cb_invoke(cb);
}

// POST /v01/notes/
void v01_post_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    char *body = uo_http_conn_get_req_data(http_conn, uo_nameof(body));

    if (body)
    {
        PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

        if (pg_conn)
        {
            char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));

            PGresult *notes_res = PQexecParams(pg_conn,
                "SELECT create_note($1::uuid, $2::text) note_uuid;",
                2, NULL, (const char *[2]) { jwt_user_uuid, body }, NULL, NULL, 0);

            if (PQresultStatus(notes_res) != PGRES_TUPLES_OK || PQgetlength(notes_res, 0, 0) != 36)
                http_res_with_500(&http_conn->http_res);
            else
            {
                char *note_uuid = PQgetvalue(notes_res, 0, 0);

                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_201, UO_HTTP_VER_1_1);
                char json[0x40];
                size_t json_len = sprintf(json, "{ \"note_uuid\": \"%s\" }", note_uuid);
                uo_http_res_set_content(&http_conn->http_res, json, "application/json", json_len);
            }

            PQclear(notes_res);
        }
    }
    else
        http_res_with_400(&http_conn->http_res);

    uo_cb_invoke(cb);
}

// DELETE /v01/notes/{note_uuid}/
void v01_delete_notes(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *note_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(note_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS(SELECT 1 FROM notes WHERE uuid = $1::uuid AND created_by_user_uuid = $2::uuid);",
            2, NULL, (const char *[2]) { note_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *delete_note_res = PQexecParams(pg_conn,
                "SELECT delete_note($1::uuid);",
                1, NULL, (const char *[1]) { note_uuid }, NULL, NULL, 0);

            if (PQresultStatus(delete_note_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(delete_note_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}
