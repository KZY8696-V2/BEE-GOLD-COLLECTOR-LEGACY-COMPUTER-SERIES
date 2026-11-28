/* ============================================================================
 * BEE GOLD COLLECTOR v1.2.1R55 - 2026 KGAMER (VGA ONLY)
 * + REVERTED EXTRA DIFFICULTY & APPLIED DIRECT +5.5% SPEED BOOST (1.055x)
 * + SMOOTH BEE VERTICAL SPEED & DAMPING FIX
 * + UNIFIED VGA BUFFER RENDERING (NO GDI BACKEND)
 * + TWO SEPARATE COLOR TABLES FROM STANDARD WEB HEX CODES (BGR32 & RGB/BMP)
 * + BUILD-TIME SECRET KEY (BGC_VAULT_KEY)
 * + REPLAY RECORDING, VIDEO RECORDING TO FILE & SCREENSHOT (.BMP) EXPORT
 * + RUMBLE + DUALSENSE HAPTIC & LED LIGHTBAR
 * + UNIFIED MEDIA VAULT (PHOTO + REPLAY + VIDEO IN ONE SCREEN + SAVE TO DEVICE)
 * + MULTI-GOLD, SWARM BUGS & GOLD BAR COLLECTIBLE
 * + HOLD + TAP SMOOTH ASCENT & FULL MAIN MENU MULTI-INPUT (MOUSE+KEY+PAD)
 * + GAME OVER AT PAUSE POSITION WITH RETRY & MAIN MENU BUTTONS
 * + BEERACE 2P SECOND CONTROLLER LOBBY & COMPACT SPLIT-SCREEN SPRITES
 * + SMOOTH MOUSE FOLLOW IN ALL MODES + SELECTABLE CTRL SOURCE + BGCOPT.DAT
 * + WIN 1.0 / 2.0 / 3.0 / 3.1X / BOB / NT 3.1 / MT 5.0 WORKSTATION & DOS
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__DJGPP__)
    #define BGC_DOS_TARGET 1
    #define BGC_DJGPP_TARGET 1
    #define BGC_HAS_SOCKETS 0
    #include <dos.h>
    #include <conio.h>
    #include <dpmi.h>
    #include <sys/movedata.h>
    #include <sys/nearptr.h>
#elif defined(__MSDOS__) || defined(_MSDOS) || defined(__DOS__)
    #define BGC_DOS_TARGET 1
    #define BGC_DJGPP_TARGET 0
    #define BGC_HAS_SOCKETS 0
    #include <dos.h>
    #include <conio.h>
    #include <malloc.h>
#else
    #define BGC_DOS_TARGET 0
    #define BGC_DJGPP_TARGET 0
    #if !defined(_WIN16) && !defined(__WINDOWS_386__) && !defined(_M_I86) && !defined(_WIN32_WCE) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0400)
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #define BGC_HAS_SOCKETS 1
    #else
        #define BGC_HAS_SOCKETS 0
    #endif
    #include <windows.h>
    #include <mmsystem.h>
#endif

#if !BGC_DOS_TARGET
    #if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
        #define BGC_WIN_ARCH 64
    #elif defined(_WIN16) || defined(__WINDOWS_386__) || defined(_M_I86)
        #define BGC_WIN_ARCH 16
    #else
        #define BGC_WIN_ARCH 32
    #endif
#else
    #define BGC_WIN_ARCH 0
#endif

#if BGC_DOS_TARGET || (BGC_WIN_ARCH == 16)
    #define BGC_IS_16BIT 1
#else
    #define BGC_IS_16BIT 0
#endif

/* %5.5 hiz carpani (1.055 = 1055 / 1000) */
#define BGC_SPEED_NUM 1055l
#define BGC_SPEED_DEN 1000l

#define BGC_VERSION_STR "1.2.1R55"
#define BGC_UPDATE_NAME "SPEED +5.5PCT"

#ifndef BGC_TITLE
    #define BGC_TITLE "BEE GOLD COLLECTOR v1.2.1R55 (+5.5% SPEED) - 2026 KGAMER"
#endif

#ifndef BGC_VAULT_KEY
    #define BGC_VAULT_KEY 0x13572468u
#endif

#ifndef BGC_OFFICIAL_FLAG
    #define BGC_OFFICIAL_FLAG 0
#endif

#define SCR_W 320
#define SCR_H 200
#define SCR_PIXELS ((unsigned long)SCR_W * (unsigned long)SCR_H)

#define MAX_GOLDS 4
#define MAX_BUGS  10

#define VAULT_FILE      "BGCSCV.DAT"
#define OPTIONS_FILE    "BGCOPT.DAT"
#define REPLAY_FILE     "BGCRPL.RPL"
#define VIDEO_FILE      "BGCVDO.VDO"
#define SHOT_FILE       "BGCSHOT.BMP"
#define WIFI_SCORE_PORT 38696

#if BGC_IS_16BIT
    #define MAX_REPLAY_FRAMES 200
    #define MAX_VIDEO_FRAMES  8
#else
    #define MAX_REPLAY_FRAMES 600
    #define MAX_VIDEO_FRAMES  180
#endif

#define SCREEN_MAIN_MENU    0
#define SCREEN_PLAYING      1
#define SCREEN_PAUSED       2
#define SCREEN_SETTINGS     3
#define SCREEN_CONTROLS     4
#define SCREEN_BEERACE      5
#define SCREEN_WIFI_SCORE   6
#define SCREEN_MEDIA_VAULT  7

#define MEDIA_TAB_PHOTO  0
#define MEDIA_TAB_REPLAY 1
#define MEDIA_TAB_VIDEO  2

#define BACKEND_VGA 1

#define PAUSE_STYLE_FULL    0
#define PAUSE_STYLE_COMPACT 1

#define CTRL_SPEED_NORMAL 0
#define CTRL_SPEED_FAST   1

#define CTRL_SRC_AUTO     0
#define CTRL_SRC_MOUSE    1
#define CTRL_SRC_KEYBOARD 2
#define CTRL_SRC_GAMEPAD  3

#define SETTINGS_COUNT 6

#define C_VGA_SKY     11
#define C_VGA_GRASS   2
#define C_VGA_TRUNK   6
#define C_VGA_LEAF    10
#define C_VGA_BLACK   0
#define C_VGA_YELLOW  14
#define C_VGA_WING    15
#define C_VGA_RED     4
#define C_VGA_PANEL   7

/* Standart Web Hex Kodlari (#RRGGBB) */
#define WEB_HEX_BLACK   0x0C0C0Cu
#define WEB_HEX_GRASS   0x228B22u
#define WEB_HEX_RED     0xD22323u
#define WEB_HEX_TRUNK   0x734420u
#define WEB_HEX_PANEL   0xE0E8EEu
#define WEB_HEX_LEAF    0x48C640u
#define WEB_HEX_SKY     0x7DCDF5u
#define WEB_HEX_YELLOW  0xFFD700u
#define WEB_HEX_WING    0xF5FAFFu

#define WEB_R(hex)      ((unsigned char)(((hex) >> 16) & 0xFFu))
#define WEB_G(hex)      ((unsigned char)(((hex) >> 8)  & 0xFFu))
#define WEB_B(hex)      ((unsigned char)((hex)         & 0xFFu))
#define WEB_BGR32(hex)  ((unsigned int)WEB_B(hex) | ((unsigned int)WEB_G(hex) << 8) | ((unsigned int)WEB_R(hex) << 16))

static const unsigned int g_bgr32_table[16] = {
    WEB_BGR32(WEB_HEX_BLACK),  /* 0  */
    WEB_BGR32(WEB_HEX_SKY),    /* 1  */
    WEB_BGR32(WEB_HEX_GRASS),  /* 2  */
    WEB_BGR32(WEB_HEX_SKY),    /* 3  */
    WEB_BGR32(WEB_HEX_RED),    /* 4  */
    WEB_BGR32(WEB_HEX_SKY),    /* 5  */
    WEB_BGR32(WEB_HEX_TRUNK),  /* 6  */
    WEB_BGR32(WEB_HEX_PANEL),  /* 7  */
    WEB_BGR32(WEB_HEX_SKY),    /* 8  */
    WEB_BGR32(WEB_HEX_SKY),    /* 9  */
    WEB_BGR32(WEB_HEX_LEAF),   /* 10 */
    WEB_BGR32(WEB_HEX_SKY),    /* 11 */
    WEB_BGR32(WEB_HEX_SKY),    /* 12 */
    WEB_BGR32(WEB_HEX_SKY),    /* 13 */
    WEB_BGR32(WEB_HEX_YELLOW), /* 14 */
    WEB_BGR32(WEB_HEX_WING)    /* 15 */
};

static const unsigned char g_rgb_table[16][3] = {
    { WEB_R(WEB_HEX_BLACK),  WEB_G(WEB_HEX_BLACK),  WEB_B(WEB_HEX_BLACK)  }, /* 0  */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 1  */
    { WEB_R(WEB_HEX_GRASS),  WEB_G(WEB_HEX_GRASS),  WEB_B(WEB_HEX_GRASS)  }, /* 2  */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 3  */
    { WEB_R(WEB_HEX_RED),    WEB_G(WEB_HEX_RED),    WEB_B(WEB_HEX_RED)    }, /* 4  */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 5  */
    { WEB_R(WEB_HEX_TRUNK),  WEB_G(WEB_HEX_TRUNK),  WEB_B(WEB_HEX_TRUNK)  }, /* 6  */
    { WEB_R(WEB_HEX_PANEL),  WEB_G(WEB_HEX_PANEL),  WEB_B(WEB_HEX_PANEL)  }, /* 7  */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 8  */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 9  */
    { WEB_R(WEB_HEX_LEAF),   WEB_G(WEB_HEX_LEAF),   WEB_B(WEB_HEX_LEAF)   }, /* 10 */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 11 */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 12 */
    { WEB_R(WEB_HEX_SKY),    WEB_G(WEB_HEX_SKY),    WEB_B(WEB_HEX_SKY)    }, /* 13 */
    { WEB_R(WEB_HEX_YELLOW), WEB_G(WEB_HEX_YELLOW), WEB_B(WEB_HEX_YELLOW) }, /* 14 */
    { WEB_R(WEB_HEX_WING),   WEB_G(WEB_HEX_WING),   WEB_B(WEB_HEX_WING)   }  /* 15 */
};

#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16)
typedef struct {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} BGC_XINPUT_VIBRATION;

typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD dwUserIndex, BGC_XINPUT_VIBRATION *pVibration);
typedef BOOLEAN (WINAPI *PFN_HidD_SetOutputReport)(HANDLE HidDeviceObject, PVOID ReportBuffer, ULONG ReportBufferLength);
#endif

typedef struct {
    int connected;
    int dx;
    int dy;
    int btn_confirm;
    int btn_back;
    int btn_pause;
    int btn_retry;
    int prev_confirm;
    int prev_back;
    int prev_pause;
    int prev_retry;
    int nav_cooldown;
#if BGC_DOS_TARGET
    int calibrated;
    unsigned int center_x;
    unsigned int center_y;
    unsigned int dead_x;
    unsigned int dead_y;
#endif
} BGC_Pad;

typedef struct {
    int available;
    int x;
    int y;
    int prev_x;
    int prev_y;
    int left_down;
    int right_down;
    int prev_left;
    int prev_right;
    int click_left;
    int click_right;
    int pause_chord_fired;
    int act_as_pad2;
    int follow_mode;
} BGC_Mouse;

typedef struct {
    int x;
    int y;
    int vx;
    int vy;
    long sub_step;
    int gold_x[MAX_GOLDS];
    int gold_y[MAX_GOLDS];
    int gold_bar_x;
    int gold_bar_y;
    int gold_bar_active;
    int bug_x[MAX_BUGS];
    int bug_y[MAX_BUGS];
    int bug_speed[MAX_BUGS];
    int bug_phase[MAX_BUGS];
    int bug_vx_phase[MAX_BUGS];
    int score;
    int health;
    int dead;
} Racer;

typedef struct {
    int tick;
    int bee_x;
    int bee_y;
    int score;
    int health;
} ReplaySample;

typedef struct {
    int host_is_p1;
    int bee_x;
    int bee_y;
    int bee_vx;
    int bee_vy;
    long sub_step;
    int gold_x[MAX_GOLDS];
    int gold_y[MAX_GOLDS];
    int gold_bar_x;
    int gold_bar_y;
    int gold_bar_active;
    int bug_x[MAX_BUGS];
    int bug_y[MAX_BUGS];
    int bug_speed[MAX_BUGS];
    int bug_phase[MAX_BUGS];
    int bug_vx_phase[MAX_BUGS];
    int score;
    int best_score;
    int last_score;
    int health;
    int game_over;
    int gameover_idx;
    long tick;

    Racer p1;
    Racer p2;
    int race_started;
    int p1_ready;
    int p2_ready;
    int p2_is_pad;
    int race_over;
    int race_winner;
    long regen_timer_ticks;

    char peer_ip[32];
    int peer_best_score;
    int wifi_status;

    int media_tab;

    int replay_recording;
    int replay_count;
    ReplaySample replay_buf[MAX_REPLAY_FRAMES];
    int replay_play_idx;
    int replay_playing;

    int video_recording;
    int video_frame_count;
    unsigned char *video_frames[MAX_VIDEO_FRAMES];
    int video_play_idx;
    int video_playing;

    int has_photo_buf;
    unsigned char *photo_buf;

    int shot_saved_banner_ticks;
    int video_saved_banner_ticks;
    int replay_saved_banner_ticks;

    int rumble_ticks;
    unsigned short rumble_left;
    unsigned short rumble_right;
    unsigned char led_r;
    unsigned char led_g;
    unsigned char led_b;

    int screen;
    int prev_screen;
    int menu_idx;
    int pause_idx;
    int settings_idx;

    int backend;
    int pause_style;
    int ctrl_speed_mode;
    int ctrl_source;
    BGC_Pad pad1;
    BGC_Pad pad2;
    BGC_Mouse mouse;
} BGC_Game;

/* ============================================================================
 * 1. BITMAP FONT & VGA PRIMITIVES
 * ============================================================================ */

static unsigned char g_vga_buf[SCR_W * SCR_H];

#if BGC_DJGPP_TARGET
static int g_djgpp_nearptr_ready = 0;
static unsigned char *g_djgpp_vram = NULL;

static void bgc_dos_init_vram(void) {
    if (!g_djgpp_nearptr_ready) {
        if (__djgpp_nearptr_enable()) {
            g_djgpp_nearptr_ready = 1;
            g_djgpp_vram = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
        }
    }
}

static void bgc_dos_shutdown_vram(void) {
    if (g_djgpp_nearptr_ready) {
        __djgpp_nearptr_disable();
        g_djgpp_nearptr_ready = 0;
        g_djgpp_vram = NULL;
    }
}

static void bgc_dos_flush_vga(void) {
    if (g_djgpp_nearptr_ready && g_djgpp_vram) {
        memcpy(g_djgpp_vram, g_vga_buf, (size_t)SCR_PIXELS);
    } else {
        dosmemput(g_vga_buf, (size_t)SCR_PIXELS, 0xA0000L);
    }
}
#endif

static void alloc_media_buffers(BGC_Game *g) {
    int i;
    if (!g->photo_buf) {
        g->photo_buf = (unsigned char *)malloc((size_t)SCR_PIXELS);
        if (g->photo_buf) {
            memset(g->photo_buf, C_VGA_SKY, (size_t)SCR_PIXELS);
        }
    }
    for (i = 0; i < MAX_VIDEO_FRAMES; i++) {
        if (!g->video_frames[i]) {
            g->video_frames[i] = (unsigned char *)malloc((size_t)SCR_PIXELS);
        }
    }
}

static void free_media_buffers(BGC_Game *g) {
    int i;
    if (g->photo_buf) {
        free(g->photo_buf);
        g->photo_buf = NULL;
    }
    for (i = 0; i < MAX_VIDEO_FRAMES; i++) {
        if (g->video_frames[i]) {
            free(g->video_frames[i]);
            g->video_frames[i] = NULL;
        }
    }
}

static const unsigned char *get_font_rows(char ch) {
    static const unsigned char f_space[7] = {0,0,0,0,0,0,0};
    static const unsigned char f_dash[7]  = {0,0,0,0x1F,0,0,0};
    static const unsigned char f_plus[7]  = {0,0x04,0x04,0x1F,0x04,0x04,0};
    static const unsigned char f_colon[7] = {0,0x04,0x04,0,0x04,0x04,0};
    static const unsigned char f_dot[7]   = {0,0,0,0,0,0x06,0x06};
    static const unsigned char f_slash[7] = {0x01,0x02,0x04,0x08,0x10,0,0};
    static const unsigned char f_gt[7]    = {0x10,0x08,0x04,0x02,0x04,0x08,0x10};
    static const unsigned char f_0[7]     = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E};
    static const unsigned char f_1[7]     = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E};
    static const unsigned char f_2[7]     = {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F};
    static const unsigned char f_3[7]     = {0x1E,0x01,0x01,0x06,0x01,0x01,0x1E};
    static const unsigned char f_4[7]     = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02};
    static const unsigned char f_5[7]     = {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E};
    static const unsigned char f_6[7]     = {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E};
    static const unsigned char f_7[7]     = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08};
    static const unsigned char f_8[7]     = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E};
    static const unsigned char f_9[7]     = {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C};
    static const unsigned char f_A[7]     = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const unsigned char f_B[7]     = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E};
    static const unsigned char f_C[7]     = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E};
    static const unsigned char f_D[7]     = {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C};
    static const unsigned char f_E[7]     = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F};
    static const unsigned char f_F[7]     = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10};
    static const unsigned char f_G[7]     = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F};
    static const unsigned char f_H[7]     = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const unsigned char f_I[7]     = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E};
    static const unsigned char f_K[7]     = {0x11,0x12,0x14,0x18,0x14,0x12,0x11};
    static const unsigned char f_L[7]     = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F};
    static const unsigned char f_M[7]     = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11};
    static const unsigned char f_N[7]     = {0x11,0x19,0x15,0x13,0x11,0x11,0x11};
    static const unsigned char f_O[7]     = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const unsigned char f_P[7]     = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10};
    static const unsigned char f_R[7]     = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11};
    static const unsigned char f_S[7]     = {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E};
    static const unsigned char f_T[7]     = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04};
    static const unsigned char f_U[7]     = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const unsigned char f_V[7]     = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04};
    static const unsigned char f_W[7]     = {0x11,0x11,0x11,0x15,0x15,0x1B,0x11};
    static const unsigned char f_X[7]     = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11};
    static const unsigned char f_Y[7]     = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04};
    static const unsigned char f_Z[7]     = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F};

    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    switch (ch) {
        case '0': return f_0; case '1': return f_1; case '2': return f_2;
        case '3': return f_3; case '4': return f_4; case '5': return f_5;
        case '6': return f_6; case '7': return f_7; case '8': return f_8;
        case '9': return f_9; case 'A': return f_A; case 'B': return f_B;
        case 'C': return f_C; case 'D': return f_D; case 'E': return f_E;
        case 'F': return f_F; case 'G': return f_G; case 'H': return f_H;
        case 'I': return f_I; case 'K': return f_K; case 'L': return f_L;
        case 'M': return f_M; case 'N': return f_N; case 'O': return f_O;
        case 'P': return f_P; case 'R': return f_R; case 'S': return f_S;
        case 'T': return f_T; case 'U': return f_U; case 'V': return f_V;
        case 'W': return f_W; case 'X': return f_X; case 'Y': return f_Y;
        case 'Z': return f_Z; case '-': return f_dash; case '+': return f_plus;
        case ':': return f_colon; case '.': return f_dot; case '/': return f_slash;
        case '>': return f_gt;
        default: return f_space;
    }
}

#if !BGC_DOS_TARGET
static unsigned int vga_to_bgr_u32(unsigned char idx) {
    return g_bgr32_table[idx & 0x0Fu];
}

#if (BGC_WIN_ARCH == 16)
static COLORREF vga_to_gdi_color(unsigned char idx) {
    const unsigned char *rgb = g_rgb_table[idx & 0x0Fu];
    return RGB(rgb[0], rgb[1], rgb[2]);
}
#endif
#endif

static void vga_color_to_rgb(unsigned char idx, unsigned char *r, unsigned char *g, unsigned char *b) {
    const unsigned char *rgb = g_rgb_table[idx & 0x0Fu];
    *r = rgb[0];
    *g = rgb[1];
    *b = rgb[2];
}

static void draw_px(const BGC_Game *g, void *hdc_ptr, int x, int y, unsigned char vga_idx) {
    (void)g;
    (void)hdc_ptr;
    if (x < 0 || x >= SCR_W || y < 0 || y >= SCR_H) return;
    g_vga_buf[(unsigned long)y * SCR_W + (unsigned long)x] = vga_idx;
}

static void draw_box(const BGC_Game *g, void *hdc_ptr, int x, int y, int w, int h, unsigned char fill_idx, int border) {
    int ix, iy;
    (void)g;
    (void)hdc_ptr;
    for (iy = y; iy < y + h; iy++) {
        if (iy < 0 || iy >= SCR_H) continue;
        for (ix = x; ix < x + w; ix++) {
            if (ix < 0 || ix >= SCR_W) continue;
            if (border && (ix == x || ix == x + w - 1 || iy == y || iy == y + h - 1)) {
                g_vga_buf[(unsigned long)iy * SCR_W + (unsigned long)ix] = C_VGA_BLACK;
            } else {
                g_vga_buf[(unsigned long)iy * SCR_W + (unsigned long)ix] = fill_idx;
            }
        }
    }
}

static void draw_circle_fill(const BGC_Game *g, void *hdc_ptr, int cx, int cy, int r, unsigned char fill_idx, int outline_black) {
    int dx, dy;
    (void)g;
    (void)hdc_ptr;
    for (dy = -r; dy <= r; dy++) {
        int py = cy + dy;
        if (py < 0 || py >= SCR_H) continue;
        for (dx = -r; dx <= r; dx++) {
            int px = cx + dx;
            int d2;
            if (px < 0 || px >= SCR_W) continue;
            d2 = dx * dx + dy * dy;
            if (d2 <= r * r) {
                unsigned char c = (outline_black && d2 >= (r - 1) * (r - 1)) ? C_VGA_BLACK : fill_idx;
                g_vga_buf[(unsigned long)py * SCR_W + (unsigned long)px] = c;
            }
        }
    }
}

static void draw_line_simple(const BGC_Game *g, void *hdc_ptr, int x0, int y0, int x1, int y1, unsigned char col_idx) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    (void)g;
    (void)hdc_ptr;

    while (1) {
        if (x0 >= 0 && x0 < SCR_W && y0 >= 0 && y0 < SCR_H) {
            g_vga_buf[(unsigned long)y0 * SCR_W + (unsigned long)x0] = col_idx;
        }
        if (x0 == x1 && y0 == y1) break;
        {
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 <  dx) { err += dx; y0 += sy; }
        }
    }
}

static void draw_str_5x7(const BGC_Game *g, void *hdc_ptr, int x, int y, const char *s, unsigned char col_idx) {
    int cx = x;
    while (*s) {
        const unsigned char *rows = get_font_rows(*s++);
        int r, c;
        for (r = 0; r < 7; r++) {
            for (c = 0; c < 5; c++) {
                if (rows[r] & (1 << (4 - c))) {
                    draw_px(g, hdc_ptr, cx + c, y + r, col_idx);
                }
            }
        }
        cx += 6;
    }
}

static void draw_update_footer(const BGC_Game *g, void *hdc_ptr) {
    draw_box(g, hdc_ptr, 0, SCR_H - 10, SCR_W, 10, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 6, SCR_H - 8, "V" BGC_VERSION_STR "  UPDATE NAMES: " BGC_UPDATE_NAME, C_VGA_BLACK);
}

static void blit_frame_to_screen(const BGC_Game *g, void *hdc_ptr, const unsigned char *frame) {
    (void)g;
    (void)hdc_ptr;
    if (!frame) return;
    memcpy(g_vga_buf, frame, (size_t)SCR_PIXELS);
}

/* ============================================================================
 * 2. RUMBLE + DUALSENSE HAPTIC & LED LIGHTBAR (WIN32+ / NT 5.0+ SAFE)
 * ============================================================================ */

static void apply_rumble_and_dualsense(unsigned short left_val, unsigned short right_val,
                                       unsigned char r, unsigned char gr, unsigned char b) {
#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0500)
    HMODULE h_xi = LoadLibraryA("xinput1_4.dll");
    if (!h_xi) h_xi = LoadLibraryA("xinput9_1_0.dll");
    if (!h_xi) h_xi = LoadLibraryA("xinput1_3.dll");

    if (h_xi) {
        PFN_XInputSetState pSetState = (PFN_XInputSetState)GetProcAddress(h_xi, "XInputSetState");
        if (pSetState) {
            BGC_XINPUT_VIBRATION vib;
            vib.wLeftMotorSpeed = left_val;
            vib.wRightMotorSpeed = right_val;
            pSetState(0, &vib);
            pSetState(1, &vib);
        }
        FreeLibrary(h_xi);
    }

    {
        HMODULE h_hid = LoadLibraryA("hid.dll");
        if (h_hid) {
            PFN_HidD_SetOutputReport pSetOut = (PFN_HidD_SetOutputReport)GetProcAddress(h_hid, "HidD_SetOutputReport");
            if (pSetOut) {
                unsigned char ds4_rep[32];
                HANDLE h_ds4 = CreateFileA(
                    "\\\\?\\HID#VID_054C&PID_05C4",
                    GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, 0, NULL
                );
                if (h_ds4 != INVALID_HANDLE_VALUE) {
                    memset(ds4_rep, 0, sizeof(ds4_rep));
                    ds4_rep[0] = 0x05;
                    ds4_rep[1] = 0x03;
                    ds4_rep[4] = (unsigned char)(right_val >> 8);
                    ds4_rep[5] = (unsigned char)(left_val >> 8);
                    ds4_rep[6] = r;
                    ds4_rep[7] = gr;
                    ds4_rep[8] = b;
                    pSetOut(h_ds4, ds4_rep, sizeof(ds4_rep));
                    CloseHandle(h_ds4);
                }

                {
                    unsigned char ds5_rep[48];
                    HANDLE h_ds5 = CreateFileA(
                        "\\\\?\\HID#VID_054C&PID_0CE6",
                        GENERIC_WRITE | GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, 0, NULL
                    );
                    if (h_ds5 != INVALID_HANDLE_VALUE) {
                        memset(ds5_rep, 0, sizeof(ds5_rep));
                        ds5_rep[0] = 0x02;
                        ds5_rep[1] = 0x01 | 0x02 | 0x04;
                        ds5_rep[2] = 0x04;
                        ds5_rep[3] = (unsigned char)(right_val >> 8);
                        ds5_rep[4] = (unsigned char)(left_val >> 8);
                        ds5_rep[45] = r;
                        ds5_rep[46] = gr;
                        ds5_rep[47] = b;
                        pSetOut(h_ds5, ds5_rep, sizeof(ds5_rep));
                        CloseHandle(h_ds5);
                    }
                }
            }
            FreeLibrary(h_hid);
        }
    }
#else
    (void)left_val; (void)right_val; (void)r; (void)gr; (void)b;
#endif
}

static void set_dualsense_led_from_health(BGC_Game *g, int hp) {
    if (hp > 60) {
        g->led_r = 20; g->led_g = 220; g->led_b = 90;
    } else if (hp > 30) {
        g->led_r = 255; g->led_g = 190; g->led_b = 0;
    } else {
        g->led_r = 240; g->led_g = 25; g->led_b = 25;
    }
    apply_rumble_and_dualsense(g->rumble_left, g->rumble_right, g->led_r, g->led_g, g->led_b);
}

static void trigger_rumble_and_led(BGC_Game *g, unsigned short left_val, unsigned short right_val,
                                   int duration_ticks, unsigned char r, unsigned char gr, unsigned char b) {
    g->rumble_left = left_val;
    g->rumble_right = right_val;
    g->rumble_ticks = duration_ticks;
    g->led_r = r;
    g->led_g = gr;
    g->led_b = b;
    apply_rumble_and_dualsense(left_val, right_val, r, gr, b);
}

static void update_rumble_timer(BGC_Game *g) {
    if (g->rumble_ticks > 0) {
        g->rumble_ticks--;
        if (g->rumble_ticks == 0) {
            g->rumble_left = 0;
            g->rumble_right = 0;
            apply_rumble_and_dualsense(0, 0, g->led_r, g->led_g, g->led_b);
        }
    }
}

/* ============================================================================
 * 3. BUILD-TIME SECRET KEY VAULTS (BGCSCV.DAT & BGCOPT.DAT)
 * ============================================================================ */

static unsigned char obfuscate_byte(unsigned char b, unsigned int idx, unsigned int seed) {
    unsigned char k = (unsigned char)((seed * 31u + idx * 17u + 0x5Au) ^ 0xA5u);
    unsigned char x = (unsigned char)(b ^ k);
    return (unsigned char)(((x << 3) | (x >> 5)) & 0xFFu);
}

static unsigned char deobfuscate_byte(unsigned char ob, unsigned int idx, unsigned int seed) {
    unsigned char r = (unsigned char)(((ob >> 3) | (ob << 5)) & 0xFFu);
    unsigned char k = (unsigned char)((seed * 31u + idx * 17u + 0x5Au) ^ 0xA5u);
    return (unsigned char)(r ^ k);
}

static unsigned int compute_vault_tag(int best_s, int last_s, unsigned int seed) {
    unsigned int h = 0x811C9DC5u ^ seed;
    unsigned int ub = (unsigned int)((best_s < 0) ? 0 : best_s);
    unsigned int ul = (unsigned int)((last_s < 0) ? 0 : last_s);
    h ^= (ub & 0xFFu); h *= 0x01000193u;
    h ^= ((ub >> 8) & 0xFFu); h *= 0x01000193u;
    h ^= ((ub >> 16) & 0xFFu); h *= 0x01000193u;
    h ^= (ul & 0xFFu); h *= 0x01000193u;
    h ^= ((ul >> 8) & 0xFFu); h *= 0x01000193u;
    return h;
}

static unsigned int compute_options_tag(const unsigned char *opts, int count, unsigned int seed) {
    unsigned int h = 0x811C9DC5u ^ (seed * 13u);
    int i;
    for (i = 0; i < count; i++) {
        h ^= (unsigned int)opts[i];
        h *= 0x01000193u;
    }
    return h;
}

static void mark_file_hidden_if_possible(const char *path) {
    (void)path;
#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16)
    SetFileAttributesA(path, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
#endif
}

static void unmark_readonly_for_write(const char *path) {
    (void)path;
#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16)
    SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
#endif
}

static void write_obfuscated_vault(const BGC_Game *g) {
    unsigned char raw[16];
    unsigned char enc[16];
    unsigned int seed = (unsigned int)BGC_VAULT_KEY;
    unsigned int tag = compute_vault_tag(g->best_score, g->last_score, seed);
    unsigned int ub = (unsigned int)((g->best_score < 0) ? 0 : g->best_score);
    unsigned int ul = (unsigned int)((g->last_score < 0) ? 0 : g->last_score);
    int i;
    FILE *fp;

    raw[0] = 'B'; raw[1] = 'G'; raw[2] = 'C'; raw[3] = 'V';
    raw[4] = (unsigned char)(ub & 0xFFu);
    raw[5] = (unsigned char)((ub >> 8) & 0xFFu);
    raw[6] = (unsigned char)((ub >> 16) & 0xFFu);
    raw[7] = (unsigned char)((ub >> 24) & 0xFFu);

    raw[8]  = (unsigned char)(ul & 0xFFu);
    raw[9]  = (unsigned char)((ul >> 8) & 0xFFu);
    raw[10] = (unsigned char)((ul >> 16) & 0xFFu);
    raw[11] = (unsigned char)((ul >> 24) & 0xFFu);

    raw[12] = (unsigned char)(tag & 0xFFu);
    raw[13] = (unsigned char)((tag >> 8) & 0xFFu);
    raw[14] = (unsigned char)((tag >> 16) & 0xFFu);
    raw[15] = (unsigned char)((tag >> 24) & 0xFFu);

    for (i = 0; i < 16; i++) {
        enc[i] = obfuscate_byte(raw[i], (unsigned int)i, seed);
    }

    unmark_readonly_for_write(VAULT_FILE);
    fp = fopen(VAULT_FILE, "wb");
    if (fp) {
        fwrite(enc, 1, 16, fp);
        fclose(fp);
    }
    mark_file_hidden_if_possible(VAULT_FILE);
}

static void read_obfuscated_vault(BGC_Game *g) {
    unsigned char enc[16];
    unsigned char raw[16];
    unsigned int seed = (unsigned int)BGC_VAULT_KEY;
    unsigned int read_tag, calc_tag;
    int best_s, last_s;
    int i;
    FILE *fp;

    g->best_score = 0;
    g->last_score = 0;

    fp = fopen(VAULT_FILE, "rb");
    if (!fp) return;

    if (fread(enc, 1, 16, fp) != 16) {
        fclose(fp);
        return;
    }
    fclose(fp);

    for (i = 0; i < 16; i++) {
        raw[i] = deobfuscate_byte(enc[i], (unsigned int)i, seed);
    }

    if (raw[0] != 'B' || raw[1] != 'G' || raw[2] != 'C' || raw[3] != 'V') {
        return;
    }

    best_s = (int)((unsigned int)raw[4] |
                   ((unsigned int)raw[5] << 8) |
                   ((unsigned int)raw[6] << 16) |
                   ((unsigned int)raw[7] << 24));

    last_s = (int)((unsigned int)raw[8] |
                   ((unsigned int)raw[9] << 8) |
                   ((unsigned int)raw[10] << 16) |
                   ((unsigned int)raw[11] << 24));

    read_tag = (unsigned int)raw[12] |
               ((unsigned int)raw[13] << 8) |
               ((unsigned int)raw[14] << 16) |
               ((unsigned int)raw[15] << 24);

    calc_tag = compute_vault_tag(best_s, last_s, seed);
    if (read_tag != calc_tag || best_s < 0 || last_s < 0 || best_s > 100000000) {
        return;
    }

    g->best_score = best_s;
    g->last_score = last_s;
}

static void record_score_secure(BGC_Game *g, int score) {
    if (score < 0) score = 0;
    g->last_score = score;
    if (score > g->best_score) {
        g->best_score = score;
    }
    write_obfuscated_vault(g);
}

static void write_obfuscated_options(const BGC_Game *g) {
    unsigned char raw[16];
    unsigned char enc[16];
    unsigned int seed = (unsigned int)BGC_VAULT_KEY;
    unsigned int tag;
    int i;
    FILE *fp;

    memset(raw, 0, sizeof(raw));
    raw[0] = 'B'; raw[1] = 'G'; raw[2] = 'C'; raw[3] = 'O';
    raw[4] = (unsigned char)BACKEND_VGA;
    raw[5] = (unsigned char)((g->pause_style == PAUSE_STYLE_COMPACT) ? PAUSE_STYLE_COMPACT : PAUSE_STYLE_FULL);
    raw[6] = (unsigned char)((g->ctrl_speed_mode == CTRL_SPEED_FAST) ? CTRL_SPEED_FAST : CTRL_SPEED_NORMAL);
    raw[7] = (unsigned char)(g->mouse.follow_mode ? 1 : 0);
    raw[8] = (unsigned char)(g->mouse.act_as_pad2 ? 1 : 0);
    raw[9] = (unsigned char)(g->ctrl_source & 0x03);

    tag = compute_options_tag(&raw[4], 6, seed);
    raw[12] = (unsigned char)(tag & 0xFFu);
    raw[13] = (unsigned char)((tag >> 8) & 0xFFu);
    raw[14] = (unsigned char)((tag >> 16) & 0xFFu);
    raw[15] = (unsigned char)((tag >> 24) & 0xFFu);

    for (i = 0; i < 16; i++) {
        enc[i] = obfuscate_byte(raw[i], (unsigned int)i, seed);
    }

    unmark_readonly_for_write(OPTIONS_FILE);
    fp = fopen(OPTIONS_FILE, "wb");
    if (fp) {
        fwrite(enc, 1, 16, fp);
        fclose(fp);
    }
    mark_file_hidden_if_possible(OPTIONS_FILE);
}

static void read_obfuscated_options(BGC_Game *g) {
    unsigned char enc[16];
    unsigned char raw[16];
    unsigned int seed = (unsigned int)BGC_VAULT_KEY;
    unsigned int read_tag, calc_tag;
    int i;
    FILE *fp;

    g->backend = BACKEND_VGA;
    fp = fopen(OPTIONS_FILE, "rb");
    if (!fp) return;

    if (fread(enc, 1, 16, fp) != 16) {
        fclose(fp);
        return;
    }
    fclose(fp);

    for (i = 0; i < 16; i++) {
        raw[i] = deobfuscate_byte(enc[i], (unsigned int)i, seed);
    }

    if (raw[0] != 'B' || raw[1] != 'G' || raw[2] != 'C' || raw[3] != 'O') {
        return;
    }

    read_tag = (unsigned int)raw[12] |
               ((unsigned int)raw[13] << 8) |
               ((unsigned int)raw[14] << 16) |
               ((unsigned int)raw[15] << 24);
    calc_tag = compute_options_tag(&raw[4], 6, seed);
    if (read_tag != calc_tag) return;

    g->backend = BACKEND_VGA;
    if (raw[5] == PAUSE_STYLE_FULL || raw[5] == PAUSE_STYLE_COMPACT) g->pause_style = (int)raw[5];
    if (raw[6] == CTRL_SPEED_NORMAL || raw[6] == CTRL_SPEED_FAST) g->ctrl_speed_mode = (int)raw[6];
    g->mouse.follow_mode = raw[7] ? 1 : 0;
    g->mouse.act_as_pad2 = raw[8] ? 1 : 0;
    if (raw[9] <= CTRL_SRC_GAMEPAD) g->ctrl_source = (int)raw[9];
}

/* ============================================================================
 * 4. REPLAY, VIDEO, PHOTO EXPORT & SAVE TO DEVICE
 * ============================================================================ */

static void capture_replay_sample(BGC_Game *g) {
    if (!g->replay_recording) return;
    if (g->replay_count < MAX_REPLAY_FRAMES) {
        ReplaySample *s = &g->replay_buf[g->replay_count++];
        s->tick = (int)g->tick;
        s->bee_x = g->bee_x;
        s->bee_y = g->bee_y;
        s->score = g->score;
        s->health = g->health;
    }
}

static void save_replay_to_file(BGC_Game *g) {
    FILE *fp = fopen(REPLAY_FILE, "wb");
    if (!fp) return;
    fwrite("BGCRPL1", 1, 7, fp);
    fwrite(&g->replay_count, sizeof(int), 1, fp);
    if (g->replay_count > 0) {
        fwrite(g->replay_buf, sizeof(ReplaySample), (size_t)g->replay_count, fp);
    }
    fclose(fp);
    g->replay_saved_banner_ticks = 45;
}

static void load_replay_from_file(BGC_Game *g) {
    FILE *fp = fopen(REPLAY_FILE, "rb");
    char magic[7];
    int cnt = 0;
    if (!fp) return;
    if (fread(magic, 1, 7, fp) != 7 || memcmp(magic, "BGCRPL1", 7) != 0) {
        fclose(fp);
        return;
    }
    if (fread(&cnt, sizeof(int), 1, fp) == 1 && cnt > 0 && cnt <= MAX_REPLAY_FRAMES) {
        if ((int)fread(g->replay_buf, sizeof(ReplaySample), (size_t)cnt, fp) == cnt) {
            g->replay_count = cnt;
        }
    }
    fclose(fp);
}

static void capture_video_frame(BGC_Game *g) {
    if (!g->video_recording) return;
    if ((g->tick % 4l) != 0) return;
    if (g->video_frame_count < MAX_VIDEO_FRAMES && g->video_frames[g->video_frame_count]) {
        memcpy(g->video_frames[g->video_frame_count], g_vga_buf, (size_t)SCR_PIXELS);
        g->video_frame_count++;
    } else {
        g->video_recording = 0;
    }
}

static void save_video_to_file(BGC_Game *g) {
    FILE *fp = fopen(VIDEO_FILE, "wb");
    int w = SCR_W;
    int h = SCR_H;
    int i;
    if (!fp) return;
    fwrite("BGCVDO1", 1, 7, fp);
    fwrite(&w, sizeof(int), 1, fp);
    fwrite(&h, sizeof(int), 1, fp);
    fwrite(&g->video_frame_count, sizeof(int), 1, fp);
    for (i = 0; i < g->video_frame_count; i++) {
        if (g->video_frames[i]) {
            fwrite(g->video_frames[i], 1, (size_t)SCR_PIXELS, fp);
        }
    }
    fclose(fp);
    g->video_saved_banner_ticks = 45;
}

static void load_video_from_file(BGC_Game *g) {
    FILE *fp = fopen(VIDEO_FILE, "rb");
    char magic[7];
    int w = 0, h = 0, cnt = 0, i;
    alloc_media_buffers(g);
    if (!fp) return;
    if (fread(magic, 1, 7, fp) != 7 || memcmp(magic, "BGCVDO1", 7) != 0) {
        fclose(fp);
        return;
    }
    if (fread(&w, sizeof(int), 1, fp) == 1 &&
        fread(&h, sizeof(int), 1, fp) == 1 &&
        fread(&cnt, sizeof(int), 1, fp) == 1 &&
        w == SCR_W && h == SCR_H && cnt > 0 && cnt <= MAX_VIDEO_FRAMES) {
        int loaded = 0;
        for (i = 0; i < cnt; i++) {
            if (!g->video_frames[i]) break;
            if (fread(g->video_frames[i], 1, (size_t)SCR_PIXELS, fp) == (size_t)SCR_PIXELS) {
                loaded++;
            } else {
                break;
            }
        }
        g->video_frame_count = loaded;
    }
    fclose(fp);
}

static void toggle_video_recording(BGC_Game *g) {
    alloc_media_buffers(g);
    if (!g->video_recording) {
        g->video_frame_count = 0;
        g->video_recording = 1;
    } else {
        g->video_recording = 0;
        save_video_to_file(g);
    }
}

static void write_u16_le(FILE *fp, unsigned short v) {
    unsigned char b[2];
    b[0] = (unsigned char)(v & 0xFFu);
    b[1] = (unsigned char)((v >> 8) & 0xFFu);
    fwrite(b, 1, 2, fp);
}

static void write_u32_le(FILE *fp, unsigned long v) {
    unsigned char b[4];
    b[0] = (unsigned char)(v & 0xFFu);
    b[1] = (unsigned char)((v >> 8) & 0xFFu);
    b[2] = (unsigned char)((v >> 16) & 0xFFu);
    b[3] = (unsigned char)((v >> 24) & 0xFFu);
    fwrite(b, 1, 4, fp);
}

static void write_frame_as_bmp(BGC_Game *g, const unsigned char *frame_buf) {
    FILE *fp;
    unsigned long row_stride = (unsigned long)SCR_W * 3ul;
    unsigned long pad = (4ul - (row_stride % 4ul)) % 4ul;
    unsigned long pixel_bytes = (row_stride + pad) * (unsigned long)SCR_H;
    unsigned long file_size = 14ul + 40ul + pixel_bytes;
    int y, x;
    unsigned char pad_zero[3] = {0, 0, 0};

    if (!frame_buf) return;
    fp = fopen(SHOT_FILE, "wb");
    if (!fp) return;

    fwrite("BM", 1, 2, fp);
    write_u32_le(fp, file_size);
    write_u16_le(fp, 0);
    write_u16_le(fp, 0);
    write_u32_le(fp, 14ul + 40ul);

    write_u32_le(fp, 40ul);
    write_u32_le(fp, (unsigned long)SCR_W);
    write_u32_le(fp, (unsigned long)SCR_H);
    write_u16_le(fp, 1u);
    write_u16_le(fp, 24u);
    write_u32_le(fp, 0ul);
    write_u32_le(fp, pixel_bytes);
    write_u32_le(fp, 0ul);
    write_u32_le(fp, 0ul);
    write_u32_le(fp, 0ul);
    write_u32_le(fp, 0ul);

    for (y = SCR_H - 1; y >= 0; y--) {
        for (x = 0; x < SCR_W; x++) {
            unsigned char r, gr, bl;
            unsigned char bgr[3];
            vga_color_to_rgb(frame_buf[(unsigned long)y * SCR_W + (unsigned long)x], &r, &gr, &bl);
            bgr[0] = bl;
            bgr[1] = gr;
            bgr[2] = r;
            fwrite(bgr, 1, 3, fp);
        }
        if (pad > 0) fwrite(pad_zero, 1, (size_t)pad, fp);
    }

    fclose(fp);
    g->shot_saved_banner_ticks = 45;
}

static void save_screenshot_bmp(BGC_Game *g) {
    alloc_media_buffers(g);
    if (g->photo_buf) {
        memcpy(g->photo_buf, g_vga_buf, (size_t)SCR_PIXELS);
        g->has_photo_buf = 1;
        write_frame_as_bmp(g, g->photo_buf);
    } else {
        write_frame_as_bmp(g, g_vga_buf);
    }
}

/* ============================================================================
 * 5. WI-FI / IP SCORE COMPARISON (UDP)
 * ============================================================================ */

static void compare_score_over_wifi_ip(BGC_Game *g) {
#if BGC_HAS_SOCKETS
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in peer;
    char send_buf[64];
    char recv_buf[64];
    DWORD timeout_ms = 200;
    int remote_best = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        g->wifi_status = -1;
        return;
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        g->wifi_status = -1;
        WSACleanup();
        return;
    }

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms));

    memset(&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(WIFI_SCORE_PORT);
    peer.sin_addr.s_addr = inet_addr(g->peer_ip);

    sprintf(send_buf, "BGC_SCORE_REQ:%d", g->best_score);
    sendto(sock, send_buf, (int)strlen(send_buf), 0, (struct sockaddr *)&peer, sizeof(peer));

    {
        int from_len = sizeof(peer);
        int n = recvfrom(sock, recv_buf, sizeof(recv_buf) - 1, 0, (struct sockaddr *)&peer, &from_len);
        if (n > 0) {
            recv_buf[n] = '\0';
            if (sscanf(recv_buf, "BGC_SCORE_RESP:%d", &remote_best) == 1 ||
                sscanf(recv_buf, "BGC_SCORE_REQ:%d", &remote_best) == 1) {
                g->peer_best_score = remote_best;
                g->wifi_status = 1;
                closesocket(sock);
                WSACleanup();
                return;
            }
        }
    }

    g->wifi_status = -1;
    closesocket(sock);
    WSACleanup();
#else
    (void)g;
    g->wifi_status = -1;
#endif
}

/* ============================================================================
 * 6. DUAL GAMEPAD LAYER (PAD 1 & PAD 2) + LEGACY MOUSE LAYER
 * ============================================================================ */

static void init_mouse_state(BGC_Mouse *m) {
    memset(m, 0, sizeof(*m));
    m->x = SCR_W / 2;
    m->y = SCR_H / 2;
    m->prev_x = m->x;
    m->prev_y = m->y;
    m->act_as_pad2 = 1;
    m->follow_mode = 1;
}

static void finalize_mouse_frame(BGC_Mouse *m) {
    m->click_left  = (m->left_down  && !m->prev_left)  ? 1 : 0;
    m->click_right = (m->right_down && !m->prev_right) ? 1 : 0;

    if (!m->left_down || !m->right_down) {
        m->pause_chord_fired = 0;
    }

    m->prev_left  = m->left_down;
    m->prev_right = m->right_down;
}

#if BGC_DOS_TARGET

static void dos_init_mouse_driver(BGC_Mouse *m) {
    m->available = 0;
#if BGC_DJGPP_TARGET
    {
        __dpmi_regs r;
        memset(&r, 0, sizeof(r));
        r.x.ax = 0x0000;
        __dpmi_int(0x33, &r);
        if (r.x.ax != 0) m->available = 1;
    }
#elif defined(__386__)
    {
        union REGS r_in, r_out;
        memset(&r_in, 0, sizeof(r_in));
        r_in.w.ax = 0x0000;
        int386(0x33, &r_in, &r_out);
        if (r_out.w.ax != 0) m->available = 1;
    }
#else
    {
        union REGS r_in, r_out;
        memset(&r_in, 0, sizeof(r_in));
        r_in.x.ax = 0x0000;
        int86(0x33, &r_in, &r_out);
        if (r_out.x.ax != 0) m->available = 1;
    }
#endif
}

static void poll_legacy_mouse(BGC_Mouse *m) {
    int raw_x = m->x;
    int raw_y = m->y;
    unsigned int btns = 0;

    m->prev_x = m->x;
    m->prev_y = m->y;
    m->click_left = 0;
    m->click_right = 0;

    if (!m->available) return;

#if BGC_DJGPP_TARGET
    {
        __dpmi_regs r;
        memset(&r, 0, sizeof(r));
        r.x.ax = 0x0003;
        __dpmi_int(0x33, &r);
        btns = (unsigned int)r.x.bx;
        raw_x = (int)r.x.cx;
        raw_y = (int)r.x.dx;
    }
#elif defined(__386__)
    {
        union REGS r_in, r_out;
        memset(&r_in, 0, sizeof(r_in));
        r_in.w.ax = 0x0003;
        int386(0x33, &r_in, &r_out);
        btns = (unsigned int)r_out.w.bx;
        raw_x = (int)r_out.w.cx;
        raw_y = (int)r_out.w.dx;
    }
#else
    {
        union REGS r_in, r_out;
        memset(&r_in, 0, sizeof(r_in));
        r_in.x.ax = 0x0003;
        int86(0x33, &r_in, &r_out);
        btns = (unsigned int)r_out.x.bx;
        raw_x = (int)r_out.x.cx;
        raw_y = (int)r_out.x.dx;
    }
#endif

    if (raw_x > SCR_W) raw_x = raw_x / 2;
    if (raw_x < 0) raw_x = 0;
    if (raw_x >= SCR_W) raw_x = SCR_W - 1;
    if (raw_y < 0) raw_y = 0;
    if (raw_y >= SCR_H) raw_y = SCR_H - 1;

    m->x = raw_x;
    m->y = raw_y;
    m->left_down  = (btns & 0x01u) ? 1 : 0;
    m->right_down = (btns & 0x02u) ? 1 : 0;
    finalize_mouse_frame(m);
}

#else

static void update_win_mouse_pos(BGC_Game *g, HWND hwnd, LPARAM lParam) {
    RECT rc;
    int cw, ch;
    int px = (int)(short)LOWORD(lParam);
    int py = (int)(short)HIWORD(lParam);

    GetClientRect(hwnd, &rc);
    cw = rc.right - rc.left;
    ch = rc.bottom - rc.top;

    g->mouse.available = 1;
    g->mouse.prev_x = g->mouse.x;
    g->mouse.prev_y = g->mouse.y;

    if (cw > 0) g->mouse.x = (px * SCR_W) / cw;
    if (ch > 0) g->mouse.y = (py * SCR_H) / ch;

    if (g->mouse.x < 0) g->mouse.x = 0;
    if (g->mouse.x >= SCR_W) g->mouse.x = SCR_W - 1;
    if (g->mouse.y < 0) g->mouse.y = 0;
    if (g->mouse.y >= SCR_H) g->mouse.y = SCR_H - 1;
}

static void poll_legacy_mouse(BGC_Mouse *m) {
#if (BGC_WIN_ARCH != 16)
    if (GetSystemMetrics(SM_MOUSEPRESENT) != 0) {
        m->available = 1;
    }
#endif
    finalize_mouse_frame(m);
}

#endif

static void update_pad_edges(BGC_Pad *pad, int c_conf, int c_back, int c_pause, int c_retry) {
    pad->btn_confirm = (c_conf  && !pad->prev_confirm) ? 1 : 0;
    pad->btn_back    = (c_back  && !pad->prev_back)    ? 1 : 0;
    pad->btn_pause   = (c_pause && !pad->prev_pause)   ? 1 : 0;
    pad->btn_retry   = (c_retry && !pad->prev_retry)   ? 1 : 0;

    pad->prev_confirm = c_conf;
    pad->prev_back    = c_back;
    pad->prev_pause   = c_pause;
    pad->prev_retry   = c_retry;
}

#if BGC_DOS_TARGET

static void poll_single_pad(BGC_Pad *pad, int slot_idx) {
    unsigned int jx = 0, jy = 0;
    unsigned char buttons_mask = 0;
    (void)slot_idx;

    pad->dx = 0;
    pad->dy = 0;
    if (pad->nav_cooldown > 0) pad->nav_cooldown--;

#if BGC_DJGPP_TARGET
    {
        __dpmi_regs r;
        memset(&r, 0, sizeof(r));
        r.h.ah = 0x84;
        r.x.dx = 0x0000;
        __dpmi_int(0x15, &r);
        if (r.x.flags & 1u) {
            pad->connected = 0;
            update_pad_edges(pad, 0, 0, 0, 0);
            return;
        }
        buttons_mask = (unsigned char)((~r.h.al) >> 4) & 0x0Fu;

        memset(&r, 0, sizeof(r));
        r.h.ah = 0x84;
        r.x.dx = 0x0001;
        __dpmi_int(0x15, &r);
        if ((r.x.flags & 1u) || r.x.ax == 0 || r.x.bx == 0) {
            pad->connected = 0;
            update_pad_edges(pad, 0, 0, 0, 0);
            return;
        }
        jx = (unsigned int)r.x.ax;
        jy = (unsigned int)r.x.bx;
    }
#elif defined(__386__)
    {
        union REGS r_in, r_out;
        memset(&r_in, 0, sizeof(r_in));
        r_in.h.ah = 0x84;
        r_in.w.dx = 0x0000;
        int386(0x15, &r_in, &r_out);
        if (r_out.x.cflag & 1u) {
            pad->connected = 0;
            update_pad_edges(pad, 0, 0, 0, 0);
            return;
        }
        buttons_mask = (unsigned char)((~r_out.h.al) >> 4) & 0x0Fu;

        memset(&r_in, 0, sizeof(r_in));
        r_in.h.ah = 0x84;
        r_in.w.dx = 0x0001;
        int386(0x15, &r_in, &r_out);
        if ((r_out.x.cflag & 1u) || r_out.w.ax == 0 || r_out.w.bx == 0) {
            pad->connected = 0;
            update_pad_edges(pad, 0, 0, 0, 0);
            return;
        }
        jx = (unsigned int)r_out.w.ax;
        jy = (unsigned int)r_out.w.bx;
    }
#else
    {
        union REGS r_in, r_out;
        memset(&r_in, 0, sizeof(r_in));
        r_in.h.ah = 0x84;
        r_in.x.dx = 0x0000;
        int86(0x15, &r_in, &r_out);
        if (r_out.x.cflag & 1u) {
            pad->connected = 0;
            update_pad_edges(pad, 0, 0, 0, 0);
            return;
        }
        buttons_mask = (unsigned char)((~r_out.h.al) >> 4) & 0x0Fu;

        memset(&r_in, 0, sizeof(r_in));
        r_in.h.ah = 0x84;
        r_in.x.dx = 0x0001;
        int86(0x15, &r_in, &r_out);
        if ((r_out.x.cflag & 1u) || r_out.x.ax == 0 || r_out.x.bx == 0) {
            pad->connected = 0;
            update_pad_edges(pad, 0, 0, 0, 0);
            return;
        }
        jx = (unsigned int)r_out.x.ax;
        jy = (unsigned int)r_out.x.bx;
    }
#endif

    pad->connected = 1;
    if (!pad->calibrated) {
        pad->center_x = jx;
        pad->center_y = jy;
        pad->dead_x = (jx / 4 > 12) ? (jx / 4) : 12;
        pad->dead_y = (jy / 4 > 12) ? (jy / 4) : 12;
        pad->calibrated = 1;
    }

    if ((int)jx < (int)pad->center_x - (int)pad->dead_x) pad->dx = -1;
    else if ((int)jx > (int)pad->center_x + (int)pad->dead_x) pad->dx = 1;

    if ((int)jy < (int)pad->center_y - (int)pad->dead_y) pad->dy = -1;
    else if ((int)jy > (int)pad->center_y + (int)pad->dead_y) pad->dy = 1;

    update_pad_edges(pad,
                     (buttons_mask & 0x01) ? 1 : 0,
                     (buttons_mask & 0x02) ? 1 : 0,
                     (buttons_mask & 0x0C) ? 1 : 0,
                     (buttons_mask & 0x05) ? 1 : 0);
}

#else

static void poll_single_pad(BGC_Pad *pad, int slot_idx) {
    UINT id = (UINT)slot_idx;
    int found = 0;
    int rc = 0, rb = 0, rp = 0, rr = 0;

    pad->dx = 0;
    pad->dy = 0;
    if (pad->nav_cooldown > 0) pad->nav_cooldown--;

#if (BGC_WIN_ARCH != 16)
    {
        JOYINFOEX ji_ex;
        memset(&ji_ex, 0, sizeof(ji_ex));
        ji_ex.dwSize = sizeof(ji_ex);
        ji_ex.dwFlags = JOY_RETURNALL;

        if (joyGetPosEx(id, &ji_ex) == JOYERR_NOERROR) {
            DWORD mid = 32768, dead = 12000;
            found = 1;
            if (ji_ex.dwXpos < mid - dead) pad->dx = -1;
            else if (ji_ex.dwXpos > mid + dead) pad->dx = 1;
            if (ji_ex.dwYpos < mid - dead) pad->dy = -1;
            else if (ji_ex.dwYpos > mid + dead) pad->dy = 1;

            if (ji_ex.dwPOV != JOY_POVCENTERED && ji_ex.dwPOV < 36000) {
                if (ji_ex.dwPOV >= 31500 || ji_ex.dwPOV <= 4500) pad->dy = -1;
                if (ji_ex.dwPOV >= 4500  && ji_ex.dwPOV <= 13500) pad->dx = 1;
                if (ji_ex.dwPOV >= 13500 && ji_ex.dwPOV <= 22500) pad->dy = 1;
                if (ji_ex.dwPOV >= 22500 && ji_ex.dwPOV <= 31500) pad->dx = -1;
            }
            if (ji_ex.dwButtons & (JOY_BUTTON1 | JOY_BUTTON3)) rc = 1;
            if (ji_ex.dwButtons & (JOY_BUTTON2 | JOY_BUTTON4)) rb = 1;
            if (ji_ex.dwButtons & (JOY_BUTTON7 | JOY_BUTTON8 | JOY_BUTTON5 | JOY_BUTTON6)) rp = 1;
            if (ji_ex.dwButtons & (JOY_BUTTON1 | JOY_BUTTON4)) rr = 1;
        }
    }
#endif
    if (!found) {
        JOYINFO ji;
        if (joyGetPos(id, &ji) == JOYERR_NOERROR) {
            found = 1;
            if (ji.wXpos < 16000) pad->dx = -1;
            else if (ji.wXpos > 49000) pad->dx = 1;
            if (ji.wYpos < 16000) pad->dy = -1;
            else if (ji.wYpos > 49000) pad->dy = 1;
            if (ji.wButtons & JOY_BUTTON1) rc = 1;
            if (ji.wButtons & JOY_BUTTON2) rb = 1;
            if (ji.wButtons & (JOY_BUTTON3 | JOY_BUTTON4)) rp = 1;
            if (ji.wButtons & (JOY_BUTTON1 | JOY_BUTTON3)) rr = 1;
        }
    }

    pad->connected = found;
    update_pad_edges(pad, rc, rb, rp, rr);
}

#endif

/* ============================================================================
 * 7. TREES, GRASS, CARTOON BEE, BLACK BUGS, GOLDS & GOLD BAR
 * ============================================================================ */

static void draw_tree_box(const BGC_Game *g, void *hdc_ptr, int tx, int ground_y, int min_x, int max_x, int min_y) {
    if (tx < min_x || tx > max_x) return;
    draw_box(g, hdc_ptr, tx - 3, ground_y - 20, 6, 20, C_VGA_TRUNK, 1);
    if (ground_y - 26 > min_y) draw_circle_fill(g, hdc_ptr, tx, ground_y - 26, 9, C_VGA_LEAF, 1);
}

static void draw_viewport_env(const BGC_Game *g, void *hdc_ptr, int vx, int vy, int vw, int vh) {
    int ground_y = vy + vh - 20;
    int offset = (int)(((g->tick * 6l * BGC_SPEED_NUM) / BGC_SPEED_DEN) % 64l);
    int tx;

    draw_box(g, hdc_ptr, vx, vy, vw, vh - 20, C_VGA_SKY, 0);
    for (tx = vx + 24 - offset; tx < vx + vw + 32; tx += 64) {
        draw_tree_box(g, hdc_ptr, tx, ground_y, vx, vx + vw, vy);
    }
    draw_box(g, hdc_ptr, vx, ground_y, vw, 20, C_VGA_GRASS, 0);
    draw_line_simple(g, hdc_ptr, vx, ground_y, vx + vw - 1, ground_y, C_VGA_BLACK);
}

static void draw_cartoon_bee(const BGC_Game *g, void *hdc_ptr, int x, int y, long anim_frame) {
    int wing_lift = ((anim_frame % 2l) == 0) ? -4 : 2;
    int leg_swing = ((anim_frame % 2l) == 0) ? -2 : 2;

    draw_circle_fill(g, hdc_ptr, x - 2, y - 5 + wing_lift, 4, C_VGA_WING, 1);
    draw_circle_fill(g, hdc_ptr, x + 2, y - 4 + wing_lift, 3, C_VGA_WING, 1);

    draw_line_simple(g, hdc_ptr, x + 6, y - 3, x + 10, y - 8, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 8, y - 2, x + 12, y - 6, C_VGA_BLACK);

    draw_line_simple(g, hdc_ptr, x - 3, y + 4, x - 5 + leg_swing, y + 8, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 2, y + 4, x + 4 + leg_swing, y + 8, C_VGA_BLACK);

    draw_circle_fill(g, hdc_ptr, x, y, 6, C_VGA_YELLOW, 1);
    draw_line_simple(g, hdc_ptr, x - 2, y - 5, x - 2, y + 5, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 2, y - 5, x + 2, y + 5, C_VGA_BLACK);

    draw_circle_fill(g, hdc_ptr, x + 4, y - 2, 2, C_VGA_WING, 1);
    draw_px(g, hdc_ptr, x + 5, y - 2, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x - 6, y, x - 9, y, C_VGA_BLACK);
}

static void draw_mini_bee_race(const BGC_Game *g, void *hdc_ptr, int x, int y, long anim_frame) {
    int wing_lift = ((anim_frame % 2l) == 0) ? -3 : 1;
    int leg_swing = ((anim_frame % 2l) == 0) ? -1 : 1;

    draw_circle_fill(g, hdc_ptr, x - 1, y - 3 + wing_lift, 3, C_VGA_WING, 1);
    draw_line_simple(g, hdc_ptr, x + 4, y - 2, x + 7, y - 6, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x - 2, y + 3, x - 3 + leg_swing, y + 6, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 2, y + 3, x + 3 + leg_swing, y + 6, C_VGA_BLACK);

    draw_circle_fill(g, hdc_ptr, x, y, 4, C_VGA_YELLOW, 1);
    draw_line_simple(g, hdc_ptr, x - 1, y - 3, x - 1, y + 3, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 2, y - 3, x + 2, y + 3, C_VGA_BLACK);

    draw_px(g, hdc_ptr, x + 2, y - 1, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x - 4, y, x - 6, y, C_VGA_BLACK);
}

static void draw_black_bug(const BGC_Game *g, void *hdc_ptr, int x, int y, long anim_frame) {
    int leg_step = ((anim_frame % 2l) == 0) ? 2 : -2;
    int wing_up  = ((anim_frame % 2l) == 0) ? -3 : -1;

    draw_line_simple(g, hdc_ptr, x - 5, y - 2, x - 9, y - 7, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x - 4, y - 3, x - 7, y - 8, C_VGA_BLACK);

    draw_line_simple(g, hdc_ptr, x - 2, y + 4, x - 4 + leg_step, y + 8, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 2, y + 4, x + 4 + leg_step, y + 8, C_VGA_BLACK);

    draw_circle_fill(g, hdc_ptr, x + 1, y - 3 + wing_up, 3, C_VGA_WING, 1);
    draw_circle_fill(g, hdc_ptr, x, y, 5, C_VGA_BLACK, 1);
    draw_circle_fill(g, hdc_ptr, x - 3, y - 2, 2, C_VGA_WING, 1);
    draw_px(g, hdc_ptr, x - 4, y - 2, C_VGA_RED);
}

static void draw_mini_bug_race(const BGC_Game *g, void *hdc_ptr, int x, int y, long anim_frame) {
    int leg_step = ((anim_frame % 2l) == 0) ? 1 : -1;

    draw_line_simple(g, hdc_ptr, x - 3, y - 2, x - 6, y - 5, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x - 2, y + 3, x - 3 + leg_step, y + 6, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, x + 2, y + 3, x + 3 + leg_step, y + 6, C_VGA_BLACK);

    draw_circle_fill(g, hdc_ptr, x, y, 4, C_VGA_BLACK, 1);
    draw_px(g, hdc_ptr, x - 2, y - 1, C_VGA_RED);
}

static void draw_gold_coin(const BGC_Game *g, void *hdc_ptr, int x, int y) {
    draw_circle_fill(g, hdc_ptr, x, y, 4, C_VGA_YELLOW, 1);
    draw_px(g, hdc_ptr, x - 1, y - 1, C_VGA_WING);
}

static void draw_mini_gold_race(const BGC_Game *g, void *hdc_ptr, int x, int y) {
    draw_circle_fill(g, hdc_ptr, x, y, 3, C_VGA_YELLOW, 1);
    draw_px(g, hdc_ptr, x - 1, y - 1, C_VGA_WING);
}

static void draw_gold_bar(const BGC_Game *g, void *hdc_ptr, int x, int y) {
    draw_box(g, hdc_ptr, x - 7, y - 4, 14, 8, C_VGA_YELLOW, 1);
    draw_line_simple(g, hdc_ptr, x - 5, y - 2, x + 4, y - 2, C_VGA_WING);
    draw_px(g, hdc_ptr, x - 5, y - 1, C_VGA_WING);
}

static void draw_mini_gold_bar_race(const BGC_Game *g, void *hdc_ptr, int x, int y) {
    draw_box(g, hdc_ptr, x - 5, y - 3, 10, 6, C_VGA_YELLOW, 1);
    draw_px(g, hdc_ptr, x - 3, y - 1, C_VGA_WING);
}

static void draw_mouse_cursor(const BGC_Game *g, void *hdc_ptr) {
    int mx = g->mouse.x;
    int my = g->mouse.y;
    if (!g->mouse.available) return;
    if (mx < 0 || mx >= SCR_W || my < 0 || my >= SCR_H) return;

    draw_line_simple(g, hdc_ptr, mx, my, mx, my + 7, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, mx, my, mx + 5, my + 5, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, mx + 2, my + 4, mx + 5, my + 5, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, mx + 2, my + 4, mx + 3, my + 7, C_VGA_BLACK);
    draw_line_simple(g, hdc_ptr, mx + 1, my + 2, mx + 1, my + 5, C_VGA_WING);
    draw_px(g, hdc_ptr, mx + 2, my + 3, C_VGA_WING);
}

/* ============================================================================
 * 8. SINGLE & BEERACE GAMEPLAY LOGIC (+5.5% SPEED & SMOOTH BEE SPEED)
 * ============================================================================ */

static int is_gameplay_screen(const BGC_Game *g) {
    return (g->screen == SCREEN_PLAYING || g->screen == SCREEN_BEERACE) ? 1 : 0;
}

static int mouse_allowed_in_mode(const BGC_Game *g) {
    if (is_gameplay_screen(g) &&
        (g->ctrl_source == CTRL_SRC_KEYBOARD || g->ctrl_source == CTRL_SRC_GAMEPAD)) {
        return 0;
    }
    return 1;
}

static int random_y_in_range(int min_y, int max_y) {
    int span = max_y - min_y;
    if (span <= 1) return min_y;
    return min_y + (rand() % span);
}

static int wave_offset_from_phase(int phase) {
    int p = phase & 31;
    if (p < 8)  return (p < 4) ? 1 : 0;
    if (p < 16) return ((15 - p) < 4) ? 1 : 0;
    if (p < 24) return ((p - 16) < 4) ? -1 : 0;
    return ((31 - p) < 4) ? -1 : 0;
}

static int surge_x_from_phase(int phase) {
    int p = phase & 31;
    if (p < 8)  return -1;
    if (p < 16) return  0;
    if (p < 24) return  1;
    return  0;
}

/* Birim hizi tam %5.5 (1.055x) ileri tasiyan birikimli adim hesaplayici */
static int advance_scaled_units(long *accum, int base_units) {
    long total = (*accum) + ((long)base_units * BGC_SPEED_NUM);
    int steps = (int)(total / BGC_SPEED_DEN);
    *accum = total % BGC_SPEED_DEN;
    return steps;
}

static void spawn_single_gold_slot(BGC_Game *g, int idx) {
    if (idx < 0 || idx >= MAX_GOLDS) return;
    g->gold_x[idx] = SCR_W - 12 - (idx * 24);
    g->gold_y[idx] = random_y_in_range(16, SCR_H - 30);
}

static void spawn_single_bug_slot(BGC_Game *g, int idx) {
    if (idx < 0 || idx >= MAX_BUGS) return;
    g->bug_x[idx] = SCR_W + 12 + (idx * 22);
    g->bug_y[idx] = random_y_in_range(16, SCR_H - 30);
    g->bug_phase[idx] = (idx * 7 + rand()) & 31;
    g->bug_vx_phase[idx] = (idx * 5 + rand()) & 31;
    g->bug_speed[idx] = 3 + (idx % 2) + (g->score / 60);
    if (g->bug_speed[idx] > 8) g->bug_speed[idx] = 8;
}

static void spawn_single_gold_bar(BGC_Game *g) {
    g->gold_bar_x = SCR_W - 16;
    g->gold_bar_y = random_y_in_range(16, SCR_H - 30);
    g->gold_bar_active = 1;
}

static void start_single_game(BGC_Game *g) {
    int i;
    g->host_is_p1 = 1;
    g->bee_x = SCR_W / 5;
    g->bee_y = SCR_H / 2;
    g->bee_vx = 0;
    g->bee_vy = 0;
    g->sub_step = 0;
    g->score = 0;
    g->health = 100;
    g->game_over = 0;
    g->gameover_idx = 0;
    g->tick = 0;
    g->replay_recording = 1;
    g->replay_count = 0;
    g->pause_idx = 0;
    g->screen = SCREEN_PLAYING;

    for (i = 0; i < MAX_GOLDS; i++) spawn_single_gold_slot(g, i);
    for (i = 0; i < MAX_BUGS;  i++) spawn_single_bug_slot(g, i);
    spawn_single_gold_bar(g);
    set_dualsense_led_from_health(g, g->health);
}

static void spawn_racer_gold_slot(Racer *r, int idx, int pane_w, int pane_h, int seed_offset) {
    int max_y = (pane_h > 44) ? (pane_h - 24) : 24;
    if (idx < 0 || idx >= MAX_GOLDS) return;
    r->gold_x[idx] = pane_w - 10 - (idx * 18);
    r->gold_y[idx] = 14 + ((rand() + seed_offset + idx * 13) % (max_y - 12));
}

static void spawn_racer_bug_slot(Racer *r, int idx, int pane_w, int pane_h, int seed_offset) {
    int max_y = (pane_h > 44) ? (pane_h - 24) : 24;
    if (idx < 0 || idx >= MAX_BUGS) return;
    r->bug_x[idx] = pane_w + 8 + (idx * 15);
    r->bug_y[idx] = 14 + ((rand() + seed_offset * 3 + idx * 17) % (max_y - 12));
    r->bug_phase[idx] = (idx * 5 + seed_offset + rand()) & 31;
    r->bug_vx_phase[idx] = (idx * 7 + seed_offset + rand()) & 31;
    r->bug_speed[idx] = 3 + (idx % 2) + (r->score / 60);
    if (r->bug_speed[idx] > 8) r->bug_speed[idx] = 8;
}

static void spawn_racer_gold_bar(Racer *r, int pane_w, int pane_h, int seed_offset) {
    int max_y = (pane_h > 44) ? (pane_h - 24) : 24;
    r->gold_bar_x = pane_w - 12;
    r->gold_bar_y = 14 + ((rand() + seed_offset * 5) % (max_y - 12));
    r->gold_bar_active = 1;
}

static void init_racer(Racer *r, int pane_w, int pane_h, int seed_offset) {
    int i;
    r->x = pane_w / 5;
    r->y = pane_h / 2;
    r->vx = 0;
    r->vy = 0;
    r->sub_step = 0;
    r->score = 0;
    r->health = 100;
    r->dead = 0;

    for (i = 0; i < MAX_GOLDS; i++) spawn_racer_gold_slot(r, i, pane_w, pane_h, seed_offset);
    for (i = 0; i < MAX_BUGS;  i++) spawn_racer_bug_slot(r, i, pane_w, pane_h, seed_offset);
    spawn_racer_gold_bar(r, pane_w, pane_h, seed_offset);
}

static void open_beerace_lobby(BGC_Game *g) {
    int half_w = SCR_W / 2;
    g->host_is_p1 = 1;
    init_racer(&g->p1, half_w, SCR_H, 7);
    init_racer(&g->p2, half_w, SCR_H, 19);
    g->race_started = 0;
    g->p1_ready = 1;
    g->p2_ready = g->pad2.connected ? 1 : 0;
    g->p2_is_pad = g->pad2.connected ? 1 : 0;
    g->race_over = 0;
    g->race_winner = 0;
    g->regen_timer_ticks = 0;
    g->tick = 0;
    g->pause_idx = 0;
    g->screen = SCREEN_BEERACE;
}

static void launch_beerace_match(BGC_Game *g) {
    int half_w = SCR_W / 2;
    g->host_is_p1 = 1;
    init_racer(&g->p1, half_w, SCR_H, 7);
    init_racer(&g->p2, half_w, SCR_H, 19);
    g->race_started = 1;
    g->race_over = 0;
    g->race_winner = 0;
    g->gameover_idx = 0;
    g->regen_timer_ticks = 0;
    g->tick = 0;
    g->pause_idx = 0;
    set_dualsense_led_from_health(g, 100);
}

static void apply_smooth_move_single(BGC_Game *g, int dx, int dy, int is_tap) {
    int v_step = is_tap ? 2 : 1;
    int max_up = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? -4 : -3;
    int max_dn = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ?  4 :  3;
    (void)dx;

    if (g->screen != SCREEN_PLAYING || g->game_over) return;

    g->bee_vx = 0;
    if (dy < 0) {
        if (g->bee_vy > 0) g->bee_vy = 0;
        g->bee_vy -= v_step;
        if (g->bee_vy < max_up) g->bee_vy = max_up;
    } else if (dy > 0) {
        if (g->bee_vy < 0) g->bee_vy = 0;
        g->bee_vy += v_step;
        if (g->bee_vy > max_dn) g->bee_vy = max_dn;
    }
}

static void update_smooth_mouse_follow_single(BGC_Game *g) {
    int diff_y;
    int gain = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? 8 : 10;
    int max_spd = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? 3 : 2;

    if (!g->mouse.follow_mode || !mouse_allowed_in_mode(g)) return;
    if (!g->mouse.available) return;
    if (g->screen != SCREEN_PLAYING || g->game_over) return;

    diff_y = g->mouse.y - g->bee_y;
    if (abs(diff_y) > 3) {
        g->bee_vy = diff_y / gain;
        if (g->bee_vy > max_spd) g->bee_vy = max_spd;
        if (g->bee_vy < -max_spd) g->bee_vy = -max_spd;
    }
}

static void update_smooth_bee_physics(BGC_Game *g) {
    int vy_step;
    if (g->screen != SCREEN_PLAYING || g->game_over) return;

    update_smooth_mouse_follow_single(g);

    g->bee_x = SCR_W / 5;
    g->bee_vx = 0;
    vy_step = advance_scaled_units(&g->sub_step, g->bee_vy);
    g->bee_y += vy_step;
    g->bee_vy = g->bee_vy / 2;

    if (g->bee_y < 10) { g->bee_y = 10; g->bee_vy = 0; }
    if (g->bee_y > SCR_H - 26) { g->bee_y = SCR_H - 26; g->bee_vy = 0; }
}

static void apply_smooth_move_racer(const BGC_Game *g, Racer *r, int dx, int dy, int is_tap) {
    int v_step = is_tap ? 2 : 1;
    int max_up = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? -4 : -3;
    int max_dn = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ?  4 :  3;
    (void)dx;

    if (r->dead) return;

    r->vx = 0;
    if (dy < 0) {
        if (r->vy > 0) r->vy = 0;
        r->vy -= v_step;
        if (r->vy < max_up) r->vy = max_up;
    } else if (dy > 0) {
        if (r->vy < 0) r->vy = 0;
        r->vy += v_step;
        if (r->vy > max_dn) r->vy = max_dn;
    }
}

static void update_smooth_mouse_follow_racer(const BGC_Game *g, Racer *r, int pane_w, int pane_h, int x_offset) {
    int target_y = g->mouse.y;
    int diff_y;
    int gain = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? 8 : 10;
    int max_spd = (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? 3 : 2;
    (void)pane_w;
    (void)x_offset;

    if (!g->mouse.follow_mode || !mouse_allowed_in_mode(g)) return;
    if (!g->mouse.available || r->dead) return;
    if (target_y < 24) target_y = 24;
    if (target_y > pane_h - 25) target_y = pane_h - 25;

    diff_y = target_y - r->y;
    if (abs(diff_y) > 3) {
        r->vy = diff_y / gain;
        if (r->vy > max_spd) r->vy = max_spd;
        if (r->vy < -max_spd) r->vy = -max_spd;
    }
}

static void update_smooth_racer_physics(Racer *r, int pane_w, int pane_h) {
    int vy_step;
    if (r->dead) return;

    r->x = pane_w / 5;
    r->vx = 0;
    vy_step = advance_scaled_units(&r->sub_step, r->vy);
    r->y += vy_step;
    r->vy = r->vy / 2;

    if (r->y < 24) { r->y = 24; r->vy = 0; }
    if (r->y > pane_h - 25) { r->y = pane_h - 25; r->vy = 0; }
}

static void step_racer(BGC_Game *g, Racer *r, int pane_w, int pane_h, int x_offset, int is_p2, int seed_offset) {
    long world_move = (long)(2 + (int)((g->tick * BGC_SPEED_NUM) / BGC_SPEED_DEN) - (int)g->tick);
    int i;
    if (r->dead) return;

    if (is_p2 && g->mouse.act_as_pad2 && !g->pad2.connected) {
        update_smooth_mouse_follow_racer(g, r, pane_w, pane_h, x_offset);
    }
    update_smooth_racer_physics(r, pane_w, pane_h);

    for (i = 0; i < MAX_GOLDS; i++) {
        int g_spd = (int)(((long)(4 + (i % 2)) * BGC_SPEED_NUM) / BGC_SPEED_DEN) + (int)world_move;
        r->gold_x[i] -= g_spd;
        if (r->gold_x[i] < 6) {
            spawn_racer_gold_slot(r, i, pane_w, pane_h, seed_offset + (int)g->tick);
        }
        if (abs(r->x - r->gold_x[i]) < 10 && abs(r->y - r->gold_y[i]) < 10) {
            r->score += 10;
            r->health += 15;
            if (r->health > 100) r->health = 100;
            trigger_rumble_and_led(g, 14000, 14000, 2, 255, 215, 0);
            spawn_racer_gold_slot(r, i, pane_w, pane_h, seed_offset + (int)g->tick);
        }
    }

    if (!r->gold_bar_active && ((g->tick % 110l) == 0)) {
        spawn_racer_gold_bar(r, pane_w, pane_h, seed_offset + (int)g->tick);
    }
    if (r->gold_bar_active) {
        int b_spd = (int)((4l * BGC_SPEED_NUM) / BGC_SPEED_DEN) + (int)world_move;
        r->gold_bar_x -= b_spd;
        if (r->gold_bar_x < 8) {
            r->gold_bar_active = 0;
        } else if (abs(r->x - r->gold_bar_x) < 11 && abs(r->y - r->gold_bar_y) < 10) {
            r->score += 30;
            r->health += 25;
            if (r->health > 100) r->health = 100;
            r->gold_bar_active = 0;
            trigger_rumble_and_led(g, 26000, 26000, 4, 255, 235, 30);
        }
    }

    for (i = 0; i < MAX_BUGS; i++) {
        int vx_step = 0;
        int spd_scaled = (int)(((long)r->bug_speed[i] * BGC_SPEED_NUM) / BGC_SPEED_DEN);
        r->bug_vx_phase[i] = (r->bug_vx_phase[i] + 1) & 31;
        if ((r->bug_vx_phase[i] & 1) == 0) {
            vx_step = surge_x_from_phase(r->bug_vx_phase[i]);
        }
        r->bug_x[i] -= spd_scaled + vx_step;
        if (r->bug_x[i] > pane_w + 18) r->bug_x[i] = pane_w + 18;

        r->bug_phase[i] = (r->bug_phase[i] + 1) & 31;
        if ((r->bug_phase[i] & 1) == 0) {
            r->bug_y[i] += wave_offset_from_phase(r->bug_phase[i]);
            if (r->bug_y[i] < 14) r->bug_y[i] = 14;
            if (r->bug_y[i] > pane_h - 25) r->bug_y[i] = pane_h - 25;
        }
        if (r->bug_x[i] < 6) {
            spawn_racer_bug_slot(r, i, pane_w, pane_h, seed_offset + (int)g->tick);
        }
        if (abs(r->x - r->bug_x[i]) < 10 && abs(r->y - r->bug_y[i]) < 10) {
            r->health -= 35;
            trigger_rumble_and_led(g, 48000, 48000, 5, 240, 25, 25);
            spawn_racer_bug_slot(r, i, pane_w, pane_h, seed_offset + (int)g->tick);
            if (r->health <= 0) {
                r->health = 0;
                r->dead = 1;
                return;
            }
        }
    }
}

static void step_beerace(BGC_Game *g) {
    int half_w = SCR_W / 2;
    if (!g->race_started || g->race_over) return;

    g->regen_timer_ticks++;
    if (g->regen_timer_ticks >= 480l) {
        g->regen_timer_ticks = 0;
        if (!g->p1.dead && g->p1.health < 100) {
            g->p1.health += 2;
            if (g->p1.health > 100) g->p1.health = 100;
        }
        if (!g->p2.dead && g->p2.health < 100) {
            g->p2.health += 2;
            if (g->p2.health > 100) g->p2.health = 100;
        }
    }

    step_racer(g, &g->p1, half_w, SCR_H, 0, 0, 7);
    step_racer(g, &g->p2, half_w, SCR_H, half_w, 1, 19);

    if (g->p1.dead && !g->p2.dead) {
        g->race_over = 1;
        g->race_winner = 2;
        g->gameover_idx = 0;
        record_score_secure(g, g->p2.score);
    } else if (g->p2.dead && !g->p1.dead) {
        g->race_over = 1;
        g->race_winner = 1;
        g->gameover_idx = 0;
        record_score_secure(g, g->p1.score);
    } else if (g->p1.dead && g->p2.dead) {
        g->race_over = 1;
        g->race_winner = (g->p1.score >= g->p2.score) ? 1 : 2;
        g->gameover_idx = 0;
        record_score_secure(g, (g->p1.score >= g->p2.score) ? g->p1.score : g->p2.score);
    }
}

static void step_single_game(BGC_Game *g) {
    int i;
    if (g->game_over) return;

    update_smooth_bee_physics(g);

    for (i = 0; i < MAX_GOLDS; i++) {
        int g_spd = (int)(((long)(4 + (i % 2)) * BGC_SPEED_NUM) / BGC_SPEED_DEN);
        g->gold_x[i] -= g_spd;
        if (g->gold_x[i] < 8) {
            spawn_single_gold_slot(g, i);
        }
        if (abs(g->bee_x - g->gold_x[i]) < 13 && abs(g->bee_y - g->gold_y[i]) < 13) {
            g->score += 10;
            g->health += 15;
            if (g->health > 100) g->health = 100;
            trigger_rumble_and_led(g, 14000, 14000, 2, 255, 215, 0);
            spawn_single_gold_slot(g, i);
        }
    }

    if (!g->gold_bar_active && ((g->tick % 110l) == 0)) {
        spawn_single_gold_bar(g);
    }
    if (g->gold_bar_active) {
        int b_spd = (int)((4l * BGC_SPEED_NUM) / BGC_SPEED_DEN);
        g->gold_bar_x -= b_spd;
        if (g->gold_bar_x < 8) {
            g->gold_bar_active = 0;
        } else if (abs(g->bee_x - g->gold_bar_x) < 15 && abs(g->bee_y - g->gold_bar_y) < 13) {
            g->score += 30;
            g->health += 25;
            if (g->health > 100) g->health = 100;
            g->gold_bar_active = 0;
            trigger_rumble_and_led(g, 28000, 28000, 4, 255, 235, 30);
        }
    }

    for (i = 0; i < MAX_BUGS; i++) {
        int vx_step = 0;
        int spd_scaled = (int)(((long)g->bug_speed[i] * BGC_SPEED_NUM) / BGC_SPEED_DEN);
        g->bug_vx_phase[i] = (g->bug_vx_phase[i] + 1) & 31;
        if ((g->bug_vx_phase[i] & 1) == 0) {
            vx_step = surge_x_from_phase(g->bug_vx_phase[i]);
        }
        g->bug_x[i] -= spd_scaled + vx_step;
        if (g->bug_x[i] > SCR_W + 24) g->bug_x[i] = SCR_W + 24;

        g->bug_phase[i] = (g->bug_phase[i] + 1) & 31;
        if ((g->bug_phase[i] & 1) == 0) {
            g->bug_y[i] += wave_offset_from_phase(g->bug_phase[i]);
            if (g->bug_y[i] < 14) g->bug_y[i] = 14;
            if (g->bug_y[i] > SCR_H - 28) g->bug_y[i] = SCR_H - 28;
        }
        if (g->bug_x[i] < 8) {
            spawn_single_bug_slot(g, i);
        }
        if (abs(g->bee_x - g->bug_x[i]) < 13 && abs(g->bee_y - g->bug_y[i]) < 13) {
            g->health -= 35;
            trigger_rumble_and_led(g, 48000, 48000, 5, 240, 25, 25);
            spawn_single_bug_slot(g, i);
            if (g->health <= 0) {
                g->health = 0;
                g->game_over = 1;
                g->gameover_idx = 0;
                g->replay_recording = 0;
                save_replay_to_file(g);
                record_score_secure(g, g->score);
                trigger_rumble_and_led(g, 60000, 60000, 8, 255, 20, 20);
                return;
            }
        }
    }

    capture_replay_sample(g);
}

static void step_world(BGC_Game *g) {
    g->tick++;
    update_rumble_timer(g);
    if (g->shot_saved_banner_ticks > 0) g->shot_saved_banner_ticks--;
    if (g->video_saved_banner_ticks > 0) g->video_saved_banner_ticks--;
    if (g->replay_saved_banner_ticks > 0) g->replay_saved_banner_ticks--;

    if (g->screen == SCREEN_PLAYING) {
        step_single_game(g);
    } else if (g->screen == SCREEN_BEERACE) {
        step_beerace(g);
    } else if (g->screen == SCREEN_MEDIA_VAULT) {
        if (g->media_tab == MEDIA_TAB_REPLAY && g->replay_playing && g->replay_count > 0) {
            g->replay_play_idx++;
            if (g->replay_play_idx >= g->replay_count) {
                g->replay_play_idx = 0;
                g->replay_playing = 0;
            }
        } else if (g->media_tab == MEDIA_TAB_VIDEO && g->video_playing && g->video_frame_count > 0) {
            if ((g->tick % 2l) == 0) {
                g->video_play_idx++;
                if (g->video_play_idx >= g->video_frame_count) {
                    g->video_play_idx = 0;
                    g->video_playing = 0;
                }
            }
        }
    }
}

/* ============================================================================
 * 9. UNIFIED MEDIA VAULT (PHOTO + REPLAY + VIDEO IN ONE SCREEN)
 * ============================================================================ */

static void sync_media_tab_state(BGC_Game *g) {
    g->replay_play_idx = 0;
    g->video_play_idx = 0;
    g->replay_playing = (g->media_tab == MEDIA_TAB_REPLAY && g->replay_count > 0) ? 1 : 0;
    g->video_playing  = (g->media_tab == MEDIA_TAB_VIDEO && g->video_frame_count > 0) ? 1 : 0;
}

static void open_media_vault(BGC_Game *g) {
    if (g->replay_count == 0) load_replay_from_file(g);
    if (g->video_frame_count == 0) load_video_from_file(g);
    sync_media_tab_state(g);
    g->screen = SCREEN_MEDIA_VAULT;
}

static void set_media_tab(BGC_Game *g, int new_tab) {
    if (new_tab < MEDIA_TAB_PHOTO) new_tab = MEDIA_TAB_PHOTO;
    if (new_tab > MEDIA_TAB_VIDEO) new_tab = MEDIA_TAB_VIDEO;
    g->media_tab = new_tab;
    sync_media_tab_state(g);
}

static void switch_media_tab(BGC_Game *g, int dir) {
    g->media_tab = (g->media_tab + (dir > 0 ? 1 : 2)) % 3;
    sync_media_tab_state(g);
}

static void save_current_media_to_device(BGC_Game *g) {
    if (g->media_tab == MEDIA_TAB_PHOTO) {
        if (g->has_photo_buf && g->photo_buf) write_frame_as_bmp(g, g->photo_buf);
        else save_screenshot_bmp(g);
    } else if (g->media_tab == MEDIA_TAB_REPLAY) {
        save_replay_to_file(g);
    } else if (g->media_tab == MEDIA_TAB_VIDEO) {
        save_video_to_file(g);
    }
}

static void toggle_current_media_play(BGC_Game *g) {
    if (g->media_tab == MEDIA_TAB_PHOTO) {
        save_current_media_to_device(g);
    } else if (g->media_tab == MEDIA_TAB_REPLAY) {
        if (g->replay_count > 0) g->replay_playing = !g->replay_playing;
    } else if (g->media_tab == MEDIA_TAB_VIDEO) {
        if (g->video_frame_count > 0) g->video_playing = !g->video_playing;
    }
}

static void activate_pause_item(BGC_Game *g) {
    if (g->pause_idx == 0) {
        g->screen = g->prev_screen;
    } else if (g->pause_idx == 1) {
        g->settings_idx = 0;
        g->screen = SCREEN_SETTINGS;
    } else if (g->pause_idx == 2) {
        g->menu_idx = 0;
        g->screen = SCREEN_MAIN_MENU;
    }
}

static void activate_gameover_item(BGC_Game *g, int is_race) {
    if (g->gameover_idx == 0) {
        if (is_race) launch_beerace_match(g);
        else start_single_game(g);
    } else if (g->gameover_idx == 1) {
        g->menu_idx = 0;
        g->game_over = 0;
        g->race_over = 0;
        g->race_started = 0;
        g->screen = SCREEN_MAIN_MENU;
    } else if (g->gameover_idx == 2) {
        g->pause_style = (g->pause_style == PAUSE_STYLE_FULL) ? PAUSE_STYLE_COMPACT : PAUSE_STYLE_FULL;
        write_obfuscated_options(g);
    }
}

static void toggle_setting_item(BGC_Game *g) {
    g->backend = BACKEND_VGA;
    if (g->settings_idx == 0) {
        g->pause_style = (g->pause_style == PAUSE_STYLE_FULL) ? PAUSE_STYLE_COMPACT : PAUSE_STYLE_FULL;
    } else if (g->settings_idx == 1) {
        g->ctrl_speed_mode = (g->ctrl_speed_mode == CTRL_SPEED_NORMAL) ? CTRL_SPEED_FAST : CTRL_SPEED_NORMAL;
    } else if (g->settings_idx == 2) {
        g->mouse.follow_mode = g->mouse.follow_mode ? 0 : 1;
    } else if (g->settings_idx == 3) {
        g->ctrl_source = (g->ctrl_source + 1) % 4;
    } else if (g->settings_idx == 4) {
        g->mouse.act_as_pad2 = g->mouse.act_as_pad2 ? 0 : 1;
    } else if (g->settings_idx == 5) {
        g->screen = g->prev_screen;
    }
    write_obfuscated_options(g);
}

static void activate_main_item(BGC_Game *g, int *running) {
    g->host_is_p1 = 1;
    if (g->menu_idx == 0) {
        start_single_game(g);
    } else if (g->menu_idx == 1) {
        open_beerace_lobby(g);
    } else if (g->menu_idx == 2) {
        g->wifi_status = 0;
        g->screen = SCREEN_WIFI_SCORE;
    } else if (g->menu_idx == 3) {
        open_media_vault(g);
    } else if (g->menu_idx == 4) {
        g->prev_screen = SCREEN_MAIN_MENU;
        g->screen = SCREEN_CONTROLS;
    } else if (g->menu_idx == 5) {
        g->prev_screen = SCREEN_MAIN_MENU;
        g->settings_idx = 0;
        g->screen = SCREEN_SETTINGS;
    } else if (g->menu_idx == 6 && running) {
        *running = 0;
    }
}

static int point_in_box(int px, int py, int bx, int by, int bw, int bh) {
    return (px >= bx && px < bx + bw && py >= by && py < by + bh) ? 1 : 0;
}

static void apply_mouse_to_game(BGC_Game *g, int *running) {
    BGC_Mouse *m = &g->mouse;
    int pause_requested = 0;

    poll_legacy_mouse(m);

    if (m->left_down && m->right_down && !m->pause_chord_fired) {
        m->pause_chord_fired = 1;
        pause_requested = 1;
    } else if (m->click_right) {
        pause_requested = 1;
    }

    if (g->screen == SCREEN_MAIN_MENU) {
        int i;
        for (i = 0; i < 7; i++) {
            int y = 44 + i * 19;
            if (point_in_box(m->x, m->y, 68, y - 3, 184, 13)) {
                g->menu_idx = i;
                if (m->click_left) {
                    activate_main_item(g, running);
                    return;
                }
            }
        }
        return;
    }

    if (g->screen == SCREEN_MEDIA_VAULT) {
        if (m->click_left) {
            if (point_in_box(m->x, m->y, 12, 6, 92, 10)) {
                set_media_tab(g, MEDIA_TAB_PHOTO);
                return;
            } else if (point_in_box(m->x, m->y, 114, 6, 92, 10)) {
                set_media_tab(g, MEDIA_TAB_REPLAY);
                return;
            } else if (point_in_box(m->x, m->y, 216, 6, 92, 10)) {
                set_media_tab(g, MEDIA_TAB_VIDEO);
                return;
            } else if (point_in_box(m->x, m->y, 12, 166, 296, 15)) {
                toggle_current_media_play(g);
                return;
            }
        }
        return;
    }

    if (g->screen == SCREEN_PLAYING) {
        if (g->game_over) {
            int bx = 58, by = 42, bw = 204, i;
            for (i = 0; i < 3; i++) {
                int y = by + 30 + i * 22;
                if (point_in_box(m->x, m->y, bx + 10, y - 3, bw - 20, 14)) {
                    g->gameover_idx = i;
                    if (m->click_left) {
                        activate_gameover_item(g, 0);
                        return;
                    }
                }
            }
            return;
        }

        if (pause_requested) {
            g->prev_screen = SCREEN_PLAYING;
            g->pause_idx = 0;
            g->screen = SCREEN_PAUSED;
            return;
        }

        if (!m->follow_mode && mouse_allowed_in_mode(g) &&
            (m->left_down || m->y != m->prev_y)) {
            int mdy = 0;
            if (m->y < g->bee_y - 4) mdy = -1;
            else if (m->y > g->bee_y + 4) mdy = 1;
            if (mdy != 0) {
                apply_smooth_move_single(g, 0, mdy, m->click_left ? 1 : 0);
            }
        }
    } else if (g->screen == SCREEN_BEERACE) {
        if (!g->race_started) {
            if (m->click_left && point_in_box(m->x, m->y, 54, 108, 212, 20)) {
                launch_beerace_match(g);
                return;
            }
            return;
        }
        if (g->race_over) {
            int bx = 58, by = 42, bw = 204, i;
            for (i = 0; i < 3; i++) {
                int y = by + 30 + i * 22;
                if (point_in_box(m->x, m->y, bx + 10, y - 3, bw - 20, 14)) {
                    g->gameover_idx = i;
                    if (m->click_left) {
                        activate_gameover_item(g, 1);
                        return;
                    }
                }
            }
            return;
        }
        if (pause_requested) {
            g->prev_screen = SCREEN_BEERACE;
            g->pause_idx = 0;
            g->screen = SCREEN_PAUSED;
            return;
        }
        if (!m->follow_mode && m->act_as_pad2 && !g->pad2.connected && mouse_allowed_in_mode(g) &&
            (m->left_down || m->y != m->prev_y)) {
            int pdy = 0;
            g->p2_ready = 1;
            if (m->y < g->p2.y - 4) pdy = -1;
            else if (m->y > g->p2.y + 4) pdy = 1;
            if (pdy != 0) {
                apply_smooth_move_racer(g, &g->p2, 0, pdy, m->click_left ? 1 : 0);
            }
        }
    } else if (g->screen == SCREEN_PAUSED) {
        int bx = (g->pause_style == PAUSE_STYLE_COMPACT) ? 78 : 58;
        int by = (g->pause_style == PAUSE_STYLE_COMPACT) ? 54 : 42;
        int bw = (g->pause_style == PAUSE_STYLE_COMPACT) ? 164 : 204;
        int sy = (g->pause_style == PAUSE_STYLE_COMPACT) ? (by + 26) : (by + 30);
        int i;
        for (i = 0; i < 3; i++) {
            int y = sy + i * 18;
            if (point_in_box(m->x, m->y, bx + 10, y - 2, bw - 20, 12)) {
                g->pause_idx = i;
                if (m->click_left) {
                    activate_pause_item(g);
                    return;
                }
            }
        }
        if (m->click_right) {
            g->screen = g->prev_screen;
        }
    } else if (g->screen == SCREEN_SETTINGS) {
        int i;
        for (i = 0; i < SETTINGS_COUNT; i++) {
            int y = 42 + i * 20;
            if (point_in_box(m->x, m->y, 44, y - 2, 232, 12)) {
                g->settings_idx = i;
                if (m->click_left) {
                    toggle_setting_item(g);
                    return;
                }
            }
        }
        if (m->click_right) g->screen = g->prev_screen;
    } else if (g->screen == SCREEN_CONTROLS) {
        if (m->click_left && point_in_box(m->x, m->y, 84, 150, 152, 15)) {
            g->screen = g->prev_screen;
        } else if (m->click_right) {
            g->screen = g->prev_screen;
        }
    } else if (g->screen == SCREEN_WIFI_SCORE) {
        if (m->click_left && point_in_box(m->x, m->y, 68, 124, 184, 15)) {
            compare_score_over_wifi_ip(g);
        } else if (m->click_right) {
            g->screen = SCREEN_MAIN_MENU;
        }
    }
}

static void draw_capture_banners(const BGC_Game *g, void *hdc_ptr) {
    if (g->video_recording) {
        draw_box(g, hdc_ptr, 250, 16, 64, 11, C_VGA_RED, 1);
        draw_str_5x7(g, hdc_ptr, 255, 18, "REC VDO", C_VGA_WING);
    } else if (g->video_saved_banner_ticks > 0) {
        draw_box(g, hdc_ptr, 214, 16, 100, 11, C_VGA_PANEL, 1);
        draw_str_5x7(g, hdc_ptr, 219, 18, "VDO SAVED", C_VGA_BLACK);
    }
    if (g->shot_saved_banner_ticks > 0) {
        draw_box(g, hdc_ptr, 214, 28, 100, 11, C_VGA_PANEL, 1);
        draw_str_5x7(g, hdc_ptr, 219, 30, "BMP SAVED", C_VGA_BLACK);
    }
    if (g->replay_saved_banner_ticks > 0) {
        draw_box(g, hdc_ptr, 214, 40, 100, 11, C_VGA_PANEL, 1);
        draw_str_5x7(g, hdc_ptr, 219, 42, "RPL SAVED", C_VGA_BLACK);
    }
}

static void render_gameover_at_pause_pos(const BGC_Game *g, void *hdc_ptr, const char *header_txt) {
    const char *items[3];
    char style_line[32];
    int bx = 58, by = 42, bw = 204, bh = 114;
    int i;

    sprintf(style_line, "STYLE: %s", (g->pause_style == PAUSE_STYLE_COMPACT) ? "COMPACT" : "FULL");
    items[0] = "RETRY GAME";
    items[1] = "MAIN MENU";
    items[2] = style_line;

    draw_box(g, hdc_ptr, bx, by, bw, bh, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, bx + 14, by + 8, header_txt, C_VGA_RED);

    for (i = 0; i < 3; i++) {
        int y = by + 30 + i * 22;
        if (g->gameover_idx == i) {
            draw_box(g, hdc_ptr, bx + 10, y - 3, bw - 20, 14, C_VGA_YELLOW, 1);
            draw_str_5x7(g, hdc_ptr, bx + 15, y, ">", C_VGA_BLACK);
        }
        draw_str_5x7(g, hdc_ptr, bx + 26, y, items[i], C_VGA_BLACK);
    }
}

static void render_main_menu(BGC_Game *g, void *hdc_ptr) {
    const char *items[7] = {
        "START GAME", "BEERACE 2P", "WIFI SCORE",
        "MEDIA VAULT", "CONTROLS", "SETTINGS", "EXIT"
    };
    char best_line[48];
    int i;

    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_cartoon_bee(g, hdc_ptr, 28, 40, g->tick);
    draw_black_bug(g, hdc_ptr, 292, 42, g->tick);

    draw_box(g, hdc_ptr, 54, 10, 212, 176, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 62, 14, "BEE GOLD V" BGC_VERSION_STR " - " BGC_UPDATE_NAME, C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 122, 23, "2026 KGAMER", C_VGA_BLACK);
    sprintf(best_line, "BEST:%d LAST:%d", g->best_score, g->last_score);
    draw_str_5x7(g, hdc_ptr, 84, 32, best_line, C_VGA_RED);

    for (i = 0; i < 7; i++) {
        int y = 44 + i * 19;
        if (g->menu_idx == i) {
            draw_box(g, hdc_ptr, 68, y - 3, 184, 13, C_VGA_YELLOW, 1);
            draw_str_5x7(g, hdc_ptr, 74, y, ">", C_VGA_BLACK);
        }
        draw_str_5x7(g, hdc_ptr, 86, y, items[i], C_VGA_BLACK);
    }
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_beerace_lobby(const BGC_Game *g, void *hdc_ptr) {
    int half_w = SCR_W / 2;
    char p2_state[48];

    draw_viewport_env(g, hdc_ptr, 0, 0, half_w, SCR_H);
    draw_viewport_env(g, hdc_ptr, half_w, 0, SCR_W - half_w, SCR_H);
    draw_box(g, hdc_ptr, half_w - 1, 0, 2, SCR_H, C_VGA_BLACK, 0);

    draw_mini_bee_race(g, hdc_ptr, 36, 78, g->tick);
    draw_mini_bee_race(g, hdc_ptr, half_w + 36, 78, g->tick);

    draw_box(g, hdc_ptr, 26, 26, 268, 146, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 66, 34, "BEERACE 2P - 2026 KGAMER", C_VGA_BLACK);

    draw_str_5x7(g, hdc_ptr, 38, 54, "HOST P1: MENU + START + PAD1", C_VGA_GRASS);

    if (g->p2_ready) {
        sprintf(p2_state, "P2 SLOT: GAMEPLAY ONLY READY");
        draw_str_5x7(g, hdc_ptr, 38, 70, p2_state, C_VGA_GRASS);
    } else {
        draw_str_5x7(g, hdc_ptr, 38, 70, "P2 SLOT: IN-GAME ONLY (PAD2)", C_VGA_BLACK);
        draw_str_5x7(g, hdc_ptr, 38, 84, "P2 CANNOT CONTROL MENUS", C_VGA_RED);
    }

    draw_box(g, hdc_ptr, 54, 108, 212, 20, C_VGA_YELLOW, 1);
    draw_str_5x7(g, hdc_ptr, 76, 115, "> HOST P1: START (ENTER)", C_VGA_BLACK);

    draw_str_5x7(g, hdc_ptr, 66, 148, "ESC: MAIN MENU", C_VGA_BLACK);
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_media_vault(BGC_Game *g, void *hdc_ptr) {
    char status_line[64];

    if (g->media_tab == MEDIA_TAB_PHOTO) {
        if (g->has_photo_buf && g->photo_buf) blit_frame_to_screen(g, hdc_ptr, g->photo_buf);
        else draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
        sprintf(status_line, "PHOTO (%s)", SHOT_FILE);
    } else if (g->media_tab == MEDIA_TAB_REPLAY) {
        draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
        if (g->replay_count > 0) {
            const ReplaySample *s = &g->replay_buf[g->replay_play_idx];
            draw_cartoon_bee(g, hdc_ptr, s->bee_x, s->bee_y, g->tick);
            sprintf(status_line, "REPLAY %d/%d S:%d", g->replay_play_idx + 1, g->replay_count, s->score);
        } else {
            sprintf(status_line, "REPLAY EMPTY (.RPL)");
        }
    } else {
        if (g->video_frame_count > 0 && g->video_frames[g->video_play_idx]) {
            blit_frame_to_screen(g, hdc_ptr, g->video_frames[g->video_play_idx]);
        } else {
            draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
        }
        sprintf(status_line, "VIDEO %d/%d", (g->video_frame_count > 0) ? (g->video_play_idx + 1) : 0, g->video_frame_count);
    }

    draw_box(g, hdc_ptr, 8, 4, 304, 14, C_VGA_PANEL, 1);
    draw_box(g, hdc_ptr, 12, 6, 92, 10, (g->media_tab == MEDIA_TAB_PHOTO) ? C_VGA_YELLOW : C_VGA_PANEL, 1);
    draw_box(g, hdc_ptr, 114, 6, 92, 10, (g->media_tab == MEDIA_TAB_REPLAY) ? C_VGA_YELLOW : C_VGA_PANEL, 1);
    draw_box(g, hdc_ptr, 216, 6, 92, 10, (g->media_tab == MEDIA_TAB_VIDEO) ? C_VGA_YELLOW : C_VGA_PANEL, 1);

    draw_str_5x7(g, hdc_ptr, 36, 8, "PHOTO", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 134, 8, "REPLAY", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 240, 8, "VIDEO", C_VGA_BLACK);

    draw_box(g, hdc_ptr, 12, 20, 296, 12, C_VGA_WING, 1);
    draw_str_5x7(g, hdc_ptr, 18, 23, status_line, C_VGA_BLACK);

    draw_box(g, hdc_ptr, 12, 166, 296, 15, C_VGA_YELLOW, 1);
    draw_str_5x7(g, hdc_ptr, 18, 170, "< >:MODE  SPACE:PLAY  S:SAVE  ESC", C_VGA_BLACK);
    draw_capture_banners(g, hdc_ptr);
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_wifi_score(BGC_Game *g, void *hdc_ptr) {
    char l1[48], l2[48], l3[48];
    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_box(g, hdc_ptr, 34, 28, 252, 140, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 92, 36, "WIFI / IP SCORE", C_VGA_BLACK);

    sprintf(l1, "PEER IP : %s", g->peer_ip);
    sprintf(l2, "MY BEST : %d", g->best_score);
    if (g->wifi_status == 1) {
        sprintf(l3, "PEER BEST: %d", g->peer_best_score);
    } else if (g->wifi_status == -1) {
        sprintf(l3, "STATUS  : NO PEER");
    } else {
        sprintf(l3, "ENTER: COMPARE NOW");
    }

    draw_str_5x7(g, hdc_ptr, 48, 56, l1, C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 48, 72, l2, C_VGA_RED);
    draw_str_5x7(g, hdc_ptr, 48, 88, l3, C_VGA_BLACK);

    draw_box(g, hdc_ptr, 68, 124, 184, 15, C_VGA_YELLOW, 1);
    draw_str_5x7(g, hdc_ptr, 76, 128, "> ENTER:SYNC  ESC:BACK", C_VGA_BLACK);
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_controls(BGC_Game *g, void *hdc_ptr) {
    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_box(g, hdc_ptr, 20, 16, 280, 164, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 118, 24, "CONTROLS", C_VGA_BLACK);

    draw_str_5x7(g, hdc_ptr, 28, 42, "HOST P1 : ALL MENUS + START", C_VGA_GRASS);
    draw_str_5x7(g, hdc_ptr, 28, 56, "P1 MOVE : UP/DOWN / W-S / PAD1", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 70, "MOUSE   : Y FOLLOW + R PAUSE", C_VGA_RED);
    draw_str_5x7(g, hdc_ptr, 28, 84, "SPEED   : DIRECT +5.5PCT BOOST", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 98, "COIN:+10 BAR:+30 BUG:-35", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 112, "OPTS SAVED: BGCOPT.DAT  BMP:F12", C_VGA_BLACK);

    draw_box(g, hdc_ptr, 84, 150, 152, 15, C_VGA_YELLOW, 1);
    draw_str_5x7(g, hdc_ptr, 92, 154, "> CLICK OR ESC BACK", C_VGA_BLACK);
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_settings(BGC_Game *g, void *hdc_ptr) {
    char l_pau[48], l_spd[48], l_flw[48], l_src[48], l_mp2[48];
    const char *src_name = "AUTO";
    const char *rows[SETTINGS_COUNT];
    int i;

    if (g->ctrl_source == CTRL_SRC_MOUSE) src_name = "MOUSE";
    else if (g->ctrl_source == CTRL_SRC_KEYBOARD) src_name = "KEYBRD";
    else if (g->ctrl_source == CTRL_SRC_GAMEPAD) src_name = "PAD";

    sprintf(l_pau, "PAUSE UI  : %s", (g->pause_style == PAUSE_STYLE_COMPACT) ? "COMPACT" : "FULL");
    sprintf(l_spd, "CTRL SPEED: %s", (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? "FAST" : "NORMAL");
    sprintf(l_flw, "MSE FOLLOW: %s", g->mouse.follow_mode ? "ON" : "OFF");
    sprintf(l_src, "CTRL SRC  : %s", src_name);
    sprintf(l_mp2, "MSE AS PAD2: %s", g->mouse.act_as_pad2 ? "ON" : "OFF");

    rows[0] = l_pau;
    rows[1] = l_spd;
    rows[2] = l_flw;
    rows[3] = l_src;
    rows[4] = l_mp2;
    rows[5] = "BACK (SAVED TO BGCOPT.DAT)";

    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_box(g, hdc_ptr, 32, 22, 256, 156, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 118, 28, "SETTINGS", C_VGA_BLACK);

    for (i = 0; i < SETTINGS_COUNT; i++) {
        int y = 42 + i * 20;
        if (g->settings_idx == i) {
            draw_box(g, hdc_ptr, 44, y - 2, 232, 12, C_VGA_YELLOW, 1);
            draw_str_5x7(g, hdc_ptr, 50, y, ">", C_VGA_BLACK);
        }
        draw_str_5x7(g, hdc_ptr, 62, y, rows[i], C_VGA_BLACK);
    }
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_pause(BGC_Game *g, void *hdc_ptr) {
    const char *items[3] = {"RESUME GAME", "SETTINGS", "MAIN MENU"};
    int bx = (g->pause_style == PAUSE_STYLE_COMPACT) ? 78 : 58;
    int by = (g->pause_style == PAUSE_STYLE_COMPACT) ? 54 : 42;
    int bw = (g->pause_style == PAUSE_STYLE_COMPACT) ? 164 : 204;
    int bh = (g->pause_style == PAUSE_STYLE_COMPACT) ? 92 : 114;
    int sy = by + 26;
    int i;

    draw_box(g, hdc_ptr, bx, by, bw, bh, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, bx + 14, by + 8, "PAUSE MENU", C_VGA_BLACK);

    if (g->pause_style == PAUSE_STYLE_FULL) {
        char info[48];
        sprintf(info, "BEST:%d LAST:%d", g->best_score, g->last_score);
        draw_str_5x7(g, hdc_ptr, bx + 14, by + 17, info, C_VGA_RED);
        sy = by + 30;
    }

    for (i = 0; i < 3; i++) {
        int y = sy + i * 18;
        if (g->pause_idx == i) {
            draw_box(g, hdc_ptr, bx + 10, y - 2, bw - 20, 12, C_VGA_YELLOW, 1);
            draw_str_5x7(g, hdc_ptr, bx + 15, y, ">", C_VGA_BLACK);
        }
        draw_str_5x7(g, hdc_ptr, bx + 26, y, items[i], C_VGA_BLACK);
    }
}

static void render_racer_pane(BGC_Game *g, void *hdc_ptr, const Racer *r, int vx, int vw, const char *tag) {
    char hud[48];
    int bar_w = (r->health * (vw - 10)) / 100;
    int i;
    if (bar_w > vw - 10) bar_w = vw - 10;
    if (bar_w < 0) bar_w = 0;

    draw_viewport_env(g, hdc_ptr, vx, 0, vw, SCR_H);
    for (i = 0; i < MAX_GOLDS; i++) {
        draw_mini_gold_race(g, hdc_ptr, vx + r->gold_x[i], r->gold_y[i]);
    }
    if (r->gold_bar_active) {
        draw_mini_gold_bar_race(g, hdc_ptr, vx + r->gold_bar_x, r->gold_bar_y);
    }
    for (i = 0; i < MAX_BUGS; i++) {
        draw_mini_bug_race(g, hdc_ptr, vx + r->bug_x[i], r->bug_y[i], g->tick);
    }
    draw_mini_bee_race(g, hdc_ptr, vx + r->x, r->y, g->tick);

    draw_box(g, hdc_ptr, vx + 4, 4, vw - 8, 6, C_VGA_WING, 1);
    if (bar_w > 0) {
        draw_box(g, hdc_ptr, vx + 5, 5, bar_w, 4, (r->health > 30) ? C_VGA_GRASS : C_VGA_RED, 0);
    }

    sprintf(hud, "%s HP:%d S:%d", tag, r->health, r->score);
    draw_str_5x7(g, hdc_ptr, vx + 5, 12, hud, C_VGA_BLACK);
}

static void render_beerace_split(BGC_Game *g, void *hdc_ptr) {
    int half_w = SCR_W / 2;

    if (!g->race_started) {
        render_beerace_lobby(g, hdc_ptr);
        return;
    }

    render_racer_pane(g, hdc_ptr, &g->p1, 0, half_w, "P1");
    render_racer_pane(g, hdc_ptr, &g->p2, half_w, SCR_W - half_w, "P2");
    draw_box(g, hdc_ptr, half_w - 1, 0, 2, SCR_H, C_VGA_BLACK, 0);

    if (g->race_over) {
        char win_txt[32];
        sprintf(win_txt, "P%d WINS", g->race_winner);
        render_gameover_at_pause_pos(g, hdc_ptr, win_txt);
    }
    draw_capture_banners(g, hdc_ptr);
    draw_update_footer(g, hdc_ptr);
    draw_mouse_cursor(g, hdc_ptr);
}

static void render_scene(BGC_Game *g, void *hdc_ptr) {
    char hud[64];
    int i;

    if (g->screen == SCREEN_MAIN_MENU) {
        render_main_menu(g, hdc_ptr);
        return;
    }
    if (g->screen == SCREEN_MEDIA_VAULT) {
        render_media_vault(g, hdc_ptr);
        return;
    }
    if (g->screen == SCREEN_WIFI_SCORE) {
        render_wifi_score(g, hdc_ptr);
        return;
    }
    if (g->screen == SCREEN_CONTROLS) {
        render_controls(g, hdc_ptr);
        return;
    }
    if (g->screen == SCREEN_SETTINGS) {
        render_settings(g, hdc_ptr);
        return;
    }
    if (g->screen == SCREEN_BEERACE) {
        render_beerace_split(g, hdc_ptr);
        if (g->screen == SCREEN_PAUSED) render_pause(g, hdc_ptr);
        draw_update_footer(g, hdc_ptr);
        draw_mouse_cursor(g, hdc_ptr);
        capture_video_frame(g);
        return;
    }

    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    for (i = 0; i < MAX_GOLDS; i++) {
        draw_gold_coin(g, hdc_ptr, g->gold_x[i], g->gold_y[i]);
    }
    if (g->gold_bar_active) {
        draw_gold_bar(g, hdc_ptr, g->gold_bar_x, g->gold_bar_y);
    }
    for (i = 0; i < MAX_BUGS; i++) {
        draw_black_bug(g, hdc_ptr, g->bug_x[i], g->bug_y[i], g->tick);
    }
    draw_cartoon_bee(g, hdc_ptr, g->bee_x, g->bee_y, g->tick);

    draw_box(g, hdc_ptr, 6, 5, 102, 8, C_VGA_WING, 1);
    if (g->health > 0) {
        draw_box(g, hdc_ptr, 7, 6, g->health, 6, (g->health > 30) ? C_VGA_GRASS : C_VGA_RED, 0);
    }

    sprintf(hud, "S:%d B:%d V" BGC_VERSION_STR, g->score, g->best_score);
    draw_str_5x7(g, hdc_ptr, 114, 6, hud, C_VGA_BLACK);

    draw_capture_banners(g, hdc_ptr);
    draw_update_footer(g, hdc_ptr);

    if (g->screen == SCREEN_PAUSED) {
        render_pause(g, hdc_ptr);
    } else if (g->game_over) {
        render_gameover_at_pause_pos(g, hdc_ptr, "GAME OVER");
    }

    draw_mouse_cursor(g, hdc_ptr);
    capture_video_frame(g);
}

#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0400)
static void poll_held_keyboard_win(BGC_Game *g) {
    int p1_dy = 0;
    int p2_dy = 0;

    if (GetAsyncKeyState(VK_UP)   & 0x8000) p1_dy -= 1;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) p1_dy += 1;

    if (g->screen == SCREEN_PLAYING && !g->game_over) {
        if (GetAsyncKeyState('W') & 0x8000) p1_dy -= 1;
        if (GetAsyncKeyState('S') & 0x8000) p1_dy += 1;
        if (p1_dy != 0) {
            apply_smooth_move_single(g, 0, p1_dy, 0);
        }
    } else if (g->screen == SCREEN_BEERACE && g->race_started && !g->race_over) {
        if (p1_dy != 0) {
            apply_smooth_move_racer(g, &g->p1, 0, p1_dy, 0);
        }
        if (GetAsyncKeyState('I') & 0x8000 || GetAsyncKeyState('T') & 0x8000) p2_dy -= 1;
        if (GetAsyncKeyState('K') & 0x8000 || GetAsyncKeyState('G') & 0x8000) p2_dy += 1;
        if (p2_dy != 0) {
            g->p2_ready = 1;
            apply_smooth_move_racer(g, &g->p2, 0, p2_dy, 0);
        }
    }
}
#endif

static void poll_dual_pads_and_apply(BGC_Game *g, int *running) {
    int in_active_gameplay = (g->screen == SCREEN_PLAYING && !g->game_over) ||
                             (g->screen == SCREEN_BEERACE && g->race_started && !g->race_over);

    poll_single_pad(&g->pad1, 0);
    poll_single_pad(&g->pad2, 1);

    if (!in_active_gameplay) {
        g->pad2.dx = 0;
        g->pad2.dy = 0;
        g->pad2.btn_confirm = 0;
        g->pad2.btn_back = 0;
        g->pad2.btn_pause = 0;
        g->pad2.btn_retry = 0;
    }

    if (in_active_gameplay && g->ctrl_source == CTRL_SRC_KEYBOARD) {
        return;
    }

    if (in_active_gameplay && g->screen == SCREEN_BEERACE &&
        !g->pad2.connected && g->mouse.act_as_pad2 && g->mouse.available && mouse_allowed_in_mode(g)) {
        int mdy = 0;
        if (g->mouse.y < SCR_H / 2 - 12) mdy = -1;
        else if (g->mouse.y > SCR_H / 2 + 12) mdy = 1;
        g->pad2.dx = 0;
        g->pad2.dy = mdy;
    }

    if (g->screen == SCREEN_MAIN_MENU) {
        if (g->pad1.dy != 0 && g->pad1.nav_cooldown == 0) {
            if (g->pad1.dy < 0) g->menu_idx = (g->menu_idx + 6) % 7;
            else if (g->pad1.dy > 0) g->menu_idx = (g->menu_idx + 1) % 7;
            g->pad1.nav_cooldown = 7;
        }
        if (g->pad1.btn_confirm) activate_main_item(g, running);
    } else if (g->screen == SCREEN_MEDIA_VAULT) {
        if (g->pad1.dx != 0 && g->pad1.nav_cooldown == 0) {
            switch_media_tab(g, g->pad1.dx);
            g->pad1.nav_cooldown = 7;
        }
        if (g->pad1.btn_confirm) toggle_current_media_play(g);
        if (g->pad1.btn_retry) save_current_media_to_device(g);
        if (g->pad1.btn_back) g->screen = SCREEN_MAIN_MENU;
    } else if (g->screen == SCREEN_PLAYING) {
        if (g->game_over) {
            if (g->pad1.dy != 0 && g->pad1.nav_cooldown == 0) {
                if (g->pad1.dy < 0) g->gameover_idx = (g->gameover_idx + 2) % 3;
                else if (g->pad1.dy > 0) g->gameover_idx = (g->gameover_idx + 1) % 3;
                g->pad1.nav_cooldown = 7;
            }
            if (g->pad1.btn_confirm) activate_gameover_item(g, 0);
            if (g->pad1.btn_retry) start_single_game(g);
            if (g->pad1.btn_back) {
                g->menu_idx = 0;
                g->game_over = 0;
                g->screen = SCREEN_MAIN_MENU;
            }
            return;
        }
        if (!g->mouse.follow_mode && g->pad1.dy != 0) {
            apply_smooth_move_single(g, 0, g->pad1.dy, 0);
        }
        if (g->pad1.btn_pause || g->pad1.btn_back) {
            g->prev_screen = SCREEN_PLAYING;
            g->pause_idx = 0;
            g->screen = SCREEN_PAUSED;
        }
    } else if (g->screen == SCREEN_BEERACE) {
        if (!g->race_started) {
            if (g->pad2.connected) {
                g->p2_ready = 1;
                g->p2_is_pad = 1;
            }
            if (g->pad1.btn_confirm) {
                launch_beerace_match(g);
            }
            if (g->pad1.btn_back) {
                g->menu_idx = 0;
                g->screen = SCREEN_MAIN_MENU;
            }
            return;
        }
        if (g->race_over) {
            if (g->pad1.dy != 0 && g->pad1.nav_cooldown == 0) {
                if (g->pad1.dy < 0) g->gameover_idx = (g->gameover_idx + 2) % 3;
                else if (g->pad1.dy > 0) g->gameover_idx = (g->gameover_idx + 1) % 3;
                g->pad1.nav_cooldown = 7;
            }
            if (g->pad1.btn_confirm) activate_gameover_item(g, 1);
            if (g->pad1.btn_retry) launch_beerace_match(g);
            if (g->pad1.btn_back) {
                g->menu_idx = 0;
                g->race_over = 0;
                g->race_started = 0;
                g->screen = SCREEN_MAIN_MENU;
            }
            return;
        }
        if (g->pad1.dy != 0) {
            apply_smooth_move_racer(g, &g->p1, 0, g->pad1.dy, 0);
        }
        if (!g->mouse.follow_mode && g->pad2.dy != 0) {
            g->p2_ready = 1;
            apply_smooth_move_racer(g, &g->p2, 0, g->pad2.dy, 0);
        }
        if (g->pad1.btn_pause) {
            g->prev_screen = SCREEN_BEERACE;
            g->pause_idx = 0;
            g->screen = SCREEN_PAUSED;
        }
    } else if (g->screen == SCREEN_PAUSED) {
        if (g->pad1.dy != 0 && g->pad1.nav_cooldown == 0) {
            if (g->pad1.dy < 0) g->pause_idx = (g->pause_idx + 2) % 3;
            else if (g->pad1.dy > 0) g->pause_idx = (g->pause_idx + 1) % 3;
            g->pad1.nav_cooldown = 7;
        }
        if (g->pad1.btn_confirm) activate_pause_item(g);
        if (g->pad1.btn_pause || g->pad1.btn_back) {
            g->screen = g->prev_screen;
        }
    } else if (g->screen == SCREEN_SETTINGS) {
        if (g->pad1.dy != 0 && g->pad1.nav_cooldown == 0) {
            if (g->pad1.dy < 0) g->settings_idx = (g->settings_idx + SETTINGS_COUNT - 1) % SETTINGS_COUNT;
            else if (g->pad1.dy > 0) g->settings_idx = (g->settings_idx + 1) % SETTINGS_COUNT;
            g->pad1.nav_cooldown = 7;
        }
        if (g->pad1.btn_confirm || (g->pad1.dx != 0 && g->pad1.nav_cooldown == 0)) {
            toggle_setting_item(g);
            g->pad1.nav_cooldown = 7;
        }
        if (g->pad1.btn_back) g->screen = g->prev_screen;
    } else if (g->screen == SCREEN_CONTROLS) {
        if (g->pad1.btn_confirm || g->pad1.btn_back || g->pad1.btn_pause) {
            g->screen = g->prev_screen;
        }
    } else if (g->screen == SCREEN_WIFI_SCORE) {
        if (g->pad1.btn_confirm) compare_score_over_wifi_ip(g);
        if (g->pad1.btn_back) g->screen = SCREEN_MAIN_MENU;
    }
}

/* ============================================================================
 * 10. DOS / FREEDOS (VGA MODE 13H) & WINDOWS 1.0..MT5.0/BOB (VGA BUFFER)
 * ============================================================================ */
#if BGC_DOS_TARGET

static void dos_set_video(unsigned char mode) {
#if BGC_DJGPP_TARGET
    __dpmi_regs r;
    memset(&r, 0, sizeof(r));
    r.h.ah = 0x00;
    r.h.al = mode;
    __dpmi_int(0x10, &r);
#elif defined(__386__)
    union REGS r;
    memset(&r, 0, sizeof(r));
    r.h.ah = 0x00;
    r.h.al = mode;
    int386(0x10, &r, &r);
#else
    union REGS r;
    memset(&r, 0, sizeof(r));
    r.h.ah = 0x00;
    r.h.al = mode;
    int86(0x10, &r, &r);
#endif
}

int main(int argc, char **argv) {
    BGC_Game game;
    int running = 1;
    int i;
    (void)argc;
    (void)argv;

    srand((unsigned int)time(NULL));
    memset(&game, 0, sizeof(game));
    game.host_is_p1 = 1;
    init_mouse_state(&game.mouse);
    dos_init_mouse_driver(&game.mouse);
    strcpy(game.peer_ip, "192.168.1.16");
    game.backend = BACKEND_VGA;
    game.pause_style = PAUSE_STYLE_FULL;
    game.ctrl_speed_mode = CTRL_SPEED_NORMAL;
    game.ctrl_source = CTRL_SRC_AUTO;
    game.screen = SCREEN_MAIN_MENU;
    read_obfuscated_vault(&game);
    read_obfuscated_options(&game);
    for (i = 0; i < MAX_GOLDS; i++) spawn_single_gold_slot(&game, i);
    for (i = 0; i < MAX_BUGS;  i++) spawn_single_bug_slot(&game, i);
    spawn_single_gold_bar(&game);

#if BGC_DJGPP_TARGET
    bgc_dos_init_vram();
#endif
    dos_set_video(0x13);

    while (running) {
        while (kbhit()) {
            int ch = getch();
            if (ch == 0 || ch == 224) {
                int ext = getch();
                if (ext == 134) {
                    save_screenshot_bmp(&game);
                } else if (game.screen == SCREEN_MEDIA_VAULT) {
                    if (ext == 75) switch_media_tab(&game, -1);
                    if (ext == 77) switch_media_tab(&game,  1);
                } else if (game.screen == SCREEN_MAIN_MENU) {
                    if (ext == 72) game.menu_idx = (game.menu_idx + 6) % 7;
                    if (ext == 80) game.menu_idx = (game.menu_idx + 1) % 7;
                } else if (game.screen == SCREEN_PAUSED) {
                    if (ext == 72) game.pause_idx = (game.pause_idx + 2) % 3;
                    if (ext == 80) game.pause_idx = (game.pause_idx + 1) % 3;
                } else if (game.screen == SCREEN_SETTINGS) {
                    if (ext == 72) game.settings_idx = (game.settings_idx + SETTINGS_COUNT - 1) % SETTINGS_COUNT;
                    if (ext == 80) game.settings_idx = (game.settings_idx + 1) % SETTINGS_COUNT;
                    if (ext == 75 || ext == 77) toggle_setting_item(&game);
                } else if (game.screen == SCREEN_PLAYING) {
                    if (game.game_over) {
                        if (ext == 72) game.gameover_idx = (game.gameover_idx + 2) % 3;
                        if (ext == 80) game.gameover_idx = (game.gameover_idx + 1) % 3;
                        if (ext == 75 || ext == 77) activate_gameover_item(&game, 0);
                    } else {
                        if (ext == 72) apply_smooth_move_single(&game, 0, -1, 1);
                        if (ext == 80) apply_smooth_move_single(&game, 0,  1, 1);
                    }
                } else if (game.screen == SCREEN_BEERACE) {
                    if (game.race_started && game.race_over) {
                        if (ext == 72) game.gameover_idx = (game.gameover_idx + 2) % 3;
                        if (ext == 80) game.gameover_idx = (game.gameover_idx + 1) % 3;
                        if (ext == 75 || ext == 77) activate_gameover_item(&game, 1);
                    } else if (game.race_started) {
                        if (ext == 72) apply_smooth_move_racer(&game, &game.p1, 0, -1, 1);
                        if (ext == 80) apply_smooth_move_racer(&game, &game.p1, 0,  1, 1);
                    }
                }
            } else {
                if (ch == 'v' || ch == 'V') {
                    toggle_video_recording(&game);
                } else if (ch == 'b' || ch == 'B') {
                    save_screenshot_bmp(&game);
                } else if (game.screen == SCREEN_MEDIA_VAULT) {
                    if (ch == ' ') toggle_current_media_play(&game);
                    else if (ch == 's' || ch == 'S' || ch == 13) save_current_media_to_device(&game);
                    else if (ch == 'c' || ch == 'C') save_screenshot_bmp(&game);
                    else if (ch == 27) game.screen = SCREEN_MAIN_MENU;
                } else if (game.screen == SCREEN_MAIN_MENU) {
                    if (ch == 13) activate_main_item(&game, &running);
                    else if (ch == 27) running = 0;
                } else if (game.screen == SCREEN_PLAYING) {
                    if (game.game_over) {
                        if (ch == 13 || ch == ' ') activate_gameover_item(&game, 0);
                        else if (ch == 'r' || ch == 'R') start_single_game(&game);
                        else if (ch == 'm' || ch == 'M' || ch == 27) { game.menu_idx = 0; game.game_over = 0; game.screen = SCREEN_MAIN_MENU; }
                    } else {
                        if (ch == 'w' || ch == 'W') apply_smooth_move_single(&game, 0, -1, 1);
                        if (ch == 's' || ch == 'S') apply_smooth_move_single(&game, 0,  1, 1);
                        if (ch == 'p' || ch == 'P' || ch == 27) {
                            game.prev_screen = SCREEN_PLAYING;
                            game.pause_idx = 0;
                            game.screen = SCREEN_PAUSED;
                        }
                    }
                } else if (game.screen == SCREEN_BEERACE) {
                    if (!game.race_started) {
                        if (ch == 13) {
                            launch_beerace_match(&game);
                        } else if (ch == 27) {
                            game.menu_idx = 0;
                            game.screen = SCREEN_MAIN_MENU;
                        }
                    } else if (game.race_over) {
                        if (ch == 13 || ch == ' ') activate_gameover_item(&game, 1);
                        else if (ch == 'r' || ch == 'R') launch_beerace_match(&game);
                        else if (ch == 'm' || ch == 'M' || ch == 27) { game.menu_idx = 0; game.race_over = 0; game.race_started = 0; game.screen = SCREEN_MAIN_MENU; }
                    } else {
                        if (ch == 'i' || ch == 'I') { game.p2_ready = 1; apply_smooth_move_racer(&game, &game.p2, 0, -1, 1); }
                        if (ch == 'k' || ch == 'K') { game.p2_ready = 1; apply_smooth_move_racer(&game, &game.p2, 0,  1, 1); }
                        if (ch == 'p' || ch == 'P' || ch == 27) {
                            game.prev_screen = SCREEN_BEERACE;
                            game.pause_idx = 0;
                            game.screen = SCREEN_PAUSED;
                        }
                    }
                } else if (game.screen == SCREEN_PAUSED) {
                    if (ch == 13) activate_pause_item(&game);
                    else if (ch == '1') game.screen = game.prev_screen;
                    else if (ch == '2') { game.settings_idx = 0; game.screen = SCREEN_SETTINGS; }
                    else if (ch == '3') { game.menu_idx = 0; game.screen = SCREEN_MAIN_MENU; }
                    else if (ch == 'p' || ch == 'P' || ch == 27) game.screen = game.prev_screen;
                } else if (game.screen == SCREEN_SETTINGS) {
                    if (ch == 13 || ch == ' ') toggle_setting_item(&game);
                    else if (ch == 27) game.screen = game.prev_screen;
                } else if (game.screen == SCREEN_CONTROLS || game.screen == SCREEN_WIFI_SCORE) {
                    if (ch == 13 || ch == 27) game.screen = game.prev_screen;
                }
            }
        }

        apply_mouse_to_game(&game, &running);
        poll_dual_pads_and_apply(&game, &running);
        step_world(&game);
        render_scene(&game, NULL);
#if BGC_DJGPP_TARGET
        bgc_dos_flush_vga();
#endif
        delay(12);
    }

    write_obfuscated_options(&game);
    if (game.video_recording) save_video_to_file(&game);
    free_media_buffers(&game);
#if BGC_DJGPP_TARGET
    bgc_dos_shutdown_vram();
#endif
    dos_set_video(0x03);
    printf("%s - BEST: %d\n", BGC_TITLE, game.best_score);
    return 0;
}

#else

static BGC_Game g_win;

static void init_vga_mode(BGC_Game *g, const char *cmd) {
    (void)cmd;
    g->backend = BACKEND_VGA;
    read_obfuscated_options(g);
    g->backend = BACKEND_VGA;
    if (cmd && cmd[0] != '\0') {
        if (strstr(cmd, "--mouse-pad2-off") != NULL) g->mouse.act_as_pad2 = 0;
        else if (strstr(cmd, "--mouse-pad2") != NULL) g->mouse.act_as_pad2 = 1;
    }
}

static void blit_vga_buffer_to_gdi(HDC hdc, const RECT *rc) {
    int cw = rc->right - rc->left;
    int ch = rc->bottom - rc->top;
    if (cw <= 0 || ch <= 0) return;

#if (BGC_WIN_ARCH != 16)
    {
        BITMAPINFO bmi;
        unsigned int pixels[SCR_W * SCR_H];
        unsigned long i;
        for (i = 0; i < SCR_PIXELS; i++) pixels[i] = vga_to_bgr_u32(g_vga_buf[i]);

        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = SCR_W;
        bmi.bmiHeader.biHeight = -SCR_H;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        StretchDIBits(hdc, 0, 0, cw, ch, 0, 0, SCR_W, SCR_H, pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
    }
#else
    {
        int x, y;
        for (y = 0; y < SCR_H; y++) {
            int y0 = (y * ch) / SCR_H;
            int y1 = ((y + 1) * ch) / SCR_H;
            for (x = 0; x < SCR_W; x++) {
                int x0 = (x * cw) / SCR_W;
                int x1 = ((x + 1) * cw) / SCR_W;
                unsigned char idx = g_vga_buf[(unsigned long)y * SCR_W + (unsigned long)x] & 0x0Fu;
                HBRUSH br = CreateSolidBrush(vga_to_gdi_color(idx));
                RECT cell;
                cell.left = x0; cell.top = y0; cell.right = x1; cell.bottom = y1;
                FillRect(hdc, &cell, br);
                DeleteObject(br);
            }
        }
    }
#endif
}

#if (BGC_WIN_ARCH == 16) && defined(__WATCOMC__)
LRESULT CALLBACK __export BGC_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
LRESULT CALLBACK BGC_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
#endif
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    int running = 1;

    switch (msg) {
        case WM_CREATE:
            {
                int i;
                srand((unsigned int)time(NULL));
                for (i = 0; i < MAX_GOLDS; i++) spawn_single_gold_slot(&g_win, i);
                for (i = 0; i < MAX_BUGS;  i++) spawn_single_bug_slot(&g_win, i);
                spawn_single_gold_bar(&g_win);
                SetTimer(hwnd, 1, 15, NULL);
            }
            return 0;

        case WM_MOUSEMOVE:
            update_win_mouse_pos(&g_win, hwnd, lParam);
            g_win.mouse.left_down  = (wParam & MK_LBUTTON) ? 1 : 0;
            g_win.mouse.right_down = (wParam & MK_RBUTTON) ? 1 : 0;
            return 0;

        case WM_LBUTTONDOWN:
            update_win_mouse_pos(&g_win, hwnd, lParam);
            g_win.mouse.left_down = 1;
            if (wParam & MK_RBUTTON) g_win.mouse.right_down = 1;
            return 0;

        case WM_LBUTTONUP:
            update_win_mouse_pos(&g_win, hwnd, lParam);
            g_win.mouse.left_down = 0;
            return 0;

        case WM_RBUTTONDOWN:
            update_win_mouse_pos(&g_win, hwnd, lParam);
            g_win.mouse.right_down = 1;
            if (wParam & MK_LBUTTON) g_win.mouse.left_down = 1;
            return 0;

        case WM_RBUTTONUP:
            update_win_mouse_pos(&g_win, hwnd, lParam);
            g_win.mouse.right_down = 0;
            return 0;

        case WM_TIMER:
#if (BGC_WIN_ARCH != 16) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0400)
            poll_held_keyboard_win(&g_win);
#endif
            apply_mouse_to_game(&g_win, &running);
            poll_dual_pads_and_apply(&g_win, &running);
            if (!running) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                return 0;
            }
            step_world(&g_win);
            memset(g_vga_buf, C_VGA_SKY, sizeof(g_vga_buf));
            render_scene(&g_win, NULL);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_F12 || wParam == 'B') {
                save_screenshot_bmp(&g_win);
            } else if (wParam == 'V') {
                toggle_video_recording(&g_win);
            } else if (g_win.screen == SCREEN_MEDIA_VAULT) {
                if (wParam == VK_LEFT)  switch_media_tab(&g_win, -1);
                else if (wParam == VK_RIGHT) switch_media_tab(&g_win,  1);
                else if (wParam == VK_SPACE) toggle_current_media_play(&g_win);
                else if (wParam == 'S' || wParam == VK_RETURN) save_current_media_to_device(&g_win);
                else if (wParam == 'C') save_screenshot_bmp(&g_win);
                else if (wParam == VK_ESCAPE) g_win.screen = SCREEN_MAIN_MENU;
            } else if (g_win.screen == SCREEN_MAIN_MENU) {
                if (wParam == VK_UP)   g_win.menu_idx = (g_win.menu_idx + 6) % 7;
                if (wParam == VK_DOWN) g_win.menu_idx = (g_win.menu_idx + 1) % 7;
                if (wParam == VK_RETURN || wParam == VK_SPACE) activate_main_item(&g_win, &running);
                if (!running) PostMessage(hwnd, WM_CLOSE, 0, 0);
            } else if (g_win.screen == SCREEN_PLAYING) {
                if (g_win.game_over) {
                    if (wParam == VK_UP)   g_win.gameover_idx = (g_win.gameover_idx + 2) % 3;
                    if (wParam == VK_DOWN) g_win.gameover_idx = (g_win.gameover_idx + 1) % 3;
                    if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_RETURN || wParam == VK_SPACE) {
                        activate_gameover_item(&g_win, 0);
                    }
                    if (wParam == 'R') start_single_game(&g_win);
                    if (wParam == 'M' || wParam == VK_ESCAPE) {
                        g_win.menu_idx = 0;
                        g_win.game_over = 0;
                        g_win.screen = SCREEN_MAIN_MENU;
                    }
                } else {
                    if (wParam == VK_UP   || wParam == 'W') apply_smooth_move_single(&g_win, 0, -1, 1);
                    if (wParam == VK_DOWN || wParam == 'S') apply_smooth_move_single(&g_win, 0,  1, 1);
                    if (wParam == 'P' || wParam == VK_ESCAPE) {
                        g_win.prev_screen = SCREEN_PLAYING;
                        g_win.pause_idx = 0;
                        g_win.screen = SCREEN_PAUSED;
                    }
                }
            } else if (g_win.screen == SCREEN_BEERACE) {
                if (!g_win.race_started) {
                    if (wParam == VK_RETURN || wParam == VK_SPACE) {
                        launch_beerace_match(&g_win);
                    } else if (wParam == VK_ESCAPE) {
                        g_win.menu_idx = 0;
                        g_win.screen = SCREEN_MAIN_MENU;
                    }
                } else if (g_win.race_over) {
                    if (wParam == VK_UP)   g_win.gameover_idx = (g_win.gameover_idx + 2) % 3;
                    if (wParam == VK_DOWN) g_win.gameover_idx = (g_win.gameover_idx + 1) % 3;
                    if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_RETURN || wParam == VK_SPACE) {
                        activate_gameover_item(&g_win, 1);
                    }
                    if (wParam == 'R') launch_beerace_match(&g_win);
                    if (wParam == 'M' || wParam == VK_ESCAPE) {
                        g_win.menu_idx = 0;
                        g_win.race_over = 0;
                        g_win.race_started = 0;
                        g_win.screen = SCREEN_MAIN_MENU;
                    }
                } else {
                    if (wParam == VK_UP)   apply_smooth_move_racer(&g_win, &g_win.p1, 0, -1, 1);
                    if (wParam == VK_DOWN) apply_smooth_move_racer(&g_win, &g_win.p1, 0,  1, 1);

                    if (wParam == 'I' || wParam == 'T') { g_win.p2_ready = 1; apply_smooth_move_racer(&g_win, &g_win.p2, 0, -1, 1); }
                    if (wParam == 'K' || wParam == 'G') { g_win.p2_ready = 1; apply_smooth_move_racer(&g_win, &g_win.p2, 0,  1, 1); }

                    if (wParam == 'P' || wParam == VK_ESCAPE) {
                        g_win.prev_screen = SCREEN_BEERACE;
                        g_win.pause_idx = 0;
                        g_win.screen = SCREEN_PAUSED;
                    }
                }
            } else if (g_win.screen == SCREEN_PAUSED) {
                if (wParam == VK_UP)   g_win.pause_idx = (g_win.pause_idx + 2) % 3;
                if (wParam == VK_DOWN) g_win.pause_idx = (g_win.pause_idx + 1) % 3;
                if (wParam == VK_RETURN || wParam == VK_SPACE) activate_pause_item(&g_win);
                if (wParam == '1') g_win.screen = g_win.prev_screen;
                if (wParam == '2') { g_win.settings_idx = 0; g_win.screen = SCREEN_SETTINGS; }
                if (wParam == '3') { g_win.menu_idx = 0; g_win.screen = SCREEN_MAIN_MENU; }
                if (wParam == 'P' || wParam == VK_ESCAPE) g_win.screen = g_win.prev_screen;
            } else if (g_win.screen == SCREEN_SETTINGS) {
                if (wParam == VK_UP)   g_win.settings_idx = (g_win.settings_idx + SETTINGS_COUNT - 1) % SETTINGS_COUNT;
                if (wParam == VK_DOWN) g_win.settings_idx = (g->settings_idx + 1) % SETTINGS_COUNT;
                if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_RETURN || wParam == VK_SPACE) {
                    toggle_setting_item(&g_win);
                }
                if (wParam == VK_ESCAPE) g_win.screen = g_win.prev_screen;
            } else if (g_win.screen == SCREEN_CONTROLS) {
                if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_SPACE) {
                    g_win.screen = g_win.prev_screen;
                }
            } else if (g_win.screen == SCREEN_WIFI_SCORE) {
                if (wParam == VK_RETURN) compare_score_over_wifi_ip(&g_win);
                if (wParam == VK_ESCAPE) g_win.screen = SCREEN_MAIN_MENU;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rc);
            blit_vga_buffer_to_gdi(hdc, &rc);
            EndPaint(hwnd, &ps);
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            write_obfuscated_options(&g_win);
            apply_rumble_and_dualsense(0, 0, 0, 0, 0);
            if (g_win.video_recording) save_video_to_file(&g_win);
            free_media_buffers(&g_win);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

#if (BGC_WIN_ARCH == 16)
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
#endif
{
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    (void)hPrev;

    memset(&g_win, 0, sizeof(g_win));
    g_win.host_is_p1 = 1;
    init_mouse_state(&g_win.mouse);
    strcpy(g_win.peer_ip, "192.168.1.16");
    g_win.screen = SCREEN_MAIN_MENU;
    g_win.pause_style = PAUSE_STYLE_FULL;
    g_win.ctrl_speed_mode = CTRL_SPEED_NORMAL;
    g_win.ctrl_source = CTRL_SRC_AUTO;
    read_obfuscated_vault(&g_win);
    init_vga_mode(&g_win, lpCmd);

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = BGC_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "BGC_ClassicWinClass";

    if (!RegisterClass(&wc)) return 0;

    hwnd = CreateWindow(
        "BGC_ClassicWinClass",
        BGC_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 400,
        NULL, NULL, hInst, NULL
    );

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    write_obfuscated_options(&g_win);
    apply_rumble_and_dualsense(0, 0, 0, 0, 0);
    free_media_buffers(&g_win);
    return (int)msg.wParam;
}

#endif
