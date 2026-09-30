#include "starwars.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static StarWars crawl, other;
static uint16_t guarded[STARWARS_WIDTH*STARWARS_HEIGHT+2];

static uint32_t preview(const StarWars *s, const char *path)
{
    guarded[0] = 0x1234; guarded[STARWARS_WIDTH*STARWARS_HEIGHT+1] = 0x5678;
    memset(guarded+1,0xFF,STARWARS_WIDTH*STARWARS_HEIGHT*2);
    starwars_render(guarded+1,s);
    assert(guarded[0] == 0x1234 && guarded[STARWARS_WIDTH*STARWARS_HEIGHT+1] == 0x5678);
    uint32_t hash = 2166136261U;
    FILE *f = path ? fopen(path,"wb") : NULL;
    if (path) { assert(f); fprintf(f,"P6\n480 800\n255\n"); }
    for (unsigned i = 1; i <= STARWARS_WIDTH*STARWARS_HEIGHT; ++i) {
        uint16_t c = guarded[i];
        hash = (hash^c)*16777619U;
        if (f) {
            fputc(((c>>11)&31)*255/31,f); fputc(((c>>5)&63)*255/63,f); fputc((c&31)*255/31,f);
        }
    }
    if (f) fclose(f);
    return hash;
}

int main(void)
{
    assert(starwars_init(&crawl,"hello-\n",0));
    assert(crawl.lines == 1 && crawl.texture_height == 7);
    assert(starwars_init(&other,"HELLO-\n",0));
    assert(memcmp(crawl.texture,other.texture,sizeof(crawl.texture)) == 0);
    uint32_t short_duration = crawl.crawl_ms;
    assert(starwars_init(&crawl,"# TITLE\n\nA MUCH LONGER BODY THAT WRAPS ACROSS MULTIPLE LINES.\n",0));
    assert(crawl.lines == 4 && crawl.crawl_ms > short_duration);
    assert(crawl.texture[0][0] == 0); /* Centered title has a left margin. */
    assert(crawl.row_ink[12] == 0);   /* Explicit blank line preserved. */
    char word[71]; memset(word,'W',70); word[70] = 0;
    assert(starwars_init(&crawl,word,0));
    assert(crawl.lines == 3 && crawl.texture_height == 31);
    char many_lines[(CRAWL_MAX_LINES+1)*2+1];
    for (int i = 0; i <= CRAWL_MAX_LINES; ++i) { many_lines[i*2] = 'X'; many_lines[i*2+1] = '\n'; }
    many_lines[CRAWL_MAX_LINES*2] = 0;
    assert(starwars_init(&crawl,many_lines,0));
    assert(crawl.lines == CRAWL_MAX_LINES);
    many_lines[CRAWL_MAX_LINES*2] = 'X'; many_lines[(CRAWL_MAX_LINES+1)*2] = 0;
    assert(!starwars_init(&crawl,many_lines,0));

    assert(starwars_init(&other,"",0));
    assert(other.crawl_ms == 0 && other.cycle_ms > 0);
    uint32_t starfield = preview(&other,NULL);
    assert(starwars_init(&crawl,NULL,100));
    uint32_t first = preview(&crawl,"out/starwars-start.ppm");
    starwars_update(&crawl,18100);
    assert(preview(&crawl,"out/starwars-crawl.ppm") != first);
    starwars_update(&crawl,34100);
    preview(&crawl,"out/starwars-distant.ppm");
    /* Actual last text is gone before the restart pause begins. */
    crawl.phase_ms = crawl.crawl_ms-1;
    assert(preview(&crawl,NULL) == starfield);
    crawl.phase_ms = crawl.crawl_ms;
    assert(preview(&crawl,"out/starwars-gap.ppm") == starfield);
    assert(starwars_init(&crawl,NULL,100));
    starwars_update(&crawl,100+crawl.cycle_ms-1);
    assert(crawl.cycles == 0 && preview(&crawl,NULL) == starfield);
    starwars_update(&crawl,100+crawl.cycle_ms);
    assert(crawl.cycles == 1 && crawl.phase_ms == 0 && preview(&crawl,NULL) == first);
    uint32_t duration = crawl.cycle_ms;
    starwars_update(&crawl,100+duration*100+1234);
    assert(crawl.cycles == 100 && crawl.phase_ms == 1234);
    assert(starwars_init(&crawl,NULL,UINT32_MAX-10U));
    starwars_update(&crawl,20);
    assert(crawl.phase_ms == 31);
    /* Sweep a full cycle, checking the renderer at varied perspective depths. */
    for (uint32_t phase = 0; phase < duration; phase += 1700) {
        crawl.phase_ms = phase;
        preview(&crawl,NULL);
    }
    printf("PASS: wrapping, headings, case, long words, capacity, empty text, perspective, final fade, exact repeat, 100 cycles, timer wrap, framebuffer guards. Default loop: %.1f s\n",duration/1000.0);
    return 0;
}
