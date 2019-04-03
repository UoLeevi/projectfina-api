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
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *watchlist_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(watchlist_uuid));

        PGresult *member_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM get_watchlists_for_user($1::uuid)"
                " WHERE watchlist_uuid = $2::uuid"
            ");",
            2, NULL, (const char *[2]) { jwt_user_uuid, watchlist_uuid }, NULL, NULL, 0);

        if (PQresultStatus(member_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(member_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *instruments_res = PQexecParams(pg_conn,
                "SELECT get_instrument_uuids_for_watchlist_as_json($1::uuid) json;",
                2, NULL, (const char *[1]) { watchlist_uuid }, NULL, NULL, 0);

            uo_pg_http_res_json_from_pg_res(&http_conn->http_res, instruments_res);
        }

        PQclear(member_res);
    }

    uo_cb_invoke(cb);
}

// GET /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/
void v01_get_watchlists_instruments_notes(
    uo_cb *cb);

// POST /v01/watchlists/
void v01_post_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    char *name = uo_http_conn_get_req_data(http_conn, uo_nameof(name));

    if (name)
    {
        PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

        if (pg_conn)
        {
            char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));

            PGresult *watchlists_res = PQexecParams(pg_conn,
                "SELECT create_watchlist($1::uuid, $2::text) watchlist_uuid;",
                2, NULL, (const char *[2]) { jwt_user_uuid, name }, NULL, NULL, 0);

            if (PQresultStatus(watchlists_res) != PGRES_TUPLES_OK || PQgetlength(watchlists_res, 0, 0) != 36)
                http_res_with_500(&http_conn->http_res);
            else
            {
                char *watchlist_uuid = PQgetvalue(watchlists_res, 0, 0);

                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_201, UO_HTTP_VER_1_1);
                char json[0x45];
                size_t json_len = sprintf(json, "{ \"watchlist_uuid\": \"%s\" }", watchlist_uuid);
                uo_http_res_set_content(&http_conn->http_res, json, "application/json", json_len);
            }

            PQclear(watchlists_res);
        }
    }
    else
        http_res_with_400(&http_conn->http_res);

    uo_cb_invoke(cb);
}

// PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/
void v01_put_watchlists_instruments(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *watchlist_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(watchlist_uuid));
        char *instrument_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(instrument_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM get_watchlists_for_user($1::uuid)"
                " WHERE watchlist_uuid = $2::uuid"
                " AND user_is_owner"
            ");",
            2, NULL, (const char *[2]) { watchlist_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *add_instrument_res = PQexecParams(pg_conn,
                "SELECT add_instrument_to_watchlist($1::uuid, $2::uuid);",
                2, NULL, (const char *[2]) { instrument_uuid, watchlist_uuid }, NULL, NULL, 0);

            if (PQresultStatus(add_instrument_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(add_instrument_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_put_watchlists_instruments_notes(
    uo_cb *cb);

// DELETE /v01/watchlists/{watchlist_uuid}/
void v01_delete_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *watchlist_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(watchlist_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM get_watchlists_for_user($1::uuid)"
                " WHERE watchlist_uuid = $2::uuid"
                " AND user_is_owner"
            ");",
            2, NULL, (const char *[2]) { watchlist_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *delete_res = PQexecParams(pg_conn,
                "SELECT delete_watchlist($1::uuid);",
                1, NULL, (const char *[1]) { watchlist_uuid }, NULL, NULL, 0);

            if (PQresultStatus(delete_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(delete_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/
void v01_delete_watchlists_instruments(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *watchlist_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(watchlist_uuid));
        char *instrument_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(instrument_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM get_watchlists_for_user($1::uuid)"
                " WHERE watchlist_uuid = $2::uuid"
                " AND user_is_owner"
            ");",
            2, NULL, (const char *[2]) { watchlist_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *remove_instrument_res = PQexecParams(pg_conn,
                "SELECT remove_instrument_from_watchlist($1::uuid, $2::uuid);",
                2, NULL, (const char *[2]) { instrument_uuid, watchlist_uuid }, NULL, NULL, 0);

            if (PQresultStatus(remove_instrument_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(remove_instrument_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_delete_watchlists_instruments_notes(
    uo_cb *cb);