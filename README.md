# projectfina-api

## API Endpoints

#### note about authorization

Requests to API endpoints with URI path starting with `/user/` require a valid JWT token in the `Authorization: Bearer {token}` HTTP header.  

### markets

`GET /v01/markets/`  
Get a list of markets. 

`GET /v01/markets/{market_uuid}/instruments/`  
Get a list of instruments for a market.  

### instruments

`GET /v01/instruments/{instrument_uuid}/`  
Get an instrument.  

`GET /user/v01/instruments/{instrument_uuid}/notes/`  
Get a list of notes for an instrument.  

`PUT /user/v01/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Add a note to instrument.

`DELETE /user/v01/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Remove a note from instrument.

### notes

`GET /user/v01/notes/`  
Get a list of notes created by current user.

`POST /user/v01/notes/`  
Create a note.

`DELETE /user/v01/notes/{note_uuid}/`  
Delete a note.

### watchlists

`GET /user/v01/watchlists/`  
Get a list of watchlists for current user.

`GET /user/v01/watchlists/{watchlist_uuid}/instruments/`  
Get a list of instruments for a watchlist.  

`GET /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/`  
Get a list of notes for an instrument on a watchlist.  

`POST /user/v01/watchlists/`  
Create a watchlist.

`PUT /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/`  
Add an instrument to watchlist.

`PUT /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Add a note to instrument on watchlist.

`DELETE /user/v01/watchlists/{watchlist_uuid}/`  
Delete a watchlist.

`DELETE /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/`  
Remove an instrument from watchlist.

`DELETE /user/v01/watchlists/{watchlist_uuid}/instruments/{instrument_uuid}/notes/{note_uuid}/`  
Remove a note from instrument on watchlist.

### groups

`GET /user/v01/groups/`  
Get a list of groups for current user.

`GET /user/v01/groups/{group_uuid}/users/`  
Get a list of users for group.

`GET /user/v01/groups/{group_uuid}/watchlists/`  
Get a list of watchlists for group.

`POST /user/v01/groups/`  
Create a group.

`PUT /user/v01/groups/{group_uuid}/users/{user_uuid}/`  
Add a user for group.

`PUT /user/v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/`  
Add a watchlist for group.

`DELETE /user/v01/groups/{group_uuid}/`  
Delete a group.

`DELETE /user/v01/groups/{group_uuid}/users/{user_uuid}/`  
Remove a user from group.

`DELETE /user/v01/groups/{group_uuid}/watchlists/{watchlist_uuid}/`  
Remove a watchlist from group.

### current

`GET /v01/current/_{mic}.json`  
Get most recent quotes for market specified by MIC

### eod

`GET /v01/eod/{year}/{mic}/{symbol[0]}/_{symbol}.json`  
Get one year end of day quotes for instrument specified by MIC, symbol and year.

