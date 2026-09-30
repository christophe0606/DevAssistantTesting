#include "arkanoid.h"
#include <math.h>
#include <string.h>

static float clamp(float x, float low, float high)
{
    return fminf(high, fmaxf(low, x));
}

float arkanoid_paddle_width(const Arkanoid *g)
{
    return g->wide_ms ? 136.0f : 88.0f;
}

static void reset_trail(Arkanoid *g)
{
    for (int i = 0; i < 8; ++i) {
        g->trail_x[i] = g->ball_x; g->trail_y[i] = g->ball_y;
    }
}

static void serve(Arkanoid *g)
{
    g->ball_x = g->paddle_x;
    g->ball_y = PADDLE_Y - BALL_RADIUS - 1.0f;
    g->vx = 0; g->vy = 0;
    g->state = ARK_SERVE;
    reset_trail(g);
}

static void load_level(Arkanoid *g)
{
    g->remaining = 0;
    for (int row = 0; row < BRICK_ROWS; ++row)
        for (int col = 0; col < BRICK_COLS; ++col) {
            /* Alternate full walls and staggered openings on later levels. */
            bool gap = g->level > 1 && g->level % 2 == 0 &&
                       row > 1 && (col + row) % 5 == 0;
            g->bricks[row][col] = gap ? 0 :
                (uint8_t)(row < 2 && (g->level > 1 || col % 3 == 0) ? 2 : 1);
            if (!gap) ++g->remaining;
        }
    g->speed = fminf(520.0f, 290.0f + (float)(g->level - 1U) * 25.0f);
    g->paddle_x = 240;
    g->wide_ms = 0; g->flash_ms = 0; g->power_active = false;
    serve(g);
}

void arkanoid_init(Arkanoid *g, uint32_t now)
{
    memset(g, 0, sizeof(*g));
    g->level = 1; g->lives = 3; g->last_ms = now;
    load_level(g);
    g->state = ARK_READY;
}

static void launch(Arkanoid *g)
{
    g->vx = 0.35f * g->speed;
    g->vy = -sqrtf(g->speed * g->speed - g->vx * g->vx);
    g->state = ARK_PLAYING;
}

/* Sweep a point against a rectangle expanded by the ball radius. Slab
 * intersection finds the first contact even when one step crosses a brick.
 * A box envelope at corners gives stable, slightly forgiving arcade hits. */
static bool sweep(float x, float y, float vx, float vy, float duration,
                  float left, float top, float right, float bottom,
                  float *time, int *nx, int *ny)
{
    float enter = -1.0e20f, leave = 1.0e20f;
    int normal_x = 0, normal_y = 0;
    for (int axis = 0; axis < 2; ++axis) {
        float p = axis ? y : x, v = axis ? vy : vx;
        float low = axis ? top : left, high = axis ? bottom : right;
        if (fabsf(v) < 0.0001f) {
            if (p < low || p > high) return false;
            continue;
        }
        float first = (low-p)/v, last = (high-p)/v;
        int normal = -1;
        if (first > last) { float temp = first; first = last; last = temp; normal = 1; }
        if (first > enter + 0.000001f) {
            enter = first; normal_x = axis ? 0 : normal; normal_y = axis ? normal : 0;
        } else if (fabsf(first-enter) < 0.000001f) {
            if (axis) normal_y = normal; else normal_x = normal;
        }
        leave = fminf(leave,last);
        if (enter > leave) return false;
    }
    if (enter < -0.000001f || enter > duration || leave < 0) return false;
    *time = fmaxf(0,enter); *nx = normal_x; *ny = normal_y;
    return true;
}

static void move_ball(Arkanoid *g, float dt, int paddle_direction)
{
    for (int collision = 0; collision < 6 && dt > 0.000001f; ++collision) {
        float first = dt + 1.0f;
        int nx = 0, ny = 0, row_hit = -1, col_hit = -1;
        bool paddle_hit = false;
        float t;
        if (g->vx < 0) {
            t = (COURT_LEFT + BALL_RADIUS - g->ball_x) / g->vx;
            if (t >= 0 && t <= dt) { first = t; nx = 1; }
        } else if (g->vx > 0) {
            t = (COURT_RIGHT - BALL_RADIUS - g->ball_x) / g->vx;
            if (t >= 0 && t <= dt) { first = t; nx = -1; }
        }
        if (g->vy < 0) {
            t = (COURT_TOP + BALL_RADIUS - g->ball_y) / g->vy;
            if (t >= 0 && t <= dt && t < first) { first = t; nx = 0; ny = 1; }
        }
        /* Paddle catches only a descending ball crossing its upper face.
         * A miss cannot bounce back up from the underside. */
        if (g->vy > 0 && g->ball_y <= PADDLE_Y - BALL_RADIUS) {
            t = (PADDLE_Y - BALL_RADIUS - g->ball_y) / g->vy;
            float hit_x = g->ball_x + g->vx*t;
            if (t >= 0 && t <= dt && t < first &&
                fabsf(hit_x-g->paddle_x) <= arkanoid_paddle_width(g)/2 + BALL_RADIUS) {
                first = t; nx = 0; ny = -1; paddle_hit = true;
            }
        }
        for (int row = 0; row < BRICK_ROWS; ++row)
            for (int col = 0; col < BRICK_COLS; ++col) {
                if (!g->bricks[row][col]) continue;
                int bx, by;
                float left = BRICK_X + col*BRICK_PITCH_X;
                float top = BRICK_Y + row*BRICK_PITCH_Y;
                if (sweep(g->ball_x,g->ball_y,g->vx,g->vy,dt,
                          left-BALL_RADIUS,top-BALL_RADIUS,
                          left+BRICK_W+BALL_RADIUS,top+BRICK_H+BALL_RADIUS,&t,&bx,&by)
                    && t < first) {
                    first = t; nx = bx; ny = by; row_hit = row; col_hit = col;
                    paddle_hit = false;
                }
            }
        if (first > dt) {
            g->ball_x += g->vx*dt; g->ball_y += g->vy*dt;
            return;
        }
        g->ball_x += g->vx*first; g->ball_y += g->vy*first;
        dt -= first;
        if (row_hit >= 0) {
            --g->bricks[row_hit][col_hit];
            g->flash_row = row_hit; g->flash_col = col_hit; g->flash_ms = 100;
            if (!g->bricks[row_hit][col_hit]) {
                --g->remaining; ++g->destroyed;
                g->score += (uint32_t)(BRICK_ROWS-row_hit)*50U;
                if (!g->power_active && g->destroyed % 7 == 0) {
                    g->power_active = true;
                    g->power_x = BRICK_X + col_hit*BRICK_PITCH_X + BRICK_W/2;
                    g->power_y = BRICK_Y + row_hit*BRICK_PITCH_Y + BRICK_H/2;
                }
                if (!g->remaining) {
                    g->score += 500U*g->level;
                    g->state = ARK_LEVEL_CLEAR;
                    return;
                }
            } else g->score += 25;
        }
        if (paddle_hit) {
            float offset = (g->ball_x-g->paddle_x)/(arkanoid_paddle_width(g)/2);
            float angle = clamp(offset*0.82f + paddle_direction*0.12f,-0.88f,0.88f);
            if (fabsf(angle) < 0.12f) angle = angle < 0 ? -0.12f : 0.12f;
            g->speed = fminf(560.0f,g->speed+3.0f);
            g->vx = g->speed*angle;
            g->vy = -sqrtf(g->speed*g->speed-g->vx*g->vx);
        } else {
            if (nx) g->vx = -g->vx;
            if (ny) g->vy = -g->vy;
        }
        /* Separate contact surfaces so a zero-time hit cannot repeat. */
        g->ball_x += (float)nx*0.05f; g->ball_y += (float)ny*0.05f;
    }
}

static void step(Arkanoid *g, uint32_t keys)
{
    const float dt = 0.005f;
    int direction = !!(keys & KEY_RIGHT) - !!(keys & KEY_LEFT);
    if (g->wide_ms) g->wide_ms = g->wide_ms > 5 ? g->wide_ms-5 : 0;
    float half = arkanoid_paddle_width(g)/2;
    g->paddle_x = clamp(g->paddle_x + direction*((keys & KEY_DOWN) ? 170.0f : 420.0f)*dt,
                        COURT_LEFT+half,COURT_RIGHT-half);
    if (g->state == ARK_SERVE || g->state == ARK_READY) {
        g->ball_x = g->paddle_x; reset_trail(g); return;
    }
    if (g->flash_ms) g->flash_ms = g->flash_ms > 5 ? g->flash_ms-5 : 0;
    for (int i = 7; i > 0; --i) {
        g->trail_x[i] = g->trail_x[i-1]; g->trail_y[i] = g->trail_y[i-1];
    }
    g->trail_x[0] = g->ball_x; g->trail_y[0] = g->ball_y;
    move_ball(g,dt,direction);
    if (g->state != ARK_PLAYING) return;
    if (g->power_active) {
        float old_y = g->power_y;
        g->power_y += 145.0f*dt;
        if (old_y <= PADDLE_Y+PADDLE_H && g->power_y+7 >= PADDLE_Y &&
            fabsf(g->power_x-g->paddle_x) <= half+14) {
            g->power_active = false; g->wide_ms = 12000; g->score += 100;
            half = arkanoid_paddle_width(g)/2;
            g->paddle_x = clamp(g->paddle_x,COURT_LEFT+half,COURT_RIGHT-half);
        } else if (g->power_y > COURT_BOTTOM) g->power_active = false;
    }
    if (g->ball_y-BALL_RADIUS > COURT_BOTTOM) {
        --g->lives; g->power_active = false; g->wide_ms = 0;
        serve(g);
        if (!g->lives) g->state = ARK_GAME_OVER;
    }
}

void arkanoid_update(Arkanoid *g, uint32_t now, uint32_t keys)
{
    uint32_t elapsed = now-g->last_ms;
    g->last_ms = now;
    uint32_t pressed = keys & ~g->previous_keys;
    g->previous_keys = keys;
    if (pressed & KEY_CENTER) {
        if (g->state == ARK_READY || g->state == ARK_GAME_OVER) {
            uint32_t best = g->best;
            arkanoid_init(g,now); g->best = best; g->previous_keys = keys;
            launch(g);
        } else if (g->state == ARK_LEVEL_CLEAR) {
            ++g->level; load_level(g); launch(g);
        } else if (g->state == ARK_SERVE) launch(g);
        else g->state = g->state == ARK_PAUSED ? ARK_PLAYING : ARK_PAUSED;
        g->accumulator = 0;
        return;
    }
    if (g->state == ARK_PAUSED || g->state == ARK_GAME_OVER || g->state == ARK_LEVEL_CLEAR)
        return;
    /* Fixed 5 ms simulation independent of rendering. Discard long stalls
     * (e.g. debugging) instead of teleporting the ball on resume. */
    g->accumulator += elapsed > 100 ? 100 : elapsed;
    while (g->accumulator >= 5) {
        g->accumulator -= 5;
        step(g,keys);
        if (g->state != ARK_PLAYING && g->state != ARK_SERVE && g->state != ARK_READY) {
            g->accumulator = 0; break;
        }
    }
    if (g->score > g->best) g->best = g->score;
}
