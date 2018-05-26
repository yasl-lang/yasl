#pragma once

// NOTE: this must stay synced with the jumptable in builtins.h
enum Functions {
    F_PRINT = 0x00,
    F_INPUT = 0x01,
    F_OPEN  = 0x02,
    F_POPEN = 0x03,
};
