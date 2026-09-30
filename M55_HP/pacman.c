#include "pacman.h"
#include <string.h>
#include <stdlib.h>

static const char maze[MAZE_H][MAZE_W+1] = {
    "###################",
    "#o.......#.......o#",
    "#.##.###.#.###.##.#",
    "#.................#",
    "#.##.#.#####.#.##.#",
    "#....#...#...#....#",
    "####.###.#.###.####",
    "#....#.......#....#",
    "#.##.#.##.##.#.##.#",
    "#......#...#......#",
    "#.####.#...#.####.#",
    "#......#...#......#",
    ".......     .......",
    "#.####.#...#.####.#",
    "#......#...#......#",
    "#.##.#.#####.#.##.#",
    "#....#...#...#....#",
    "####.###.#.###.####",
    "#.................#",
    "#.##.###.#.###.##.#",
    "#o.#..... .....#.o#",
    "##.#.#.#####.#.#.##",
    "#....#...#...#....#",
    "#.................#",
    "###################"
};
static const int dx[4] = {-1,1,0,0}, dy[4] = {0,0,-1,1};
static Direction reverse(Direction d) { return (Direction)(d^1); }
bool pacman_wall(int x, int y)
{
    if (y == 12) { if (x < 0) x = MAZE_W-1; if (x >= MAZE_W) x = 0; }
    return x < 0 || x >= MAZE_W || y < 0 || y >= MAZE_H || maze[y][x] == '#';
}
static void score(Pacman *g, unsigned amount)
{
    g->score += amount;
    if (g->score > g->best) g->best = g->score;
}
static void actors(Pacman *g)
{
    g->player = (Actor){9*8+4,20*8+4,LEFT,0};
    for (int i = 0; i < 4; ++i)
        g->ghosts[i] = (Actor){(8+i%3)*8+4,(11+i/3)*8+4,UP,1500U+(unsigned)i*700U};
    g->wanted = LEFT; g->power_ms = 0; g->combo = 0; g->ghost_clock = 0;
}
static void board(Pacman *g)
{
    g->remaining = 0;
    for (int y = 0; y < MAZE_H; ++y)
        for (int x = 0; x < MAZE_W; ++x) {
            g->dots[y][x] = maze[y][x] == '.' ? 1 : maze[y][x] == 'o' ? 2 : 0;
            if (g->dots[y][x]) ++g->remaining;
        }
    actors(g);
}
void pacman_init(Pacman *g, uint32_t now)
{
    memset(g,0,sizeof(*g));
    g->last_ms = now; g->random = 0xA11F1234U;
    g->level = 1; g->lives = 3; g->state = GAME_READY;
    board(g);
}
static bool center(const Actor *a) { return a->x%8 == 4 && a->y%8 == 4; }
static bool can_move(const Actor *a, Direction d)
{
    return d < NONE && !pacman_wall(a->x/8+dx[d],a->y/8+dy[d]);
}
static void move(Actor *a)
{
    if (a->dir == NONE || (center(a) && !can_move(a,a->dir))) return;
    a->x += dx[a->dir]; a->y += dy[a->dir];
    if (a->x < 0) a->x += MAZE_W*8;
    if (a->x >= MAZE_W*8) a->x -= MAZE_W*8;
}
/* Breadth-first distances make ghost routes respect the maze walls. */
static void distances(int tx, int ty, int16_t dist[MAZE_H][MAZE_W])
{
    int queue[MAZE_W*MAZE_H], head = 0, tail = 0;
    memset(dist,0xFF,sizeof(int16_t)*MAZE_H*MAZE_W);
    if (pacman_wall(tx,ty)) return;
    queue[tail++] = ty*MAZE_W+tx; dist[ty][tx] = 0;
    while (head < tail) {
        int cell = queue[head++], x = cell%MAZE_W, y = cell/MAZE_W;
        for (int d = 0; d < 4; ++d) {
            int nx = x+dx[d], ny = y+dy[d];
            if (ny == 12) { if (nx < 0) nx = MAZE_W-1; if (nx == MAZE_W) nx = 0; }
            if (pacman_wall(nx,ny) || dist[ny][nx] >= 0) continue;
            dist[ny][nx] = dist[y][x]+1;
            queue[tail++] = ny*MAZE_W+nx;
        }
    }
}
static void steer(Pacman *g, unsigned index)
{
    Actor *a = &g->ghosts[index];
    int tx = g->player.x/8, ty = g->player.y/8;
    static const int corners[4][2] = {{17,1},{1,1},{17,23},{1,23}};
    bool scatter = (g->clock_ms%27000U < 6000U);
    if (index == 3 && abs(a->x-g->player.x)+abs(a->y-g->player.y) < 48) scatter = true;
    if (scatter) { tx = corners[index][0]; ty = corners[index][1]; }
    else if ((index == 1 || index == 2) && g->player.dir < NONE) {
        for (unsigned i = 0; i < index*2; ++i) {
            int nx = tx+dx[g->player.dir], ny = ty+dy[g->player.dir];
            if (nx < 0 || nx >= MAZE_W || pacman_wall(nx,ny)) break;
            tx = nx; ty = ny;
        }
    }
    int16_t dist[MAZE_H][MAZE_W];
    distances(tx,ty,dist);
    Direction chosen = NONE;
    int best = 100000;
    for (Direction d = LEFT; d < NONE; d = (Direction)(d+1)) {
        if (!can_move(a,d) || d == reverse(a->dir)) continue;
        int nx = (a->x/8+dx[d]+MAZE_W)%MAZE_W, ny = a->y/8+dy[d];
        int value = dist[ny][nx] < 0 ? 9999 : dist[ny][nx];
        if (g->power_ms) { g->random = g->random*1664525U+1013904223U; value = (int)(g->random>>16); }
        if (value < best) { best = value; chosen = d; }
    }
    if (chosen == NONE && can_move(a,reverse(a->dir))) chosen = reverse(a->dir);
    a->dir = chosen;
}
static void collide(Pacman *g)
{
    for (int i = 0; i < 4; ++i) {
        Actor *a = &g->ghosts[i];
        if (a->wait) continue;
        int x = abs(a->x-g->player.x), y = abs(a->y-g->player.y);
        if (x > MAZE_W*4) x = MAZE_W*8-x;
        if (x*x+y*y > 25) continue;
        if (g->power_ms) {
            score(g,200U << (g->combo < 3 ? g->combo : 3)); ++g->combo;
            *a = (Actor){9*8+4,11*8+4,UP,2500};
        } else {
            --g->lives; g->state = GAME_DYING; g->state_ms = 1400; return;
        }
    }
}
static void tick(Pacman *g)
{
    if (g->state == GAME_DYING || g->state == GAME_LEVEL_CLEAR) {
        if (g->state_ms > 20) { g->state_ms -= 20; return; }
        if (g->state == GAME_DYING) {
            if (!g->lives) { g->state = GAME_OVER; return; }
            actors(g);
        } else { ++g->level; board(g); }
        g->state = GAME_PLAYING; g->state_ms = 1000; return;
    }
    if (g->state != GAME_PLAYING) return;
    if (g->state_ms) { g->state_ms = g->state_ms > 20 ? g->state_ms-20 : 0; return; }
    g->clock_ms += 20;
    if (g->power_ms) g->power_ms = g->power_ms > 20 ? g->power_ms-20 : 0;
    Actor *p = &g->player;
    if ((center(p) && can_move(p,g->wanted)) || g->wanted == reverse(p->dir)) p->dir = g->wanted;
    move(p);
    if (center(p)) {
        uint8_t *dot = &g->dots[p->y/8][p->x/8];
        if (*dot) {
            score(g,*dot == 2 ? 50 : 10);
            if (*dot == 2) {
                g->power_ms = 6500; g->combo = 0;
                for (int i = 0; i < 4; ++i)
                    if (!g->ghosts[i].wait && g->ghosts[i].dir < NONE)
                        g->ghosts[i].dir = reverse(g->ghosts[i].dir);
            }
            *dot = 0;
            if (--g->remaining == 0) { g->state = GAME_LEVEL_CLEAR; g->state_ms = 1800; return; }
        }
    }
    collide(g);
    if (g->state != GAME_PLAYING) return;
    for (int i = 0; i < 4; ++i)
        if (g->ghosts[i].wait) g->ghosts[i].wait = g->ghosts[i].wait > 20 ? g->ghosts[i].wait-20 : 0;
    g->ghost_clock += 20;
    unsigned interval = g->power_ms ? 40 : g->level >= 5 ? 20 : 30-g->level*2;
    while (g->ghost_clock >= interval) {
        g->ghost_clock -= interval;
        for (unsigned i = 0; i < 4; ++i) {
            Actor *a = &g->ghosts[i];
            if (a->wait) continue;
            if (center(a)) steer(g,i);
            move(a);
        }
        collide(g);
        if (g->state != GAME_PLAYING) return;
    }
}
void pacman_update(Pacman *g, uint32_t now, uint32_t keys)
{
    uint32_t elapsed = now-g->last_ms;
    g->last_ms = now;
    uint32_t pressed = keys & ~g->keys;
    g->keys = keys;
    if (pressed & PAC_CENTER) {
        if (g->state == GAME_READY || g->state == GAME_OVER) {
            uint32_t best = g->best;
            pacman_init(g,now); g->best = best; g->keys = keys;
            g->state = GAME_PLAYING; g->state_ms = 1000;
        } else if (g->state == GAME_PLAYING) g->state = GAME_PAUSED;
        else if (g->state == GAME_PAUSED) g->state = GAME_PLAYING;
        g->accumulator = 0;
        return;
    }
    /* Retain a requested turn until the next legal intersection. */
    for (unsigned d = 0; d < 4; ++d)
        if (keys & (1U<<d)) { g->wanted = (Direction)d; break; }
    if (g->state == GAME_READY || g->state == GAME_PAUSED || g->state == GAME_OVER) return;
    if (elapsed > 250) elapsed = 250;
    g->accumulator += elapsed;
    while (g->accumulator >= 20) { g->accumulator -= 20; tick(g); }
}
