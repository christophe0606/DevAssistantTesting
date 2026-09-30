#include "tetris.h"
#include <string.h>

bool tetris_cell(uint8_t piece, uint8_t rotation, int x, int y)
{
    static const uint16_t masks[7] = {0x00F0, 0x0066, 0x0072, 0x0036,
                                      0x0063, 0x0071, 0x0074};
    if (piece >= 7 || x < 0 || y < 0 || x > 3 || y > 3) return false;
    const int size = piece == 0 || piece == 1 ? 4 : 3;
    if (x >= size || y >= size) return false;
    if (piece != 1) {
        for (unsigned i = 0; i < (rotation & 3U); ++i) {
            const int old_x = x;
            x = y;
            y = size - 1 - old_x;
        }
    }
    return (masks[piece] & (1U << (y * 4 + x))) != 0;
}

bool tetris_fits(const Tetris *g, int x, int y, uint8_t rotation)
{
    for (int py = 0; py < 4; ++py) {
        for (int px = 0; px < 4; ++px) {
            if (!tetris_cell(g->piece, rotation, px, py)) continue;
            int bx = x + px, by = y + py;
            if (bx < 0 || bx >= TETRIS_COLS || by < 0 || by >= TETRIS_ROWS)
                return false;
            if (g->board[by][bx]) return false;
        }
    }
    return true;
}

static uint32_t random_next(Tetris *g)
{
    uint32_t x = g->random;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return g->random = x;
}

static uint8_t take_piece(Tetris *g)
{
    if (g->bag_pos >= 7) {
        for (uint8_t i = 0; i < 7; ++i) g->bag[i] = i;
        for (int i = 6; i > 0; --i) {
            unsigned j = random_next(g) % (unsigned)(i + 1);
            uint8_t swap = g->bag[i]; g->bag[i] = g->bag[j]; g->bag[j] = swap;
        }
        g->bag_pos = 0;
    }
    return g->bag[g->bag_pos++];
}

static void spawn(Tetris *g, uint32_t now)
{
    g->piece = g->next;
    g->next = take_piece(g);
    g->x = 3; g->y = 0; g->rotation = 0;
    g->fall_at = now; g->grounded = false; g->lock_resets = 0;
    if (!tetris_fits(g, g->x, g->y, g->rotation)) g->state = GAME_OVER;
}

void tetris_init(Tetris *g, uint32_t seed)
{
    memset(g, 0, sizeof(*g));
    g->random = seed ? seed : 0x7351A2B9U;
    g->bag_pos = 7;
    g->level = 1;
    g->state = GAME_READY;
}

void tetris_start(Tetris *g, uint32_t now)
{
    uint32_t best = g->best, seed = g->random ^ now;
    tetris_init(g, seed);
    g->best = best;
    g->state = GAME_PLAYING;
    g->next = take_piece(g);
    spawn(g, now);
    g->center_consumed = true; /* Starting press cannot also drop the piece. */
}

int tetris_landing_y(const Tetris *g)
{
    int y = g->y;
    while (tetris_fits(g, g->x, y + 1, g->rotation)) ++y;
    return y;
}

static void lock_piece(Tetris *g, uint32_t now)
{
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            if (tetris_cell(g->piece, g->rotation, x, y))
                g->board[g->y + y][g->x + x] = g->piece + 1;

    unsigned cleared = 0;
    int dest = TETRIS_ROWS - 1;
    for (int row = TETRIS_ROWS - 1; row >= 0; --row) {
        bool full = true;
        for (int x = 0; x < TETRIS_COLS; ++x) full &= g->board[row][x] != 0;
        if (full) ++cleared;
        else {
            if (dest != row) memcpy(g->board[dest], g->board[row], TETRIS_COLS);
            --dest;
        }
    }
    while (dest >= 0) memset(g->board[dest--], 0, TETRIS_COLS);
    static const unsigned awards[] = {0, 100, 300, 500, 800};
    g->score += awards[cleared] * g->level;
    g->lines += cleared;
    g->level = 1 + g->lines / 10;
    if (g->score > g->best) g->best = g->score;
    for (int row = 0; row < TETRIS_HIDDEN; ++row)
        for (int x = 0; x < TETRIS_COLS; ++x)
            if (g->board[row][x]) { g->state = GAME_OVER; return; }
    spawn(g, now);
}

void tetris_hard_drop(Tetris *g, uint32_t now)
{
    if (g->state != GAME_PLAYING) return;
    int landing = tetris_landing_y(g);
    g->score += (unsigned)(landing - g->y) * 2U;
    g->y = landing;
    lock_piece(g, now);
}

static void adjusted(Tetris *g, uint32_t now)
{
    if (g->grounded && g->lock_resets < 12) {
        g->lock_at = now;
        ++g->lock_resets;
    }
    if (tetris_fits(g, g->x, g->y + 1, g->rotation)) g->grounded = false;
}

bool tetris_rotate(Tetris *g, uint32_t now)
{
    if (g->state != GAME_PLAYING) return false;
    static const int8_t kicks[][2] = {{0,0}, {-1,0}, {1,0}, {-2,0}, {2,0},
                                     {0,-1}, {-1,-1}, {1,-1}, {0,-2}};
    uint8_t rotation = (g->rotation + 1U) & 3U;
    for (unsigned i = 0; i < sizeof(kicks) / sizeof(kicks[0]); ++i) {
        int x = g->x + kicks[i][0], y = g->y + kicks[i][1];
        if (tetris_fits(g, x, y, rotation)) {
            g->x = x; g->y = y; g->rotation = rotation;
            adjusted(g, now);
            return true;
        }
    }
    return false;
}

void tetris_update(Tetris *g, uint32_t now, uint32_t keys)
{
    uint32_t pressed = keys & ~g->previous_keys;
    uint32_t released = g->previous_keys & ~keys;
    g->previous_keys = keys;
    if (g->state == GAME_READY || g->state == GAME_OVER) {
        if (pressed & KEY_CENTER) {
            tetris_start(g, now);
            g->previous_keys = keys;
        }
        return;
    }
    if (pressed & KEY_CENTER) { g->center_at = now; g->center_consumed = false; }
    if ((keys & KEY_CENTER) && !g->center_consumed && now - g->center_at >= 600U) {
        g->state = g->state == GAME_PAUSED ? GAME_PLAYING : GAME_PAUSED;
        g->center_consumed = true;
        g->fall_at = now; g->lock_at = now;
    }
    if ((released & KEY_CENTER) && !g->center_consumed) {
        if (g->state == GAME_PAUSED) {
            g->state = GAME_PLAYING; g->fall_at = now; g->lock_at = now;
        } else tetris_hard_drop(g, now);
        g->center_consumed = true;
        return;
    }
    if (g->state != GAME_PLAYING) return;

    int horizontal = !!(keys & KEY_RIGHT) - !!(keys & KEY_LEFT);
    bool move = false;
    if (horizontal != g->horizontal) {
        g->horizontal = horizontal; g->horizontal_at = now;
        g->horizontal_repeating = false; move = horizontal != 0;
    } else if (horizontal && now - g->horizontal_at >=
               (g->horizontal_repeating ? 70U : 180U)) {
        move = true; g->horizontal_at = now; g->horizontal_repeating = true;
    }
    if (move && tetris_fits(g, g->x + horizontal, g->y, g->rotation)) {
        g->x += horizontal; adjusted(g, now);
    }
    if (pressed & KEY_UP) tetris_rotate(g, now);

    uint32_t gravity = g->level < 15 ? 800U - (g->level - 1U) * 50U : 100U;
    bool soft = (keys & KEY_DOWN) &&
                ((pressed & KEY_DOWN) || now - g->down_at >= 50U);
    if (soft || now - g->fall_at >= gravity) {
        g->fall_at = now;
        if (soft) g->down_at = now;
        if (tetris_fits(g, g->x, g->y + 1, g->rotation)) {
            ++g->y; g->grounded = false;
            if (soft) ++g->score;
        }
    }
    if (!tetris_fits(g, g->x, g->y + 1, g->rotation)) {
        if (!g->grounded) { g->grounded = true; g->lock_at = now; }
        if (now - g->lock_at >= 400U) lock_piece(g, now);
    }
    if (g->score > g->best) g->best = g->score;
}
