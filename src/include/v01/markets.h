#ifndef UO_V01_MARKETS_H
#define UO_V01_MARKETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

// GET /v01/markets/
void v01_get_markets(
    uo_cb *cb);

// GET /v01/markets/{market_uuid}/instruments/
void v01_get_markets_instruments(
    uo_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
