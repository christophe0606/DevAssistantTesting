/* CMSIS MCP orchestration only. Read into functions.exec and evaluate there.
 * All 146 individual checks are complete. Loading makes no board call.
 * Retain passing results; refresh symbol addresses after rebuilding. */
const checkpointState = {
  "xs_decode_words": "r => { const t=r.content.filter(x=>x.type===\"text\").map(x=>x.text).join(\"\\n\");const b=[...t.matchAll(/0x[0-9a-f]+: ([0-9a-f ]+)/gi)].flatMap(m=>m[1].trim().split(/\\s+/).map(x=>parseInt(x,16)));return Array.from({length:b.length/4},(_,i)=>(b[i*4]+b[i*4+1]*256+b[i*4+2]*65536+b[i*4+3]*16777216)>>>0); }",
  "xs_names": [
    "attraction",
    "blitspin",
    "bouboule",
    "braid",
    "decayscreen",
    "deco",
    "drift",
    "flame",
    "galaxy",
    "grav",
    "greynetic",
    "halo",
    "helix",
    "hopalong",
    "ifs",
    "imsmap",
    "julia",
    "kaleidescope",
    "maze",
    "moire",
    "noseguy",
    "pedal",
    "penrose",
    "pyro",
    "qix",
    "rocks",
    "rorschach",
    "sierpinski",
    "slidescreen",
    "slip",
    "strange",
    "swirl",
    "goop",
    "starfish",
    "munch",
    "fadeplot",
    "rdbomb",
    "coral",
    "mountain",
    "triangle",
    "xjack",
    "xlyap",
    "cynosure",
    "moire2",
    "flow",
    "epicycle",
    "interference",
    "truchet",
    "bsod",
    "crystal",
    "discrete",
    "distort",
    "kumppa",
    "demon",
    "loop",
    "penetrate",
    "deluxe",
    "compass",
    "squiral",
    "xflame",
    "wander",
    "spotlight",
    "phosphor",
    "xmatrix",
    "petri",
    "shadebobs",
    "ccurve",
    "blaster",
    "bumps",
    "ripples",
    "xspirograph",
    "nerverot",
    "xrayswarm",
    "zoom",
    "whirlwindwarp",
    "rotzoomer",
    "speedmine",
    "vermiculate",
    "twang",
    "apollonian",
    "euler2d",
    "polyominoes",
    "fluidballs",
    "anemone",
    "halftone",
    "metaballs",
    "eruption",
    "popsquares",
    "barcode",
    "piecewise",
    "cloudlife",
    "fontglide",
    "apple2",
    "xanalogtv",
    "pong",
    "filmleader",
    "wormhole",
    "pacman",
    "fuzzyflakes",
    "anemotaxis",
    "memscroller",
    "substrate",
    "intermomentary",
    "fireworkx",
    "fiberlamp",
    "boxfit",
    "interaggregate",
    "celtic",
    "cwaves",
    "m6502",
    "abstractile",
    "lcdscrub",
    "hexadrop",
    "tessellimage",
    "binaryring",
    "glitchpeg",
    "vfeedback",
    "scooter",
    "marbling",
    "binaryhorizon",
    "droste",
    "ant",
    "bubbles",
    "critical",
    "flag",
    "forest",
    "hyperball",
    "hypercube",
    "laser",
    "lightning",
    "lisa",
    "lissie",
    "lmorph",
    "rotor",
    "sphere",
    "spiral",
    "t3d",
    "vines",
    "whirlygig",
    "worm",
    "xsublim",
    "juggle",
    "thornbird",
    "mismunch",
    "webcollage",
    "vidwhacker"
  ],
  "xs_board_addresses": {
    "ms_ticks": "0x2000a258",
    "app_stage": "0x2000a25c",
    "xs_last_error": "0x2000a4d8",
    "xs_current_saver": "0x2000a4ec",
    "xs_memory_peak": "0x2000a40c",
    "started": "0x2000a4f4",
    "xs_cycle_enabled": "0x2000001c",
    "xs_requested_saver": "0x20000020",
    "display_events": "0x2000a260",
    "app_error": "0x2000a264",
    "app_service_error": "0x2000a268",
    "frames_presented": "0x2000a26c",
    "xs_failures": "0x2000a4e0",
    "next_frame": "0x2000a4f0"
  },
  "xs_board_control_build": "b-41",
  "xs_main_test_line": 158,
  "xs_individual_results": [
    {
      "index": 20,
      "name": "noseguy",
      "passed": true,
      "elapsed_ms": 3066,
      "frames": 93,
      "peak_bytes": 149584,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:02:37 UTC",
      "build": "b-32"
    },
    {
      "index": 21,
      "name": "pedal",
      "passed": true,
      "elapsed_ms": 3113,
      "frames": 93,
      "peak_bytes": 6880,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:07 UTC",
      "build": "b-32"
    },
    {
      "index": 22,
      "name": "penrose",
      "passed": true,
      "elapsed_ms": 3201,
      "frames": 96,
      "peak_bytes": 10832,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:12 UTC",
      "build": "b-32"
    },
    {
      "index": 23,
      "name": "pyro",
      "passed": true,
      "elapsed_ms": 3368,
      "frames": 102,
      "peak_bytes": 84112,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:18 UTC",
      "build": "b-32"
    },
    {
      "index": 24,
      "name": "qix",
      "passed": true,
      "elapsed_ms": 3068,
      "frames": 93,
      "peak_bytes": 52496,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:23 UTC",
      "build": "b-32"
    },
    {
      "index": 25,
      "name": "rocks",
      "passed": true,
      "elapsed_ms": 3262,
      "frames": 99,
      "peak_bytes": 269328,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:29 UTC",
      "build": "b-32"
    },
    {
      "index": 26,
      "name": "rorschach",
      "passed": true,
      "elapsed_ms": 3074,
      "frames": 93,
      "peak_bytes": 2640,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:33 UTC",
      "build": "b-32"
    },
    {
      "index": 27,
      "name": "sierpinski",
      "passed": true,
      "elapsed_ms": 3063,
      "frames": 92,
      "peak_bytes": 39136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:38 UTC",
      "build": "b-32"
    },
    {
      "index": 28,
      "name": "slidescreen",
      "passed": true,
      "elapsed_ms": 3047,
      "frames": 92,
      "peak_bytes": 150608,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:42 UTC",
      "build": "b-32"
    },
    {
      "index": 29,
      "name": "slip",
      "passed": true,
      "elapsed_ms": 3144,
      "frames": 27,
      "peak_bytes": 542352,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:47 UTC",
      "build": "b-32"
    },
    {
      "index": 30,
      "name": "strange",
      "passed": true,
      "elapsed_ms": 3075,
      "frames": 91,
      "peak_bytes": 125344,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:52 UTC",
      "build": "b-32"
    },
    {
      "index": 31,
      "name": "swirl",
      "passed": true,
      "elapsed_ms": 3084,
      "frames": 90,
      "peak_bytes": 394912,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:03:57 UTC",
      "build": "b-32"
    },
    {
      "index": 32,
      "name": "goop",
      "passed": true,
      "elapsed_ms": 3237,
      "frames": 40,
      "peak_bytes": 534960,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:04:02 UTC",
      "build": "b-32"
    },
    {
      "index": 33,
      "name": "starfish",
      "passed": true,
      "elapsed_ms": 3118,
      "frames": 94,
      "peak_bytes": 9136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:04:07 UTC",
      "build": "b-32"
    },
    {
      "index": 34,
      "name": "munch",
      "passed": true,
      "elapsed_ms": 3173,
      "frames": 96,
      "peak_bytes": 576,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:04:11 UTC",
      "build": "b-32"
    },
    {
      "index": 35,
      "name": "fadeplot",
      "passed": true,
      "elapsed_ms": 3071,
      "frames": 92,
      "peak_bytes": 22400,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:04:16 UTC",
      "build": "b-32"
    },
    {
      "index": 36,
      "name": "rdbomb",
      "passed": true,
      "elapsed_ms": 3230,
      "frames": 22,
      "peak_bytes": 1244336,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:04:20 UTC",
      "build": "b-32"
    },
    {
      "index": 37,
      "name": "coral",
      "build": "b-34",
      "passed": true,
      "elapsed_ms": 2753,
      "frames": 82,
      "peak_bytes": 113072,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:10:17 UTC"
    },
    {
      "index": 38,
      "name": "mountain",
      "build": "b-34",
      "passed": true,
      "elapsed_ms": 3059,
      "frames": 91,
      "peak_bytes": 325680,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:10:35 UTC"
    },
    {
      "index": 39,
      "name": "triangle",
      "build": "b-37",
      "passed": true,
      "elapsed_ms": 2780,
      "frames": 82,
      "peak_bytes": 2324256,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:15:37 UTC"
    },
    {
      "index": 40,
      "name": "xjack",
      "build": "b-37",
      "passed": true,
      "elapsed_ms": 3069,
      "frames": 92,
      "peak_bytes": 3344,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:16:35 UTC"
    },
    {
      "index": 41,
      "name": "xlyap",
      "build": "b-37",
      "passed": true,
      "elapsed_ms": 3206,
      "frames": 7,
      "peak_bytes": 1828352,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:17:14 UTC"
    },
    {
      "index": 42,
      "name": "cynosure",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3476,
      "frames": 103,
      "peak_bytes": 3472,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:19:54 UTC"
    },
    {
      "index": 43,
      "name": "moire2",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3164,
      "frames": 6,
      "peak_bytes": 110320,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:26 UTC"
    },
    {
      "index": 44,
      "name": "flow",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3332,
      "frames": 47,
      "peak_bytes": 398320,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:31 UTC"
    },
    {
      "index": 45,
      "name": "epicycle",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3098,
      "frames": 93,
      "peak_bytes": 1824,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:36 UTC"
    },
    {
      "index": 46,
      "name": "interference",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3271,
      "frames": 50,
      "peak_bytes": 396720,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:41 UTC"
    },
    {
      "index": 47,
      "name": "truchet",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3074,
      "frames": 68,
      "peak_bytes": 2048464,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:46 UTC"
    },
    {
      "index": 48,
      "name": "bsod",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3350,
      "frames": 26,
      "peak_bytes": 1738016,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:51 UTC"
    },
    {
      "index": 49,
      "name": "crystal",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3081,
      "frames": 47,
      "peak_bytes": 41008,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:20:55 UTC"
    },
    {
      "index": 50,
      "name": "discrete",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3168,
      "frames": 39,
      "peak_bytes": 25248,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:00 UTC"
    },
    {
      "index": 51,
      "name": "distort",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3038,
      "frames": 85,
      "peak_bytes": 852608,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:05 UTC"
    },
    {
      "index": 52,
      "name": "kumppa",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3179,
      "frames": 58,
      "peak_bytes": 5584,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:10 UTC"
    },
    {
      "index": 53,
      "name": "demon",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3194,
      "frames": 95,
      "peak_bytes": 10032,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:16 UTC"
    },
    {
      "index": 54,
      "name": "loop",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3161,
      "frames": 94,
      "peak_bytes": 37696,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:21 UTC"
    },
    {
      "index": 55,
      "name": "penetrate",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3047,
      "frames": 91,
      "peak_bytes": 52320,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:25 UTC"
    },
    {
      "index": 56,
      "name": "deluxe",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3159,
      "frames": 33,
      "peak_bytes": 1136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:31 UTC"
    },
    {
      "index": 57,
      "name": "compass",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3327,
      "frames": 68,
      "peak_bytes": 640,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:36 UTC"
    },
    {
      "index": 58,
      "name": "squiral",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3144,
      "frames": 94,
      "peak_bytes": 387488,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:41 UTC"
    },
    {
      "index": 59,
      "name": "xflame",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3130,
      "frames": 64,
      "peak_bytes": 414144,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:46 UTC"
    },
    {
      "index": 60,
      "name": "wander",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3084,
      "frames": 92,
      "peak_bytes": 3312,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:51 UTC"
    },
    {
      "index": 61,
      "name": "spotlight",
      "build": "b-39",
      "passed": true,
      "elapsed_ms": 3050,
      "frames": 49,
      "peak_bytes": 542480,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 08:21:55 UTC"
    },
    {
      "index": 62,
      "name": "phosphor",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3106,
      "frames": 95,
      "peak_bytes": 263328,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:28:29 UTC"
    },
    {
      "index": 63,
      "name": "xmatrix",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 2213,
      "frames": 68,
      "peak_bytes": 1450928,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:29:12 UTC"
    },
    {
      "index": 64,
      "name": "petri",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3965,
      "frames": 121,
      "peak_bytes": 579552,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:29:20 UTC"
    },
    {
      "index": 65,
      "name": "shadebobs",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3238,
      "frames": 99,
      "peak_bytes": 392896,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:29:26 UTC"
    },
    {
      "index": 66,
      "name": "ccurve",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3414,
      "frames": 94,
      "peak_bytes": 387680,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:29:33 UTC"
    },
    {
      "index": 67,
      "name": "blaster",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3059,
      "frames": 64,
      "peak_bytes": 2256,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:03 UTC"
    },
    {
      "index": 68,
      "name": "bumps",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3173,
      "frames": 96,
      "peak_bytes": 1024800,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:08 UTC"
    },
    {
      "index": 69,
      "name": "ripples",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3562,
      "frames": 56,
      "peak_bytes": 939568,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:15 UTC"
    },
    {
      "index": 70,
      "name": "xspirograph",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 4784,
      "frames": 145,
      "peak_bytes": 3056,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:22 UTC"
    },
    {
      "index": 71,
      "name": "nerverot",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3386,
      "frames": 53,
      "peak_bytes": 121728,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:36 UTC"
    },
    {
      "index": 72,
      "name": "xrayswarm",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3594,
      "frames": 109,
      "peak_bytes": 68336,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:44 UTC"
    },
    {
      "index": 73,
      "name": "zoom",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3410,
      "frames": 71,
      "peak_bytes": 534656,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:50 UTC"
    },
    {
      "index": 74,
      "name": "whirlwindwarp",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3599,
      "frames": 109,
      "peak_bytes": 420704,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:31:56 UTC"
    },
    {
      "index": 75,
      "name": "rotzoomer",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3273,
      "frames": 52,
      "peak_bytes": 1152624,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:07 UTC"
    },
    {
      "index": 76,
      "name": "speedmine",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3539,
      "frames": 43,
      "peak_bytes": 280128,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:13 UTC"
    },
    {
      "index": 77,
      "name": "vermiculate",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3635,
      "frames": 111,
      "peak_bytes": 339296,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:19 UTC"
    },
    {
      "index": 78,
      "name": "twang",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3685,
      "frames": 38,
      "peak_bytes": 1152928,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:25 UTC"
    },
    {
      "index": 79,
      "name": "apollonian",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3005,
      "frames": 91,
      "peak_bytes": 15824,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:35 UTC"
    },
    {
      "index": 80,
      "name": "euler2d",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3231,
      "frames": 50,
      "peak_bytes": 223344,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:41 UTC"
    },
    {
      "index": 81,
      "name": "polyominoes",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3253,
      "frames": 97,
      "peak_bytes": 72432,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:48 UTC"
    },
    {
      "index": 82,
      "name": "fluidballs",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 3169,
      "frames": 50,
      "peak_bytes": 11104,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:32:55 UTC"
    },
    {
      "index": 83,
      "name": "anemone",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7436,
      "frames": 126,
      "peak_bytes": 335520,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:34:01 UTC"
    },
    {
      "index": 84,
      "name": "halftone",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8406,
      "frames": 121,
      "peak_bytes": 7248,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:34:12 UTC"
    },
    {
      "index": 85,
      "name": "metaballs",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7462,
      "frames": 93,
      "peak_bytes": 497600,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:34:22 UTC"
    },
    {
      "index": 86,
      "name": "eruption",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7346,
      "frames": 114,
      "peak_bytes": 538976,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:34:35 UTC"
    },
    {
      "index": 87,
      "name": "popsquares",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7249,
      "frames": 150,
      "peak_bytes": 2256,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:34:50 UTC"
    },
    {
      "index": 88,
      "name": "barcode",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7562,
      "frames": 223,
      "peak_bytes": 1464944,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:35:01 UTC"
    },
    {
      "index": 89,
      "name": "piecewise",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7206,
      "frames": 75,
      "peak_bytes": 20800,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:35:11 UTC"
    },
    {
      "index": 90,
      "name": "cloudlife",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7474,
      "frames": 227,
      "peak_bytes": 266608,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:35:21 UTC"
    },
    {
      "index": 91,
      "name": "fontglide",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7206,
      "frames": 149,
      "peak_bytes": 21904,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:35:36 UTC"
    },
    {
      "index": 92,
      "name": "apple2",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7282,
      "frames": 57,
      "peak_bytes": 1728768,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:35:46 UTC"
    },
    {
      "index": 93,
      "name": "xanalogtv",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6670,
      "frames": 78,
      "peak_bytes": 3014720,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:35:55 UTC"
    },
    {
      "index": 94,
      "name": "pong",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7047,
      "frames": 63,
      "peak_bytes": 1692384,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:36:06 UTC"
    },
    {
      "index": 95,
      "name": "filmleader",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7140,
      "frames": 28,
      "peak_bytes": 3105136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:36:23 UTC"
    },
    {
      "index": 96,
      "name": "wormhole",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7076,
      "frames": 146,
      "peak_bytes": 109808,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:36:33 UTC"
    },
    {
      "index": 97,
      "name": "pacman",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6677,
      "frames": 203,
      "peak_bytes": 2137136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:36:42 UTC"
    },
    {
      "index": 98,
      "name": "fuzzyflakes",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7023,
      "frames": 145,
      "peak_bytes": 1216,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:36:52 UTC"
    },
    {
      "index": 99,
      "name": "anemotaxis",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7263,
      "frames": 150,
      "peak_bytes": 2320,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:37:05 UTC"
    },
    {
      "index": 100,
      "name": "memscroller",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7456,
      "frames": 207,
      "peak_bytes": 67200,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:37:15 UTC"
    },
    {
      "index": 101,
      "name": "substrate",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7208,
      "frames": 219,
      "peak_bytes": 769040,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:37:24 UTC"
    },
    {
      "index": 102,
      "name": "intermomentary",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7348,
      "frames": 152,
      "peak_bytes": 145840,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:37:34 UTC"
    },
    {
      "index": 103,
      "name": "fireworkx",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6293,
      "frames": 73,
      "peak_bytes": 1196528,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:37:51 UTC"
    },
    {
      "index": 104,
      "name": "fiberlamp",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6971,
      "frames": 40,
      "peak_bytes": 610736,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:38:00 UTC"
    },
    {
      "index": 105,
      "name": "boxfit",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7283,
      "frames": 197,
      "peak_bytes": 25760,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:38:10 UTC"
    },
    {
      "index": 106,
      "name": "interaggregate",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7398,
      "frames": 82,
      "peak_bytes": 401232,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:38:21 UTC"
    },
    {
      "index": 107,
      "name": "celtic",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7019,
      "frames": 65,
      "peak_bytes": 33008,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:38:35 UTC"
    },
    {
      "index": 108,
      "name": "cwaves",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7077,
      "frames": 146,
      "peak_bytes": 7744,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:38:46 UTC"
    },
    {
      "index": 109,
      "name": "m6502",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7146,
      "frames": 56,
      "peak_bytes": 1844944,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:38:55 UTC"
    },
    {
      "index": 110,
      "name": "abstractile",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7142,
      "frames": 216,
      "peak_bytes": 2710096,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:39:05 UTC"
    },
    {
      "index": 111,
      "name": "lcdscrub",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7121,
      "frames": 189,
      "peak_bytes": 336,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:39:18 UTC"
    },
    {
      "index": 112,
      "name": "hexadrop",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7486,
      "frames": 98,
      "peak_bytes": 20608,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:39:30 UTC"
    },
    {
      "index": 113,
      "name": "tessellimage",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8043,
      "frames": 73,
      "peak_bytes": 685568,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:39:42 UTC"
    },
    {
      "index": 114,
      "name": "binaryring",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6925,
      "frames": 62,
      "peak_bytes": 1312352,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:39:52 UTC"
    },
    {
      "index": 115,
      "name": "glitchpeg",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7106,
      "frames": 74,
      "peak_bytes": 592192,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:40:05 UTC"
    },
    {
      "index": 116,
      "name": "vfeedback",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6863,
      "frames": 24,
      "peak_bytes": 3685440,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:40:14 UTC"
    },
    {
      "index": 117,
      "name": "scooter",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6910,
      "frames": 143,
      "peak_bytes": 179744,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:40:23 UTC"
    },
    {
      "index": 118,
      "name": "marbling",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7015,
      "frames": 28,
      "peak_bytes": 387408,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:40:33 UTC"
    },
    {
      "index": 119,
      "name": "binaryhorizon",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7405,
      "frames": 77,
      "peak_bytes": 1312384,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:40:50 UTC"
    },
    {
      "index": 120,
      "name": "droste",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8927,
      "frames": 80,
      "peak_bytes": 3481152,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:41:02 UTC"
    },
    {
      "index": 121,
      "name": "ant",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7350,
      "frames": 223,
      "peak_bytes": 19664,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:41:14 UTC"
    },
    {
      "index": 122,
      "name": "bubbles",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8171,
      "frames": 244,
      "peak_bytes": 81552,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:41:25 UTC"
    },
    {
      "index": 123,
      "name": "critical",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7040,
      "frames": 214,
      "peak_bytes": 22656,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:41:41 UTC"
    },
    {
      "index": 124,
      "name": "flag",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7138,
      "frames": 111,
      "peak_bytes": 57712,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:41:51 UTC"
    },
    {
      "index": 125,
      "name": "forest",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6989,
      "frames": 212,
      "peak_bytes": 4752,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:42:01 UTC"
    },
    {
      "index": 126,
      "name": "hyperball",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7044,
      "frames": 110,
      "peak_bytes": 12368,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:42:12 UTC"
    },
    {
      "index": 127,
      "name": "hypercube",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7202,
      "frames": 149,
      "peak_bytes": 1152,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:42:26 UTC"
    },
    {
      "index": 128,
      "name": "laser",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7188,
      "frames": 217,
      "peak_bytes": 12544,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:42:36 UTC"
    },
    {
      "index": 129,
      "name": "lightning",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7168,
      "frames": 210,
      "peak_bytes": 29680,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:42:46 UTC"
    },
    {
      "index": 130,
      "name": "lisa",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7413,
      "frames": 115,
      "peak_bytes": 11600,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:42:55 UTC"
    },
    {
      "index": 131,
      "name": "lissie",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7295,
      "frames": 221,
      "peak_bytes": 7616,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:43:09 UTC"
    },
    {
      "index": 132,
      "name": "lmorph",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8172,
      "frames": 202,
      "peak_bytes": 14000,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:43:23 UTC"
    },
    {
      "index": 133,
      "name": "rotor",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8274,
      "frames": 251,
      "peak_bytes": 8928,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:43:34 UTC"
    },
    {
      "index": 134,
      "name": "sphere",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7350,
      "frames": 223,
      "peak_bytes": 7040,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:43:43 UTC"
    },
    {
      "index": 135,
      "name": "spiral",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7348,
      "frames": 223,
      "peak_bytes": 12576,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:43:57 UTC"
    },
    {
      "index": 136,
      "name": "t3d",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7056,
      "frames": 88,
      "peak_bytes": 902208,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:44:06 UTC"
    },
    {
      "index": 137,
      "name": "vines",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7218,
      "frames": 213,
      "peak_bytes": 5792,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:44:16 UTC"
    },
    {
      "index": 138,
      "name": "whirlygig",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7537,
      "frames": 229,
      "peak_bytes": 121696,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:44:26 UTC"
    },
    {
      "index": 139,
      "name": "worm",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7168,
      "frames": 218,
      "peak_bytes": 42208,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:44:41 UTC"
    },
    {
      "index": 140,
      "name": "xsublim",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7342,
      "frames": 182,
      "peak_bytes": 2736,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:44:51 UTC"
    },
    {
      "index": 141,
      "name": "juggle",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6783,
      "frames": 57,
      "peak_bytes": 476096,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:45:01 UTC"
    },
    {
      "index": 142,
      "name": "thornbird",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6858,
      "frames": 208,
      "peak_bytes": 176560,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:45:11 UTC"
    },
    {
      "index": 143,
      "name": "mismunch",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7434,
      "frames": 226,
      "peak_bytes": 576,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:45:25 UTC"
    },
    {
      "index": 144,
      "name": "webcollage",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7168,
      "frames": 218,
      "peak_bytes": 150432,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:45:35 UTC"
    },
    {
      "index": 145,
      "name": "vidwhacker",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7132,
      "frames": 214,
      "peak_bytes": 150432,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:45:45 UTC"
    },
    {
      "index": 0,
      "name": "attraction",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7238,
      "frames": 219,
      "peak_bytes": 912,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:45:55 UTC"
    },
    {
      "index": 1,
      "name": "blitspin",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6955,
      "frames": 166,
      "peak_bytes": 712192,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:46:09 UTC"
    },
    {
      "index": 2,
      "name": "bouboule",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 6962,
      "frames": 144,
      "peak_bytes": 24384,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:46:19 UTC"
    },
    {
      "index": 3,
      "name": "braid",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7387,
      "frames": 150,
      "peak_bytes": 112192,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:46:30 UTC"
    },
    {
      "index": 4,
      "name": "decayscreen",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7141,
      "frames": 216,
      "peak_bytes": 384224,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:46:40 UTC"
    },
    {
      "index": 5,
      "name": "deco",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7197,
      "frames": 212,
      "peak_bytes": 3312,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:46:54 UTC"
    },
    {
      "index": 6,
      "name": "drift",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 8138,
      "frames": 237,
      "peak_bytes": 85136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:47:05 UTC"
    },
    {
      "index": 7,
      "name": "flame",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7588,
      "frames": 197,
      "peak_bytes": 1232,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:47:15 UTC"
    },
    {
      "index": 8,
      "name": "galaxy",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7217,
      "frames": 90,
      "peak_bytes": 524768,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:47:29 UTC"
    },
    {
      "index": 9,
      "name": "grav",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7221,
      "frames": 219,
      "peak_bytes": 6544,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:47:45 UTC"
    },
    {
      "index": 10,
      "name": "greynetic",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7413,
      "frames": 221,
      "peak_bytes": 2224,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:47:55 UTC"
    },
    {
      "index": 11,
      "name": "halo",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7362,
      "frames": 69,
      "peak_bytes": 25824,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:48:06 UTC"
    },
    {
      "index": 12,
      "name": "helix",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7389,
      "frames": 224,
      "peak_bytes": 16000,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:48:16 UTC"
    },
    {
      "index": 13,
      "name": "hopalong",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7355,
      "frames": 223,
      "peak_bytes": 18880,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:48:30 UTC"
    },
    {
      "index": 14,
      "name": "ifs",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 9025,
      "frames": 269,
      "peak_bytes": 23664,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:48:43 UTC"
    },
    {
      "index": 15,
      "name": "imsmap",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7492,
      "frames": 225,
      "peak_bytes": 97120,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:48:53 UTC"
    },
    {
      "index": 16,
      "name": "julia",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7005,
      "frames": 210,
      "peak_bytes": 353744,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:49:02 UTC"
    },
    {
      "index": 17,
      "name": "kaleidescope",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7256,
      "frames": 220,
      "peak_bytes": 101280,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:49:16 UTC"
    },
    {
      "index": 18,
      "name": "maze",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7150,
      "frames": 217,
      "peak_bytes": 181136,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:49:25 UTC"
    },
    {
      "index": 19,
      "name": "moire",
      "build": "b-41",
      "passed": true,
      "elapsed_ms": 7020,
      "frames": 213,
      "peak_bytes": 20352,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "utc": "2026-10-01 12:49:34 UTC"
    }
  ],
  "xs_individual_pending_index": null,
  "xs_individual_complete": true,
  "xs_board_samples": [],
  "xs_read_state": "async () => {\nconst s=await tools.mcp__cmsis_developer_assistant__get_session_status({});\nconst status=s.content.filter(c=>c.type===\"text\").map(c=>c.text).join(\"\\n\");\nif(!status.includes(\"State: running\")&&!status.includes(\"State: stopped\")){text(s);return false;}\nif(status.includes(\"State: running\")){\n const p=await tools.mcp__cmsis_developer_assistant__pause_execution({timeoutMs:10000});\n if(p.isError){text(p);return false;}\n}\nconst d=eval(load(\"xs_decode_words\"));\nconst r1=await tools.mcp__cmsis_developer_assistant__read_memory({address:load(\"xs_board_addresses\").ms_ticks,length:24});\nif(r1.isError){text(r1);return false;}\nconst r2=await tools.mcp__cmsis_developer_assistant__read_memory({address:load(\"xs_board_addresses\").xs_last_error,length:32});\nif(r2.isError){text(r2);return false;}\nconst r3=await tools.mcp__cmsis_developer_assistant__read_memory({address:load(\"xs_board_addresses\").xs_memory_peak,length:4});if(r3.isError){text(r3);return false;}const a=d(r1),b=d(r2),peak=d(r3)[0];\nconst fault=await tools.mcp__cmsis_developer_assistant__get_fault_info({});\nif(fault.isError){text(fault);return false;}\nconst faultText=fault.content.filter(c=>c.type===\"text\").map(c=>c.text).join(\"\\n\");\nconst faultStatus=Object.fromEntries([\"HFSR\",\"MMFSR\",\"BFSR\",\"UFSR\"].map(n=>[n,parseInt(faultText.match(new RegExp(n+\"\\\\s*=\\\\s*0x([0-9a-f]+)\",\"i\"))?.[1]??\"ffffffff\",16)]));\nconst sample={utc:(await tools.clock__curr_time({})).current_time,fault_status:faultStatus,ms_ticks:a[0],app_stage:a[1],display_events:a[2],app_error:a[3],app_service_error:a[4],frames_presented:a[5],xs_last_error:b[0],xs_failures:b[2],xs_current_saver:b[5],next_frame:b[6],started:b[7],xs_memory_peak:peak};\nconst samples=load(\"xs_board_samples\"); const last=samples.at(-1); samples.push(sample);store(\"xs_board_samples\",samples);\nif(Object.values(faultStatus).some(v=>v!==0)||sample.app_stage!==8||sample.app_error||sample.app_service_error||sample.display_events||sample.xs_failures){text(\"Inspection needed; left halted.\");return false;}\nreturn sample;\n}",
  "xs_test_one": "async index => {\n const file=\"C:/Users/chrfav01/benchresults/TEMP/testassistant/Blinky_M55_HP/M55_HP/main.c\";\n const selected=await tools.mcp__cmsis_developer_assistant__evaluate_expression({expression:\"(xs_cycle_enabled = 0, xs_requested_saver = \"+index+\", frames_presented)\",timeoutMs:5000});\n if(selected.isError){text(selected);return false;}\n const frames=parseInt(selected.content[0].text.match(/Result: (\\d+)/)?.[1]);\n store(\"xs_test_baseline\",{index,frames});if(!Number.isFinite(frames)){text(selected);return false;}\n const removed=await tools.mcp__cmsis_developer_assistant__clear_all_breakpoints({});\n if(removed.isError){text(removed);return false;}\n const continued=await tools.mcp__cmsis_developer_assistant__continue_execution({timeoutMs:5000});\n if(continued.isError){text(continued);return false;}\n await new Promise(resolve=>setTimeout(resolve,2000));\n const added=await tools.mcp__cmsis_developer_assistant__add_breakpoint({fileFullPath:file,line:load(\"xs_main_test_line\")});\n if(added.isError){text(added);return false;}\n const stop=await tools.mcp__cmsis_developer_assistant__wait_for_stop({timeoutMs:5000});\n if(stop.isError){text(stop);return false;}\n if(!stop.content.some(c=>c.text?.includes('\"currentLine\": '+load(\"xs_main_test_line\")))){\n  const status=await tools.mcp__cmsis_developer_assistant__get_session_status({});text(status);\n  if(status.content.some(c=>c.text?.includes(\"State: running\"))){\n   const paused=await tools.mcp__cmsis_developer_assistant__pause_execution({timeoutMs:10000});\n   if(paused.isError){text(paused);return false;}\n  }\n\n }\n let sample=await eval(load(\"xs_read_state\"))();\n if(!sample)return false;\n for(let extra=0;extra<2 && (sample.frames_presented-frames<2 || ((sample.ms_ticks-sample.started)>>>0)<2000);extra++){\n  const clear=await tools.mcp__cmsis_developer_assistant__clear_all_breakpoints({});if(clear.isError){text(clear);return false;}\n  const more=await tools.mcp__cmsis_developer_assistant__continue_execution({timeoutMs:5000});if(more.isError){text(more);return false;}\n  await new Promise(resolve=>setTimeout(resolve,1000));\n  const stopAt=await tools.mcp__cmsis_developer_assistant__add_breakpoint({fileFullPath:file,line:load(\"xs_main_test_line\")});if(stopAt.isError){text(stopAt);return false;}\n  const stopped=await tools.mcp__cmsis_developer_assistant__wait_for_stop({timeoutMs:5000});if(stopped.isError){text(stopped);return false;}\n  sample=await eval(load(\"xs_read_state\"))();if(!sample)return false;\n }\n const result={index,name:load(\"xs_names\")[index],build:load(\"xs_board_control_build\"),passed:sample.xs_current_saver===index&&sample.frames_presented-frames>=2 && ((sample.ms_ticks-sample.started)>>>0)>=2000 && sample.xs_last_error===0,elapsed_ms:(sample.ms_ticks-sample.started)>>>0,frames:sample.frames_presented-frames,peak_bytes:sample.xs_memory_peak,fault_status:sample.fault_status,utc:sample.utc};\n const results=load(\"xs_individual_results\");results.push(result);store(\"xs_individual_results\",results);text(result);\n return result.passed;\n}",
  "xs_board_test_runner": "async () => {\nstore(\"xs_individual_complete\",false);\nconst passed=new Set(load(\"xs_individual_results\").filter(r=>r.passed).map(r=>r.index));\nconst order=[...Array.from({length:108},(_,i)=>i+38),...Array.from({length:20},(_,i)=>i)].filter(i=>!passed.has(i));\nfor(let position=0;position<order.length;position++){\n const index=order[position];\n store(\"xs_individual_pending_index\",index);\n const ok=await eval(load(\"xs_test_one\"))(index);\n if(!ok){text({stopped_at:index,name:load(\"xs_names\")[index],passed:load(\"xs_individual_results\").filter(r=>r.passed).length});return false;}\n if((position+1)%8===0){text({passed:load(\"xs_individual_results\").filter(r=>r.passed).length});await yield_control();}\n}\nstore(\"xs_individual_complete\",true);\ntext({message:\"All 146 individual board tests passed.\",count:load(\"xs_individual_results\").length});\nreturn true;\n}",
  "xs_auto_transitions": [
    {
      "from": 62,
      "to": 63,
      "visible_ms": 10013,
      "started": 735083,
      "ms_ticks": 745096,
      "frames_presented": 15877,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "passed": true,
      "utc": "2026-10-01 12:51:27 UTC"
    },
    {
      "from": 63,
      "to": 64,
      "visible_ms": 10010,
      "started": 746444,
      "ms_ticks": 756454,
      "frames_presented": 16181,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "passed": true,
      "utc": "2026-10-01 12:51:39 UTC"
    },
    {
      "from": 145,
      "to": 0,
      "visible_ms": 10025,
      "started": 756554,
      "ms_ticks": 766579,
      "frames_presented": 16483,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "passed": true,
      "utc": "2026-10-01 12:52:11 UTC"
    },
    {
      "from": 0,
      "to": 1,
      "visible_ms": 10021,
      "started": 766643,
      "ms_ticks": 776664,
      "frames_presented": 16787,
      "fault_status": {
        "HFSR": 0,
        "MMFSR": 0,
        "BFSR": 0,
        "UFSR": 0
      },
      "passed": true,
      "utc": "2026-10-01 12:52:22 UTC"
    }
  ]
};
for (const [key,value] of Object.entries(checkpointState)) store(key,value);
text({complete:load('xs_individual_complete'),resume_index:load('xs_individual_pending_index'),passed:load('xs_individual_results').filter(r=>r.passed).length});
