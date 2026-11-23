/* ============================================================================
 * BEE GOLD COLLECTOR: LEGACY COMPUTERS SERIES + BEERACE + WIFI SCORE
 * + BUILD-TIME SECRET KEY (BGC_VAULT_KEY)
 * + REPLAY RECORDING, VIDEO RECORDING TO FILE & SCREENSHOT (.BMP) EXPORT
 * + DOUBLE-BUFFERED GDI (CreateCompatibleDC/Bitmap/BitBlt) + GDI FONT
 * + RUMBLE + DUALSENSE HAPTIC & LED LIGHTBAR
 * + UNIFIED MEDIA VAULT (PHOTO + REPLAY + VIDEO IN ONE SCREEN + SAVE TO DEVICE)
 * + SINGLE-FILE AUTO-ADAPTATION FOR MS-DOS, FREEDOS, WIN16, WIN9X/2000/XP & WIN32/64
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__MSDOS__) || defined(_MSDOS) || defined(__DOS__)
    #define BGC_DOS_TARGET 1
    #define BGC_HAS_SOCKETS 0
    #include <dos.h>
    #include <conio.h>
    #include <malloc.h>
#else
    #define BGC_DOS_TARGET 0
    #include <windows.h>
    #include <mmsystem.h>
    #if !defined(_WIN16) && !defined(__WINDOWS_386__)
        #include <winsock2.h>
        #define BGC_HAS_SOCKETS 1
    #else
        #define BGC_HAS_SOCKETS 0
    #endif
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

#ifndef BGC_TITLE
    #define BGC_TITLE "BEE GOLD COLLECTOR: LEGACY & BEERACE"
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

#define VAULT_FILE      "BGCSCV.DAT"
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
#define BACKEND_GDI 2

#define PAUSE_STYLE_FULL    0
#define PAUSE_STYLE_COMPACT 1

#define CTRL_SPEED_NORMAL 0
#define CTRL_SPEED_FAST   1

#define C_VGA_SKY     11
#define C_VGA_GRASS   2
#define C_VGA_TRUNK   6
#define C_VGA_LEAF    10
#define C_VGA_BLACK   0
#define C_VGA_YELLOW  14
#define C_VGA_WING    15
#define C_VGA_RED     4
#define C_VGA_PANEL   7

#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16)
typedef struct {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} BGC_XINPUT_VIBRATION;

typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD dwUserIndex, BGC_XINPUT_VIBRATION *pVibration);
typedef HANDLE (WINAPI *PFN_CreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL (WINAPI *PFN_WriteFile)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL (WINAPI *PFN_CloseHandle)(HANDLE);
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
#if BGC_DOS_TARGET
    int calibrated;
    unsigned int center_x;
    unsigned int center_y;
    unsigned int dead_x;
    unsigned int dead_y;
#endif
} BGC_Pad;

typedef struct {
    int x;
    int y;
    int gold_x;
    int gold_y;
    int bug_x;
    int bug_y;
    int bug_speed;
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
    int bee_x;
    int bee_y;
    int gold_x;
    int gold_y;
    int bug_x;
    int bug_y;
    int bug_speed;
    int score;
    int best_score;
    int last_score;
    int health;
    int game_over;
    long tick;

    Racer p1;
    Racer p2;
    int race_over;
    int race_winner;
    long regen_timer_ticks;

    char peer_ip[32];
    int peer_best_score;
    int wifi_status;

    /* Unified Media Vault state (PHOTO / REPLAY / VIDEO) */
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

    /* Rumble & DualSense LED/Haptic state */
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
    int gdi_available;
    int pause_style;
    int ctrl_speed_mode;
    BGC_Pad pad1;
    BGC_Pad pad2;

#if !BGC_DOS_TARGET
    HDC mem_dc;
    HBITMAP mem_bmp;
    HBITMAP old_bmp;
    HFONT gdi_font;
#endif
} BGC_Game;

/* ============================================================================
 * 1. BITMAP FONT & LOW-LEVEL PRIMITIVES (DOUBLE-BUFFERED GDI + VGA + MIRROR)
 * ============================================================================ */

static unsigned char g_vga_buf[SCR_W * SCR_H];

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
static COLORREF vga_to_gdi_color(unsigned char idx) {
    switch (idx) {
        case C_VGA_BLACK:  return RGB(12, 12, 12);
        case C_VGA_GRASS:  return RGB(34, 139, 34);
        case C_VGA_LEAF:   return RGB(72, 198, 64);
        case C_VGA_TRUNK:  return RGB(115, 68, 32);
        case C_VGA_SKY:    return RGB(125, 205, 245);
        case C_VGA_YELLOW: return RGB(255, 215, 0);
        case C_VGA_WING:   return RGB(245, 250, 255);
        case C_VGA_RED:    return RGB(210, 35, 35);
        case C_VGA_PANEL:  return RGB(224, 232, 238);
        default:           return RGB(125, 205, 245);
    }
}

static void init_gdi_double_buffer(BGC_Game *g, HWND hwnd) {
    HDC screen_dc = GetDC(hwnd);
    g->mem_dc = NULL;
    g->mem_bmp = NULL;
    g->old_bmp = NULL;
    g->gdi_font = NULL;

    if (screen_dc) {
        g->mem_dc = CreateCompatibleDC(screen_dc);
        if (g->mem_dc) {
            g->mem_bmp = CreateCompatibleBitmap(screen_dc, SCR_W, SCR_H);
            if (g->mem_bmp) {
                g->old_bmp = (HBITMAP)SelectObject(g->mem_dc, g->mem_bmp);
            }
#if (BGC_WIN_ARCH != 16)
            g->gdi_font = CreateFontA(
                8, 5, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Terminal"
            );
#endif
            if (!g->gdi_font) {
                g->gdi_font = (HFONT)GetStockObject(SYSTEM_FONT);
            }
        }
        ReleaseDC(hwnd, screen_dc);
    }
}

static void cleanup_gdi_double_buffer(BGC_Game *g) {
    if (g->gdi_font && g->gdi_font != (HFONT)GetStockObject(SYSTEM_FONT)) {
        DeleteObject(g->gdi_font);
        g->gdi_font = NULL;
    }
    if (g->mem_dc) {
        if (g->old_bmp) {
            SelectObject(g->mem_dc, g->old_bmp);
            g->old_bmp = NULL;
        }
        if (g->mem_bmp) {
            DeleteObject(g->mem_bmp);
            g->mem_bmp = NULL;
        }
        DeleteDC(g->mem_dc);
        g->mem_dc = NULL;
    }
}
#endif

static void vga_color_to_rgb(unsigned char idx, unsigned char *r, unsigned char *g, unsigned char *b) {
    switch (idx) {
        case C_VGA_BLACK:  *r = 12;  *g = 12;  *b = 12;  break;
        case C_VGA_GRASS:  *r = 34;  *g = 139; *b = 34;  break;
        case C_VGA_LEAF:   *r = 72;  *g = 198; *b = 64;  break;
        case C_VGA_TRUNK:  *r = 115; *g = 68;  *b = 32;  break;
        case C_VGA_SKY:    *r = 125; *g = 205; *b = 245; break;
        case C_VGA_YELLOW: *r = 255; *g = 215; *b = 0;   break;
        case C_VGA_WING:   *r = 245; *g = 250; *b = 255; break;
        case C_VGA_RED:    *r = 210; *g = 35;  *b = 35;  break;
        case C_VGA_PANEL:  *r = 224; *g = 232; *b = 238; break;
        default:           *r = 125; *g = 205; *b = 245; break;
    }
}

static void raw_put_pixel(int x, int y, unsigned char vga_idx) {
    if (x < 0 || x >= SCR_W || y < 0 || y >= SCR_H) return;
    g_vga_buf[(unsigned long)y * SCR_W + (unsigned long)x] = vga_idx;
#if BGC_DOS_TARGET
    {
        unsigned char far *vram = (unsigned char far *)0xA0000000L;
        vram[(unsigned long)y * SCR_W + (unsigned long)x] = vga_idx;
    }
#endif
}

static void draw_px(const BGC_Game *g, void *hdc_ptr, int x, int y, unsigned char vga_idx) {
    if (x < 0 || x >= SCR_W || y < 0 || y >= SCR_H) return;
    g_vga_buf[(unsigned long)y * SCR_W + (unsigned long)x] = vga_idx;
    if (g->backend == BACKEND_VGA || !g->gdi_available) {
        raw_put_pixel(x, y, vga_idx);
    }
#if !BGC_DOS_TARGET
    if (g->backend == BACKEND_GDI && g->gdi_available && hdc_ptr) {
        SetPixel((HDC)hdc_ptr, x, y, vga_to_gdi_color(vga_idx));
    }
#endif
}

static void draw_box(const BGC_Game *g, void *hdc_ptr, int x, int y, int w, int h, unsigned char fill_idx, int border) {
    int ix, iy;
    for (iy = y; iy < y + h; iy++) {
        for (ix = x; ix < x + w; ix++) {
            if (ix >= 0 && ix < SCR_W && iy >= 0 && iy < SCR_H) {
                unsigned long idx = (unsigned long)iy * SCR_W + (unsigned long)ix;
                if (border && (ix == x || ix == x + w - 1 || iy == y || iy == y + h - 1)) {
                    g_vga_buf[idx] = C_VGA_BLACK;
                } else {
                    g_vga_buf[idx] = fill_idx;
                }
            }
        }
    }

#if !BGC_DOS_TARGET
    if (g->backend == BACKEND_GDI && g->gdi_available && hdc_ptr) {
        RECT rc;
        HBRUSH br = CreateSolidBrush(vga_to_gdi_color(fill_idx));
        rc.left = x;
        rc.top = y;
        rc.right = x + w;
        rc.bottom = y + h;
        FillRect((HDC)hdc_ptr, &rc, br);
        DeleteObject(br);

        if (border) {
            HPEN pen = CreatePen(PS_SOLID, 1, vga_to_gdi_color(C_VGA_BLACK));
            HGDIOBJ old_pen = SelectObject((HDC)hdc_ptr, pen);
            HGDIOBJ old_br = SelectObject((HDC)hdc_ptr, GetStockObject(NULL_BRUSH));
            Rectangle((HDC)hdc_ptr, x, y, x + w, y + h);
            SelectObject((HDC)hdc_ptr, old_br);
            SelectObject((HDC)hdc_ptr, old_pen);
            DeleteObject(pen);
        }
        return;
    }
#endif
    for (iy = y; iy < y + h; iy++) {
        for (ix = x; ix < x + w; ix++) {
            if (border && (ix == x || ix == x + w - 1 || iy == y || iy == y + h - 1)) {
                draw_px(g, hdc_ptr, ix, iy, C_VGA_BLACK);
            } else {
                draw_px(g, hdc_ptr, ix, iy, fill_idx);
            }
        }
    }
}

static void draw_circle_fill(const BGC_Game *g, void *hdc_ptr, int cx, int cy, int r, unsigned char fill_idx, int outline_black) {
    int dx, dy;
#if !BGC_DOS_TARGET
    if (g->backend == BACKEND_GDI && g->gdi_available && hdc_ptr) {
        HBRUSH br = CreateSolidBrush(vga_to_gdi_color(fill_idx));
        HGDIOBJ old_br = SelectObject((HDC)hdc_ptr, br);
        HPEN pen = CreatePen(PS_SOLID, 1, vga_to_gdi_color(outline_black ? C_VGA_BLACK : fill_idx));
        HGDIOBJ old_pen = SelectObject((HDC)hdc_ptr, pen);
        Ellipse((HDC)hdc_ptr, cx - r, cy - r, cx + r + 1, cy + r + 1);
        SelectObject((HDC)hdc_ptr, old_pen);
        SelectObject((HDC)hdc_ptr, old_br);
        DeleteObject(pen);
        DeleteObject(br);
    }
#endif
    for (dy = -r; dy <= r; dy++) {
        for (dx = -r; dx <= r; dx++) {
            int d2 = dx * dx + dy * dy;
            if (d2 <= r * r) {
                unsigned char c = (outline_black && d2 >= (r - 1) * (r - 1)) ? C_VGA_BLACK : fill_idx;
                if (cx + dx >= 0 && cx + dx < SCR_W && cy + dy >= 0 && cy + dy < SCR_H) {
                    g_vga_buf[(unsigned long)(cy + dy) * SCR_W + (unsigned long)(cx + dx)] = c;
                }
                if (g->backend == BACKEND_VGA || !g->gdi_available || !hdc_ptr) {
                    raw_put_pixel(cx + dx, cy + dy, c);
                }
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

#if !BGC_DOS_TARGET
    if (g->backend == BACKEND_GDI && g->gdi_available && hdc_ptr) {
        HPEN pen = CreatePen(PS_SOLID, 1, vga_to_gdi_color(col_idx));
        HGDIOBJ old_pen = SelectObject((HDC)hdc_ptr, pen);
        MoveToEx((HDC)hdc_ptr, x0, y0, NULL);
        LineTo((HDC)hdc_ptr, x1, y1);
        SelectObject((HDC)hdc_ptr, old_pen);
        DeleteObject(pen);
    }
#endif
    while (1) {
        if (x0 >= 0 && x0 < SCR_W && y0 >= 0 && y0 < SCR_H) {
            g_vga_buf[(unsigned long)y0 * SCR_W + (unsigned long)x0] = col_idx;
        }
        if (g->backend == BACKEND_VGA || !g->gdi_available || !hdc_ptr) {
            raw_put_pixel(x0, y0, col_idx);
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
#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16)
    if (g->backend == BACKEND_GDI && g->gdi_available && hdc_ptr) {
        HDC dc = (HDC)hdc_ptr;
        HFONT old_f = NULL;
        int old_bk = SetBkMode(dc, TRANSPARENT);
        COLORREF old_col = SetTextColor(dc, vga_to_gdi_color(col_idx));
        if (g->gdi_font) {
            old_f = (HFONT)SelectObject(dc, g->gdi_font);
        }
        TextOutA(dc, x, y, s, (int)strlen(s));
        if (old_f) SelectObject(dc, old_f);
        SetTextColor(dc, old_col);
        SetBkMode(dc, old_bk);
        return;
    }
#endif
    {
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
}

static void blit_frame_to_screen(const BGC_Game *g, void *hdc_ptr, const unsigned char *frame) {
    int x, y;
    if (!frame) return;
    for (y = 0; y < SCR_H; y++) {
        for (x = 0; x < SCR_W; x++) {
            unsigned char idx = frame[(unsigned long)y * SCR_W + (unsigned long)x];
            g_vga_buf[(unsigned long)y * SCR_W + (unsigned long)x] = idx;
            if (g->backend == BACKEND_VGA || !g->gdi_available) {
                raw_put_pixel(x, y, idx);
            }
        }
    }
#if !BGC_DOS_TARGET
    if (g->backend == BACKEND_GDI && g->gdi_available && hdc_ptr) {
        for (y = 0; y < SCR_H; y++) {
            for (x = 0; x < SCR_W; x++) {
                SetPixel((HDC)hdc_ptr, x, y, vga_to_gdi_color(frame[(unsigned long)y * SCR_W + (unsigned long)x]));
            }
        }
    }
#endif
}

/* ============================================================================
 * 2. RUMBLE + DUALSENSE HAPTIC & LED LIGHTBAR (XINPUT + HID)
 * ============================================================================ */

static void apply_rumble_and_dualsense(unsigned short left_val, unsigned short right_val,
                                       unsigned char r, unsigned char gr, unsigned char b) {
#if !BGC_DOS_TARGET && (BGC_WIN_ARCH != 16)
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
        HMODULE h_k32 = GetModuleHandleA("kernel32.dll");
        HMODULE h_hid = LoadLibraryA("hid.dll");
        if (h_k32 && h_hid) {
            PFN_CreateFileA pCreateFileA = (PFN_CreateFileA)GetProcAddress(h_k32, "CreateFileA");
            PFN_WriteFile pWriteFile = (PFN_WriteFile)GetProcAddress(h_k32, "WriteFile");
            PFN_CloseHandle pCloseHandle = (PFN_CloseHandle)GetProcAddress(h_k32, "CloseHandle");
            PFN_HidD_SetOutputReport pSetOut = (PFN_HidD_SetOutputReport)GetProcAddress(h_hid, "HidD_SetOutputReport");

            if (pCreateFileA && pCloseHandle) {
                unsigned char ds4_rep[32];
                HANDLE h_ds4 = pCreateFileA(
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
                    if (pSetOut) pSetOut(h_ds4, ds4_rep, sizeof(ds4_rep));
                    else if (pWriteFile) {
                        DWORD written = 0;
                        pWriteFile(h_ds4, ds4_rep, sizeof(ds4_rep), &written, NULL);
                    }
                    pCloseHandle(h_ds4);
                }

                {
                    unsigned char ds5_rep[48];
                    HANDLE h_ds5 = pCreateFileA(
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
                        if (pSetOut) pSetOut(h_ds5, ds5_rep, sizeof(ds5_rep));
                        else if (pWriteFile) {
                            DWORD written = 0;
                            pWriteFile(h_ds5, ds5_rep, sizeof(ds5_rep), &written, NULL);
                        }
                        pCloseHandle(h_ds5);
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
 * 3. BUILD-TIME SECRET KEY VAULT (BGCSCV.DAT)
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
    int timeout_ms = 600;
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
 * 6. DUAL GAMEPAD LAYER (PAD 1 & PAD 2)
 * ============================================================================ */

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
    union REGS r_in, r_out;
    unsigned int jx, jy;
    unsigned char buttons_mask;
    (void)slot_idx;

    pad->dx = 0;
    pad->dy = 0;

    r_in.h.ah = 0x84;
    r_in.x.dx = 0x0000;
    int86(0x15, &r_in, &r_out);
    if (r_out.x.cflag & 1) {
        pad->connected = 0;
        update_pad_edges(pad, 0, 0, 0, 0);
        return;
    }
    buttons_mask = (unsigned char)((~r_out.h.al) >> 4) & 0x0F;

    r_in.h.ah = 0x84;
    r_in.x.dx = 0x0001;
    int86(0x15, &r_in, &r_out);
    if (r_out.x.cflag & 1 || r_out.x.ax == 0 || r_out.x.by == 0) {
        pad->connected = 0;
        update_pad_edges(pad, 0, 0, 0, 0);
        return;
    }
    jx = r_out.x.ax;
    jy = r_out.x.bx;

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

#if (BGC_WIN_ARCH != 16)
    {
        JOYINFOEX ji_ex;
        memset(&ji_ex, 0, sizeof(ji_ex));
        ji_ex.dwSize = sizeof(ji_ex);
        ji_ex.dwFlags = JOY_RETURNALL;

        if (joyGetPosEx(id, &ji_ex) == JOYERR_NOERROR) {
            DWORD mid = 32768, dead = 9000;
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
            if (ji_ex.dwButtons & JOY_BUTTON1) rc = 1;
            if (ji_ex.dwButtons & JOY_BUTTON2) rb = 1;
            if (ji_ex.dwButtons & (JOY_BUTTON3 | JOY_BUTTON4 | JOY_BUTTON7 | JOY_BUTTON8)) rp = 1;
            if (ji_ex.dwButtons & (JOY_BUTTON1 | JOY_BUTTON3)) rr = 1;
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
 * 7. TREES, GRASS, CARTOON BEE & BLACK BUGS (SINGLE & SPLIT-SCREEN)
 * ============================================================================ */

static void draw_tree_box(const BGC_Game *g, void *hdc_ptr, int tx, int ground_y, int min_x, int max_x, int min_y) {
    if (tx < min_x || tx > max_x) return;
    draw_box(g, hdc_ptr, tx - 3, ground_y - 20, 6, 20, C_VGA_TRUNK, 1);
    if (ground_y - 26 > min_y) draw_circle_fill(g, hdc_ptr, tx, ground_y - 26, 9, C_VGA_LEAF, 1);
}

static void draw_viewport_env(const BGC_Game *g, void *hdc_ptr, int vx, int vy, int vw, int vh) {
    int ground_y = vy + vh - 20;
    int offset = (int)((g->tick * 2l) % 64l);
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

static void draw_gold_coin(const BGC_Game *g, void *hdc_ptr, int x, int y) {
    draw_circle_fill(g, hdc_ptr, x, y, 4, C_VGA_YELLOW, 1);
    draw_px(g, hdc_ptr, x - 1, y - 1, C_VGA_WING);
}

/* ============================================================================
 * 8. SINGLE & BEERACE GAMEPLAY LOGIC
 * ============================================================================ */

static void spawn_single_gold(BGC_Game *g) {
    g->gold_x = SCR_W - 14;
    g->gold_y = 24 + (rand() % (SCR_H - 54));
}

static void spawn_single_bug(BGC_Game *g) {
    g->bug_x = SCR_W - 12;
    g->bug_y = 24 + (rand() % (SCR_H - 54));
    g->bug_speed = 2 + (g->score / 50);
    if (g->bug_speed > 8) g->bug_speed = 8;
}

static void start_single_game(BGC_Game *g) {
    g->bee_x = SCR_W / 5;
    g->bee_y = SCR_H / 2;
    g->score = 0;
    g->health = 100;
    g->game_over = 0;
    g->tick = 0;
    g->replay_recording = 1;
    g->replay_count = 0;
    g->pause_idx = 0;
    g->screen = SCREEN_PLAYING;
    spawn_single_gold(g);
    spawn_single_bug(g);
    set_dualsense_led_from_health(g, g->health);
}

static void init_racer(Racer *r, int pane_w, int pane_h, int seed_offset) {
    r->x = pane_w / 5;
    r->y = pane_h / 2;
    r->score = 0;
    r->health = 100;
    r->dead = 0;
    r->gold_x = pane_w - 14;
    r->gold_y = 20 + ((rand() + seed_offset) % ((pane_h > 45) ? (pane_h - 42) : 20));
    r->bug_x = pane_w - 12;
    r->bug_y = 24 + ((rand() + seed_offset * 3) % ((pane_h > 45) ? (pane_h - 42) : 20));
    r->bug_speed = 3;
}

static void start_beerace(BGC_Game *g) {
    int half_w = SCR_W / 2;
    init_racer(&g->p1, half_w, SCR_H, 7);
    init_racer(&g->p2, half_w, SCR_H, 19);
    g->race_over = 0;
    g->race_winner = 0;
    g->regen_timer_ticks = 0;
    g->tick = 0;
    g->pause_idx = 0;
    g->screen = SCREEN_BEERACE;
    set_dualsense_led_from_health(g, 100);
}

static int move_step_val(const BGC_Game *g) {
    return (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? 8 : 5;
}

static void move_single_bee(BGC_Game *g, int dx, int dy) {
    int stp = move_step_val(g);
    if (g->screen != SCREEN_PLAYING || g->game_over) return;
    g->bee_x += dx * stp;
    g->bee_y += dy * stp;
    if (g->bee_x < 12) g->bee_x = 12;
    if (g->bee_x > SCR_W - 14) g->bee_x = SCR_W - 14;
    if (g->bee_y < 18) g->bee_y = 18;
    if (g->bee_y > SCR_H - 28) g->bee_y = SCR_H - 28;
}

static void move_racer(const BGC_Game *g, Racer *r, int dx, int dy, int pane_w, int pane_h) {
    int stp = move_step_val(g);
    if (r->dead) return;
    r->x += dx * stp;
    r->y += dy * stp;
    if (r->x < 10) r->x = 10;
    if (r->x > pane_w - 12) r->x = pane_w - 12;
    if (r->y < 18) r->y = 18;
    if (r->y > pane_h - 26) r->y = pane_h - 26;
}

static void step_racer(BGC_Game *g, Racer *r, int pane_w, int pane_h) {
    if (r->dead) return;

    r->gold_x -= 2;
    r->bug_x -= r->bug_speed;
    r->health -= 1;

    if (r->health <= 0) {
        r->health = 0;
        r->dead = 1;
        trigger_rumble_and_led(g, 60000, 60000, 8, 255, 20, 20);
        return;
    }

    if (r->gold_x < 8) {
        r->gold_x = pane_w - 12;
        r->gold_y = 20 + (rand() % ((pane_h > 45) ? (pane_h - 42) : 20));
    }

    if (r->bug_x < 8) {
        r->bug_x = pane_w - 10;
        r->bug_y = 20 + (rand() % ((pane_h > 45) ? (pane_h - 42) : 20));
        r->bug_speed = 3 + (r->score / 50);
        if (r->bug_speed > 9) r->bug_speed = 9;
    }

    if (abs(r->x - r->gold_x) < 11 && abs(r->y - r->gold_y) < 11) {
        r->score += 10;
        r->health += 15;
        trigger_rumble_and_led(g, 14000, 14000, 2, 255, 215, 0);
        r->gold_x = pane_w - 12;
        r->gold_y = 20 + (rand() % ((pane_h > 45) ? (pane_h - 42) : 20));
    }

    if (abs(r->x - r->bug_x) < 11 && abs(r->y - r->bug_y) < 11) {
        r->health -= 35;
        trigger_rumble_and_led(g, 48000, 48000, 5, 240, 25, 25);
        r->bug_x = pane_w - 10;
        r->bug_y = 20 + (rand() % ((pane_h > 45) ? (pane_h - 42) : 20));
        if (r->health <= 0) {
            r->health = 0;
            r->dead = 1;
        }
    }
}

static void step_beerace(BGC_Game *g) {
    int half_w = SCR_W / 2;
    if (g->race_over) return;

    g->regen_timer_ticks++;
    if (g->regen_timer_ticks >= 250l) {
        g->regen_timer_ticks = 0;
        if (!g->p1.dead) g->p1.health += 2;
        if (!g->p2.dead) g->p2.health += 2;
    }

    step_racer(g, &g->p1, half_w, SCR_H);
    step_racer(g, &g->p2, half_w, SCR_H);

    if (g->p1.dead && !g->p2.dead) {
        g->race_over = 1;
        g->race_winner = 2;
        record_score_secure(g, g->p2.score);
    } else if (g->p2.dead && !g->p1.dead) {
        g->race_over = 1;
        g->race_winner = 1;
        record_score_secure(g, g->p1.score);
    } else if (g->p1.dead && g->p2.dead) {
        g->race_over = 1;
        g->race_winner = (g->p1.score >= g->p2.score) ? 1 : 2;
        record_score_secure(g, (g->p1.score >= g->p2.score) ? g->p1.score : g->p2.score);
    }
}

static void step_single_game(BGC_Game *g) {
    if (g->game_over) return;

    g->gold_x -= 2;
    g->bug_x -= g->bug_speed;
    g->health -= 1;

    if (g->health <= 0) {
        g->health = 0;
        g->game_over = 1;
        g->replay_recording = 0;
        save_replay_to_file(g);
        record_score_secure(g, g->score);
        trigger_rumble_and_led(g, 60000, 60000, 8, 255, 20, 20);
        return;
    }

    if (g->gold_x < 8) spawn_single_gold(g);
    if (g->bug_x < 8) spawn_single_bug(g);

    if (abs(g->bee_x - g->gold_x) < 12 && abs(g->bee_y - g->gold_y) < 12) {
        g->score += 10;
        g->health += 15;
        if (g->health > 100) g->health = 100;
        trigger_rumble_and_led(g, 14000, 14000, 2, 255, 215, 0);
        spawn_single_gold(g);
    }

    if (abs(g->bee_x - g->bug_x) < 12 && abs(g->bee_y - g->bug_y) < 12) {
        g->health -= 35;
        trigger_rumble_and_led(g, 48000, 48000, 5, 240, 25, 25);
        spawn_single_bug(g);
        if (g->health <= 0) {
            g->health = 0;
            g->game_over = 1;
            g->replay_recording = 0;
            save_replay_to_file(g);
            record_score_secure(g, g->score);
            trigger_rumble_and_led(g, 60000, 60000, 8, 255, 20, 20);
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
            g->video_play_idx++;
            if (g->video_play_idx >= g->video_frame_count) {
                g->video_play_idx = 0;
                g->video_playing = 0;
            }
        }
    }
}

/* ============================================================================
 * 9. UNIFIED MEDIA VAULT (PHOTO + REPLAY + VIDEO IN ONE SCREEN)
 * ============================================================================ */

static void open_media_vault(BGC_Game *g) {
    if (g->replay_count == 0) load_replay_from_file(g);
    if (g->video_frame_count == 0) load_video_from_file(g);
    g->replay_play_idx = 0;
    g->video_play_idx = 0;
    g->replay_playing = (g->media_tab == MEDIA_TAB_REPLAY && g->replay_count > 0) ? 1 : 0;
    g->video_playing  = (g->media_tab == MEDIA_TAB_VIDEO && g->video_frame_count > 0) ? 1 : 0;
    g->screen = SCREEN_MEDIA_VAULT;
}

static void switch_media_tab(BGC_Game *g, int dir) {
    g->media_tab = (g->media_tab + (dir > 0 ? 1 : 2)) % 3;
    g->replay_play_idx = 0;
    g->video_play_idx = 0;
    g->replay_playing = (g->media_tab == MEDIA_TAB_REPLAY && g->replay_count > 0) ? 1 : 0;
    g->video_playing  = (g->media_tab == MEDIA_TAB_VIDEO && g->video_frame_count > 0) ? 1 : 0;
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
        g->replay_playing = !g->replay_playing;
    } else if (g->media_tab == MEDIA_TAB_VIDEO) {
        g->video_playing = !g->video_playing;
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

static void toggle_setting_item(BGC_Game *g) {
    if (g->settings_idx == 0) {
        if (g->gdi_available) g->backend = (g->backend == BACKEND_GDI) ? BACKEND_VGA : BACKEND_GDI;
        else g->backend = BACKEND_VGA;
    } else if (g->settings_idx == 1) {
        g->pause_style = (g->pause_style == PAUSE_STYLE_FULL) ? PAUSE_STYLE_COMPACT : PAUSE_STYLE_FULL;
    } else if (g->settings_idx == 2) {
        g->ctrl_speed_mode = (g->ctrl_speed_mode == CTRL_SPEED_NORMAL) ? CTRL_SPEED_FAST : CTRL_SPEED_NORMAL;
    } else if (g->settings_idx == 3) {
        g->prev_screen = SCREEN_SETTINGS;
        g->screen = SCREEN_CONTROLS;
    } else if (g->settings_idx == 4) {
        g->screen = g->prev_screen;
    }
}

static void activate_main_item(BGC_Game *g, int *running) {
    if (g->menu_idx == 0) {
        start_single_game(g);
    } else if (g->menu_idx == 1) {
        start_beerace(g);
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

    draw_box(g, hdc_ptr, 54, 12, 212, 174, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 72, 18, "BEE GOLD COLLECTOR", C_VGA_BLACK);
    sprintf(best_line, "BEST:%d LAST:%d", g->best_score, g->last_score);
    draw_str_5x7(g, hdc_ptr, 84, 28, best_line, C_VGA_RED);

    for (i = 0; i < 7; i++) {
        int y = 42 + i * 20;
        if (g->menu_idx == i) {
            draw_box(g, hdc_ptr, 68, y - 3, 184, 13, C_VGA_YELLOW, 1);
            draw_str_5x7(g, hdc_ptr, 74, y, ">", C_VGA_BLACK);
        }
        draw_str_5x7(g, hdc_ptr, 86, y, items[i], C_VGA_BLACK);
    }
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
        sprintf(status_line, "VIDEO %d/%d", g->video_play_idx + 1, g->video_frame_count);
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

    draw_box(g, hdc_ptr, 12, 178, 296, 15, C_VGA_YELLOW, 1);
    draw_str_5x7(g, hdc_ptr, 18, 182, "< >:MODE  SPACE:PLAY  S:SAVE  ESC", C_VGA_BLACK);
    draw_capture_banners(g, hdc_ptr);
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
}

static void render_controls(BGC_Game *g, void *hdc_ptr) {
    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_box(g, hdc_ptr, 20, 16, 280, 168, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 118, 24, "CONTROLS", C_VGA_BLACK);

    draw_str_5x7(g, hdc_ptr, 28, 42, "P1 MOVE : ARROWS / WASD / PAD1", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 56, "P2 RACE : IJKL / TFGH / PAD2", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 70, "DUALSENSE: HAPTIC + LED LIGHTBAR", C_VGA_RED);
    draw_str_5x7(g, hdc_ptr, 28, 84, "SHOT BMP: F12  |  VIDEO VDO: V", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 98, "VAULT   : PHOTO + REPLAY + VIDEO", C_VGA_BLACK);
    draw_str_5x7(g, hdc_ptr, 28, 112, "BEERACE : STACK HP +2HP/10SEC", C_VGA_BLACK);

    draw_box(g, hdc_ptr, 84, 148, 152, 15, C_VGA_YELLOW, 1);
    draw_str_5x7(g, hdc_ptr, 92, 152, "> ENTER OR ESC BACK", C_VGA_BLACK);
}

static void render_settings(BGC_Game *g, void *hdc_ptr) {
    char l_gfx[48], l_pau[48], l_spd[48];
    const char *rows[5];
    int i;

    sprintf(l_gfx, "GFX MODE  : %s", (g->backend == BACKEND_GDI && g->gdi_available) ? "GDI" : "VGA");
    sprintf(l_pau, "PAUSE UI  : %s", (g->pause_style == PAUSE_STYLE_COMPACT) ? "COMPACT" : "FULL");
    sprintf(l_spd, "CTRL SPEED: %s", (g->ctrl_speed_mode == CTRL_SPEED_FAST) ? "FAST" : "NORMAL");

    rows[0] = l_gfx;
    rows[1] = l_pau;
    rows[2] = l_spd;
    rows[3] = "CONTROLS";
    rows[4] = "BACK";

    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_box(g, hdc_ptr, 38, 24, 244, 148, C_VGA_PANEL, 1);
    draw_str_5x7(g, hdc_ptr, 118, 32, "SETTINGS", C_VGA_BLACK);

    for (i = 0; i < 5; i++) {
        int y = 50 + i * 22;
        if (g->settings_idx == i) {
            draw_box(g, hdc_ptr, 50, y - 3, 220, 12, C_VGA_YELLOW, 1);
            draw_str_5x7(g, hdc_ptr, 56, y, ">", C_VGA_BLACK);
        }
        draw_str_5x7(g, hdc_ptr, 68, y, rows[i], C_VGA_BLACK);
    }
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
    int bar_w = r->health;
    if (bar_w > vw - 14) bar_w = vw - 14;
    if (bar_w < 0) bar_w = 0;

    draw_viewport_env(g, hdc_ptr, vx, 0, vw, SCR_H);
    draw_gold_coin(g, hdc_ptr, vx + r->gold_x, r->gold_y);
    draw_black_bug(g, hdc_ptr, vx + r->bug_x, r->bug_y, g->tick);
    draw_cartoon_bee(g, hdc_ptr, vx + r->x, r->y, g->tick);

    draw_box(g, hdc_ptr, vx + 4, 4, vw - 8, 7, C_VGA_WING, 1);
    if (bar_w > 0) {
        draw_box(g, hdc_ptr, vx + 5, 5, bar_w, 5, (r->health > 30) ? C_VGA_GRASS : C_VGA_RED, 0);
    }

    sprintf(hud, "%s HP:%d S:%d", tag, r->health, r->score);
    draw_str_5x7(g, hdc_ptr, vx + 6, 13, hud, C_VGA_BLACK);
}

static void render_beerace_split(BGC_Game *g, void *hdc_ptr) {
    int half_w = SCR_W / 2;

    render_racer_pane(g, hdc_ptr, &g->p1, 0, half_w, "P1");
    render_racer_pane(g, hdc_ptr, &g->p2, half_w, SCR_W - half_w, "P2");
    draw_box(g, hdc_ptr, half_w - 1, 0, 2, SCR_H, C_VGA_BLACK, 0);

    if (g->race_over) {
        char win_txt[48];
        draw_box(g, hdc_ptr, 54, 72, 212, 48, C_VGA_PANEL, 1);
        sprintf(win_txt, "P%d WINS FIRST DEATH OUT", g->race_winner);
        draw_str_5x7(g, hdc_ptr, 64, 82, win_txt, C_VGA_RED);
        draw_str_5x7(g, hdc_ptr, 72, 98, "PRESS R RETRY OR M MENU", C_VGA_BLACK);
    }
    draw_capture_banners(g, hdc_ptr);
}

static void render_scene(BGC_Game *g, void *hdc_ptr) {
    char hud[64];

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
        capture_video_frame(g);
        return;
    }

    draw_viewport_env(g, hdc_ptr, 0, 0, SCR_W, SCR_H);
    draw_gold_coin(g, hdc_ptr, g->gold_x, g->gold_y);
    draw_black_bug(g, hdc_ptr, g->bug_x, g->bug_y, g->tick);
    draw_cartoon_bee(g, hdc_ptr, g->bee_x, g->bee_y, g->tick);

    draw_box(g, hdc_ptr, 6, 5, 102, 8, C_VGA_WING, 1);
    if (g->health > 0) {
        draw_box(g, hdc_ptr, 7, 6, g->health, 6, (g->health > 30) ? C_VGA_GRASS : C_VGA_RED, 0);
    }

    sprintf(hud, "S:%d B:%d %s",
            g->score,
            g->best_score,
            (g->backend == BACKEND_GDI && g->gdi_available) ? "[GDI]" : "[VGA]");
    draw_str_5x7(g, hdc_ptr, 114, 6, hud, C_VGA_BLACK);

    draw_capture_banners(g, hdc_ptr);

    if (g->screen == SCREEN_PAUSED) {
        render_pause(g, hdc_ptr);
    } else if (g->game_over) {
        draw_box(g, hdc_ptr, 52, 74, 216, 42, C_VGA_PANEL, 1);
        draw_str_5x7(g, hdc_ptr, 118, 80, "GAME OVER", C_VGA_RED);
        draw_str_5x7(g, hdc_ptr, 66, 94, "PRESS R RETRY OR M MENU", C_VGA_BLACK);
    }

    capture_video_frame(g);
}

static void poll_dual_pads_and_apply(BGC_Game *g, int *running) {
    int half_w = SCR_W / 2;
    poll_single_pad(&g->pad1, 0);
    poll_single_pad(&g->pad2, 1);

    if (g->screen == SCREEN_MAIN_MENU) {
        if (g->pad1.dy < 0 && ((g->tick % 4l) == 0)) g->menu_idx = (g->menu_idx + 6) % 7;
        if (g->pad1.dy > 0 && ((g->tick % 4l) == 0)) g->menu_idx = (g->menu_idx + 1) % 7;
        if (g->pad1.btn_confirm) activate_main_item(g, running);
    } else if (g->screen == SCREEN_MEDIA_VAULT) {
        if (g->pad1.dx < 0 && ((g->tick % 4l) == 0)) switch_media_tab(g, -1);
        if (g->pad1.dx > 0 && ((g->tick % 4l) == 0)) switch_media_tab(g,  1);
        if (g->pad1.btn_confirm) toggle_current_media_play(g);
        if (g->pad1.btn_retry) save_current_media_to_device(g);
        if (g->pad1.btn_back) g->screen = SCREEN_MAIN_MENU;
    } else if (g->screen == SCREEN_PLAYING) {
        if (g->pad1.dx != 0 || g->pad1.dy != 0) move_single_bee(g, g->pad1.dx, g->pad1.dy);
        if (g->pad1.btn_pause || g->pad1.btn_back) {
            g->prev_screen = SCREEN_PLAYING;
            g->pause_idx = 0;
            g->screen = SCREEN_PAUSED;
        }
        if (g->game_over && g->pad1.btn_retry) start_single_game(g);
    } else if (g->screen == SCREEN_BEERACE) {
        if (g->pad1.dx != 0 || g->pad1.dy != 0) move_racer(g, &g->p1, g->pad1.dx, g->pad1.dy, half_w, SCR_H);
        if (g->pad2.dx != 0 || g->pad2.dy != 0) move_racer(g, &g->p2, g->pad2.dx, g->pad2.dy, half_w, SCR_H);
        if (g->pad1.btn_pause || g->pad2.btn_pause) {
            g->prev_screen = SCREEN_BEERACE;
            g->pause_idx = 0;
            g->screen = SCREEN_PAUSED;
        }
        if (g->race_over && (g->pad1.btn_retry || g->pad2.btn_retry)) start_beerace(g);
    } else if (g->screen == SCREEN_PAUSED) {
        if (g->pad1.dy < 0 && ((g->tick % 4l) == 0)) g->pause_idx = (g->pause_idx + 2) % 3;
        if (g->pad1.dy > 0 && ((g->tick % 4l) == 0)) g->pause_idx = (g->pause_idx + 1) % 3;
        if (g->pad1.btn_confirm) activate_pause_item(g);
        if (g->pad1.btn_pause || g->pad1.btn_back) g->screen = g->prev_screen;
    } else if (g->screen == SCREEN_SETTINGS) {
        if (g->pad1.dy < 0 && ((g->tick % 4l) == 0)) g->settings_idx = (g->settings_idx + 4) % 5;
        if (g->pad1.dy > 0 && ((g->tick % 4l) == 0)) g->settings_idx = (g->settings_idx + 1) % 5;
        if (g->pad1.btn_confirm || (g->pad1.dx != 0 && ((g->tick % 4l) == 0))) toggle_setting_item(g);
        if (g->pad1.btn_back) g->screen = g->prev_screen;
    } else if (g->screen == SCREEN_CONTROLS) {
        if (g->pad1.btn_confirm || g->pad1.btn_back || g->pad1.btn_pause) g->screen = g->prev_screen;
    } else if (g->screen == SCREEN_WIFI_SCORE) {
        if (g->pad1.btn_confirm) compare_score_over_wifi_ip(g);
        if (g->pad1.btn_back) g->screen = SCREEN_MAIN_MENU;
    }
}

/* ============================================================================
 * 10. DOS / FREEDOS (VGA MODE 13H) & WINDOWS (DOUBLE-BUFFERED GDI + VGA)
 * ============================================================================ */
#if BGC_DOS_TARGET

static void dos_set_video(unsigned char mode) {
    union REGS r;
    r.h.ah = 0x00;
    r.h.al = mode;
    int86(0x10, &r, &r);
}

int main(int argc, char **argv) {
    BGC_Game game;
    int running = 1;
    (void)argc;
    (void)argv;

    srand((unsigned int)time(NULL));
    memset(&game, 0, sizeof(game));
    strcpy(game.peer_ip, "192.168.1.16");
    game.gdi_available = 0;
    game.backend = BACKEND_VGA;
    game.pause_style = PAUSE_STYLE_FULL;
    game.ctrl_speed_mode = CTRL_SPEED_NORMAL;
    game.screen = SCREEN_MAIN_MENU;
    read_obfuscated_vault(&game);
    spawn_single_gold(&game);
    spawn_single_bug(&game);

    dos_set_video(0x13);

    while (running) {
        if (kbhit()) {
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
                    if (ext == 72) game.settings_idx = (game.settings_idx + 4) % 5;
                    if (ext == 80) game.settings_idx = (game.settings_idx + 1) % 5;
                    if (ext == 75 || ext == 77) toggle_setting_item(&game);
                } else if (game.screen == SCREEN_PLAYING) {
                    if (ext == 75) move_single_bee(&game, -1, 0);
                    if (ext == 77) move_single_bee(&game,  1, 0);
                    if (ext == 72) move_single_bee(&game, 0, -1);
                    if (ext == 80) move_single_bee(&game, 0,  1);
                } else if (game.screen == SCREEN_BEERACE) {
                    if (ext == 75) move_racer(&game, &game.p1, -1, 0, SCR_W / 2, SCR_H);
                    if (ext == 77) move_racer(&game, &game.p1,  1, 0, SCR_W / 2, SCR_H);
                    if (ext == 72) move_racer(&game, &game.p1, 0, -1, SCR_W / 2, SCR_H);
                    if (ext == 80) move_racer(&game, &game.p1, 0,  1, SCR_W / 2, SCR_H);
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
                    if (ch == 'p' || ch == 'P' || ch == 27) {
                        game.prev_screen = SCREEN_PLAYING;
                        game.pause_idx = 0;
                        game.screen = SCREEN_PAUSED;
                    } else if (game.game_over && (ch == 'r' || ch == 'R')) start_single_game(&game);
                    else if (game.game_over && (ch == 'm' || ch == 'M')) { game.menu_idx = 0; game.screen = SCREEN_MAIN_MENU; }
                } else if (game.screen == SCREEN_BEERACE) {
                    if (ch == 'j' || ch == 'J') move_racer(&game, &game.p2, -1, 0, SCR_W / 2, SCR_H);
                    if (ch == 'l' || ch == 'L') move_racer(&game, &game.p2,  1, 0, SCR_W / 2, SCR_H);
                    if (ch == 'i' || ch == 'I') move_racer(&game, &game.p2, 0, -1, SCR_W / 2, SCR_H);
                    if (ch == 'k' || ch == 'K') move_racer(&game, &game.p2, 0,  1, SCR_W / 2, SCR_H);
                    if (ch == 'p' || ch == 'P' || ch == 27) {
                        game.prev_screen = SCREEN_BEERACE;
                        game.pause_idx = 0;
                        game.screen = SCREEN_PAUSED;
                    } else if (game.race_over && (ch == 'r' || ch == 'R')) start_beerace(&game);
                    else if (game.race_over && (ch == 'm' || ch == 'M')) { game.menu_idx = 0; game.screen = SCREEN_MAIN_MENU; }
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

        poll_dual_pads_and_apply(&game, &running);
        step_world(&game);
        render_scene(&game, NULL);
        delay(40);
    }

    if (game.video_recording) save_video_to_file(&game);
    free_media_buffers(&game);
    dos_set_video(0x03);
    printf("%s - BEST: %d\n", BGC_TITLE, game.best_score);
    return 0;
}

#else

static BGC_Game g_win;

static void detect_backend(BGC_Game *g, const char *cmd) {
    HDC dc = GetDC(NULL);
    g->gdi_available = (dc != NULL) ? 1 : 0;
    if (dc) ReleaseDC(NULL, dc);

    g->backend = g->gdi_available ? BACKEND_GDI : BACKEND_VGA;
    if (cmd && cmd[0] != '\0') {
        if (strstr(cmd, "--vga") != NULL) g->backend = BACKEND_VGA;
        else if (strstr(cmd, "--gdi") != NULL && g->gdi_available) g->backend = BACKEND_GDI;
    }
}

static void blit_vga_buffer_to_gdi(HDC hdc, const RECT *rc) {
    int cw = rc->right - rc->left;
    int ch = rc->bottom - rc->top;
    int x, y;
    if (cw <= 0 || ch <= 0) return;

    for (y = 0; y < SCR_H; y++) {
        int y0 = (y * ch) / SCR_H;
        int y1 = ((y + 1) * ch) / SCR_H;
        for (x = 0; x < SCR_W; x++) {
            int x0 = (x * cw) / SCR_W;
            int x1 = ((x + 1) * cw) / SCR_W;
            unsigned char idx = g_vga_buf[(unsigned long)y * SCR_W + (unsigned long)x];
            HBRUSH br = CreateSolidBrush(vga_to_gdi_color(idx));
            RECT cell;
            cell.left = x0;
            cell.top = y0;
            cell.right = (x1 > x0) ? x1 : (x0 + 1);
            cell.bottom = (y1 > y0) ? y1 : (y0 + 1);
            FillRect(hdc, &cell, br);
            DeleteObject(br);
        }
    }
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
            srand((unsigned int)time(NULL));
            init_gdi_double_buffer(&g_win, hwnd);
            spawn_single_gold(&g_win);
            spawn_single_bug(&g_win);
            SetTimer(hwnd, 1, 40, NULL);
            return 0;

        case WM_TIMER:
            poll_dual_pads_and_apply(&g_win, &running);
            if (!running) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                return 0;
            }
            step_world(&g_win);
            if (g_win.backend == BACKEND_GDI && g_win.gdi_available && g_win.mem_dc) {
                render_scene(&g_win, (void *)g_win.mem_dc);
            } else {
                memset(g_vga_buf, C_VGA_SKY, sizeof(g_vga_buf));
                render_scene(&g_win, NULL);
            }
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
                if (wParam == VK_RETURN) activate_main_item(&g_win, &running);
                if (!running) PostMessage(hwnd, WM_CLOSE, 0, 0);
            } else if (g_win.screen == SCREEN_PLAYING) {
                if (wParam == VK_LEFT  || wParam == 'A') move_single_bee(&g_win, -1, 0);
                if (wParam == VK_RIGHT || wParam == 'D') move_single_bee(&g_win,  1, 0);
                if (wParam == VK_UP    || wParam == 'W') move_single_bee(&g_win, 0, -1);
                if (wParam == VK_DOWN  || wParam == 'S') move_single_bee(&g_win, 0,  1);
                if (wParam == 'P' || wParam == VK_ESCAPE) {
                    g_win.prev_screen = SCREEN_PLAYING;
                    g_win.pause_idx = 0;
                    g_win.screen = SCREEN_PAUSED;
                }
                if (g_win.game_over && wParam == 'R') start_single_game(&g_win);
                if (g_win.game_over && wParam == 'M') { g_win.menu_idx = 0; g_win.screen = SCREEN_MAIN_MENU; }
            } else if (g_win.screen == SCREEN_BEERACE) {
                if (wParam == VK_LEFT)  move_racer(&g_win, &g_win.p1, -1, 0, SCR_W / 2, SCR_H);
                if (wParam == VK_RIGHT) move_racer(&g_win, &g_win.p1,  1, 0, SCR_W / 2, SCR_H);
                if (wParam == VK_UP)    move_racer(&g_win, &g_win.p1, 0, -1, SCR_W / 2, SCR_H);
                if (wParam == VK_DOWN)  move_racer(&g_win, &g_win.p1, 0,  1, SCR_W / 2, SCR_H);

                if (wParam == 'J') move_racer(&g_win, &g_win.p2, -1, 0, SCR_W / 2, SCR_H);
                if (wParam == 'L') move_racer(&g_win, &g_win.p2,  1, 0, SCR_W / 2, SCR_H);
                if (wParam == 'I') move_racer(&g_win, &g_win.p2, 0, -1, SCR_W / 2, SCR_H);
                if (wParam == 'K') move_racer(&g_win, &g_win.p2, 0,  1, SCR_W / 2, SCR_H);

                if (wParam == 'P' || wParam == VK_ESCAPE) {
                    g_win.prev_screen = SCREEN_BEERACE;
                    g_win.pause_idx = 0;
                    g_win.screen = SCREEN_PAUSED;
                }
                if (g_win.race_over && wParam == 'R') start_beerace(&g_win);
                if (g_win.race_over && wParam == 'M') { g_win.menu_idx = 0; g_win.screen = SCREEN_MAIN_MENU; }
            } else if (g_win.screen == SCREEN_PAUSED) {
                if (wParam == VK_UP)   g_win.pause_idx = (g_win.pause_idx + 2) % 3;
                if (wParam == VK_DOWN) g_win.pause_idx = (g_win.pause_idx + 1) % 3;
                if (wParam == VK_RETURN) activate_pause_item(&g_win);
                if (wParam == '1') g_win.screen = g_win.prev_screen;
                if (wParam == '2') { g_win.settings_idx = 0; g_win.screen = SCREEN_SETTINGS; }
                if (wParam == '3') { g_win.menu_idx = 0; g_win.screen = SCREEN_MAIN_MENU; }
                if (wParam == 'P' || wParam == VK_ESCAPE) g_win.screen = g_win.prev_screen;
            } else if (g_win.screen == SCREEN_SETTINGS) {
                if (wParam == VK_UP)   g_win.settings_idx = (g_win.settings_idx + 4) % 5;
                if (wParam == VK_DOWN) g_win.settings_idx = (g_win.settings_idx + 1) % 5;
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
            if (g_win.backend == BACKEND_GDI && g_win.gdi_available && g_win.mem_dc) {
                int cw = rc.right - rc.left;
                int ch = rc.bottom - rc.top;
                if (cw > 0 && ch > 0) {
                    StretchBlt(hdc, 0, 0, cw, ch, g_win.mem_dc, 0, 0, SCR_W, SCR_H, SRCCOPY);
                }
            } else {
                memset(g_vga_buf, C_VGA_SKY, sizeof(g_vga_buf));
                render_scene(&g_win, NULL);
                blit_vga_buffer_to_gdi(hdc, &rc);
            }
            EndPaint(hwnd, &ps);
            return 0;

        case WM_DESTROY:
            apply_rumble_and_dualsense(0, 0, 0, 0, 0);
            if (g_win.video_recording) save_video_to_file(&g_win);
            cleanup_gdi_double_buffer(&g_win);
            free_media_buffers(&g_win);
            KillTimer(hwnd, 1);
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
    strcpy(g_win.peer_ip, "192.168.1.16");
    g_win.screen = SCREEN_MAIN_MENU;
    g_win.pause_style = PAUSE_STYLE_FULL;
    g_win.ctrl_speed_mode = CTRL_SPEED_NORMAL;
    read_obfuscated_vault(&g_win);
    detect_backend(&g_win, lpCmd);

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = BGC_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "BGC_BeeraceNetClass";

    if (!RegisterClass(&wc)) return 0;

    hwnd = CreateWindow(
        "BGC_BeeraceNetClass",
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
    apply_rumble_and_dualsense(0, 0, 0, 0, 0);
    cleanup_gdi_double_buffer(&g_win);
    free_media_buffers(&g_win);
    return (int)msg.wParam;
}

#endif
