#include "v01/groups.h"
#include "uo_http_util.h"
#include "uo_pg.h"

// GET /user/v01/groups/
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

// GET /user/v01/groups/{group_uuid}/users/
void v01_get_groups_users(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));

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

// GET /user/v01/groups/{group_uuid}/watchlists/
void v01_get_groups_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));

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

// POST /user/v01/groups/
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

// PUT /user/v01/groups/{group_uuid}/users/{user_uuid}/
void v01_put_groups_users(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));
        char *user_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(user_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM users_x_groups u_x_g"
                " WHERE u_x_g.group_uuid = $1::uuid"
                " AND u_x_g.user_uuid = $2::uuid"
                " AND u_x_g.is_owner"
            ");",
            2, NULL, (const char *[2]) { group_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *add_user_res = PQexecParams(pg_conn,
                "SELECT add_user_to_group($1::uuid, $2::uuid);",
                2, NULL, (const char *[2]) { user_uuid, group_uuid }, NULL, NULL, 0);

            if (PQresultStatus(add_user_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(add_user_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// PUT /user/v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/
void v01_put_groups_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *watchlist_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(watchlist_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM get_watchlists_for_user($1::uuid)"
                " WHERE watchlist_uuid = $2::uuid"
                " AND user_is_owner"
            ");",
            2, NULL, (const char *[2]) { jwt_user_uuid, watchlist_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PQclear(owner_res);
            owner_res = PQexecParams(pg_conn,
                "SELECT EXISTS("
                    "SELECT 1 FROM users_x_groups u_x_g"
                    " WHERE u_x_g.group_uuid = $1::uuid"
                    " AND u_x_g.user_uuid = $2::uuid"
                    " AND u_x_g.is_owner"
                ");",
                2, NULL, (const char *[2]) { group_uuid, jwt_user_uuid }, NULL, NULL, 0);

            if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else if (*PQgetvalue(owner_res, 0, 0) != 't')
                http_res_with_401(&http_conn->http_res);
            else
            {

                PGresult *add_watchlist_res = PQexecParams(pg_conn,
                    "SELECT add_watchlist_for_group($1::uuid, $2::uuid);",
                    2, NULL, (const char *[2]) { watchlist_uuid, group_uuid }, NULL, NULL, 0);

                if (PQresultStatus(add_watchlist_res) != PGRES_TUPLES_OK)
                    http_res_with_500(&http_conn->http_res);
                else
                    uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

                PQclear(add_watchlist_res);
            }
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// DELETE /user/v01/groups/{group_uuid}/
void v01_delete_groups(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM users_x_groups u_x_g"
                " WHERE u_x_g.group_uuid = $1::uuid"
                " AND u_x_g.user_uuid = $2::uuid"
                " AND u_x_g.is_owner"
            ");",
            2, NULL, (const char *[2]) { group_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *delete_res = PQexecParams(pg_conn,
                "SELECT delete_group($1::uuid);",
                1, NULL, (const char *[1]) { group_uuid }, NULL, NULL, 0);

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

// DELETE /user/v01/groups/{group_uuid}/users/{user_uuid}/
void v01_delete_groups_users(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));
        char *user_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(user_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM users_x_groups u_x_g"
                " WHERE u_x_g.group_uuid = $1::uuid"
                " AND u_x_g.user_uuid = $2::uuid"
                " AND u_x_g.is_owner"
            ");",
            2, NULL, (const char *[2]) { group_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *remove_user_res = PQexecParams(pg_conn,
                "SELECT remove_user_from_group($1::uuid, $2::uuid);",
                2, NULL, (const char *[2]) { user_uuid, group_uuid }, NULL, NULL, 0);

            if (PQresultStatus(remove_user_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(remove_user_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}

// DELETE /user/v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/
void v01_delete_groups_watchlists(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    PGconn *pg_conn = uo_pg_http_conn_get_pg_conn(http_conn);

    if (pg_conn)
    {
        char *jwt_user_uuid = uo_http_conn_get_user_data(http_conn, uo_nameof(jwt_user_uuid));
        char *group_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(group_uuid));
        char *watchlist_uuid = uo_http_conn_get_req_data(http_conn, uo_nameof(watchlist_uuid));

        PGresult *owner_res = PQexecParams(pg_conn,
            "SELECT EXISTS("
                "SELECT 1 FROM users_x_groups u_x_g"
                " WHERE u_x_g.group_uuid = $1::uuid"
                " AND u_x_g.user_uuid = $2::uuid"
                " AND u_x_g.is_owner"
            ");",
            2, NULL, (const char *[2]) { group_uuid, jwt_user_uuid }, NULL, NULL, 0);

        if (PQresultStatus(owner_res) != PGRES_TUPLES_OK)
            http_res_with_500(&http_conn->http_res);
        else if (*PQgetvalue(owner_res, 0, 0) != 't')
            http_res_with_401(&http_conn->http_res);
        else
        {
            PGresult *remove_watchlist_res = PQexecParams(pg_conn,
                "SELECT remove_watchlist_from_group($1::uuid, $2::uuid);",
                2, NULL, (const char *[2]) { watchlist_uuid, group_uuid }, NULL, NULL, 0);

            if (PQresultStatus(remove_watchlist_res) != PGRES_TUPLES_OK)
                http_res_with_500(&http_conn->http_res);
            else
                uo_http_res_set_status_line(&http_conn->http_res, UO_HTTP_204, UO_HTTP_VER_1_1);

            PQclear(remove_watchlist_res);
        }

        PQclear(owner_res);
    }

    uo_cb_invoke(cb);
}
