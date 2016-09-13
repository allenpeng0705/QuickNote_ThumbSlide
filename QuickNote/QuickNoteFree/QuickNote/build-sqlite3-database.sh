#! /bin/sh
#
# Builds a SQLite database from the given SQL script
# TODO: Proper parameter validation

if test -f "$3"
then
	rm "$3"
fi

sqlite3 "$3" < "$1.sql"