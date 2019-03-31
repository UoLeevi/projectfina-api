#ifndef UO_V01_GROUPS_H
#define UO_V01_GROUPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

// GET /v01/groups/
void v01_get_groups(
    uo_cb *cb);

// GET /v01/groups/{group_uuid}/users/
void v01_get_groups_users(
    uo_cb *cb);

// GET /v01/groups/{group_uuid}/watchlists/
void v01_get_groups_watchlists(
    uo_cb *cb);

// POST /v01/groups/
void v01_post_groups(
    uo_cb *cb);

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

#ifdef __cplusplus
}
#endif

#endif
