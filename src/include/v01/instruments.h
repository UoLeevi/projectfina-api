#ifndef UO_V01_INSTRUMENTS_H
#define UO_V01_INSTRUMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

// GET /v01/instruments/{instrument_uuid}/
void v01_get_instruments(
    uo_cb *cb);

// GET /user/v01/instruments/{instrument_uuid}/notes/
void v01_get_instruments_notes(
    uo_cb *cb);

// PUT /user/v01/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_put_instruments_notes(
    uo_cb *cb);

// DELETE /user/v01/instruments/{instrument_uuid}/notes/{note_uuid}/
void v01_delete_instruments_notes(
    uo_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
