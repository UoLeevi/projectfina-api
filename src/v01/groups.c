#include "v01/groups.h"
#include "uo_http_util.h"
#include "uo_pg.h"

// GET /v01/groups/
void v01_get_groups(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));

        PGresult *groups_res = PQexecParams(pg_conn,
            "SELECT get_groups_for_user_as_json($1::uuid) json;",
            1, NULL, (const char *[1]) { jwt_user_uuid }, NULL, NULL, 0);

        uo_pg_http_res_json_from_pg_res(&http_conn->http_res, groups_res);
    }

    uo_cb_invoke(cb);
}

// GET /v01/groups/{group_uuid}/users/
void v01_get_groups_users(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(note_uuid));

        PGresult *member_res = PQexecParams(pg_conn,
            "SELECT EXISTS(SELECT 1 FROM users_x_groups WHERE user_uuid = $1::uuid AND group_uuid = $2::uuid);",
            2, NULL, (const char *[2]) { jwt_user_uuid, group_uuid }, NULL, NULL, 0);

        if (PQresultStatus(member_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(member_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *users_res = PQexecParams(pg_conn,
                "SELECT get_group_users_as_json($1::uuid) json;",
                2, NULL, (const char *[1]) { group_uuid }, NULL, NULL, 0);

            uo_pg_http_res_json_from_pg_res(&http_conn->http_res, users_res);
        }

        PQclear(member_res);
    }

    uo_cb_invoke(cb);
}

// GET /v01/groups/{group_uuid}/watchlists/
void v01_get_groups_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(note_uuid));

        PGresult *member_res = PQexecParams(pg_conn,
            "SELECT EXISTS(SELECT 1 FROM users_x_groups WHERE user_uuid = $1::uuid AND group_uuid = $2::uuid);",
            2, NULL, (const char *[2]) { jwt_user_uuid, group_uuid }, NULL, NULL, 0);

        if (PQresultStatus(member_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(member_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *watchlists_res = PQexecParams(pg_conn,
                "SELECT get_watchlists_for_group_as_json($1::uuid) json;",
                2, NULL, (const char *[1]) { group_uuid }, NULL, NULL, 0);

            uo_pg_http_res_json_from_pg_res(&http_conn->http_res, watchlists_res);
        }

        PQclear(member_res);
    }

    uo_cb_invoke(cb);
}

// POST /v01/groups/
void v01_post_groups(
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

            PGresult *groups_res = PQexecParams(pg_conn,
                "SELECT create_group($1::uuid, $2::text) group_uuid;",
                2, NULL, (const char *[2]) { jwt_user_uuid, name }, NULL, NULL, 0);

            if (PQresultStatus(groups_res) != PGRES_TUPLES_OK || PQgetlength(groups_res, 0, 0) != 36)
                http_res_with_500(&http_conn->http_res);
            else
            {
                char *group_uuid = PQgetvalue(groups_res, 0, 0);

                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_201, UO_HTTP_VER_1_1);
                char json[0x41];
                size_t json_len = sprintf(json, "{ \"group_uuid\": \"%s\" }", group_uuid);
                uo_http_res_set_content(&http_conn->http_res, json, "application/json", json_len);
            }

            PQclear(groups_res);
        }
    }
    else
        http_res_with_400(&http_conn->http_res);

    uo_cb_invoke(cb);
}

// PUT /v01/groups/{group_uuid}/users/{user_uuid}/
void v01_put_groups_users(
    uo_cb *cb);

// PUT /v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/
void v01_put_groups_watchlists(
    uo_cb *cb);

// DELETE /v01/groups/{group_uuid}/
void v01_delete_groups(
    uo_cb *cb);

// DELETE /v01/groups/{group_uuid}/users/{user_uuid}/
void v01_delete_groups_users(
    uo_cb *cb);

// DELETE /v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/
void v01_delete_groups_watchlists(
    uo_cb *cb);

