// ==WindhawkMod==
// @id              gridline-window-frames
// @name            Gridline Window Frames
// @description     Gridline OS window frames: near-black title bars, pale cyan titles and a thin cyan border (light versions in light mode)
// @version         1.0.0
// @author          Anant777
// @include         *
// @exclude         dwm.exe
// @exclude         csrss.exe
// @exclude         winlogon.exe
// @exclude         LogonUI.exe
// @exclude         lsass.exe
// @exclude         TextInputHost.exe
// @architecture    x86-64
// @compilerOptions -ldwmapi -luser32 -ladvapi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Gridline Window Frames
Gives every app window a Gridline OS-style frame using Windows' own per-window
colour attributes (no drawing over apps):

* **Dark mode:** near-black title bar, pale cyan title text, thin cyan border.
* **Light mode:** pale cyan-white title bar, dark teal title text, thin cyan border.

Apps that draw their own title bar (browsers, Electron apps) keep their title
bar but still get the border. Turn the mod off to restore Windows' frames.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- dark:
  - caption: "020608"
    $name: Title bar
  - text: "D8FBFF"
    $name: Title text
  - border: "0F8FA3"
    $name: Border
  $name: Dark mode colours (RRGGBB)
- light:
  - caption: "F3FBFC"
    $name: Title bar
  - text: "02262E"
    $name: Title text
  - border: "5FC3D3"
    $name: Border
  $name: Light mode colours (RRGGBB)
- captionColors: true
  $name: Colour title bars
  $description: When off, only the border colour is changed
*/
// ==/WindhawkModSettings==

#include <dwmapi.h>
#include <windows.h>

#include <string>

#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#define DWMWA_CAPTION_COLOR 35
#define DWMWA_TEXT_COLOR 36
#endif
static const DWORD kDefault = 0xFFFFFFFF;  // DWMWA_COLOR_DEFAULT

struct Colors {
    COLORREF caption, text, border;
};
static Colors g_dark{RGB(0x02, 0x06, 0x08), RGB(0xD8, 0xFB, 0xFF), RGB(0x0F, 0x8F, 0xA3)};
static Colors g_light{RGB(0xF3, 0xFB, 0xFC), RGB(0x02, 0x26, 0x2E), RGB(0x5F, 0xC3, 0xD3)};
static bool g_captionColors = true;

static COLORREF ParseHex(PCWSTR s, COLORREF fallback) {
    if (!s) return fallback;
    std::wstring h = s;
    if (!h.empty() && h[0] == L'#') h = h.substr(1);
    if (h.size() != 6) return fallback;
    for (wchar_t c : h)
        if (!iswxdigit(c)) return fallback;
    unsigned long v = wcstoul(h.c_str(), nullptr, 16);
    return RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

static COLORREF GetHexSetting(PCWSTR name, COLORREF fallback) {
    PCWSTR v = Wh_GetStringSetting(name);
    COLORREF c = ParseHex(v, fallback);
    Wh_FreeStringSetting(v);
    return c;
}

static void LoadSettings() {
    g_dark = {GetHexSetting(L"dark.caption", RGB(0x02, 0x06, 0x08)), GetHexSetting(L"dark.text", RGB(0xD8, 0xFB, 0xFF)),
              GetHexSetting(L"dark.border", RGB(0x0F, 0x8F, 0xA3))};
    g_light = {GetHexSetting(L"light.caption", RGB(0xF3, 0xFB, 0xFC)), GetHexSetting(L"light.text", RGB(0x02, 0x26, 0x2E)),
               GetHexSetting(L"light.border", RGB(0x5F, 0xC3, 0xD3))};
    g_captionColors = Wh_GetIntSetting(L"captionColors");
}

static bool IsLightTheme() {
    DWORD v = 0, sz = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &v, &sz) == ERROR_SUCCESS)
        return v != 0;
    return false;
}

static bool IsFramedWindow(HWND hwnd) {
    if (!hwnd || GetAncestor(hwnd, GA_ROOT) != hwnd) return false;
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (ex & WS_EX_TOOLWINDOW) return false;
    if (style & WS_CHILD) return false;
    return (style & WS_CAPTION) == WS_CAPTION || (style & WS_THICKFRAME);
}

static void ApplyFrame(HWND hwnd, bool restore = false) {
    if (!IsFramedWindow(hwnd)) return;
    if (restore) {
        DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &kDefault, sizeof(kDefault));
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &kDefault, sizeof(kDefault));
        DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &kDefault, sizeof(kDefault));
        return;
    }
    const Colors& c = IsLightTheme() ? g_light : g_dark;
    DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &c.border, sizeof(c.border));
    if (g_captionColors) {
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &c.caption, sizeof(c.caption));
        DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &c.text, sizeof(c.text));
    } else {
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &kDefault, sizeof(kDefault));
        DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &kDefault, sizeof(kDefault));
    }
}

static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM restore) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId()) ApplyFrame(hwnd, restore != 0);
    return TRUE;
}

static void ApplyToAll(bool restore) { EnumWindows(EnumProc, restore ? 1 : 0); }

using ShowWindow_t = BOOL(WINAPI*)(HWND, int);
static ShowWindow_t ShowWindow_Original;
static BOOL WINAPI ShowWindow_Hook(HWND hwnd, int cmd) {
    if (cmd != SW_HIDE) ApplyFrame(hwnd);
    return ShowWindow_Original(hwnd, cmd);
}

BOOL Wh_ModInit() {
    LoadSettings();
    Wh_SetFunctionHook((void*)ShowWindow, (void*)ShowWindow_Hook, (void**)&ShowWindow_Original);
    return TRUE;
}

void Wh_ModAfterInit() { ApplyToAll(false); }

void Wh_ModSettingsChanged() {
    LoadSettings();
    ApplyToAll(false);
}

void Wh_ModUninit() { ApplyToAll(true); }
