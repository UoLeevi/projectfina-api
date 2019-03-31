#ifndef UO_HTTP_UTIL_H
#define UO_HTTP_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"

#define HTTP_400_JSON \
    "{ \"message\": \"bad request\" }"
#define HTTP_401_JSON \
    "{ \"message\": \"unauthorized\" }"
#define HTTP_500_JSON \
    "{ \"message\": \"server error\" }"

static inline void http_res_with_400(
    uo_http_msg *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_400, UO_HTTP_VER_1_1);
    uo_http_res_set_content(http_res, HTTP_400_JSON, "application/json", UO_STRLEN(HTTP_400_JSON));
}

static inline void http_res_with_401(
    uo_http_msg *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_401, UO_HTTP_VER_1_1);
    uo_http_res_set_content(http_res, HTTP_401_JSON, "application/json", UO_STRLEN(HTTP_401_JSON));
}

static inline void http_res_with_500(
    uo_http_msg *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_500, UO_HTTP_VER_1_1);
    uo_http_res_set_content(http_res, HTTP_500_JSON, "application/json", UO_STRLEN(HTTP_500_JSON));
}

void uo_http_req_handler_parse_user_uuid_from_jwt(
    uo_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
