#pragma once

enum Methods {
    M_TOFLOAT64   = 0x0A,
    M_TOINT64     = 0x0B,
    M_TOBOOL      = 0x0C,
    M_TOSTR       = 0x0D,
    M_TOLIST      = 0x0E,
    M_TOMAP       = 0x0F,

    M_UPCASE      = 0x10,
    M_DOWNCASE    = 0x11,
    M_ISALNUM     = 0x12,
    M_ISAL        = 0x13,
    M_ISNUM       = 0x14,
    M_ISSPACE     = 0x15,
    M_STARTSWITH  = 0x16,
    M_ENDSWITH    = 0x17,
    M_SEARCH      = 0x18,
    M_SPLIT       = 0x19,
    M_LTRIM       = 0x1A,
    M_RTRIM       = 0x1B,
    M_TRIM        = 0x1C,

    M_APPEND      = 0x20,

    M_KEYS        = 0x30,
    M_VALUES      = 0x31,
    M_CLONE       = 0x32,

    M_CLOSE       = 0x40,
    M_PCLOSE      = 0x41,
    M_READ        = 0x42,
    M_WRITE       = 0x43,
    M_READLINE    = 0x44,

    M___GET       = 0xF0,
    M___SET       = 0xF1,

};