#ifndef INSTRUCTION_H
#define INSTRUCTION_H

enum instruction_type {
    None = 0,
    R,
    I,
    S,
    U,
    B,
    J,
};

static enum instruction_type instruction_map[128] = {
    [0b0000011] = I,
    [0b0001111] = I,
    [0b0010011] = I,
    [0b0010111] = U,
    [0b0011011] = I,
    [0b0100011] = S,
    [0b0101111] = R,
    [0b0110011] = R,
    [0b0110111] = U,
    [0b0111011] = R,
    [0b1100011] = B,
    [0b1100111] = I,
    [0b1101111] = J,
    [0b1110011] = I,
};

#endif
