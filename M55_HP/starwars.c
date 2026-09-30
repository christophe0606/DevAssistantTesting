#include "starwars.h"
#include "crawl_text.h"
#include <math.h>
#include <string.h>

#define HORIZON 150.0f
#define NEAR_Y 820.0f
#define FOCAL 420.0f
#define TEXT_SCALE 2.5f
#define START_DEPTH 50.0f
#define FADE_END_Y 270.0f
#define FADE_START_Y 385.0f

static const uint8_t font[44][7] = {
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14},
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14}, {7,2,2,2,2,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31},
    {0,0,0,0,0,12,12}, /* . */
    {0,0,0,0,0,4,8},   /* , */
    {4,4,4,4,4,0,4},   /* ! */
    {14,17,1,2,4,0,4}, /* ? */
    {0,0,0,31,0,0,0},  /* - */
    {4,4,8,0,0,0,0},   /* ' */
    {0,4,4,0,4,4,0},   /* : */
    {17,10,4,10,17,0,0} /* * / fallback decoration */
};

static int glyph_index(char c)
{
    if (c >= 'a' && c <= 'z') c = (char)(c-'a'+'A');
    if (c >= '0' && c <= '9') return c-'0';
    if (c >= 'A' && c <= 'Z') return c-'A'+10;
    const char *punctuation = ".,!?-':*";
    for (int i = 0; punctuation[i]; ++i) if (punctuation[i] == c) return 36+i;
    return c == ' ' ? -1 : 39;
}

static bool emit_line(StarWars *s, const char *line, unsigned length, bool centered)
{
    if (s->lines >= CRAWL_MAX_LINES) return false;
    unsigned y = s->lines*CRAWL_LINE_HEIGHT;
    unsigned start_x = centered && length ? (CRAWL_TEXTURE_WIDTH-length*6+1)/2 : 0;
    for (unsigned c = 0; c < length; ++c) {
        int glyph = glyph_index(line[c]);
        if (glyph < 0) continue;
        for (unsigned row = 0; row < 7; ++row)
            for (unsigned col = 0; col < 5; ++col)
                if (font[glyph][row] & (1U<<(4-col))) {
                    s->texture[y+row][start_x+c*6+col] = 1;
                    s->row_ink[y+row] = 1;
                    if (y+row+1 > s->texture_height)
                        s->texture_height = (uint16_t)(y+row+1);
                }
    }
    ++s->lines;
    return true;
}

static bool layout(StarWars *s, const char *text)
{
    while (*text) {
        const char *end = text;
        while (*end && *end != '\n') ++end;
        bool center = *text == '#';
        if (center) ++text;
        char line[CRAWL_COLS];
        unsigned length = 0;
        while (text < end) {
            while (text < end && (*text == ' ' || *text == '\t' || *text == '\r')) ++text;
            if (text == end) break;
            const char *word_end = text;
            while (word_end < end && *word_end != ' ' && *word_end != '\t' && *word_end != '\r') ++word_end;
            unsigned word_length = (unsigned)(word_end-text);
            if (length && length+1+word_length > CRAWL_COLS) {
                if (!emit_line(s,line,length,center)) return false;
                length = 0;
            }
            if (length) line[length++] = ' ';
            while (text < word_end) {
                if (length == CRAWL_COLS) {
                    if (!emit_line(s,line,length,center)) return false;
                    length = 0;
                }
                line[length++] = *text++;
            }
        }
        if (!emit_line(s,line,length,center)) return false;
        text = *end ? end+1 : end;
    }
    return true;
}

bool starwars_init(StarWars *s, const char *text, uint32_t now)
{
    memset(s,0,sizeof(*s));
    s->last_ms = now;
    if (!text) text = CRAWL_TEXT;
    if (!(CRAWL_SPEED > 0.0f) || CRAWL_RESTART_MS == 0) return false;
    if (!layout(s,text)) return false;
    uint32_t random = 0x53544152U;
    for (unsigned i = 0; i < CRAWL_STAR_COUNT; ++i) {
        random = random*1664525U+1013904223U;
        s->star_x[i] = (uint16_t)((random>>8)%STARWARS_WIDTH);
        random = random*1664525U+1013904223U;
        s->star_y[i] = (uint16_t)((random>>8)%STARWARS_HEIGHT);
        s->star_light[i] = (uint8_t)(160U+(random>>24)%96U);
    }
    /* At this depth even the final ink row is fully above the fade cutoff.
     * Deriving duration from actual ink height accommodates any custom text. */
    float far_depth = FOCAL*((NEAR_Y-HORIZON)/(FADE_END_Y-HORIZON)-1.0f);
    if (s->texture_height)
        s->crawl_ms = (uint32_t)ceilf((s->texture_height*TEXT_SCALE+far_depth-START_DEPTH)*
                                     (1000.0f/CRAWL_SPEED))+1U;
    s->cycle_ms = s->crawl_ms+CRAWL_RESTART_MS;
    return true;
}

void starwars_update(StarWars *s, uint32_t now)
{
    uint32_t elapsed = now-s->last_ms;
    s->last_ms = now;
    uint64_t phase = (uint64_t)s->phase_ms+elapsed;
    s->cycles += (uint32_t)(phase/s->cycle_ms);
    s->phase_ms = (uint32_t)(phase%s->cycle_ms);
}

static uint16_t rgb(unsigned r, unsigned g, unsigned b)
{
    return (uint16_t)((r>>3)<<11 | (g>>2)<<5 | (b>>3));
}

void starwars_render(uint16_t *fb, const StarWars *s)
{
    memset(fb,0,STARWARS_WIDTH*STARWARS_HEIGHT*sizeof(*fb));
    for (unsigned i = 0; i < CRAWL_STAR_COUNT; ++i) {
        unsigned light = s->star_light[i];
        /* A bright 3x3 core remains visible on the kit LCD. The brightest
         * stars have a softer 5x5 halo, with corners trimmed for a round shape. */
        int radius = light >= 224 ? 2 : 1;
        for (int dy = -radius; dy <= radius; ++dy) {
            int y = (int)s->star_y[i]+dy;
            if (y < 0 || y >= STARWARS_HEIGHT) continue;
            for (int dx = -radius; dx <= radius; ++dx) {
                int x = (int)s->star_x[i]+dx;
                if (x < 0 || x >= STARWARS_WIDTH) continue;
                int distance = dx*dx+dy*dy;
                if (distance > 5) continue;
                unsigned glow = distance > 2 ? light/2 :
                                distance == 2 ? light*3/4 : light;
                uint16_t color = rgb(glow*4/5,glow*9/10,glow);
                unsigned pixel = (unsigned)y*STARWARS_WIDTH+(unsigned)x;
                if (color > fb[pixel]) fb[pixel] = color;
            }
        }
    }
    if (s->phase_ms >= s->crawl_ms) return;
    float scroll = START_DEPTH+(float)s->phase_ms*(CRAWL_SPEED/1000.0f);
    /* Inverse-map screen scanlines onto a receding plane. Four subpixel
     * samples preserve small distant glyphs and soften the bitmap edges. */
    for (int y = (int)FADE_END_Y+1; y < STARWARS_HEIGHT; ++y) {
        const uint8_t *rows[2] = {0,0};
        float inverse_scale[2];
        float widest = 0;
        for (int sample = 0; sample < 2; ++sample) {
            float sy = (float)y+(sample ? 0.75f : 0.25f);
            float scale = (sy-HORIZON)/(NEAR_Y-HORIZON);
            inverse_scale[sample] = 1.0f/(scale*TEXT_SCALE);
            float depth = FOCAL*(1.0f/scale-1.0f);
            float source_y = (scroll-depth)/TEXT_SCALE;
            if (source_y >= 0 && source_y < s->texture_height && s->row_ink[(int)source_y])
                rows[sample] = s->texture[(int)source_y];
            widest = scale*TEXT_SCALE;
        }
        if (!rows[0] && !rows[1]) continue;
        float fade = fminf(1.0f,((float)y-FADE_END_Y)/(FADE_START_Y-FADE_END_Y));
        fade = fade*fade*(3.0f-2.0f*fade);
        if (y > 775) fade *= (float)(STARWARS_HEIGHT-y)/25.0f;
        uint16_t colors[5];
        for (unsigned coverage = 1; coverage <= 4; ++coverage) {
            float alpha = fade*(float)coverage/4.0f;
            colors[coverage] = rgb((unsigned)(255*alpha),(unsigned)(198*alpha),(unsigned)(40*alpha));
        }
        int left = (int)(STARWARS_WIDTH/2-CRAWL_TEXTURE_WIDTH*widest/2)-1;
        int right = STARWARS_WIDTH-left;
        if (left < 0) left = 0;
        if (right > STARWARS_WIDTH) right = STARWARS_WIDTH;
        for (int x = left; x < right; ++x) {
            unsigned coverage = 0;
            for (int sample_y = 0; sample_y < 2; ++sample_y) {
                if (!rows[sample_y]) continue;
                for (int sample_x = 0; sample_x < 2; ++sample_x) {
                    float sx = ((float)x+(sample_x ? 0.75f : 0.25f)-STARWARS_WIDTH/2)*
                               inverse_scale[sample_y]+CRAWL_TEXTURE_WIDTH/2;
                    if (sx >= 0 && sx < CRAWL_TEXTURE_WIDTH) coverage += rows[sample_y][(int)sx];
                }
            }
            if (coverage && colors[coverage]) fb[y*STARWARS_WIDTH+x] = colors[coverage];
        }
    }
}
