// Gridline Clock screensaver: a big light-line clock drifting slowly above a
// scrolling perspective floor grid.
// Build: clang++ -O2 -municode -mwindows gridline-clock.cpp -o GridlineClock.scr -lgdiplus
// Args: /s run, /p <hwnd> preview, /c settings
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <string>
#include <vector>

using namespace Gdiplus;

struct Glyph { wchar_t ch; uint8_t w; uint8_t rows[7]; };
static const Glyph kGlyphs[] = {
    {L'0', 5, {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}}, {L'1', 5, {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {L'2', 5, {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}}, {L'3', 5, {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}},
    {L'4', 5, {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}}, {L'5', 5, {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
    {L'6', 5, {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}}, {L'7', 5, {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
    {L'8', 5, {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}}, {L'9', 5, {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
    {L'A', 5, {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}}, {L'B', 5, {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {L'C', 5, {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}}, {L'D', 5, {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {L'E', 5, {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}}, {L'F', 5, {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
    {L'G', 5, {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}}, {L'H', 5, {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {L'I', 3, {0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x07}}, {L'J', 5, {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}},
    {L'L', 5, {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}}, {L'M', 5, {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {L'N', 5, {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}}, {L'O', 5, {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {L'P', 5, {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}}, {L'R', 5, {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {L'S', 5, {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}}, {L'T', 5, {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {L'U', 5, {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}}, {L'V', 5, {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
    {L'W', 5, {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}}, {L'Y', 5, {0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04}},
    {L' ', 3, {0, 0, 0, 0, 0, 0, 0}}, {L':', 1, {0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00}},
};

static const Glyph* Find(wchar_t c) {
    for (auto& g : kGlyphs) if (g.ch == c) return &g;
    return &kGlyphs[32];
}
static int Cols(const std::wstring& s) {
    int c = 0;
    for (size_t i = 0; i < s.size(); i++) c += Find(s[i])->w + (i + 1 < s.size() ? 1 : 0);
    return c;
}
// Draws text as glowing light-line strokes on the 5x7 grid (same style as the widgets).
static void DrawLines(Graphics& g, const std::wstring& s, float x, float y, float pitch, Color on, Color colon) {
    float core = pitch * 0.34f > 1 ? pitch * 0.34f : 1, cx = x;
    for (wchar_t ch : s) {
        const Glyph* gl = Find(ch);
        Color col = ch == L':' ? colon : on;
        auto lit = [&](int r, int c) {
            return r >= 0 && r < 7 && c >= 0 && c < gl->w && ((gl->rows[r] >> (gl->w - 1 - c)) & 1);
        };
        auto P = [&](int r, int c) { return PointF(cx + c * pitch + pitch / 2, y + r * pitch + pitch / 2); };
        GraphicsPath path;
        std::vector<PointF> lone;
        for (int r = 0; r < 7; r++)
            for (int c = 0; c < gl->w; c++) {
                if (!lit(r, c)) continue;
                auto seg = [&](int r2, int c2) { path.StartFigure(); path.AddLine(P(r, c), P(r2, c2)); };
                if (lit(r, c + 1)) seg(r, c + 1);
                if (lit(r + 1, c)) seg(r + 1, c);
                for (int dc = -1; dc <= 1; dc += 2)
                    if (lit(r + 1, c + dc) && !lit(r, c + dc) && !lit(r + 1, c)) seg(r + 1, c + dc);
                bool alone = true;
                for (int dr = -1; dr <= 1; dr++)
                    for (int dc = -1; dc <= 1; dc++)
                        if ((dr || dc) && lit(r + dr, c + dc)) alone = false;
                if (alone) lone.push_back(P(r, c));
            }
        if (col.GetA()) {
            // soft glow: wide faint pass, then a tighter brighter one, then the core line
            const float widths[2] = {4.6f, 2.2f};
            const BYTE alphas[2] = {(BYTE)(col.GetA() / 14), (BYTE)(col.GetA() / 6)};
            for (int i = 0; i < 2; i++) {
                Pen glow(Color(alphas[i], col.GetR(), col.GetG(), col.GetB()), core * widths[i]);
                glow.SetStartCap(LineCapRound); glow.SetEndCap(LineCapRound); glow.SetLineJoin(LineJoinRound);
                g.DrawPath(&glow, &path);
            }
            Pen pen(col, core);
            pen.SetStartCap(LineCapSquare); pen.SetEndCap(LineCapSquare);
            g.DrawPath(&pen, &path);
            SolidBrush halo(Color(col.GetA() / 5, col.GetR(), col.GetG(), col.GetB())), b(col);
            for (auto& q : lone) {
                g.FillRectangle(&halo, q.X - core * 1.8f, q.Y - core * 1.8f, core * 3.6f, core * 3.6f);
                g.FillRectangle(&b, q.X - core * 0.8f, q.Y - core * 0.8f, core * 1.6f, core * 1.6f);
            }
        }
        cx += (gl->w + 1) * pitch;
    }
}

// Perspective floor grid that scrolls towards the viewer, fading into a glowing horizon.
static void DrawFloor(Graphics& g, int W, int H, double t) {
    float hy = H * 0.62f, vx = W / 2.0f;
    // horizon glow
    for (int i = 0; i < 6; i++) {
        Pen p(Color((BYTE)(60 - i * 9), 0, 229, 255), 2.0f + i * 5.0f);
        g.DrawLine(&p, 0.0f, hy, (float)W, hy);
    }
    Pen hl(Color(230, 120, 245, 255), 1.6f);
    g.DrawLine(&hl, 0.0f, hy, (float)W, hy);
    // converging lines
    int n = 28;
    for (int i = -n; i <= n; i++) {
        float bx = vx + i * (W / 9.0f);
        Pen p(Color(90, 0, 200, 230), 1.2f);
        g.DrawLine(&p, vx + i * 3.0f, hy, bx, (float)H);
    }
    // receding cross lines (scroll with time)
    double phase = fmod(t * 0.35, 1.0);
    for (int k = 0; k < 18; k++) {
        double z = k + 1 - phase;                       // depth 0..18
        float yy = hy + (H - hy) * (float)(1.0 / (z * 0.55 + 0.45) - 0.03);
        if (yy <= hy + 1 || yy > H) continue;
        BYTE a = (BYTE)(40 + 150 * ((yy - hy) / (H - hy)));
        Pen p(Color(a, 0, 210, 240), 1.2f);
        g.DrawLine(&p, 0.0f, yy, (float)W, yy);
    }
}

static const wchar_t* kDays[] = {L"SUN", L"MON", L"TUE", L"WED", L"THU", L"FRI", L"SAT"};
static const wchar_t* kMonths[] = {L"JAN", L"FEB", L"MAR", L"APR", L"MAY", L"JUN",
                                   L"JUL", L"AUG", L"SEP", L"OCT", L"NOV", L"DEC"};

static bool g_preview = false;
static POINT g_startMouse{-1, -1};
static float g_px = 0.2f, g_py = 0.3f, g_vx = 0.00045f, g_vy = 0.0003f;

static void DrawScene(Graphics& g, int W, int H) {
    {
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.Clear(Color(255, 1, 4, 6));
        DrawFloor(g, W, H, GetTickCount64() / 1000.0);
        SYSTEMTIME st;
        GetLocalTime(&st);
        wchar_t t[16], dt[32];
        swprintf(t, 16, L"%02d:%02d", st.wHour, st.wMinute);
        swprintf(dt, 32, L"%ls %d %ls", kDays[st.wDayOfWeek], st.wDay, kMonths[st.wMonth - 1]);
        std::wstring ts = t, ds = dt;
        float pitch = H / 30.0f;
        float tw = Cols(ts) * pitch, th = 7 * pitch;
        float dp = pitch * 0.32f, dw = Cols(ds) * dp;
        float blockH = th + pitch * 1.6f + 7 * dp;
        // drift within the sky above the horizon
        float x = g_px * (W - tw), y = g_py * (H * 0.58f - blockH);
        if (y < 0) y = 0;
        Color cyan(255, 141, 244, 255), orange(255, 255, 138, 31), muted(255, 62, 157, 173), none(0, 0, 0, 0);
        bool blink = st.wSecond % 2 == 0;
        DrawLines(g, ts, x, y, pitch, cyan, blink ? orange : none);
        DrawLines(g, ds, x + tw - dw, y + th + pitch * 1.6f, dp, muted, muted);
    }
}

static void Paint(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int W = rc.right, H = rc.bottom;
    if (W <= 0 || H <= 0) return;
    HDC wdc = GetDC(hwnd);
    HDC mdc = CreateCompatibleDC(wdc);
    HBITMAP bmp = CreateCompatibleBitmap(wdc, W, H);
    HGDIOBJ old = SelectObject(mdc, bmp);
    {
        Graphics g(mdc);
        DrawScene(g, W, H);
    }
    BitBlt(wdc, 0, 0, W, H, mdc, 0, 0, SRCCOPY);
    SelectObject(mdc, old);
    DeleteObject(bmp);
    DeleteDC(mdc);
    ReleaseDC(hwnd, wdc);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 33, nullptr);
            if (!g_preview) ShowCursor(FALSE);
            return 0;
        case WM_TIMER:
            g_px += g_vx; g_py += g_vy;
            if (g_px < 0 || g_px > 1) { g_vx = -g_vx; g_px = g_px < 0 ? 0 : 1; }
            if (g_py < 0 || g_py > 1) { g_vy = -g_vy; g_py = g_py < 0 ? 0 : 1; }
            Paint(hwnd);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            Paint(hwnd);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_MOUSEMOVE:
            if (!g_preview) {
                POINT p;
                GetCursorPos(&p);
                if (g_startMouse.x < 0) g_startMouse = p;
                else if (abs(p.x - g_startMouse.x) > 8 || abs(p.y - g_startMouse.y) > 8) PostQuitMessage(0);
            }
            return 0;
        case WM_KEYDOWN: case WM_SYSKEYDOWN: case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
            if (!g_preview) PostQuitMessage(0);
            return 0;
        case WM_ACTIVATEAPP:
            if (!g_preview && !wp) PostQuitMessage(0);
            return 0;
        case WM_SETCURSOR:
            if (!g_preview) { SetCursor(nullptr); return TRUE; }
            break;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (!g_preview) PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR cmd, int) {
    std::wstring a = cmd ? cmd : L"";
    for (auto& c : a) c = towlower(c);
    HWND parent = nullptr;
    // /png <file>: render one 1600x900 frame (used for the README preview)
    if (a.rfind(L"/png ", 0) == 0) {
        std::wstring out = std::wstring(cmd).substr(5);
        GdiplusStartupInput gsi;
        ULONG_PTR token;
        GdiplusStartup(&token, &gsi, nullptr);
        {
            g_px = 0.12f; g_py = 0.25f;
            Bitmap bmp(1600, 900, PixelFormat32bppARGB);
            Graphics g(&bmp);
            DrawScene(g, 1600, 900);
            CLSID png{0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}};
            bmp.Save(out.c_str(), &png, nullptr);
        }
        GdiplusShutdown(token);
        return 0;
    }
    if (a.find(L"/c") != std::wstring::npos || a.find(L"-c") != std::wstring::npos) {
        MessageBoxW(nullptr, L"Gridline Clock has no settings.\nA light-line clock that drifts slowly above a moving grid to protect your screen.",
                    L"Gridline Clock", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    size_t p = a.find(L"/p");
    if (p == std::wstring::npos) p = a.find(L"-p");
    if (p != std::wstring::npos) {
        g_preview = true;
        parent = (HWND)(intptr_t)_wtoi64(a.c_str() + p + 2);
        if (!IsWindow(parent)) return 0;
    } else if (a.find(L"/s") == std::wstring::npos && a.find(L"-s") == std::wstring::npos && !a.empty()) {
        return 0;
    }

    SetProcessDPIAware();
    GdiplusStartupInput gsi;
    ULONG_PTR token;
    GdiplusStartup(&token, &gsi, nullptr);
    srand(GetTickCount());
    g_px = (rand() % 100) / 100.0f;
    g_py = (rand() % 100) / 100.0f;

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.lpszClassName = L"GridlineClockSaver";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassW(&wc);

    HWND hwnd;
    if (g_preview) {
        RECT r;
        GetClientRect(parent, &r);
        hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_CHILD | WS_VISIBLE, 0, 0, r.right, r.bottom, parent,
                               nullptr, inst, nullptr);
    } else {
        int x = GetSystemMetrics(SM_XVIRTUALSCREEN), y = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int w = GetSystemMetrics(SM_CXVIRTUALSCREEN), h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, L"Gridline Clock", WS_POPUP | WS_VISIBLE,
                               x, y, w, h, nullptr, nullptr, inst, nullptr);
        SetForegroundWindow(hwnd);
    }
    if (!hwnd) return 1;
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
        if (g_preview && !IsWindow(parent)) break;
    }
    GdiplusShutdown(token);
    return 0;
}
