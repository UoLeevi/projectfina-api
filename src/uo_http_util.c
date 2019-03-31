#include "uo_http_util.h"
#include "uo_jwt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool uo_http_req_parse_user_uuid_from_jwt(
    uo_http_req *http_req,
    char user_uuid[37])
{
    char *hdr_authorization = uo_http_msg_get_header(http_req, "authorization");
    char jwt[0x400];

    if (!hdr_authorization || sscanf(hdr_authorization, "Bearer %1023s", jwt) != 1)
        return false;

    char *jwt_payload = uo_jwt_decode_payload(NULL, jwt, strlen(jwt));
    if (!jwt_payload)
        return false;

    char *jwt_claim_sub = uo_json_find_value(jwt_payload, "sub");
    if (!jwt_claim_sub)
        return false;

    memcpy(user_uuid, jwt_claim_sub + 1, 36);
    user_uuid[36] = '\0';

    return true;
}

void uo_http_req_handler_parse_user_uuid_from_jwt(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_req = &http_conn->http_req;

    char buf[37];

    if (uo_http_req_parse_user_uuid_from_jwt(http_req, buf))
    {
        char *jwt_user_uuid = strdup(buf);
        uo_refstack_push(&http_req->refstack, jwt_user_uuid, free);
        uo_http_conn_set_user_data(http_conn, uo_nameof(jwt_user_uuid), jwt_user_uuid);
    }
    else
        http_res_with_401(&http_conn->http_res);
    
    uo_cb_invoke(cb);
}
