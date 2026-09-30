#ifndef CRAWL_TEXT_H
#define CRAWL_TEXT_H

/* EDIT THIS TEXT, then rebuild and load the application.
 * Words wrap automatically at 28 columns. Newlines force line breaks;
 * an empty line separates paragraphs. Prefix a heading with # to center it.
 * Lowercase is displayed as uppercase. Maximum: 96 wrapped lines.
 */
static const char CRAWL_TEXT[] =
    "# EPISODE I\n"
    "\n"
    "# A NEW SIGNAL\n"
    "\n"
    "Beyond the familiar stars, a small explorer awakens.\n"
    "\n"
    "Across the silent galaxy, its beacon carries a message of curiosity, "
    "courage, and possibility.\n"
    "\n"
    "Powered by a tiny core, the journey begins one pixel at a time.\n"
    "\n"
    "New worlds wait beyond the horizon. The next chapter is yours to write.\n"
    "\n"
    "# MAY THE SOURCE BE WITH YOU.\n";

#define CRAWL_SPEED 32.0f      /* Plane units per second; must be positive. */
#define CRAWL_RESTART_MS 1400U /* Starfield pause AFTER the final text fades. */
#endif
