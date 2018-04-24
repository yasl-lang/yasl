#pragma once

enum Methods {
    TOFLOAT64   = 0x0A,
    TOINT64     = 0x0B,
    TOBOOL      = 0x0C,
    TOSTR       = 0x0D,
    TOLIST      = 0x0E,
    TOMAP       = 0x0F,

    UPCASE      = 0x10,
    DOWNCASE    = 0x11,
    ISALNUM     = 0x12,
    ISAL        = 0x13,
    ISNUM       = 0x14,
    ISSPACE     = 0x15,
    STARTSWITH  = 0x16,
    ENDSWITH    = 0x17,
    SEARCH      = 0x18,
    SPLIT       = 0x19,
    LTRIM       = 0x1A,
    RTRIM       = 0x1B,
    TRIM        = 0x1C,

    APPEND      = 0x20,

    KEYS        = 0x30,
    VALUES      = 0x31,

    CLOSE       = 0x40,
    PCLOSE      = 0x41,
    READ        = 0x42,
    WRITE       = 0x43,
    READLINE    = 0x44,

    GET__       = 0xF0,
    SET__       = 0xF1,

};