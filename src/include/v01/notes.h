#ifndef UO_V01_NOTES_H
#define UO_V01_NOTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

// GET /user/v01/notes/
void v01_get_notes(
    uo_cb *cb);

// POST /user/v01/notes/
void v01_post_notes(
    uo_cb *cb);

// DELETE /user/v01/notes/{note_uuid}/
void v01_delete_notes(
    uo_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
