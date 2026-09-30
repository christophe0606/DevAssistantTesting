#ifndef STARWARS_H
#define STARWARS_H
#include <stdbool.h>
#include <stdint.h>

#define STARWARS_WIDTH 480
#define STARWARS_HEIGHT 800
#define CRAWL_COLS 28
#define CRAWL_MAX_LINES 96
#define CRAWL_LINE_HEIGHT 12
#define CRAWL_TEXTURE_WIDTH (CRAWL_COLS * 6)
#define CRAWL_TEXTURE_HEIGHT (CRAWL_MAX_LINES * CRAWL_LINE_HEIGHT)
#define CRAWL_STAR_COUNT 110

typedef struct {
    uint8_t texture[CRAWL_TEXTURE_HEIGHT][CRAWL_TEXTURE_WIDTH];
    uint8_t row_ink[CRAWL_TEXTURE_HEIGHT];
    uint16_t star_x[CRAWL_STAR_COUNT], star_y[CRAWL_STAR_COUNT];
    uint8_t star_light[CRAWL_STAR_COUNT];
    uint32_t last_ms, phase_ms, crawl_ms, cycle_ms, cycles;
    uint16_t lines, texture_height;
} StarWars;

/* NULL text selects crawl_text.h. False means too many lines or bad config. */
bool starwars_init(StarWars *s, const char *text, uint32_t now);
void starwars_update(StarWars *s, uint32_t now);
void starwars_render(uint16_t *pixels, const StarWars *s);
#endif
