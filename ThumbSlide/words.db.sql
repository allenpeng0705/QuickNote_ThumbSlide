BEGIN TRANSACTION;

-- The user lexicon table
CREATE TABLE "user_dict" ('WORD' CHAR(100) PRIMARY KEY, 'COUNT' INTEGER, 'ADD_DATE' REAL,'LAST_SELECTED_DATE' REAL);

-- The active lexicon table
CREATE TABLE "active_dict" ('WORD' CHAR(100) PRIMARY KEY, 'COUNT' INTEGER, 'ADD_DATE' REAL,'LAST_SELECTED_DATE' REAL);

COMMIT;