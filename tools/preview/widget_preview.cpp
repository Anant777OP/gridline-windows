// Renders every Gridline Desktop widget into a PNG, for README previews.
// It includes the real mod source and replaces the few Windhawk API calls with
// fixed settings, so the pictures show exactly what the mod draws.
//
// build: see build_preview.ps1
// usage: widget_preview.exe out.png [dark|light] [solid|outline|none] [background.png]

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <map>
#include <string>

static std::wstring g_theme = L"dark", g_card = L"solid";

static std::wstring Fmt(PCWSTR fmt, va_list ap) {
    wchar_t buf[256];
    vswprintf(buf, 256, fmt, ap);
    return buf;
}

int Wh_GetIntSetting(PCWSTR name, ...) {
    va_list ap; va_start(ap, name); std::wstring n = Fmt(name, ap); va_end(ap);
    static const std::map<std::wstring, int> v = {
        {L"cardOpacity", 92}, {L"scale", 100}, {L"locked", 1}, {L"unlitDots", 0},
        {L"clock.use24h", 1}, {L"clock.showSeconds", 0}, {L"clock.redColon", 1},
        {L"weather.fahrenheit", 0}, {L"weather.intervalMinutes", 20}, {L"focus.minutes", 25},
        {L"keepCursors", 0}, {L"glyph.enabled", 0},
    };
    auto it = v.find(n);
    if (it != v.end()) return it->second;
    return n.size() > 8 && n.substr(n.size() - 8) == L".enabled" ? 1 : 0;
}

PCWSTR Wh_GetStringSetting(PCWSTR name, ...) {
    va_list ap; va_start(ap, name); std::wstring n = Fmt(name, ap); va_end(ap);
    std::wstring r;
    if (n == L"theme") r = g_theme;
    else if (n == L"cardStyle") r = g_card;
    else if (n == L"accentColor") r = L"#FF8A1F";
    else if (n == L"labelFont") r = L"Share Tech Mono";
    else if (n == L"weather.city") r = L"GRID";
    else if (n == L"quote.mode") r = L"daily";
    wchar_t* out = (wchar_t*)malloc((r.size() + 1) * sizeof(wchar_t));
    wcscpy(out, r.c_str());
    return out;
}
void Wh_FreeStringSetting(PCWSTR s) { free((void*)s); }
int Wh_GetIntValue(PCWSTR, int def) { return def; }
BOOL Wh_SetIntValue(PCWSTR, int) { return TRUE; }
BOOL Wh_DeleteValue(PCWSTR) { return TRUE; }
#define Wh_Log(...) ((void)0)

#define Wh_ModInit Preview_ModInit
#define Wh_ModUninit Preview_ModUninit
#define Wh_ModSettingsChanged Preview_ModSettingsChanged
#include "../../mods/gridline-desktop.wh.cpp"

static int EncoderClsid(const wchar_t* mime, CLSID* clsid) {
    UINT n = 0, size = 0;
    GetImageEncodersSize(&n, &size);
    std::vector<BYTE> buf(size);
    auto* info = (ImageCodecInfo*)buf.data();
    GetImageEncoders(n, size, info);
    for (UINT i = 0; i < n; i++)
        if (!wcscmp(info[i].MimeType, mime)) { *clsid = info[i].Clsid; return 0; }
    return -1;
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) return 1;
    if (argc > 2) g_theme = argv[2];
    if (argc > 3) g_card = argv[3];
    GdiplusStartupInput gsi;
    ULONG_PTR token;
    GdiplusStartup(&token, &gsi, nullptr);
    {
        LoadSettings();
        // use the bundled label font without installing it
        auto* fonts = new PrivateFontCollection();  // freed with GDI+ at exit
        wchar_t exeDir[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, exeDir);
        std::wstring ttf = std::wstring(exeDir) + L"\\fonts\\ShareTechMono-Regular.ttf";
        fonts->AddFontFile(ttf.c_str());
        g_family = std::make_unique<FontFamily>(L"Share Tech Mono", fonts);
        if (g_family->GetLastStatus() != Ok) g_family = std::make_unique<FontFamily>(L"Consolas");
        {   // sample weather so the weather and forecast widgets have something to show
            std::lock_guard<std::mutex> lock(g_weatherMutex);
            g_weather.valid = true; g_weather.temp = 24; g_weather.hi = 28; g_weather.lo = 19;
            g_weather.code = 2; g_weather.isDay = true; g_weather.hours = 6;
            int codes[6] = {0, 1, 2, 3, 61, 95};
            for (int i = 0; i < 6; i++) { g_weather.hHour[i] = 14 + i; g_weather.hTemp[i] = 25 - i; g_weather.hCode[i] = codes[i]; }
        }
        std::vector<std::unique_ptr<Widget>> ws;
        ws.push_back(std::make_unique<ClockWidget>());
        ws.push_back(std::make_unique<AnalogWidget>());
        ws.push_back(std::make_unique<CalendarWidget>());
        ws.push_back(std::make_unique<BatteryWidget>());
        ws.push_back(std::make_unique<WeatherWidget>());
        ws.push_back(std::make_unique<SystemWidget>());
        ws.push_back(std::make_unique<YearWidget>());
        ws.push_back(std::make_unique<FocusWidget>());
        ws.push_back(std::make_unique<QuoteWidget>());
        ws.push_back(std::make_unique<ForecastWidget>());
        ws.push_back(std::make_unique<StopwatchWidget>());
        ws.push_back(std::make_unique<NowPlayingWidget>());

        const int U = 150, G = 12, M = 28, cols = 6;
        // simple row packing: 6 units wide
        struct Slot { Widget* w; int x, y; };
        std::vector<Slot> slots;
        int cx = 0, cy = 0;
        for (auto& w : ws) {
            w->w = w->unitsW * U + (w->unitsW - 1) * G;
            w->h = w->unitsH * U + (w->unitsH - 1) * G;
            if (cx + w->unitsW > cols) { cx = 0; cy++; }
            slots.push_back({w.get(), M + cx * (U + G), M + cy * (U + G)});
            cx += w->unitsW;
        }
        int W = M * 2 + cols * U + (cols - 1) * G, H = M * 2 + (cy + 1) * U + cy * G;
        Bitmap canvas(W, H, PixelFormat32bppARGB);
        Graphics cg(&canvas);
        cg.SetSmoothingMode(SmoothingModeAntiAlias);
        cg.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        bool light = g_theme == L"light";
        if (argc > 4) {
            Bitmap bg(argv[4]);
            cg.DrawImage(&bg, Rect(0, 0, W, H));
        } else {
            cg.Clear(light ? Color(255, 0xE4, 0xF2, 0xF5) : Color(255, 0x01, 0x04, 0x06));
        }
        Ctx c = MakeCtx();
        c.s = 1.0f;
        for (auto& sl : slots) {
            Bitmap bmp(sl.w->w, sl.w->h, PixelFormat32bppARGB);
            Graphics g(&bmp);
            g.SetSmoothingMode(SmoothingModeAntiAlias);
            g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
            g.Clear(Color(0, 0, 0, 0));
            PaintWidget(g, sl.w, c);
            cg.DrawImage(&bmp, sl.x, sl.y);
        }
        CLSID png;
        EncoderClsid(L"image/png", &png);
        canvas.Save(argv[1], &png, nullptr);
    }
    g_family.reset();  // GDI+ objects must go before GdiplusShutdown
    GdiplusShutdown(token);
    return 0;
}
