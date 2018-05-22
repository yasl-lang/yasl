#pragma once

// NOTE: this must stay synced with the jumptable in builtins.h
enum Functions {
    PRINT = 0x00,
    INPUT = 0x01,
    OPEN  = 0x02,
    POPEN = 0x03,
};
