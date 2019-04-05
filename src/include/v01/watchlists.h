#ifndef UO_V01_WATCHLISTS_H
#define UO_V01_WATCHLISTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

// GET /user/v01/watchlists/
void v01_get_watchlists(
    uo_cb *cb);

// GET /user/v01/watchlists/{watchlist_uuid}/instruments/
void v01_get_watchlists_instruments(
    uo_cb *cb);

// GET /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/
void v01_get_watchlists_instruments_notes(
    uo_cb *cb);

// POST /user/v01/watchlists/
void v01_post_watchlists(
    uo_cb *cb);

// PUT /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/
void v01_put_watchlists_instruments(
    uo_cb *cb);

// PUT /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_put_watchlists_instruments_notes(
    uo_cb *cb);

// DELETE /user/v01/watchlists/{watchlist_uuid}/
void v01_delete_watchlists(
    uo_cb *cb);

// DELETE /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/
void v01_delete_watchlists_instruments(
    uo_cb *cb);

// DELETE /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_delete_watchlists_instruments_notes(
    uo_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
