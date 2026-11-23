// ==WindhawkMod==
// @id              gridline-desktop
// @name            Gridline Desktop
// @description     Neon light-line widgets for the Windows desktop: clock, analog, calendar, battery, weather, forecast, system, focus, stopwatch, now playing, year grid, quote and a Pulse notification light
// @version         1.3.0
// @author          Anant777
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lgdiplus -lgdi32 -luser32 -lwinhttp -lole32 -loleaut32 -ladvapi32 -lruntimeobject -lwtsapi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Gridline Desktop

Brings the Gridline OS look to the Windows desktop: near-black cards with thin
cyan frames and corner brackets, glowing cyan **light-line** text and an orange
accent. Numbers and icons are drawn as straight strokes on a 5x7 grid, like
circuit traces (no font install needed for the widgets).

## Widgets
| Widget   | Size  | What it shows |
|----------|-------|---------------|
| Clock    | wide  | Big light-line time, orange colon, date |
| Analog   | small | Grid-tick dial, glowing hands, orange seconds marker orbiting |
| Calendar | small | Weekday, big light-line date, month |
| Battery  | small | Ring of cells that fills with charge, turns orange when low |
| Weather  | small | Line icon + temperature (Open-Meteo, no API key) |
| System   | wide  | CPU / RAM / disk as cell meters |
| Focus    | small | 25-minute focus timer — click to start/pause, right-click to reset |
| Year     | small | Every day of the year as a grid cell, today in orange |
| Now Playing | wide | Song, artist and progress from any media app; tap ◀ ▶ ⏯ |
| Forecast | wide  | The next 6 hours as line icons and temperatures |
| Stopwatch | small | Click to start/stop, right-click to reset |
| Pulse light | — | A light-line bracket pulses in a screen corner when a notification pops up |
| Quote    | wide  | A new everyday-life quote each day (click for the next), or your own text |

## Using it
* **Move:** drag a widget (when *Lock widgets* is off). Positions are saved.
* **Right-click** any widget for *Reset layout* and *Refresh weather*.
* **Weather:** set latitude/longitude in the settings (e.g. `28.61` /
  `77.21`). Only those coordinates are sent, to `api.open-meteo.com`.

Pair it with the included Taskbar / Start / Notification Center styler themes
and the Orbitron + Share Tech Mono fonts for the full Gridline OS look.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- theme: dark
  $name: Theme
  $options:
  - dark: Dark (near-black cards, cyan lines)
  - light: Light (pale cards, dark cyan lines)
  - auto: Follow Windows app theme
- accentColor: "#FF8A1F"
  $name: Accent color
  $description: Gridline orange is #FF8A1F (cyan is #00E5FF)
- cardStyle: solid
  $name: Card style
  $options:
  - solid: Solid card
  - outline: Outline only
  - none: No card (lines float on the wallpaper)
- cardOpacity: 100
  $name: Card opacity (%)
- scale: 100
  $name: Scale (%)
  $description: Multiplied with your display scaling
- locked: false
  $name: Lock widgets
  $description: When off, drag widgets to rearrange them
- unlitDots: false
  $name: Show grid
  $description: Draw faint grid marks behind the light-line text
- labelFont: Share Tech Mono
  $name: Label font
  $description: Font for small labels. Falls back to Consolas if not installed
- clock:
  - enabled: true
    $name: Show clock
  - use24h: true
    $name: 24-hour time
  - showSeconds: false
    $name: Show seconds
  - redColon: true
    $name: Accent-coloured colon
  $name: Digital clock
- analog:
  - enabled: true
    $name: Show analog clock
  $name: Analog clock
- calendar:
  - enabled: true
    $name: Show calendar
  $name: Calendar
- battery:
  - enabled: true
    $name: Show battery
  $name: Battery
- weather:
  - enabled: true
    $name: Show weather
  - latitude: ""
    $name: Latitude
    $description: e.g. 28.61
  - longitude: ""
    $name: Longitude
    $description: e.g. 77.21
  - city: ""
    $name: Place label
    $description: Short name shown on the widget, e.g. DELHI
  - fahrenheit: false
    $name: Use Fahrenheit
  - intervalMinutes: 20
    $name: Refresh interval (minutes)
  $name: Weather
- system:
  - enabled: true
    $name: Show system monitor
  $name: System monitor
- focus:
  - enabled: true
    $name: Show focus timer
  - minutes: 25
    $name: Focus length (minutes)
  $name: Focus timer
- year:
  - enabled: true
    $name: Show year progress
  $name: Year progress
- quote:
  - enabled: true
    $name: Show quote
  - mode: daily
    $name: Quote source
    $options:
    - daily: Daily quotes (changes every day, click for the next one)
    - custom: My own text
  - text: ""
    $name: Custom text
    $description: Used when Quote source is "My own text"
  - author: ""
    $name: Caption
    $description: Optional small line under the quote. Leave empty for none
  $name: Quote
- nowplaying:
  - enabled: true
    $name: Show Now Playing
  $name: Now Playing
- forecast:
  - enabled: true
    $name: Show hourly forecast
    $description: Uses the weather location above
  $name: Hourly forecast
- stopwatch:
  - enabled: true
    $name: Show stopwatch
  $name: Stopwatch
- glyph:
  - enabled: true
    $name: Pulse notification light
    $description: Pulses a light-line bracket in a screen corner when a notification pop-up appears
  - color: cyan
    $name: Light color
    $options:
    - cyan: Cyan
    - accent: Accent color
  - corner: topRight
    $name: Corner
    $options:
    - topRight: Top right
    - topLeft: Top left
    - bottomRight: Bottom right
    - bottomLeft: Bottom left
  $name: Pulse light
- keepCursors: true
  $name: Keep Gridline cursors and sounds
  $description: >-
    Windows' theme service puts its own cursors and sound scheme back after you sign in,
    unlock or switch light/dark. When on, this reapplies yours right after those moments.
- soundScheme: .GridlineSound
  $name: Sound scheme to keep
  $description: >-
    Registry name of the sound scheme to put back (install-sounds.ps1 creates
    ".GridlineSound"). Only used while that scheme is installed. Leave empty to keep sounds as
    Windows sets them.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <objidl.h>
#include <winhttp.h>
#include <wtsapi32.h>

#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

#include <gdiplus.h>

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cwchar>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

using namespace Gdiplus;

#ifndef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4
#endif

static constexpr double kPi = 3.14159265358979323846;

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

enum class ThemeMode { Dark, Light, Auto };
enum class CardStyle { Solid, Outline, None };

struct Settings {
    ThemeMode theme = ThemeMode::Dark;
    ARGB accent = 0xFFFF8A1F;
    CardStyle cardStyle = CardStyle::Solid;
    int cardOpacity = 100;
    int scale = 100;
    bool locked = false;
    bool unlitDots = false;
    std::wstring labelFont = L"Share Tech Mono";

    bool clock = true, use24h = true, showSeconds = false, redColon = true;
    bool analog = true, calendar = true, battery = true;
    bool weather = true;
    std::wstring lat, lon, city;
    bool fahrenheit = false;
    int weatherInterval = 20;
    bool system = true;
    bool focus = true;
    int focusMinutes = 25;
    bool year = true;
    bool quote = true;
    bool quoteDaily = true;
    std::wstring quoteText, quoteAuthor;
    bool nowPlaying = true, forecast = true, stopwatch = true;
    bool glyph = true, glyphCyan = true;
    std::wstring glyphCorner = L"topRight";
    bool keepCursors = true;
    std::wstring soundScheme = L".GridlineSound";
};

static Settings g_s;
static std::mutex g_settingsMutex;  // guards lat/lon/fahrenheit for the weather thread

static std::wstring GetStr(PCWSTR name) {
    PCWSTR v = Wh_GetStringSetting(name);
    std::wstring r = v ? v : L"";
    Wh_FreeStringSetting(v);
    return r;
}

static ARGB ParseColor(const std::wstring& s, ARGB fallback) {
    std::wstring h = s;
    if (!h.empty() && h[0] == L'#') h = h.substr(1);
    if (h.size() != 6 && h.size() != 8) return fallback;
    for (wchar_t c : h)
        if (!iswxdigit(c)) return fallback;
    unsigned long v = wcstoul(h.c_str(), nullptr, 16);
    if (h.size() == 6) return 0xFF000000u | (ARGB)v;
    return (ARGB)v;  // AARRGGBB
}

static bool IsCoordinate(const std::wstring& s) {
    if (s.empty() || s.size() > 12) return false;
    for (wchar_t c : s)
        if (!(iswdigit(c) || c == L'.' || c == L'-')) return false;
    return true;
}

static void LoadSettings() {
    Settings s;
    std::wstring t = GetStr(L"theme");
    s.theme = t == L"light" ? ThemeMode::Light
            : t == L"auto"  ? ThemeMode::Auto
                            : ThemeMode::Dark;
    s.accent = ParseColor(GetStr(L"accentColor"), 0xFFFF8A1F);
    std::wstring cs = GetStr(L"cardStyle");
    s.cardStyle = cs == L"outline" ? CardStyle::Outline
                : cs == L"none"    ? CardStyle::None
                                   : CardStyle::Solid;
    s.cardOpacity = std::clamp(Wh_GetIntSetting(L"cardOpacity"), 0, 100);
    s.scale = std::clamp(Wh_GetIntSetting(L"scale"), 50, 300);
    s.locked = Wh_GetIntSetting(L"locked");
    s.unlitDots = Wh_GetIntSetting(L"unlitDots");
    s.labelFont = GetStr(L"labelFont");

    s.clock = Wh_GetIntSetting(L"clock.enabled");
    s.use24h = Wh_GetIntSetting(L"clock.use24h");
    s.showSeconds = Wh_GetIntSetting(L"clock.showSeconds");
    s.redColon = Wh_GetIntSetting(L"clock.redColon");
    s.analog = Wh_GetIntSetting(L"analog.enabled");
    s.calendar = Wh_GetIntSetting(L"calendar.enabled");
    s.battery = Wh_GetIntSetting(L"battery.enabled");
    s.weather = Wh_GetIntSetting(L"weather.enabled");
    s.lat = GetStr(L"weather.latitude");
    s.lon = GetStr(L"weather.longitude");
    s.city = GetStr(L"weather.city");
    s.fahrenheit = Wh_GetIntSetting(L"weather.fahrenheit");
    s.weatherInterval = std::clamp(Wh_GetIntSetting(L"weather.intervalMinutes"), 5, 240);
    s.system = Wh_GetIntSetting(L"system.enabled");
    s.focus = Wh_GetIntSetting(L"focus.enabled");
    s.focusMinutes = std::clamp(Wh_GetIntSetting(L"focus.minutes"), 1, 180);
    s.year = Wh_GetIntSetting(L"year.enabled");
    s.quote = Wh_GetIntSetting(L"quote.enabled");
    s.quoteDaily = GetStr(L"quote.mode") != L"custom";
    s.quoteText = GetStr(L"quote.text");
    s.quoteAuthor = GetStr(L"quote.author");
    s.nowPlaying = Wh_GetIntSetting(L"nowplaying.enabled");
    s.forecast = Wh_GetIntSetting(L"forecast.enabled");
    s.stopwatch = Wh_GetIntSetting(L"stopwatch.enabled");
    s.glyph = Wh_GetIntSetting(L"glyph.enabled");
    s.glyphCyan = GetStr(L"glyph.color") != L"accent";
    s.glyphCorner = GetStr(L"glyph.corner");
    s.keepCursors = Wh_GetIntSetting(L"keepCursors");
    s.soundScheme = GetStr(L"soundScheme");

    std::lock_guard<std::mutex> lock(g_settingsMutex);
    g_s = s;
}

// ---------------------------------------------------------------------------
// 5x7 grid font (drawn as light-line strokes)
// ---------------------------------------------------------------------------

struct DotGlyph {
    wchar_t ch;
    uint8_t width;    // columns
    uint8_t rows[7];  // bit (width-1-col) set = lit
};

static const DotGlyph kGlyphs[] = {
    {L'0', 5, {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {L'1', 5, {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {L'2', 5, {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
    {L'3', 5, {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}},
    {L'4', 5, {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
    {L'5', 5, {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
    {L'6', 5, {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
    {L'7', 5, {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
    {L'8', 5, {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
    {L'9', 5, {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
    {L'A', 5, {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {L'B', 5, {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {L'C', 5, {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {L'D', 5, {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {L'E', 5, {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {L'F', 5, {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
    {L'G', 5, {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}},
    {L'H', 5, {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {L'I', 3, {0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x07}},
    {L'J', 5, {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}},
    {L'K', 5, {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
    {L'L', 5, {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {L'M', 5, {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {L'N', 5, {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}},
    {L'O', 5, {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {L'P', 5, {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
    {L'Q', 5, {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
    {L'R', 5, {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {L'S', 5, {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {L'T', 5, {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {L'U', 5, {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {L'V', 5, {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
    {L'W', 5, {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}},
    {L'X', 5, {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
    {L'Y', 5, {0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04}},
    {L'Z', 5, {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}},
    {L' ', 3, {0, 0, 0, 0, 0, 0, 0}},
    {L':', 1, {0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00}},
    {L'.', 1, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}},
    {L',', 2, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02}},
    {L'\'', 1, {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {L'!', 1, {0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01}},
    {L'?', 5, {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}},
    {L'-', 3, {0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00}},
    {L'+', 5, {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}},
    {L'/', 5, {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}},
    {L'%', 5, {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}},
    {L'#', 5, {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A}},
    {L'(', 3, {0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01}},
    {L')', 3, {0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x04}},
    {L'&', 5, {0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D}},
    {0x00B0, 3, {0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00}},  // degree
};

static const DotGlyph* FindGlyph(wchar_t c) {
    c = towupper(c);
    for (const auto& g : kGlyphs)
        if (g.ch == c) return &g;
    for (const auto& g : kGlyphs)
        if (g.ch == L'?') return &g;
    return nullptr;
}

// Width in dot columns (1 empty column between glyphs).
static int DotTextCols(const std::wstring& text) {
    int cols = 0;
    for (size_t i = 0; i < text.size(); i++) {
        cols += FindGlyph(text[i])->width;
        if (i + 1 < text.size()) cols += 1;
    }
    return cols;
}

static REAL DotTextWidth(const std::wstring& text, REAL pitch) {
    return DotTextCols(text) * pitch;
}

// ---------------------------------------------------------------------------
// Gridline drawing: glowing grid cells and light-line strokes
// ---------------------------------------------------------------------------

static bool BrushColor(Brush* b, Color* out) {
    if (!b || b->GetType() != BrushTypeSolidColor) return false;
    static_cast<SolidBrush*>(b)->GetColor(out);
    return true;
}

static Color Faded(const Color& c, float k) {
    return Color((BYTE)(c.GetA() * k), c.GetR(), c.GetG(), c.GetB());
}

// A square grid cell with a soft glow (used wherever a single point is drawn).
static void FillCell(Graphics& g, Brush* b, REAL x, REAL y, REAL w, REAL h) {
    if (!b) return;
    REAL side = (std::min)(w, h) * 0.86f;
    REAL cx = x + w / 2, cy = y + h / 2;
    Color col;
    if (BrushColor(b, &col) && col.GetA() > 120) {
        REAL gs = side * 1.8f;
        SolidBrush glow(Faded(col, 0.18f));
        g.FillRectangle(&glow, cx - gs / 2, cy - gs / 2, gs, gs);
    }
    g.FillRectangle(b, cx - side / 2, cy - side / 2, side, side);
}

// Joins the lit cells of a small bitmap into straight glowing strokes, like
// circuit traces: right and down neighbours, plus diagonals where no right
// angle joins them. Lone cells get a small square.
template <class On>
static void DrawStrokes(Graphics& g, On on, int rows, int cols, REAL x, REAL y, REAL pitch, Brush* lit) {
    Color col(255, 255, 255, 255);
    BrushColor(lit, &col);
    REAL core = (std::max)(1.0f, pitch * 0.36f);
    Pen glow(Faded(col, 0.22f), core * 2.8f);
    glow.SetStartCap(LineCapRound);
    glow.SetEndCap(LineCapRound);
    Pen pen(col, core);
    pen.SetStartCap(LineCapSquare);
    pen.SetEndCap(LineCapSquare);
    auto P = [&](int r, int c) { return PointF(x + c * pitch + pitch / 2, y + r * pitch + pitch / 2); };
    auto at = [&](int r, int c) { return r >= 0 && r < rows && c >= 0 && c < cols && on(r, c); };
    // One path for all strokes, so overlapping segments don't double the glow.
    GraphicsPath path;
    std::vector<PointF> lone;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            if (!on(r, c)) continue;
            auto seg = [&](int r2, int c2) {
                path.StartFigure();
                path.AddLine(P(r, c), P(r2, c2));
            };
            if (at(r, c + 1)) seg(r, c + 1);
            if (at(r + 1, c)) seg(r + 1, c);
            for (int dc = -1; dc <= 1; dc += 2)
                if (at(r + 1, c + dc) && !at(r, c + dc) && !at(r + 1, c)) seg(r + 1, c + dc);
            bool alone = true;
            for (int dr = -1; dr <= 1 && alone; dr++)
                for (int dc = -1; dc <= 1; dc++)
                    if ((dr || dc) && at(r + dr, c + dc)) { alone = false; break; }
            if (alone) lone.push_back(P(r, c));
        }
    g.DrawPath(&glow, &path);
    g.DrawPath(&pen, &path);
    for (int pass = 0; pass < 2; pass++) {
        REAL s = pass ? core * 1.5f : core * 3.2f;
        SolidBrush bb(pass ? col : Faded(col, 0.22f));
        for (auto& q : lone) g.FillRectangle(&bb, q.X - s / 2, q.Y - s / 2, s, s);
    }
}

// Faint grid marks behind strokes (the "Show grid" option).
template <class On>
static void DrawGridMarks(Graphics& g, On on, int rows, int cols, REAL x, REAL y, REAL pitch, Brush* b) {
    if (!b) return;
    REAL s = pitch * 0.22f;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (!on(r, c)) g.FillRectangle(b, x + c * pitch + (pitch - s) / 2, y + r * pitch + (pitch - s) / 2, s, s);
}

// Draws text as light-line strokes on the 5x7 grid. 'colonBrush' (optional) recolors ':' glyphs.
static void DrawDotText(Graphics& g, const std::wstring& text, REAL x, REAL y,
                        REAL pitch, Brush* lit, Brush* unlit = nullptr,
                        Brush* colonBrush = nullptr) {
    REAL cx = x;
    for (wchar_t ch : text) {
        const DotGlyph* gl = FindGlyph(ch);
        Brush* b = (colonBrush && ch == L':') ? colonBrush : lit;
        auto on = [gl](int r, int c) { return ((gl->rows[r] >> (gl->width - 1 - c)) & 1) != 0; };
        if (unlit && ch != L' ') DrawGridMarks(g, on, 7, gl->width, cx, y, pitch, unlit);
        DrawStrokes(g, on, 7, gl->width, cx, y, pitch, b);
        cx += (gl->width + 1) * pitch;
    }
}

// 9x9 dot icons for weather (bit 8 = left column).
static const uint16_t kIconSun[9] = {0x010, 0x082, 0x038, 0x07C, 0x17D, 0x07C, 0x038, 0x082, 0x010};
static const uint16_t kIconMoon[9] = {0x070, 0x0C0, 0x180, 0x180, 0x180, 0x181, 0x0C3, 0x07E, 0x03C};
static const uint16_t kIconCloud[9] = {0x000, 0x000, 0x038, 0x07C, 0x0FE, 0x1FF, 0x1FF, 0x0FE, 0x000};
static const uint16_t kIconRain[9] = {0x038, 0x07C, 0x0FE, 0x1FF, 0x0FE, 0x000, 0x092, 0x000, 0x124};
static const uint16_t kIconSnow[9] = {0x038, 0x07C, 0x0FE, 0x1FF, 0x0FE, 0x000, 0x0AA, 0x000, 0x155};
static const uint16_t kIconStorm[9] = {0x038, 0x07C, 0x0FE, 0x1FF, 0x0FE, 0x010, 0x030, 0x018, 0x010};
static const uint16_t kIconFog[9] = {0x000, 0x0FE, 0x000, 0x1FC, 0x000, 0x07F, 0x000, 0x0FE, 0x000};
static const uint16_t kIconNone[9] = {0x000, 0x000, 0x000, 0x000, 0x092, 0x000, 0x000, 0x000, 0x000};

static void DrawDotIcon(Graphics& g, const uint16_t* icon, REAL x, REAL y, REAL pitch,
                        Brush* lit, Brush* unlit) {
    auto on = [icon](int r, int c) { return ((icon[r] >> (8 - c)) & 1) != 0; };
    DrawGridMarks(g, on, 9, 9, x, y, pitch, unlit);
    DrawStrokes(g, on, 9, 9, x, y, pitch, lit);
}

// ---------------------------------------------------------------------------
// Drawing context
// ---------------------------------------------------------------------------

struct Ctx {
    REAL s;  // pixel scale
    Color bg, fg, fg2, dim, accent, line;
    bool unlit;
    FontFamily* family;
};

static bool IsSystemLight() {
    DWORD v = 0, sz = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &v, &sz) == ERROR_SUCCESS)
        return v != 0;
    return false;
}

static void DrawLabel(Graphics& g, const Ctx& c, const std::wstring& text, REAL x, REAL y,
                      REAL w, REAL size, const Color& color,
                      StringAlignment align = StringAlignmentNear, bool bold = false) {
    Font font(c.family, size * c.s, bold ? FontStyleBold : FontStyleRegular, UnitPixel);
    SolidBrush b(color);
    StringFormat fmt;
    fmt.SetAlignment(align);
    fmt.SetFormatFlags(StringFormatFlagsNoWrap);
    fmt.SetTrimming(StringTrimmingNone);
    RectF rc(x, y, w, size * c.s * 1.6f);
    g.DrawString(text.c_str(), -1, &font, rc, &fmt, &b);
}

static void RoundRectPath(GraphicsPath& p, REAL x, REAL y, REAL w, REAL h, REAL r) {
    REAL d = r * 2;
    p.AddArc(x, y, d, d, 180, 90);
    p.AddArc(x + w - d, y, d, d, 270, 90);
    p.AddArc(x + w - d, y + h - d, d, d, 0, 90);
    p.AddArc(x, y + h - d, d, d, 90, 90);
    p.CloseFigure();
}

// Ring of dots; 'lit' first dots highlighted. Starts at 12 o'clock.
static void DrawDotRing(Graphics& g, REAL cx, REAL cy, REAL radius, int count, int lit,
                        REAL dot, Brush* on, Brush* off, Brush* head = nullptr) {
    for (int i = 0; i < count; i++) {
        double a = (double)i / count * 2 * kPi - kPi / 2;
        REAL x = cx + (REAL)(radius * cos(a)), y = cy + (REAL)(radius * sin(a));
        Brush* b = i < lit ? on : off;
        if (head && i == lit - 1) b = head;
        if (b) FillCell(g, b, x - dot / 2, y - dot / 2, dot, dot);
    }
}

// ---------------------------------------------------------------------------
// Weather (background thread)
// ---------------------------------------------------------------------------

struct WeatherData {
    bool valid = false;
    double temp = 0, hi = 0, lo = 0;
    int code = -1;
    bool isDay = true;
    std::wstring error;
    int hours = 0;          // hourly forecast entries
    int hHour[6] = {};
    double hTemp[6] = {};
    int hCode[6] = {};
};

static std::mutex g_weatherMutex;
static WeatherData g_weather;
static HANDLE g_weatherThread = nullptr;
static HANDLE g_weatherWake = nullptr;
static HANDLE g_stopEvent = nullptr;
static std::atomic<HWND> g_ctrl{nullptr};

#define WM_APP_WEATHER (WM_APP + 1)
#define WM_APP_MEDIA (WM_APP + 2)

// Reads "key":[a,b,c] numbers into out (up to max).
static int ParseNumArray(const std::string& s, size_t from, const char* key, double* out, int max) {
    size_t p = s.find(key, from);
    if (p == std::string::npos) return 0;
    p = s.find('[', p);
    if (p == std::string::npos) return 0;
    int n = 0;
    const char* c = s.c_str() + p + 1;
    while (n < max && *c && *c != ']') {
        char* end = nullptr;
        double v = strtod(c, &end);
        if (end == c) break;
        out[n++] = v;
        c = end;
        while (*c == ',' || *c == ' ') c++;
    }
    return n;
}

// Reads hours from "time":["2026-11-22T17:00",...].
static int ParseHours(const std::string& s, size_t from, int* out, int max) {
    size_t p = s.find("\"time\":[", from);
    if (p == std::string::npos) return 0;
    int n = 0;
    size_t end = s.find(']', p);
    while (n < max) {
        size_t t = s.find('T', p + 8);
        if (t == std::string::npos || t > end) break;
        out[n++] = atoi(s.c_str() + t + 1);
        p = t + 1;
    }
    return n;
}

static bool FindNumber(const std::string& s, size_t from, const char* key, double* out) {
    size_t p = s.find(key, from);
    if (p == std::string::npos) return false;
    p += strlen(key);
    while (p < s.size() && (s[p] == ' ' || s[p] == '[')) p++;
    char* end = nullptr;
    double v = strtod(s.c_str() + p, &end);
    if (end == s.c_str() + p) return false;
    *out = v;
    return true;
}

static void FetchWeather() {
    std::wstring lat, lon;
    bool f;
    {
        std::lock_guard<std::mutex> lock(g_settingsMutex);
        lat = g_s.lat;
        lon = g_s.lon;
        f = g_s.fahrenheit;
    }
    WeatherData wd;
    if (!IsCoordinate(lat) || !IsCoordinate(lon)) {
        wd.error = L"SET LOCATION";
    } else {
        std::wstring path = L"/v1/forecast?latitude=" + lat + L"&longitude=" + lon +
                            L"&current=temperature_2m,weather_code,is_day"
                            L"&daily=temperature_2m_max,temperature_2m_min"
                            L"&hourly=temperature_2m,weather_code&forecast_hours=7"
                            L"&timezone=auto&forecast_days=2";
        if (f) path += L"&temperature_unit=fahrenheit";

        std::string body;
        HINTERNET ses = WinHttpOpen(L"GridlineDesktop/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (ses) {
            WinHttpSetTimeouts(ses, 4000, 4000, 4000, 6000);
            HINTERNET con = WinHttpConnect(ses, L"api.open-meteo.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
            if (con) {
                HINTERNET req = WinHttpOpenRequest(con, L"GET", path.c_str(), nullptr,
                                                   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                   WINHTTP_FLAG_SECURE);
                if (req) {
                    if (WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                           WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                        WinHttpReceiveResponse(req, nullptr)) {
                        DWORD avail = 0;
                        while (WinHttpQueryDataAvailable(req, &avail) && avail &&
                               body.size() < 256 * 1024) {
                            std::string chunk(avail, '\0');
                            DWORD read = 0;
                            if (!WinHttpReadData(req, chunk.data(), avail, &read) || !read) break;
                            body.append(chunk.data(), read);
                        }
                    }
                    WinHttpCloseHandle(req);
                }
                WinHttpCloseHandle(con);
            }
            WinHttpCloseHandle(ses);
        }

        size_t cur = body.find("\"current\":{");
        size_t daily = body.find("\"daily\":{");
        double t, code, day, hi, lo;
        if (cur != std::string::npos && FindNumber(body, cur, "\"temperature_2m\":", &t) &&
            FindNumber(body, cur, "\"weather_code\":", &code)) {
            wd.valid = true;
            wd.temp = t;
            wd.code = (int)code;
            wd.isDay = !FindNumber(body, cur, "\"is_day\":", &day) || day != 0;
            if (daily != std::string::npos &&
                FindNumber(body, daily, "\"temperature_2m_max\":", &hi) &&
                FindNumber(body, daily, "\"temperature_2m_min\":", &lo)) {
                wd.hi = hi;
                wd.lo = lo;
            } else {
                wd.hi = wd.lo = t;
            }
            size_t hourly = body.find("\"hourly\":{");
            if (hourly != std::string::npos) {
                int hh[7]; double ht[7], hc[7];
                int n1 = ParseHours(body, hourly, hh, 7);
                int n2 = ParseNumArray(body, hourly, "\"temperature_2m\":", ht, 7);
                int n3 = ParseNumArray(body, hourly, "\"weather_code\":", hc, 7);
                int n = (std::min)({n1, n2, n3});
                // skip the current hour; show the next six
                int start = n > 6 ? 1 : 0;
                for (int i = start; i < n && wd.hours < 6; i++) {
                    wd.hHour[wd.hours] = hh[i];
                    wd.hTemp[wd.hours] = ht[i];
                    wd.hCode[wd.hours] = (int)hc[i];
                    wd.hours++;
                }
            }
        } else {
            wd.error = L"OFFLINE";
            Wh_Log(L"Weather fetch failed (%zu bytes)", body.size());
        }
    }
    {
        std::lock_guard<std::mutex> lock(g_weatherMutex);
        if (wd.valid || !g_weather.valid || wd.error == L"SET LOCATION") g_weather = wd;
    }
    if (HWND h = g_ctrl.load()) PostMessage(h, WM_APP_WEATHER, 0, 0);
}

static DWORD WINAPI WeatherThreadProc(LPVOID) {
    for (;;) {
        FetchWeather();
        int minutes;
        {
            std::lock_guard<std::mutex> lock(g_settingsMutex);
            minutes = g_s.weatherInterval;
        }
        HANDLE hs[2] = {g_stopEvent, g_weatherWake};
        DWORD r = WaitForMultipleObjects(2, hs, FALSE, minutes * 60 * 1000);
        if (r == WAIT_OBJECT_0) break;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Now Playing (Windows media sessions, background thread)
// ---------------------------------------------------------------------------

struct MediaData {
    bool has = false, playing = false;
    std::wstring title, artist, app;
    double pos = 0, dur = 0;
    ULONGLONG stamp = 0;
};

static std::mutex g_mediaMutex;
static MediaData g_media;
static HANDLE g_mediaThread = nullptr;
static HANDLE g_mediaWake = nullptr;
static std::atomic<int> g_mediaCmd{0};  // 1 toggle, 2 next, 3 previous

static std::wstring AppNameFromAumid(const std::wstring& id) {
    std::wstring n = id;
    size_t bang = n.find(L'!');
    if (bang != std::wstring::npos) n = n.substr(bang + 1);
    size_t slash = n.find_last_of(L"\\/");
    if (slash != std::wstring::npos) n = n.substr(slash + 1);
    size_t dot = n.find(L".exe");
    if (dot == std::wstring::npos) dot = n.find(L".EXE");
    if (dot != std::wstring::npos) n = n.substr(0, dot);
    size_t under = n.find(L'_');
    if (under != std::wstring::npos && under > 0) n = n.substr(0, under);
    size_t pd = n.find_last_of(L'.');
    if (pd != std::wstring::npos && pd + 1 < n.size()) n = n.substr(pd + 1);
    for (auto& c : n) c = towupper(c);
    if (n.size() > 14) n.resize(14);
    return n;
}

static DWORD WINAPI MediaThreadProc(LPVOID) {
    using namespace winrt::Windows::Media::Control;
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    } catch (...) {
    }
    GlobalSystemMediaTransportControlsSessionManager mgr{nullptr};
    MediaData last;
    for (;;) {
        MediaData md;
        try {
            if (!mgr) mgr = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
            auto session = mgr.GetCurrentSession();
            if (session) {
                int cmd = g_mediaCmd.exchange(0);
                if (cmd == 1) session.TryTogglePlayPauseAsync().get();
                else if (cmd == 2) session.TrySkipNextAsync().get();
                else if (cmd == 3) session.TrySkipPreviousAsync().get();
                auto props = session.TryGetMediaPropertiesAsync().get();
                md.has = true;
                md.title = props.Title().c_str();
                md.artist = props.Artist().c_str();
                md.app = AppNameFromAumid(session.SourceAppUserModelId().c_str());
                md.playing = session.GetPlaybackInfo().PlaybackStatus() ==
                             GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
                auto tl = session.GetTimelineProperties();
                md.dur = std::chrono::duration<double>(tl.EndTime() - tl.StartTime()).count();
                md.pos = std::chrono::duration<double>(tl.Position() - tl.StartTime()).count();
                if (md.playing) {
                    double since = std::chrono::duration<double>(winrt::clock::now() - tl.LastUpdatedTime()).count();
                    if (since > 0 && since < 3600) md.pos += since;
                }
                if (md.dur > 0) md.pos = (std::min)(md.pos, md.dur);
                md.stamp = GetTickCount64();
            }
        } catch (...) {
            mgr = nullptr;
        }
        bool changed = md.has != last.has || md.title != last.title || md.artist != last.artist ||
                       md.playing != last.playing;
        {
            std::lock_guard<std::mutex> lock(g_mediaMutex);
            g_media = md;
        }
        last = md;
        if (changed)
            if (HWND h = g_ctrl.load()) PostMessage(h, WM_APP_MEDIA, 0, 0);
        HANDLE hs[2] = {g_stopEvent, g_mediaWake};
        if (WaitForMultipleObjects(2, hs, FALSE, 1000) == WAIT_OBJECT_0) break;
    }
    mgr = nullptr;
    winrt::uninit_apartment();
    return 0;
}

static void WeatherLabel(int code, bool isDay, std::wstring* label, const uint16_t** icon) {
    if (code == 0) { *label = isDay ? L"CLEAR" : L"CLEAR NIGHT"; *icon = isDay ? kIconSun : kIconMoon; }
    else if (code <= 2) { *label = L"PARTLY CLOUDY"; *icon = isDay ? kIconCloud : kIconMoon; }
    else if (code == 3) { *label = L"OVERCAST"; *icon = kIconCloud; }
    else if (code == 45 || code == 48) { *label = L"FOG"; *icon = kIconFog; }
    else if (code >= 51 && code <= 57) { *label = L"DRIZZLE"; *icon = kIconRain; }
    else if (code >= 61 && code <= 67) { *label = L"RAIN"; *icon = kIconRain; }
    else if (code >= 71 && code <= 77) { *label = L"SNOW"; *icon = kIconSnow; }
    else if (code >= 80 && code <= 82) { *label = L"SHOWERS"; *icon = kIconRain; }
    else if (code >= 85 && code <= 86) { *label = L"SNOW SHOWERS"; *icon = kIconSnow; }
    else if (code >= 95) { *label = L"THUNDERSTORM"; *icon = kIconStorm; }
    else { *label = L"--"; *icon = kIconNone; }
}

// ---------------------------------------------------------------------------
// Widgets
// ---------------------------------------------------------------------------

static const wchar_t* kDays[] = {L"SUNDAY", L"MONDAY", L"TUESDAY", L"WEDNESDAY",
                                 L"THURSDAY", L"FRIDAY", L"SATURDAY"};
static const wchar_t* kMonths[] = {L"JANUARY", L"FEBRUARY", L"MARCH", L"APRIL",
                                   L"MAY", L"JUNE", L"JULY", L"AUGUST",
                                   L"SEPTEMBER", L"OCTOBER", L"NOVEMBER", L"DECEMBER"};

struct Widget {
    const wchar_t* id;
    int unitsW, unitsH;
    int defCol, defRow;
    HWND hwnd = nullptr;
    int x = 0, y = 0, w = 0, h = 0;  // screen coords / pixels
    // cached surface
    HDC memDC = nullptr;
    HBITMAP dib = nullptr;
    HGDIOBJ oldBmp = nullptr;
    void* bits = nullptr;
    // drag state
    bool pressed = false, dragging = false;
    POINT pressCursor{}, pressPos{};
    int clickX = -1, clickY = -1;  // client coords of the last click

    Widget(const wchar_t* id, int uw, int uh, int col, int row)
        : id(id), unitsW(uw), unitsH(uh), defCol(col), defRow(row) {}
    virtual ~Widget() = default;
    virtual void Draw(Graphics& g, const Ctx& c) = 0;
    virtual void Tick() {}
    // Returns true if handled (suppresses context menu).
    virtual bool OnClick(bool right) { return false; }
};

// ---- Clock ---------------------------------------------------------------
struct ClockWidget : Widget {
    ClockWidget() : Widget(L"clock", 2, 1, 0, 0) {}
    void Draw(Graphics& g, const Ctx& c) override {
        SYSTEMTIME st;
        GetLocalTime(&st);
        int hr = st.wHour;
        if (!g_s.use24h) { hr %= 12; if (hr == 0) hr = 12; }
        wchar_t buf[16];
        swprintf(buf, 16, L"%02d:%02d", hr, st.wMinute);
        std::wstring time = buf;

        REAL pad = 18 * c.s;
        REAL labelH = 16 * c.s;
        REAL availW = w - 2 * pad, availH = h - 2 * pad - labelH - 10 * c.s;
        REAL pitch = (std::min)(availW / DotTextCols(time), availH / 7);
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        DrawDotText(g, time, pad, h - pad - 7 * pitch, pitch, &fg, c.unlit ? &dim : nullptr,
                    g_s.redColon ? &acc : nullptr);

        swprintf(buf, 16, L"%.3ls %d %.3ls", kDays[st.wDayOfWeek], st.wDay, kMonths[st.wMonth - 1]);
        DrawLabel(g, c, buf, pad, pad - 2 * c.s, availW, 12, c.fg2);

        std::wstring right;
        if (g_s.showSeconds) {
            swprintf(buf, 16, L"%02d", st.wSecond);
            right = buf;
            REAL sp = 2.6f * c.s;
            SolidBrush a(c.accent);
            DrawDotText(g, right, w - pad - DotTextWidth(right, sp), pad, sp, &a);
        } else if (!g_s.use24h) {
            DrawLabel(g, c, st.wHour < 12 ? L"AM" : L"PM", pad, pad - 2 * c.s, availW, 12,
                      c.accent, StringAlignmentFar);
        } else {
            // the Gridline accent marker
            REAL d = 7 * c.s;
            FillCell(g, &acc, w - pad - d, pad + 2 * c.s, d, d);
        }
    }
};

// ---- Analog --------------------------------------------------------------
struct AnalogWidget : Widget {
    AnalogWidget() : Widget(L"analog", 1, 1, 2, 0) {}
    void Draw(Graphics& g, const Ctx& c) override {
        SYSTEMTIME st;
        GetLocalTime(&st);
        REAL cx = w / 2.0f, cy = h / 2.0f;
        REAL R = (std::min)(w, h) / 2.0f - 16 * c.s;
        SolidBrush fg(c.fg), dim(c.dim), fg2(c.fg2), acc(c.accent);
        for (int i = 0; i < 60; i++) {
            double a = i / 60.0 * 2 * kPi - kPi / 2;
            bool major = i % 5 == 0;
            REAL d = (major ? 3.4f : 1.8f) * c.s;
            REAL x = cx + (REAL)(R * cos(a)), y = cy + (REAL)(R * sin(a));
            FillCell(g, major ? &fg2 : &dim, x - d / 2, y - d / 2, d, d);
        }
        double sec = st.wSecond + st.wMilliseconds / 1000.0;
        double min = st.wMinute + sec / 60.0;
        double hour = (st.wHour % 12) + min / 60.0;
        auto hand = [&](double frac, REAL len, REAL width, const Color& col) {
            double a = frac * 2 * kPi - kPi / 2;
            Pen p(col, width);
            p.SetStartCap(LineCapRound);
            p.SetEndCap(LineCapRound);
            g.DrawLine(&p, cx, cy, cx + (REAL)(len * cos(a)), cy + (REAL)(len * sin(a)));
        };
        hand(hour / 12.0, R * 0.48f, 7 * c.s, c.fg);
        hand(min / 60.0, R * 0.74f, 5 * c.s, c.fg);
        // seconds: orange marker orbiting just inside the dial
        double a = (int)sec / 60.0 * 2 * kPi - kPi / 2;
        REAL sr = R - 9 * c.s, sd = 7 * c.s;
        FillCell(g, &acc, cx + (REAL)(sr * cos(a)) - sd / 2, cy + (REAL)(sr * sin(a)) - sd / 2, sd, sd);
        REAL hub = 6 * c.s;
        FillCell(g, &fg, cx - hub / 2, cy - hub / 2, hub, hub);
    }
};

// ---- Calendar ------------------------------------------------------------
struct CalendarWidget : Widget {
    CalendarWidget() : Widget(L"calendar", 1, 1, 0, 1) {}
    void Draw(Graphics& g, const Ctx& c) override {
        SYSTEMTIME st;
        GetLocalTime(&st);
        REAL pad = 16 * c.s;
        DrawLabel(g, c, kDays[st.wDayOfWeek], pad, pad - 2 * c.s, w - 2 * pad, 11, c.accent);
        wchar_t buf[24];
        swprintf(buf, 24, L"%d", st.wDay);
        std::wstring day = buf;
        REAL pitch = (std::min)((w - 2 * pad) / 11.0f, 9.5f * c.s);
        SolidBrush fg(c.fg), dim(c.dim);
        DrawDotText(g, day, pad, h / 2.0f - 7 * pitch / 2 + 2 * c.s, pitch, &fg, c.unlit ? &dim : nullptr);
        swprintf(buf, 24, L"%ls %d", kMonths[st.wMonth - 1], st.wYear);
        DrawLabel(g, c, buf, pad, h - pad - 16 * c.s, w - 2 * pad, 11, c.fg2);
    }
};

// ---- Battery -------------------------------------------------------------
struct BatteryWidget : Widget {
    BatteryWidget() : Widget(L"battery", 1, 1, 1, 1) {}
    void Draw(Graphics& g, const Ctx& c) override {
        SYSTEM_POWER_STATUS ps{};
        GetSystemPowerStatus(&ps);
        bool noBattery = (ps.BatteryFlag & 128) || ps.BatteryLifePercent == 255;
        int pct = noBattery ? 100 : ps.BatteryLifePercent;
        bool charging = (ps.BatteryFlag & 8) || (ps.ACLineStatus == 1 && !noBattery);
        bool low = !noBattery && pct <= 20 && !charging;

        REAL cx = w / 2.0f, cy = h / 2.0f;
        REAL R = (std::min)(w, h) / 2.0f - 16 * c.s;
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        const int n = 40;
        int lit = (int)std::lround(pct / 100.0 * n);
        DrawDotRing(g, cx, cy, R, n, lit, 4.2f * c.s, low ? &acc : &fg, &dim,
                    charging && lit < n ? &acc : nullptr);

        std::wstring txt = noBattery ? L"AC" : std::to_wstring(pct);
        REAL pitch = (std::min)(6.0f * c.s, R * 1.45f / DotTextCols(L"100"));
        REAL tw = DotTextWidth(txt, pitch);
        DrawDotText(g, txt, cx - tw / 2, cy - 7 * pitch / 2 - 6 * c.s, pitch, low ? &acc : &fg);
        const wchar_t* lbl = noBattery ? L"PLUGGED IN" : charging ? L"CHARGING" : low ? L"LOW" : L"BATTERY";
        DrawLabel(g, c, lbl, cx - R, cy + 16 * c.s, 2 * R, 9, c.fg2, StringAlignmentCenter);
    }
};

// ---- Weather -------------------------------------------------------------
struct WeatherWidget : Widget {
    WeatherWidget() : Widget(L"weather", 1, 1, 2, 1) {}
    void Draw(Graphics& g, const Ctx& c) override {
        WeatherData wd;
        {
            std::lock_guard<std::mutex> lock(g_weatherMutex);
            wd = g_weather;
        }
        REAL pad = 16 * c.s;
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        std::wstring label;
        const uint16_t* icon = kIconNone;
        if (wd.valid) WeatherLabel(wd.code, wd.isDay, &label, &icon);
        else label = wd.error.empty() ? L"LOADING" : wd.error;

        REAL ip = 4.4f * c.s;
        DrawDotIcon(g, icon, pad, pad, ip, (icon == kIconSun) ? &acc : &fg, c.unlit ? &dim : nullptr);
        std::wstring city = g_s.city;
        for (auto& ch : city) ch = towupper(ch);
        DrawLabel(g, c, city, pad + 9 * ip, pad - 2 * c.s, w - 2 * pad - 9 * ip, 10, c.fg2,
                  StringAlignmentFar);

        std::wstring temp = wd.valid ? std::to_wstring((int)std::lround(wd.temp)) + L"\u00B0" : L"--\u00B0";
        REAL tp = (std::min)(6.2f * c.s, (w - 2 * pad) / (REAL)DotTextCols(temp));
        REAL ty = h - pad - 16 * c.s - 7 * tp - 4 * c.s;
        DrawDotText(g, temp, pad, ty, tp, &fg, c.unlit ? &dim : nullptr);

        DrawLabel(g, c, label, pad, h - pad - 15 * c.s, w - 2 * pad, 9.5f, c.fg2);
        if (wd.valid) {
            wchar_t buf[32];
            swprintf(buf, 32, L"%d/%d", (int)std::lround(wd.hi), (int)std::lround(wd.lo));
            if (label.size() <= 9)
                DrawLabel(g, c, buf, pad, h - pad - 15 * c.s, w - 2 * pad, 9.5f, c.fg, StringAlignmentFar);
        }
    }
};

// ---- System --------------------------------------------------------------
static ULONGLONG FtToU64(const FILETIME& f) { return ((ULONGLONG)f.dwHighDateTime << 32) | f.dwLowDateTime; }

struct SystemWidget : Widget {
    ULONGLONG lastIdle = 0, lastTotal = 0;
    int cpu = 0, ram = 0, disk = 0;
    int diskTick = 0;
    SystemWidget() : Widget(L"system", 2, 1, 0, 2) { Tick(); }
    void Tick() override {
        FILETIME idle, kern, user;
        if (GetSystemTimes(&idle, &kern, &user)) {
            ULONGLONG i = FtToU64(idle), t = FtToU64(kern) + FtToU64(user);
            if (lastTotal && t > lastTotal)
                cpu = (int)std::lround(100.0 * (1.0 - (double)(i - lastIdle) / (double)(t - lastTotal)));
            lastIdle = i;
            lastTotal = t;
        }
        MEMORYSTATUSEX ms{sizeof(ms)};
        if (GlobalMemoryStatusEx(&ms)) ram = (int)ms.dwMemoryLoad;
        if (diskTick-- <= 0) {
            diskTick = 30;
            wchar_t win[MAX_PATH] = L"C:\\";
            if (GetWindowsDirectoryW(win, MAX_PATH) >= 3) win[3] = 0;
            ULARGE_INTEGER freeB, total, totalFree;
            if (GetDiskFreeSpaceExW(win, &freeB, &total, &totalFree) && total.QuadPart)
                disk = (int)std::lround(100.0 * (1.0 - (double)totalFree.QuadPart / total.QuadPart));
        }
        cpu = std::clamp(cpu, 0, 100);
    }
    void Draw(Graphics& g, const Ctx& c) override {
        REAL pad = 18 * c.s;
        struct Row { const wchar_t* name; int v; } rows[] = {{L"CPU", cpu}, {L"RAM", ram}, {L"DISK", disk}};
        REAL rowH = (h - 2 * pad) / 3;
        REAL labelW = 46 * c.s;
        REAL vp = 3.4f * c.s;
        REAL valW = DotTextWidth(L"100%", vp);
        REAL barX = pad + labelW, barW = w - 2 * pad - labelW - valW - 12 * c.s;
        const int n = 22;
        REAL bp = barW / n, bd = (std::min)(bp * 0.62f, 6.0f * c.s);
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        for (int r = 0; r < 3; r++) {
            REAL cy = pad + rowH * r + rowH / 2;
            DrawLabel(g, c, rows[r].name, pad, cy - 9 * c.s, labelW, 11, c.fg2);
            int lit = (int)std::lround(rows[r].v / 100.0 * n);
            for (int i = 0; i < n; i++) {
                Brush* b = i < lit ? (rows[r].v >= 90 ? (Brush*)&acc : (Brush*)&fg) : (Brush*)&dim;
                FillCell(g, b, barX + i * bp + (bp - bd) / 2, cy - bd / 2, bd, bd);
            }
            std::wstring v = std::to_wstring(rows[r].v) + L"%";
            DrawDotText(g, v, w - pad - DotTextWidth(v, vp), cy - 3.5f * vp, vp, &fg);
        }
    }
};

// ---- Focus timer ---------------------------------------------------------
struct FocusWidget : Widget {
    enum State { Idle, Running, Paused, Done } state = Idle;
    ULONGLONG endTick = 0;
    LONGLONG remainingMs = 0;
    FocusWidget() : Widget(L"focus", 1, 1, 2, 2) {}
    LONGLONG Total() const { return (LONGLONG)g_s.focusMinutes * 60 * 1000; }
    LONGLONG Remaining() const {
        if (state == Running) {
            LONGLONG r = (LONGLONG)endTick - (LONGLONG)GetTickCount64();
            return r > 0 ? r : 0;
        }
        if (state == Paused) return remainingMs;
        if (state == Done) return 0;
        return Total();
    }
    void Tick() override {
        if (state == Running && Remaining() == 0) {
            state = Done;
            MessageBeep(MB_ICONASTERISK);
        }
    }
    bool OnClick(bool right) override {
        if (right) { state = Idle; return true; }
        switch (state) {
            case Idle: case Done: state = Running; endTick = GetTickCount64() + Total(); break;
            case Running: remainingMs = Remaining(); state = Paused; break;
            case Paused: endTick = GetTickCount64() + remainingMs; state = Running; break;
        }
        return true;
    }
    void Draw(Graphics& g, const Ctx& c) override {
        REAL cx = w / 2.0f, cy = h / 2.0f;
        REAL R = (std::min)(w, h) / 2.0f - 16 * c.s;
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        LONGLONG rem = Remaining();
        const int n = 60;
        int lit = (int)std::ceil((double)rem / (double)Total() * n);
        if (state == Done && (GetTickCount64() / 500) % 2) lit = n;
        DrawDotRing(g, cx, cy, R, n, lit, 3.2f * c.s, state == Done ? &acc : &fg, &dim,
                    state == Running ? &acc : nullptr);
        int secs = (int)((rem + 999) / 1000);
        wchar_t buf[16];
        swprintf(buf, 16, L"%02d:%02d", secs / 60, secs % 60);
        std::wstring t = buf;
        REAL pitch = (std::min)(4.2f * c.s, (R * 1.72f) / DotTextCols(t));
        DrawDotText(g, t, cx - DotTextWidth(t, pitch) / 2, cy - 7 * pitch / 2 - 5 * c.s, pitch, &fg,
                    nullptr, state == Running ? &acc : nullptr);
        const wchar_t* lbl = state == Idle ? L"TAP TO FOCUS" : state == Running ? L"FOCUS"
                           : state == Paused ? L"PAUSED" : L"DONE";
        DrawLabel(g, c, lbl, cx - R, cy + 14 * c.s, 2 * R, 9, state == Done ? c.accent : c.fg2,
                  StringAlignmentCenter);
    }
};

// ---- Year progress -------------------------------------------------------
struct YearWidget : Widget {
    YearWidget() : Widget(L"year", 1, 1, 2, 3) {}
    void Draw(Graphics& g, const Ctx& c) override {
        SYSTEMTIME st;
        GetLocalTime(&st);
        static const int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool leap = (st.wYear % 4 == 0 && st.wYear % 100 != 0) || st.wYear % 400 == 0;
        int total = leap ? 366 : 365, doy = st.wDay;
        for (int m = 0; m < st.wMonth - 1; m++) doy += mdays[m] + (m == 1 && leap ? 1 : 0);

        REAL pad = 16 * c.s;
        wchar_t buf[16];
        swprintf(buf, 16, L"%d", st.wYear);
        DrawLabel(g, c, buf, pad, pad - 2 * c.s, w - 2 * pad, 11, c.fg2);
        swprintf(buf, 16, L"%d%%", (int)(100.0 * doy / total));
        DrawLabel(g, c, buf, pad, pad - 2 * c.s, w - 2 * pad, 11, c.accent, StringAlignmentFar);

        REAL top = pad + 22 * c.s;
        REAL aw = w - 2 * pad, ah = h - top - pad;
        int cols = (int)std::ceil(std::sqrt(total * aw / ah));
        int rows = (total + cols - 1) / cols;
        REAL p = (std::min)(aw / cols, ah / rows);
        REAL d = p * 0.62f;
        REAL ox = pad + (aw - cols * p) / 2;
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        for (int i = 0; i < total; i++) {
            int r = i / cols, col = i % cols;
            Brush* b = i + 1 < doy ? (Brush*)&fg : i + 1 == doy ? (Brush*)&acc : (Brush*)&dim;
            FillCell(g, b, ox + col * p + (p - d) / 2, top + r * p + (p - d) / 2, d, d);
        }
    }
};

// ---- Quote ---------------------------------------------------------------
// Short everyday lines; only characters the dot font can draw.
static const wchar_t* const kDailyQuotes[] = {
    L"SMALL STEPS EVERY DAY",
    L"DONE IS BETTER THAN PERFECT",
    L"DRINK WATER. TAKE A WALK.",
    L"ONE THING AT A TIME",
    L"REST IS PART OF THE WORK",
    L"BE KIND, IT COSTS YOU NOTHING",
    L"START WHERE YOU ARE",
    L"SLOW IS SMOOTH, SMOOTH IS FAST",
    L"TODAY IS A GOOD DAY TO BEGIN",
    L"WELL BEGUN IS HALF DONE",
    L"KEEP IT SIMPLE",
    L"CALL SOMEONE YOU LOVE",
    L"FOCUS ON WHAT YOU CAN CONTROL",
    L"LITTLE BY LITTLE, A LOT",
    L"THE BEST TIME IS NOW",
    L"BREATHE IN. BREATHE OUT.",
    L"LEARN ONE NEW THING TODAY",
    L"SAY NO TO SAY YES",
    L"PROGRESS, NOT PERFECTION",
    L"GO OUTSIDE FOR A WHILE",
    L"THIS TOO SHALL PASS",
    L"EVERY JOURNEY STARTS WITH ONE STEP",
    L"SLEEP WELL, THINK CLEARLY",
    L"LISTEN MORE THAN YOU SPEAK",
    L"CLEAR DESK, CLEAR MIND",
    L"BE HERE NOW",
    L"SHOW UP, EVEN ON SLOW DAYS",
    L"GRATITUDE TURNS ENOUGH INTO PLENTY",
    L"CONSISTENCY BEATS INTENSITY",
    L"MOVE YOUR BODY, CLEAR YOUR HEAD",
    L"WORDS ARE FREE. CHOOSE WELL.",
    L"FINISH WHAT YOU START",
    L"LESS SCROLLING, MORE LIVING",
    L"YOU HAVE TIME FOR WHAT MATTERS",
    L"EAT WELL. SLEEP WELL. BE WELL.",
    L"ASKING FOR HELP IS STRENGTH",
    L"CELEBRATE SMALL WINS",
    L"A GOOD DAY STARTS TONIGHT",
    L"YOU ARE DOING BETTER THAN YOU THINK",
    L"MAKE TODAY COUNT",
};

static int DayNumber() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return (int)(u.QuadPart / 864000000000ULL);
}

struct QuoteWidget : Widget {
    int offset = 0;  // advanced by clicking
    QuoteWidget() : Widget(L"quote", 2, 1, 0, 3) {}
    bool OnClick(bool right) override {
        if (right || !g_s.quoteDaily) return false;
        offset++;
        return true;
    }
    std::wstring CurrentText() const {
        if (!g_s.quoteDaily) return g_s.quoteText.empty() ? L"MAKE TODAY COUNT" : g_s.quoteText;
        const int n = (int)(sizeof(kDailyQuotes) / sizeof(kDailyQuotes[0]));
        return kDailyQuotes[((DayNumber() + offset) % n + n) % n];
    }
    static std::vector<std::wstring> Wrap(const std::wstring& text, REAL maxW, REAL pitch) {
        std::vector<std::wstring> lines;
        std::wstring line, word;
        auto flushWord = [&]() {
            if (word.empty()) return;
            std::wstring cand = line.empty() ? word : line + L" " + word;
            if (!line.empty() && DotTextWidth(cand, pitch) > maxW) {
                lines.push_back(line);
                line = word;
            } else {
                line = cand;
            }
            word.clear();
        };
        for (wchar_t ch : text) {
            if (ch == L' ' || ch == L'\n') {
                flushWord();
                if (ch == L'\n') { lines.push_back(line); line.clear(); }
            } else {
                word += ch;
            }
        }
        flushWord();
        if (!line.empty()) lines.push_back(line);
        return lines;
    }
    void Draw(Graphics& g, const Ctx& c) override {
        REAL pad = 18 * c.s;
        REAL capH = g_s.quoteAuthor.empty() ? 0 : 20 * c.s;
        REAL aw = w - 2 * pad, ah = h - 2 * pad - capH;
        std::wstring text = CurrentText();
        for (auto& ch : text) ch = towupper(ch);
        REAL pitch = 7.0f * c.s;
        std::vector<std::wstring> lines;
        for (; pitch > 1.5f * c.s; pitch -= 0.25f * c.s) {
            lines = Wrap(text, aw - 10 * c.s, pitch);
            bool fitsW = true;
            for (auto& l : lines) fitsW &= DotTextWidth(l, pitch) <= aw - 10 * c.s;
            if (fitsW && lines.size() * 9 * pitch - 2 * pitch <= ah) break;
        }
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        REAL dotD = 7 * c.s;
        FillCell(g, &acc, w - pad - dotD, pad, dotD, dotD);  // Gridline accent marker
        REAL blockH = lines.size() * 9 * pitch - 2 * pitch;
        REAL y = pad + (std::max)(0.0f, (ah - blockH) / 2);
        for (auto& l : lines) {
            DrawDotText(g, l, pad, y, pitch, &fg, c.unlit ? &dim : nullptr);
            y += 9 * pitch;
        }
        if (!g_s.quoteAuthor.empty()) {
            SolidBrush acc(c.accent);
            REAL d = 6 * c.s;
            FillCell(g, &acc, pad, h - pad - 10 * c.s, d, d);
            std::wstring a = g_s.quoteAuthor;
            for (auto& ch : a) ch = towupper(ch);
            DrawLabel(g, c, a, pad + 12 * c.s, h - pad - 16 * c.s, aw, 11, c.fg2);
        }
    }
};


// ---- shared small helpers -------------------------------------------------
// Draws a bitmap glyph: rows[], 'w' columns wide (bit w-1 = left column).
static void DrawBits(Graphics& g, const uint8_t* rows, int nrows, int w, REAL x, REAL y, REAL pitch, Brush* b) {
    DrawStrokes(g, [rows, w](int r, int c) { return ((rows[r] >> (w - 1 - c)) & 1) != 0; }, nrows, w, x, y, pitch, b);
}
static const uint8_t kBitsPlay[7] = {0x10, 0x18, 0x1C, 0x1E, 0x1C, 0x18, 0x10};
static const uint8_t kBitsPause[7] = {0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B};
static const uint8_t kBitsPrev[7] = {0x21, 0x23, 0x27, 0x2F, 0x27, 0x23, 0x21};  // 6 wide: bar + left triangle
static const uint8_t kBitsNext[7] = {0x21, 0x31, 0x39, 0x3D, 0x39, 0x31, 0x21};  // 6 wide: right triangle + bar

static bool DotFontCovers(const std::wstring& t) {
    for (wchar_t c : t) {
        wchar_t u = towupper(c);
        if (u == L'?') continue;
        if (FindGlyph(u)->ch == L'?') return false;
    }
    return true;
}

// ---- Now Playing -----------------------------------------------------------
struct NowPlayingWidget : Widget {
    NowPlayingWidget() : Widget(L"nowplaying", 2, 1, -2, 0) {}
    REAL btnX[3] = {}, btnY = 0, btnW = 0;  // prev, play, next hit boxes (client px)
    bool OnClick(bool right) override {
        if (right) return false;
        int cmd = 1;
        if (clickY >= 0 && clickY <= btnY + btnW * 1.6f) {
            if (clickX >= btnX[0] - btnW * 0.3f && clickX < btnX[1] - btnW * 0.15f) cmd = 3;
            else if (clickX >= btnX[2] - btnW * 0.15f) cmd = 2;
        }
        g_mediaCmd = cmd;
        if (g_mediaWake) SetEvent(g_mediaWake);
        return true;
    }
    void Draw(Graphics& g, const Ctx& c) override {
        MediaData md;
        {
            std::lock_guard<std::mutex> lock(g_mediaMutex);
            md = g_media;
        }
        REAL pad = 18 * c.s;
        SolidBrush fg(c.fg), fg2(c.fg2), dim(c.dim), acc(c.accent);
        REAL ip = 2.6f * c.s, gap = 9 * c.s;
        btnW = 6 * ip;
        btnY = pad;
        btnX[2] = w - pad - btnW;
        btnX[1] = btnX[2] - gap - btnW;
        btnX[0] = btnX[1] - gap - btnW;
        Brush* cb = md.has ? (Brush*)&fg : (Brush*)&dim;
        DrawBits(g, kBitsPrev, 7, 6, btnX[0], btnY, ip, cb);
        DrawBits(g, md.playing ? kBitsPause : kBitsPlay, 7, 5, btnX[1] + ip / 2, btnY, ip,
                 md.has ? (Brush*)&acc : (Brush*)&dim);
        DrawBits(g, kBitsNext, 7, 6, btnX[2], btnY, ip, cb);
        std::wstring head = md.has && !md.app.empty() ? md.app : L"NOW PLAYING";
        DrawLabel(g, c, head, pad, pad - 2 * c.s, btnX[0] - pad - 6 * c.s, 11, md.playing ? c.accent : c.fg2);

        if (!md.has) {
            std::wstring t = L"NO MEDIA";
            REAL pitch = 5.5f * c.s;
            DrawDotText(g, t, pad, h / 2.0f - 7 * pitch / 2, pitch, &dim);
            DrawLabel(g, c, L"PLAY SOMETHING", pad, h - pad - 16 * c.s, w - 2 * pad, 10, c.fg2);
            return;
        }
        std::wstring title = md.title.empty() ? L"UNKNOWN" : md.title;
        REAL top = pad + 24 * c.s, availW = w - 2 * pad;
        REAL artistY = h - pad - 34 * c.s;
        if (DotFontCovers(title)) {
            std::wstring up = title;
            for (auto& ch : up) ch = towupper(ch);
            REAL pitch = (std::min)(5.0f * c.s, availW / DotTextCols(up));
            if (pitch < 2.6f * c.s) {  // too long for one line: two lines
                pitch = 3.0f * c.s;
                auto lines = QuoteWidget::Wrap(up, availW, pitch);
                for (size_t i = 0; i < lines.size() && i < 2; i++) {
                    std::wstring l = lines[i];
                    if (i == 1 && lines.size() > 2) l += L"...";
                    while (DotTextWidth(l, pitch) > availW && l.size() > 4) l.erase(l.size() - 4, 1);
                    DrawDotText(g, l, pad, top + i * 9 * pitch, pitch, &fg);
                }
            } else {
                DrawDotText(g, up, pad, top + (artistY - top - 7 * pitch) / 2, pitch, &fg);
            }
        } else {
            Font f(c.family, 17 * c.s, FontStyleBold, UnitPixel);
            StringFormat fmt;
            fmt.SetTrimming(StringTrimmingEllipsisCharacter);
            RectF rc(pad, top, availW, artistY - top);
            g.DrawString(title.c_str(), -1, &f, rc, &fmt, &fg);
        }
        std::wstring artist = md.artist;
        for (auto& ch : artist) ch = towupper(ch);
        DrawLabel(g, c, artist, pad, artistY, availW, 11, c.fg2);
        double pos = md.pos;
        if (md.playing) pos += (GetTickCount64() - md.stamp) / 1000.0;
        double frac = md.dur > 1 ? (std::min)(1.0, pos / md.dur) : 0;
        const int n = 34;
        REAL bp = availW / n, bd = (std::min)(bp * 0.55f, 4.0f * c.s), by = h - pad - 4 * c.s;
        int lit = (int)std::lround(frac * n);
        for (int i = 0; i < n; i++) {
            Brush* b = i < lit ? (Brush*)&fg : (Brush*)&dim;
            if (md.dur > 1 && i == (std::max)(0, lit - 1)) b = &acc;
            FillCell(g, b, pad + i * bp + (bp - bd) / 2, by - bd / 2, bd, bd);
        }
    }
};

// ---- Hourly forecast ---------------------------------------------------------
struct ForecastWidget : Widget {
    ForecastWidget() : Widget(L"forecast", 2, 1, -2, 1) {}
    void Draw(Graphics& g, const Ctx& c) override {
        WeatherData wd;
        {
            std::lock_guard<std::mutex> lock(g_weatherMutex);
            wd = g_weather;
        }
        REAL pad = 18 * c.s;
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        DrawLabel(g, c, L"NEXT HOURS", pad, pad - 2 * c.s, w - 2 * pad, 11, c.fg2);
        std::wstring city = g_s.city;
        for (auto& ch : city) ch = towupper(ch);
        DrawLabel(g, c, city, pad, pad - 2 * c.s, w - 2 * pad, 11, c.fg2, StringAlignmentFar);
        if (!wd.valid || wd.hours == 0) {
            DrawLabel(g, c, wd.error.empty() ? L"LOADING" : wd.error, pad, h / 2.0f - 8 * c.s, w - 2 * pad, 11,
                      c.fg2);
            return;
        }
        REAL colW = (w - 2 * pad) / wd.hours;
        for (int i = 0; i < wd.hours; i++) {
            REAL cx = pad + colW * i + colW / 2;
            wchar_t hb[8];
            swprintf(hb, 8, L"%02d", wd.hHour[i]);
            DrawLabel(g, c, hb, cx - colW / 2, pad + 22 * c.s, colW, 10, c.fg2, StringAlignmentCenter);
            std::wstring label;
            const uint16_t* icon = kIconNone;
            bool day = wd.hHour[i] >= 6 && wd.hHour[i] < 18;
            WeatherLabel(wd.hCode[i], day, &label, &icon);
            REAL ip = 2.3f * c.s;
            DrawDotIcon(g, icon, cx - 4.5f * ip, pad + 46 * c.s, ip,
                        icon == kIconSun ? (Brush*)&acc : (Brush*)&fg, nullptr);
            std::wstring t = std::to_wstring((int)std::lround(wd.hTemp[i])) + L"°";
            REAL tp = 2.5f * c.s;
            DrawDotText(g, t, cx - DotTextWidth(t, tp) / 2, h - pad - 7 * tp, tp, &fg);
        }
    }
};

// ---- Stopwatch -------------------------------------------------------------
struct StopwatchWidget : Widget {
    bool running = false;
    ULONGLONG startTick = 0, accum = 0;
    StopwatchWidget() : Widget(L"stopwatch", 1, 1, -1, 2) {}
    ULONGLONG Elapsed() const { return accum + (running ? GetTickCount64() - startTick : 0); }
    bool OnClick(bool right) override {
        if (right) {
            if (!running && accum == 0) return false;  // nothing to reset: show the menu
            running = false;
            accum = 0;
            return true;
        }
        if (running) {
            accum += GetTickCount64() - startTick;
            running = false;
        } else {
            startTick = GetTickCount64();
            running = true;
        }
        return true;
    }
    void Draw(Graphics& g, const Ctx& c) override {
        ULONGLONG ms = Elapsed();
        int secs = (int)(ms / 1000), tenths = (int)(ms / 100 % 10);
        wchar_t buf[16];
        if (secs >= 3600) swprintf(buf, 16, L"%d:%02d", secs / 3600, secs / 60 % 60);
        else swprintf(buf, 16, L"%02d:%02d", secs / 60, secs % 60);
        std::wstring t = buf;
        REAL pad = 16 * c.s;
        SolidBrush fg(c.fg), dim(c.dim), acc(c.accent);
        DrawLabel(g, c, L"STOPWATCH", pad, pad - 2 * c.s, w - 2 * pad, 11, running ? c.accent : c.fg2);
        REAL pitch = (std::min)(4.6f * c.s, (w - 2 * pad) / DotTextCols(t));
        DrawDotText(g, t, (w - DotTextWidth(t, pitch)) / 2, h / 2.0f - 7 * pitch / 2 - 4 * c.s, pitch, &fg,
                    nullptr, running ? &acc : nullptr);
        REAL bp = (w - 2 * pad) / 10, bd = (std::min)(bp * 0.5f, 5.0f * c.s), by = h - pad - 22 * c.s;
        for (int i = 0; i < 10; i++)
            FillCell(g, i < tenths ? (Brush*)&fg : (Brush*)&dim, pad + i * bp + (bp - bd) / 2, by - bd / 2, bd, bd);
        DrawLabel(g, c, running ? L"TAP TO PAUSE" : (ms ? L"TAP TO RESUME" : L"TAP TO START"), pad,
                  h - pad - 12 * c.s, w - 2 * pad, 9, c.fg2, StringAlignmentCenter);
    }
};

// ---------------------------------------------------------------------------
// Host: windows, layout, rendering
// ---------------------------------------------------------------------------

static const wchar_t kWidgetClass[] = L"GridlineDesktopWidget";
static const wchar_t kCtrlClass[] = L"GridlineDesktopCtrl";
static HANDLE g_uiThread = nullptr;
static HWND g_parent = nullptr;
static std::vector<std::unique_ptr<Widget>> g_widgets;
static std::unique_ptr<FontFamily> g_family;

enum { kMenuReset = 1, kMenuWeather = 2, kMenuGlyph = 3 };
enum { kTimerTick = 1, kTimerGlyph = 2, kTimerCursor = 3 };
static int g_cursorPass = 0;

static REAL PixelScale() {
    HDC dc = GetDC(nullptr);
    int dpi = dc ? GetDeviceCaps(dc, LOGPIXELSX) : 96;
    if (dc) ReleaseDC(nullptr, dc);
    if (dpi <= 0) dpi = 96;
    return g_s.scale / 100.0f * dpi / 96.0f;
}

static Ctx MakeCtx() {
    Ctx c;
    c.s = PixelScale();
    bool light = g_s.theme == ThemeMode::Light || (g_s.theme == ThemeMode::Auto && IsSystemLight());
    BYTE a = (BYTE)(g_s.cardOpacity * 255 / 100);
    if (light) {
        c.bg = Color(a, 0xF3, 0xFB, 0xFC);
        c.fg = Color(255, 0x00, 0x6F, 0x80);
        c.fg2 = Color(255, 0x4D, 0x8C, 0x97);
        c.dim = Color(255, 0xCF, 0xE7, 0xEC);
        c.line = Color(255, 0x5F, 0xC3, 0xD3);
    } else {
        c.bg = Color(a, 0x03, 0x0B, 0x0F);
        c.fg = Color(255, 0x8D, 0xF4, 0xFF);
        c.fg2 = Color(255, 0x3E, 0x9D, 0xAD);
        c.dim = Color(255, 0x0B, 0x2A, 0x33);
        c.line = Color(255, 0x0F, 0x8F, 0xA3);
    }
    if (g_s.cardStyle == CardStyle::None)
        c.dim = Color(90, c.dim.GetR(), c.dim.GetG(), c.dim.GetB());
    c.accent = Color(g_s.accent);
    c.unlit = g_s.unlitDots;
    c.family = g_family.get();
    return c;
}

static void ReleaseSurface(Widget* wd) {
    if (wd->memDC) {
        SelectObject(wd->memDC, wd->oldBmp);
        DeleteDC(wd->memDC);
        wd->memDC = nullptr;
    }
    if (wd->dib) {
        DeleteObject(wd->dib);
        wd->dib = nullptr;
    }
}

static void PaintWidget(Graphics& g, Widget* wd, const Ctx& c) {
    REAL r = 8 * c.s;
    REAL in = 1.5f * c.s;
    GraphicsPath path;
    RoundRectPath(path, in, in, wd->w - 2 * in, wd->h - 2 * in, r);
    if (g_s.cardStyle == CardStyle::Solid) {
        SolidBrush bg(c.bg.GetA() ? c.bg : Color(1, 0, 0, 0));
        g.FillPath(&bg, &path);
    } else {
        SolidBrush hit(Color(1, 0, 0, 0));  // keeps the card clickable/draggable
        g.FillPath(&hit, &path);
    }
    if (g_s.cardStyle != CardStyle::None) {
        Pen frame(Faded(c.line, 0.55f), 1.0f * c.s);
        g.DrawPath(&frame, &path);
        // bright corner brackets with a soft glow
        REAL L = (std::min)(18 * c.s, (std::min)((REAL)wd->w, (REAL)wd->h) / 4);
        REAL x0 = in + 0.5f, y0 = in + 0.5f, x1 = wd->w - in - 0.5f, y1 = wd->h - in - 0.5f;
        PointF corners[4][3] = {
            {PointF(x0, y0 + L), PointF(x0, y0), PointF(x0 + L, y0)},
            {PointF(x1 - L, y0), PointF(x1, y0), PointF(x1, y0 + L)},
            {PointF(x1, y1 - L), PointF(x1, y1), PointF(x1 - L, y1)},
            {PointF(x0 + L, y1), PointF(x0, y1), PointF(x0, y1 - L)},
        };
        Pen glow(Faded(c.fg, 0.18f), 5.0f * c.s);
        Pen bright(Faded(c.fg, 0.9f), 1.6f * c.s);
        glow.SetLineJoin(LineJoinMiter);
        bright.SetLineJoin(LineJoinMiter);
        for (auto& k : corners) g.DrawLines(&glow, k, 3);
        for (auto& k : corners) g.DrawLines(&bright, k, 3);
    }
    wd->Draw(g, c);
}

static void Render(Widget* wd, const Ctx& c) {
    if (!wd->hwnd) return;
    HDC screen = GetDC(nullptr);
    if (!wd->memDC) {
        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = wd->w;
        bi.bmiHeader.biHeight = -wd->h;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        wd->dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &wd->bits, nullptr, 0);
        wd->memDC = CreateCompatibleDC(screen);
        if (!wd->dib || !wd->memDC) {
            ReleaseSurface(wd);
            ReleaseDC(nullptr, screen);
            return;
        }
        wd->oldBmp = SelectObject(wd->memDC, wd->dib);
    }
    {
        Bitmap bmp(wd->w, wd->h, wd->w * 4, PixelFormat32bppPARGB, (BYTE*)wd->bits);
        Graphics g(&bmp);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
        g.SetTextRenderingHint(TextRenderingHintAntiAlias);
        g.Clear(Color(0, 0, 0, 0));

        PaintWidget(g, wd, c);
    }
    SIZE sz{wd->w, wd->h};
    POINT src{0, 0};
    BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(wd->hwnd, screen, nullptr, &sz, wd->memDC, &src, 0, &bf, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
}

static void RenderAll() {
    Ctx c = MakeCtx();
    for (auto& w : g_widgets) Render(w.get(), c);
}

static void MoveWidget(Widget* wd, int x, int y) {
    wd->x = x;
    wd->y = y;
    POINT pt{x, y};
    if (g_parent) ScreenToClient(g_parent, &pt);
    SetWindowPos(wd->hwnd, nullptr, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static std::wstring Key(const Widget* wd, const wchar_t* axis) {
    return std::wstring(L"pos.") + wd->id + L"." + axis;
}

static void DefaultPos(const Widget* wd, REAL s, int* x, int* y) {
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    int U = (int)(150 * s), G = (int)(12 * s), M = (int)(40 * s);
    int gridW = 3 * U + 2 * G;
    int left = mi.rcWork.right - M - gridW;
    *x = left + wd->defCol * (U + G);
    *y = mi.rcWork.top + M + wd->defRow * (U + G);
}

static void PlaceWidget(Widget* wd, REAL s, bool forceDefault) {
    int x = Wh_GetIntValue(Key(wd, L"x").c_str(), INT_MIN);
    int y = Wh_GetIntValue(Key(wd, L"y").c_str(), INT_MIN);
    RECT vs{GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN)};
    vs.right = vs.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    vs.bottom = vs.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    bool valid = x != INT_MIN && y != INT_MIN && x + wd->w / 2 > vs.left && x + wd->w / 2 < vs.right &&
                 y + wd->h / 2 > vs.top && y + wd->h / 2 < vs.bottom;
    if (forceDefault || !valid) DefaultPos(wd, s, &x, &y);
    MoveWidget(wd, x, y);
}

static void SavePos(Widget* wd) {
    Wh_SetIntValue(Key(wd, L"x").c_str(), wd->x);
    Wh_SetIntValue(Key(wd, L"y").c_str(), wd->y);
}

static void GlyphTrigger();

static void ShowMenu(Widget* wd) {
    HWND ctrl = g_ctrl.load();
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING, kMenuReset, L"Reset widget layout");
    AppendMenuW(m, MF_STRING | (g_s.weather || g_s.forecast ? 0 : MF_GRAYED), kMenuWeather, L"Refresh weather");
    AppendMenuW(m, MF_STRING | (g_s.glyph ? 0 : MF_GRAYED), kMenuGlyph, L"Test Pulse light");
    AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m, MF_STRING | MF_GRAYED, 0,
                g_s.locked ? L"Widgets locked (change in Windhawk)" : L"Drag to move widgets");
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(ctrl);
    int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, 0, ctrl, nullptr);
    PostMessage(ctrl, WM_NULL, 0, 0);
    DestroyMenu(m);
    if (cmd == kMenuReset) {
        REAL s = PixelScale();
        for (auto& w : g_widgets) {
            Wh_DeleteValue(Key(w.get(), L"x").c_str());
            Wh_DeleteValue(Key(w.get(), L"y").c_str());
            PlaceWidget(w.get(), s, true);
        }
    } else if (cmd == kMenuWeather && g_weatherWake) {
        SetEvent(g_weatherWake);
    } else if (cmd == kMenuGlyph) {
        GlyphTrigger();
    }
}


// ---------------------------------------------------------------------------
// Pulse notification light
// ---------------------------------------------------------------------------

static HWND g_glyphWnd = nullptr;
static ULONGLONG g_glyphStart = 0, g_glyphLastTrigger = 0;
static HWINEVENTHOOK g_winEventHook = nullptr;
static const wchar_t kGlyphClass[] = L"GridlinePulseLight";
static const int kGlyphMs = 1900;

static void GlyphFrame() {
    if (!g_glyphWnd) return;
    ULONGLONG t = GetTickCount64() - g_glyphStart;
    REAL s = PixelScale();
    const int N = 8;                       // dots per arm of the corner bracket
    REAL pitch = 11 * s, dotD = 6 * s;
    int size = (int)(pitch * N + 4 * s);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    int margin = (int)(14 * s);
    std::wstring corner = g_s.glyphCorner;
    bool right = corner.find(L"Right") != std::wstring::npos, bottom = corner.find(L"bottom") == 0;
    int x = right ? mi.rcMonitor.right - margin - size : mi.rcMonitor.left + margin;
    int y = bottom ? mi.rcWork.bottom - margin - size : mi.rcMonitor.top + margin;

    HDC screen = GetDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = size;
    bi.bmiHeader.biHeight = -size;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HDC mem = CreateCompatibleDC(screen);
    HGDIOBJ old = SelectObject(mem, dib);
    {
        Bitmap bmp(size, size, size * 4, PixelFormat32bppPARGB, (BYTE*)bits);
        Graphics g(&bmp);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.Clear(Color(0, 0, 0, 0));
        Color base = g_s.glyphCyan ? Color(255, 0x00, 0xE5, 0xFF) : Color(g_s.accent);
        // corner bracket: one arm along the edge, one down/up the side, meeting at the corner
        std::vector<PointF> pts;
        for (int i = 0; i < N; i++) {
            REAL along = (N - 1 - i) * pitch + pitch / 2 + 2 * s;  // i = 0 is the corner dot
            REAL cornerX = right ? size - pitch / 2 - 2 * s : pitch / 2 + 2 * s;
            REAL cornerY = bottom ? size - pitch / 2 - 2 * s : pitch / 2 + 2 * s;
            REAL off = i * pitch;
            pts.push_back(PointF(cornerX + (right ? -off : off), cornerY));
            if (i) pts.push_back(PointF(cornerX, cornerY + (bottom ? -off : off)));
            (void)along;
        }
        // sweep in from the corner (0-450ms), two soft pulses, fade out
        double sweep = (std::min)(1.0, t / 450.0);
        double pulse = t < 450 ? 1.0 : 0.55 + 0.45 * std::cos((t - 450) / 700.0 * 2 * kPi);
        double fade = t > kGlyphMs - 400 ? (std::max)(0.0, (kGlyphMs - (double)t) / 400.0) : 1.0;
        for (size_t k = 0; k < pts.size(); k++) {
            int idx = (int)((k + 1) / 2);  // distance from the corner
            double lit = sweep * N - idx;
            if (lit <= 0) continue;
            double a = (std::min)(1.0, lit) * pulse * fade * (1.0 - idx * 0.06);
            BYTE alpha = (BYTE)(std::clamp(a, 0.0, 1.0) * 255);
            SolidBrush glow(Color((BYTE)(alpha / 4), base.GetR(), base.GetG(), base.GetB()));
            SolidBrush b(Color(alpha, base.GetR(), base.GetG(), base.GetB()));
            REAL gd = dotD * 2.2f;
            FillCell(g, &glow, pts[k].X - gd / 2, pts[k].Y - gd / 2, gd, gd);
            FillCell(g, &b, pts[k].X - dotD / 2, pts[k].Y - dotD / 2, dotD, dotD);
        }
    }
    POINT dst{x, y}, src{0, 0};
    SIZE sz{size, size};
    BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(g_glyphWnd, screen, &dst, &sz, mem, &src, 0, &bf, ULW_ALPHA);
    SetWindowPos(g_glyphWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SelectObject(mem, old);
    DeleteDC(mem);
    DeleteObject(dib);
    ReleaseDC(nullptr, screen);
}

static void GlyphTrigger() {
    HWND ctrl = g_ctrl.load();
    if (!ctrl) return;
    if (!g_glyphWnd) {
        WNDCLASSEXW wc{sizeof(wc)};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kGlyphClass;
        RegisterClassExW(&wc);
        g_glyphWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |
                                         WS_EX_NOACTIVATE,
                                     kGlyphClass, L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, nullptr, nullptr);
    }
    g_glyphStart = GetTickCount64();
    GlyphFrame();
    SetTimer(ctrl, kTimerGlyph, 30, nullptr);
}

static void GlyphStep(HWND ctrl) {
    if (GetTickCount64() - g_glyphStart >= (ULONGLONG)kGlyphMs) {
        KillTimer(ctrl, kTimerGlyph);
        if (g_glyphWnd) ShowWindow(g_glyphWnd, SW_HIDE);
        return;
    }
    GlyphFrame();
}

static void CALLBACK NotificationWinEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG idObject, LONG idChild, DWORD,
                                          DWORD) {
    if (!g_s.glyph || idObject != OBJID_WINDOW || idChild != CHILDID_SELF || !hwnd) return;
    wchar_t title[128] = L"";
    GetWindowTextW(hwnd, title, 128);
    std::wstring t = title;
    for (auto& c : t) c = towlower(c);
    if (t.find(L"new notification") == std::wstring::npos) return;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!proc) return;
    wchar_t path[MAX_PATH] = L"";
    DWORD len = MAX_PATH;
    bool shell = false;
    if (QueryFullProcessImageNameW(proc, 0, path, &len)) {
        std::wstring p = path;
        for (auto& c : p) c = towlower(c);
        shell = p.find(L"shellexperiencehost.exe") != std::wstring::npos || p.find(L"shellhost.exe") != std::wstring::npos;
    }
    CloseHandle(proc);
    if (!shell) return;
    ULONGLONG now = GetTickCount64();
    if (now - g_glyphLastTrigger < 1500) return;
    g_glyphLastTrigger = now;
    GlyphTrigger();
}

static LRESULT CALLBACK WidgetProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Widget* wd = (Widget*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_LBUTTONDOWN:
            if (!wd) break;
            SetCapture(hwnd);
            wd->pressed = true;
            wd->dragging = false;
            GetCursorPos(&wd->pressCursor);
            wd->pressPos = {wd->x, wd->y};
            return 0;
        case WM_MOUSEMOVE:
            if (wd && wd->pressed && !g_s.locked) {
                POINT p;
                GetCursorPos(&p);
                int dx = p.x - wd->pressCursor.x, dy = p.y - wd->pressCursor.y;
                if (!wd->dragging && (abs(dx) > 4 || abs(dy) > 4)) wd->dragging = true;
                if (wd->dragging) {
                    int snap = (std::max)(1, (int)(4 * PixelScale()));
                    int nx = (wd->pressPos.x + dx) / snap * snap;
                    int ny = (wd->pressPos.y + dy) / snap * snap;
                    MoveWidget(wd, nx, ny);
                }
            }
            return 0;
        case WM_LBUTTONUP:
            if (wd && wd->pressed) {
                bool wasDrag = wd->dragging;
                wd->pressed = wd->dragging = false;
                ReleaseCapture();
                wd->clickX = (short)LOWORD(lp);
                wd->clickY = (short)HIWORD(lp);
                if (wasDrag) {
                    SavePos(wd);
                } else if (wd->OnClick(false)) {
                    Render(wd, MakeCtx());
                }
            }
            return 0;
        case WM_CAPTURECHANGED:
            if (wd) {
                if (wd->dragging) SavePos(wd);
                wd->pressed = wd->dragging = false;
            }
            return 0;
        case WM_RBUTTONUP:
            if (wd) {
                if (wd->OnClick(true)) Render(wd, MakeCtx());
                else ShowMenu(wd);
            }
            return 0;
        case WM_SETCURSOR:
            SetCursor(LoadCursor(nullptr, (g_s.locked || !wd) ? IDC_ARROW : IDC_SIZEALL));
            if (wd && (!wcscmp(wd->id, L"focus") || !wcscmp(wd->id, L"stopwatch") || !wcscmp(wd->id, L"nowplaying") ||
                       !wcscmp(wd->id, L"quote")))
                SetCursor(LoadCursor(nullptr, IDC_HAND));
            return TRUE;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static HWND FindDesktopDefView() {
    HWND progman = FindWindowW(L"Progman", nullptr);
    HWND dv = progman ? FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr) : nullptr;
    if (dv) return dv;
    HWND worker = nullptr;
    while ((worker = FindWindowExW(nullptr, worker, L"WorkerW", nullptr))) {
        DWORD pid = 0;
        GetWindowThreadProcessId(worker, &pid);
        if (pid != GetCurrentProcessId()) continue;
        dv = FindWindowExW(worker, nullptr, L"SHELLDLL_DefView", nullptr);
        if (dv) return dv;
    }
    return nullptr;
}

static void DestroyWidgets() {
    for (auto& w : g_widgets) {
        if (w->hwnd && IsWindow(w->hwnd)) DestroyWindow(w->hwnd);
        w->hwnd = nullptr;
        ReleaseSurface(w.get());
    }
    g_widgets.clear();
}

static void CreateWidgets() {
    DestroyWidgets();
    g_parent = FindDesktopDefView();

    std::wstring font = g_s.labelFont.empty() ? L"Share Tech Mono" : g_s.labelFont;
    g_family = std::make_unique<FontFamily>(font.c_str());
    if (g_family->GetLastStatus() != Ok) g_family = std::make_unique<FontFamily>(L"Consolas");

    if (g_s.clock) g_widgets.push_back(std::make_unique<ClockWidget>());
    if (g_s.analog) g_widgets.push_back(std::make_unique<AnalogWidget>());
    if (g_s.calendar) g_widgets.push_back(std::make_unique<CalendarWidget>());
    if (g_s.battery) g_widgets.push_back(std::make_unique<BatteryWidget>());
    if (g_s.weather) g_widgets.push_back(std::make_unique<WeatherWidget>());
    if (g_s.system) g_widgets.push_back(std::make_unique<SystemWidget>());
    if (g_s.focus) g_widgets.push_back(std::make_unique<FocusWidget>());
    if (g_s.year) g_widgets.push_back(std::make_unique<YearWidget>());
    if (g_s.quote) g_widgets.push_back(std::make_unique<QuoteWidget>());
    if (g_s.nowPlaying) g_widgets.push_back(std::make_unique<NowPlayingWidget>());
    if (g_s.forecast) g_widgets.push_back(std::make_unique<ForecastWidget>());
    if (g_s.stopwatch) g_widgets.push_back(std::make_unique<StopwatchWidget>());

    REAL s = PixelScale();
    int U = (int)(150 * s), G = (int)(12 * s);
    Ctx c = MakeCtx();
    for (auto& w : g_widgets) {
        w->w = w->unitsW * U + (w->unitsW - 1) * G;
        w->h = w->unitsH * U + (w->unitsH - 1) * G;
        DWORD style = g_parent ? WS_CHILD | WS_CLIPSIBLINGS : WS_POPUP;
        DWORD ex = WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY |
                   (g_parent ? 0 : WS_EX_TOOLWINDOW);
        w->hwnd = CreateWindowExW(ex, kWidgetClass, L"", style, 0, 0, w->w, w->h, g_parent,
                                  nullptr, nullptr, nullptr);
        if (!w->hwnd) {
            Wh_Log(L"CreateWindowEx failed: %lu", GetLastError());
            continue;
        }
        SetWindowLongPtrW(w->hwnd, GWLP_USERDATA, (LONG_PTR)w.get());
        PlaceWidget(w.get(), s, false);
        Render(w.get(), c);
        SetWindowPos(w->hwnd, g_parent ? HWND_TOP : HWND_BOTTOM, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

static void ScheduleTick(HWND ctrl) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    bool fast = false;
    for (auto& w : g_widgets)
        if (!wcscmp(w->id, L"focus") && ((FocusWidget*)w.get())->state == FocusWidget::Done) fast = true;
    bool stopwatch = false;
    for (auto& w : g_widgets)
        if (!wcscmp(w->id, L"stopwatch") && ((StopwatchWidget*)w.get())->running) stopwatch = true;
    UINT delay = stopwatch ? 100 : fast ? 500 : 1000 - st.wMilliseconds + 15;
    SetTimer(ctrl, kTimerTick, delay, nullptr);
}

static bool WidgetsAlive() {
    if (g_parent && !IsWindow(g_parent)) return false;
    for (auto& w : g_widgets)
        if (!w->hwnd || !IsWindow(w->hwnd)) return false;
    return true;
}

// Windows' theme service resets the sound scheme to "Windows Default" when it reapplies the
// theme (sign-in, unlock, light/dark switch). If our scheme is installed (install-sounds.ps1
// creates AppEvents\Schemes\Names\<scheme>; -Restore removes it), copy it back into .Current.
static void RestoreSoundScheme() {
    if (g_s.soundScheme.empty() || g_s.soundScheme.size() > 60) return;
    const wchar_t* scheme = g_s.soundScheme.c_str();
    HKEY names;
    std::wstring namesPath = L"AppEvents\\Schemes\\Names\\" + g_s.soundScheme;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, namesPath.c_str(), 0, KEY_READ, &names) != ERROR_SUCCESS) return;
    RegCloseKey(names);
    HKEY apps;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"AppEvents\\Schemes\\Apps", 0, KEY_READ | KEY_WRITE, &apps) != ERROR_SUCCESS)
        return;
    wchar_t app[256], ev[256], val[MAX_PATH * 2], cur[MAX_PATH * 2];
    for (DWORD i = 0;; i++) {
        DWORD n = ARRAYSIZE(app);
        if (RegEnumKeyExW(apps, i, app, &n, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        HKEY appKey;
        if (RegOpenKeyExW(apps, app, 0, KEY_READ | KEY_WRITE, &appKey) != ERROR_SUCCESS) continue;
        for (DWORD j = 0;; j++) {
            DWORD m = ARRAYSIZE(ev);
            if (RegEnumKeyExW(appKey, j, ev, &m, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
            std::wstring src = std::wstring(ev) + L"\\" + scheme;
            DWORD vb = sizeof(val);
            if (RegGetValueW(appKey, src.c_str(), nullptr, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                             nullptr, val, &vb) != ERROR_SUCCESS)
                continue;
            std::wstring dst = std::wstring(ev) + L"\\.Current";
            DWORD cbCur = sizeof(cur);
            if (RegGetValueW(appKey, dst.c_str(), nullptr, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                             nullptr, cur, &cbCur) == ERROR_SUCCESS && !_wcsicmp(cur, val))
                continue;
            HKEY dk;
            if (RegCreateKeyExW(appKey, dst.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &dk, nullptr) == ERROR_SUCCESS) {
                RegSetValueExW(dk, nullptr, 0, REG_SZ, (const BYTE*)val, (DWORD)((wcslen(val) + 1) * sizeof(wchar_t)));
                RegCloseKey(dk);
            }
        }
        RegCloseKey(appKey);
    }
    RegCloseKey(apps);
    RegSetKeyValueW(HKEY_CURRENT_USER, L"AppEvents\\Schemes", nullptr, REG_SZ, scheme,
                    (DWORD)((wcslen(scheme) + 1) * sizeof(wchar_t)));
}

static LRESULT CALLBACK CtrlProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_TIMER:
            if (wp == kTimerGlyph) {
                GlyphStep(hwnd);
                return 0;
            }
            if (wp == kTimerCursor) {
                // reload the user's cursors and sound scheme; repeat with growing gaps
                SystemParametersInfoW(SPI_SETCURSORS, 0, nullptr, SPIF_SENDCHANGE);
                RestoreSoundScheme();
                static const UINT next[] = {2500, 4000, 8000};
                if (g_cursorPass < 3) SetTimer(hwnd, kTimerCursor, next[g_cursorPass++], nullptr);
                else KillTimer(hwnd, kTimerCursor);
                return 0;
            }
            if (wp == kTimerTick) {
                if (!WidgetsAlive()) {
                    // Desktop window was recreated (e.g. wallpaper/desktop refresh).
                    for (auto& w : g_widgets) w->hwnd = nullptr;
                    if (FindDesktopDefView()) CreateWidgets();
                }
                for (auto& w : g_widgets) w->Tick();
                RenderAll();
                ScheduleTick(hwnd);
            }
            return 0;
        case WM_APP_WEATHER:
            for (auto& w : g_widgets)
                if (!wcscmp(w->id, L"weather") || !wcscmp(w->id, L"forecast")) Render(w.get(), MakeCtx());
            return 0;
        case WM_APP_MEDIA:
            for (auto& w : g_widgets)
                if (!wcscmp(w->id, L"nowplaying")) Render(w.get(), MakeCtx());
            return 0;
        case WM_WTSSESSION_CHANGE:
            if (g_s.keepCursors && (wp == WTS_SESSION_UNLOCK || wp == WTS_SESSION_LOGON ||
                                    wp == WTS_CONSOLE_CONNECT || wp == WTS_REMOTE_CONNECT)) {
                g_cursorPass = 0;
                SetTimer(hwnd, kTimerCursor, 1500, nullptr);
            }
            return 0;
        case WM_SETTINGCHANGE:
            // light/dark switch or a theme being applied
            if (g_s.keepCursors && lp && !IsBadStringPtrW((LPCWSTR)lp, 64) &&
                (!wcscmp((LPCWSTR)lp, L"ImmersiveColorSet") || !wcscmp((LPCWSTR)lp, L"WindowsThemeElement"))) {
                g_cursorPass = 0;
                SetTimer(hwnd, kTimerCursor, 1500, nullptr);
            }
            break;
        case WM_THEMECHANGED:
            if (g_s.keepCursors) {
                g_cursorPass = 0;
                SetTimer(hwnd, kTimerCursor, 1500, nullptr);
            }
            break;
        case WM_DISPLAYCHANGE:
        case WM_DPICHANGED:
            CreateWidgets();
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            KillTimer(hwnd, kTimerTick);
            KillTimer(hwnd, kTimerGlyph);
            KillTimer(hwnd, kTimerCursor);
            WTSUnRegisterSessionNotification(hwnd);
            if (g_glyphWnd) {
                DestroyWindow(g_glyphWnd);
                g_glyphWnd = nullptr;
            }
            DestroyWidgets();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static DWORD WINAPI UiThreadProc(LPVOID) {
    // Only the shell's explorer.exe hosts the desktop. Wait for it to appear.
    for (int i = 0;; i++) {
        HWND progman = FindWindowW(L"Progman", nullptr);
        if (progman) {
            DWORD pid = 0;
            GetWindowThreadProcessId(progman, &pid);
            if (pid != GetCurrentProcessId()) return 0;  // a File Explorer process
            if (FindDesktopDefView()) break;
        }
        if (i > 240) return 0;  // ~2 minutes
        if (WaitForSingleObject(g_stopEvent, 500) == WAIT_OBJECT_0) return 0;
    }

    GdiplusStartupInput gsi;
    ULONG_PTR token = 0;
    if (GdiplusStartup(&token, &gsi, nullptr) != Ok) return 0;

    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = WidgetProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kWidgetClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);
    WNDCLASSEXW cc{sizeof(cc)};
    cc.lpfnWndProc = CtrlProc;
    cc.hInstance = GetModuleHandleW(nullptr);
    cc.lpszClassName = kCtrlClass;
    RegisterClassExW(&cc);

    HWND ctrl = CreateWindowExW(WS_EX_TOOLWINDOW, kCtrlClass, L"GridlineDesktop", WS_POPUP, 0, 0, 0, 0,
                                nullptr, nullptr, nullptr, nullptr);
    if (ctrl) {
        g_ctrl = ctrl;
        if (g_s.weather || g_s.forecast)
            g_weatherThread = CreateThread(nullptr, 0, WeatherThreadProc, nullptr, 0, nullptr);
        if (g_s.nowPlaying)
            g_mediaThread = CreateThread(nullptr, 0, MediaThreadProc, nullptr, 0, nullptr);
        if (g_s.glyph)
            g_winEventHook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, NotificationWinEvent, 0, 0,
                                             WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        CreateWidgets();
        ScheduleTick(ctrl);
        if (g_s.keepCursors) {
            WTSRegisterSessionNotification(ctrl, NOTIFY_FOR_THIS_SESSION);
            g_cursorPass = 0;
            SetTimer(ctrl, kTimerCursor, 3000, nullptr);
        }
        if (WaitForSingleObject(g_stopEvent, 0) == WAIT_OBJECT_0) PostMessage(ctrl, WM_CLOSE, 0, 0);
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) {
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
        g_ctrl = nullptr;
        if (g_winEventHook) {
            UnhookWinEvent(g_winEventHook);
            g_winEventHook = nullptr;
        }
    }

    g_family.reset();
    UnregisterClassW(kGlyphClass, GetModuleHandleW(nullptr));
    UnregisterClassW(kWidgetClass, GetModuleHandleW(nullptr));
    UnregisterClassW(kCtrlClass, GetModuleHandleW(nullptr));
    GdiplusShutdown(token);
    return 0;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

static void Start() {
    LoadSettings();
    {
        std::lock_guard<std::mutex> lock(g_weatherMutex);
        g_weather = WeatherData{};
    }
    ResetEvent(g_stopEvent);
    g_uiThread = CreateThread(nullptr, 0, UiThreadProc, nullptr, 0, nullptr);
}

static void Stop() {
    SetEvent(g_stopEvent);
    if (HWND h = g_ctrl.load()) PostMessage(h, WM_CLOSE, 0, 0);
    if (g_uiThread) {
        WaitForSingleObject(g_uiThread, INFINITE);
        CloseHandle(g_uiThread);
        g_uiThread = nullptr;
    }
    if (g_weatherThread) {
        WaitForSingleObject(g_weatherThread, INFINITE);
        CloseHandle(g_weatherThread);
        g_weatherThread = nullptr;
    }
    if (g_mediaThread) {
        WaitForSingleObject(g_mediaThread, INFINITE);
        CloseHandle(g_mediaThread);
        g_mediaThread = nullptr;
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L"Gridline Desktop: init");
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_weatherWake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_mediaWake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_stopEvent || !g_weatherWake || !g_mediaWake) return FALSE;
    Start();
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Gridline Desktop: uninit");
    Stop();
    CloseHandle(g_stopEvent);
    CloseHandle(g_weatherWake);
    CloseHandle(g_mediaWake);
    g_stopEvent = g_weatherWake = g_mediaWake = nullptr;
}

void Wh_ModSettingsChanged() {
    Stop();
    Start();
}
