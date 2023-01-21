set(TARGET sqlite3)

add_library(${TARGET} STATIC shell.c sqlite3.c sqlite3.h sqlite3ext.h)
