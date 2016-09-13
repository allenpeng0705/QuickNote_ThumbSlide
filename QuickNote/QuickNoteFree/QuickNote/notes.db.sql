BEGIN TRANSACTION;

-- The user lexicon table
CREATE TABLE "user_dict" ('WORD' CHAR(100) PRIMARY KEY, 'COUNT' INTEGER, 'ADD_DATE' REAL,'LAST_SELECTED_DATE' REAL);

-- The active lexicon table
CREATE TABLE "active_dict" ('WORD' CHAR(100) PRIMARY KEY, 'COUNT' INTEGER, 'ADD_DATE' REAL,'LAST_SELECTED_DATE' REAL);

-- The notes table
CREATE TABLE "note_table" ('ID' INTEGER PRIMARY KEY, 'TITLE' CHAR(30), 'DATE' REAL, 'LOCATION' CHAR(32), 'CONTENT' CHAR(10000), 'TAG' CHAR(32), 'LOCKED' INTEGER DEFAULT 0);

-- The default notes
INSERT INTO "note_table" VALUES(1,'Welcome!',2518759600.0,NULL,'Welcome!
ThumbSlide makes the inputting much easier and faster. If you want to get more familiar with ThumbSlide, please take a look at the HELP page!','Guide', 0);

COMMIT;
