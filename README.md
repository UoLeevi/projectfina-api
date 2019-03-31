# projectfina-api

## API Endpoints

### markets

`GET /v01/markets/`  
Get a list of markets. 

`GET /v01/markets/{market_uuid}/instruments/`  
Get a list of instruments for a market.  

### instruments

`GET /v01/instruments/{instrument_uuid}/`  
Get an instrument.  

`GET /v01/instruments/{instrument_uuid}/notes/`  
Get a list of notes for an instrument.  

`PUT /v01/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Add a note to instrument.

### notes

`GET /v01/notes/`  
Get a list of notes created by current user.

`POST /v01/notes/`  
Create a note.

`DELETE /v01/notes/{note_uuid}/`  
Delete a note.

### watchlists

`GET /v01/watchlists/`  
Get a list of watchlists for current user.

`GET /v01/watchlists/{watchlist_uuid}/instruments/`  
Get a list of instruments for a watchlist.  

`GET /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/`  
Get an instrument for a watchlist.  

`GET /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/`  
Get a list of notes for an instrument on a watchlist.  

`POST /v01/watchlists/`  
Create a watchlist.

`PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/`  
Add an instrument to watchlist.

`PUT /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Add a note to instrument on watchlist.

`DELETE /v01/watchlists/{watchlist_uuid}/`  
Delete a watchlist.

`DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/`  
Remove an instrument from watchlist.

`DELETE /v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Remove a note from instrument on watchlist.

### groups

`GET /v01/groups/`  
Get a list of groups for current user.

`GET /v01/groups/{group_uuid}/users/`  
Get a list of users for group.

`GET /v01/groups/{group_uuid}/watchlists/`  
Get a list of watchlists for group.

`PUT /v01/groups/{group_uuid}/users/{user_uuid}/`  
Add a user for group.

`PUT /v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/`  
Add a watchlist for group.

`DELETE /v01/groups/{group_uuid}/`  
Delete a group.

`DELETE /v01/groups/{group_uuid}/users/{user_uuid}/`  
Remove a user from group.

`DELETE /v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/`  
Remove a watchlist from group.

### current

`GET /v01/current/_{mic}.json`  
Get most recent quotes for market specified by MIC

### eod

`GET /v01/eod/{year}/{mic}/{symbol[0]}/_{symbol}.json`  
Get one year end of day quotes for instrument specified by MIC, symbol and year.

