CREATE TABLE "rom" (
	"short_name"	TEXT NOT NULL UNIQUE,
	"long_name"	TEXT,
	"year"	INTEGER,
	"short_name_parent"	TEXT,
	PRIMARY KEY("short_name")
)

CREATE TABLE "rom_files" (
	"id"	INTEGER NOT NULL UNIQUE,
	"filename"	TEXT,
	"region"	INTEGER,
	"src"	INTEGER,
	"dest"	INTEGER,
	"size"	INTEGER,
	"crc"	TEXT,
	"short_name_fk"	TEXT,
	"driver"	TEXT,
	PRIMARY KEY("id" AUTOINCREMENT),
	FOREIGN KEY("short_name_fk") REFERENCES "rom"("short_name")
)

CREATE TABLE "rom_size" (
	"id"	INTEGER NOT NULL UNIQUE,
	"region"	INTEGER,
	"size"	INTEGER,
	"short_name_fk"	TEXT,
	PRIMARY KEY("id" AUTOINCREMENT),
	FOREIGN KEY("short_name_fk") REFERENCES "rom"("short_name")
);