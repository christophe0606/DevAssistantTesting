#ifndef MATRIX_H
#define MATRIX_H
#include <stdint.h>

#define MATRIX_WIDTH 480
#define MATRIX_HEIGHT 800
#define MATRIX_CELL 16
#define MATRIX_COLS (MATRIX_WIDTH / MATRIX_CELL)
#define MATRIX_ROWS (MATRIX_HEIGHT / MATRIX_CELL)
#define MATRIX_LAYERS 2
#define MATRIX_GLYPHS 48

typedef struct {
    int32_t head_q8;
    uint32_t speed_q8, remainder;
    uint8_t length, brightness;
} MatrixStream;

typedef struct {
    MatrixStream streams[MATRIX_LAYERS][MATRIX_COLS];
    uint8_t glyphs[MATRIX_COLS][MATRIX_ROWS];
    uint32_t random, last_ms, mutation_ms, updates, respawns;
} MatrixRain;

void matrix_init(MatrixRain *rain, uint32_t seed, uint32_t now);
void matrix_update(MatrixRain *rain, uint32_t now);
void matrix_render(uint16_t *pixels, const MatrixRain *rain);
#endif
