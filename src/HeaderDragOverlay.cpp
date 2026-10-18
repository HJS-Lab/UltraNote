#include "HeaderDragOverlay.h"
#include "Utils.h"

namespace {
    const wchar_t kClassName[] = L"UltraNoteHeaderDragOverlay";
    bool g_classRegistered = false;

    // Premultiplied BGRA, opaque red
    constexpr BYTE kB = 0, kG = 0, kR = 210, kA = 255;

    // Effective DPI at a screen point (MonitorDpi in Utils.h; 96 on failure).
    UINT DpiAtPoint(int xScreen, int yScreen) {
        POINT pt = { xScreen, yScreen };
        return MonitorDpi(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST));
    }
}

HeaderDragOverlay::HeaderDragOverlay(HINSTANCE hInst) : m_hInst(hInst) {
    EnsureClassRegistered();
    m_hTop    = CreateLayeredWindow(m_arrowW,  m_arrowH);
    m_hBottom = CreateLayeredWindow(m_arrowW,  m_arrowH);
    m_hStripe = CreateLayeredWindow(m_stripeW, 1);       // height/DPI grown on first Show
    if (m_hTop)    PaintArrow(m_hTop,    /*pointingDown=*/true);
    if (m_hBottom) PaintArrow(m_hBottom, /*pointingDown=*/false);
}

HeaderDragOverlay::~HeaderDragOverlay() {
    if (m_hTop)    DestroyWindow(m_hTop);
    if (m_hBottom) DestroyWindow(m_hBottom);
    if (m_hStripe) DestroyWindow(m_hStripe);
}

void HeaderDragOverlay::EnsureClassRegistered() {
    if (g_classRegistered) return;
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = m_hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    RegisterClassW(&wc);
    g_classRegistered = true;
}

LRESULT CALLBACK HeaderDragOverlay::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND HeaderDragOverlay::CreateLayeredWindow(int w, int h) {
    return CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kClassName, nullptr,
        WS_POPUP, 0, 0, w, h,
        nullptr, nullptr, m_hInst, nullptr
    );
}

void HeaderDragOverlay::PaintArrow(HWND hwnd, bool pointingDown) {
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem    = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = m_arrowW;
    bmi.bmiHeader.biHeight      = -m_arrowH; // top-down
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    if (!hBmp || !pBits) {   // GDI exhaustion: skip the marker rather than write to null
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        return;
    }
    HBITMAP hOldBmp = static_cast<HBITMAP>(SelectObject(hdcMem, hBmp));

    BYTE* px = static_cast<BYTE*>(pBits);
    ZeroMemory(px, static_cast<size_t>(m_arrowW) * m_arrowH * 4);

    // Triangle taper scaled to the marker height so the arrow keeps its shape at
    // any DPI (1px/row at 96 dpi, proportionally more on high-DPI monitors).
    for (int y = 0; y < m_arrowH; ++y) {
        int row0 = pointingDown ? y : (m_arrowH - 1 - y);
        int inset = MulDiv(row0, m_arrowW / 2, (m_arrowH > 1 ? m_arrowH - 1 : 1));
        int xStart = inset;
        int xEnd   = m_arrowW - 1 - inset;
        if (xStart > xEnd) continue;
        BYTE* row = px + y * m_arrowW * 4;
        for (int x = xStart; x <= xEnd; ++x) {
            row[x * 4 + 0] = kB;
            row[x * 4 + 1] = kG;
            row[x * 4 + 2] = kR;
            row[x * 4 + 3] = kA;
        }
    }

    SIZE sz = { m_arrowW, m_arrowH };
    POINT ptSrc = { 0, 0 };
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd, hdcScreen, nullptr, &sz, hdcMem, &ptSrc, 0, &bf, ULW_ALPHA);

    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

void HeaderDragOverlay::PaintStripe(HWND hwnd, int height) {
    if (height <= 0) return;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem    = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = m_stripeW;
    bmi.bmiHeader.biHeight      = -height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    if (!hBmp || !pBits) {   // see PaintArrow
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        return;
    }
    HBITMAP hOldBmp = static_cast<HBITMAP>(SelectObject(hdcMem, hBmp));

    BYTE* px = static_cast<BYTE*>(pBits);
    // Full rectangle, opaque red — fully covers Windows' default 1px insertion line.
    for (int y = 0; y < height; ++y) {
        BYTE* row = px + y * m_stripeW * 4;
        for (int x = 0; x < m_stripeW; ++x) {
            row[x * 4 + 0] = kB;
            row[x * 4 + 1] = kG;
            row[x * 4 + 2] = kR;
            row[x * 4 + 3] = kA;
        }
    }

    SIZE sz = { m_stripeW, height };
    POINT ptSrc = { 0, 0 };
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd, hdcScreen, nullptr, &sz, hdcMem, &ptSrc, 0, &bf, ULW_ALPHA);

    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

void HeaderDragOverlay::EnsureDpiScale(int xScreen, int yTopScreen) {
    UINT dpi = DpiAtPoint(xScreen, yTopScreen);
    if (dpi == m_dpi) return;        // unchanged since last Show
    m_dpi = dpi;
    m_arrowW  = MulDiv(kArrowW,  dpi, 96);
    m_arrowH  = MulDiv(kArrowH,  dpi, 96);
    m_stripeW = MulDiv(kStripeW, dpi, 96);
    // Re-render the fixed-shape arrows at the new size; force the stripe to repaint
    // at the new width on the Show below (m_stripeHeight = 0 invalidates its cache).
    PaintArrow(m_hTop,    /*pointingDown=*/true);
    PaintArrow(m_hBottom, /*pointingDown=*/false);
    m_stripeHeight = 0;
}

void HeaderDragOverlay::Show(int xScreen, int yTopScreen, int yBottomScreen) {
    if (!m_hTop || !m_hBottom || !m_hStripe) return;

    EnsureDpiScale(xScreen, yTopScreen);   // rescale markers for the target monitor

    int stripeH = yBottomScreen - yTopScreen;
    if (stripeH < 1) stripeH = 1;
    if (stripeH != m_stripeHeight) {
        PaintStripe(m_hStripe, stripeH);
        m_stripeHeight = stripeH;
    }

    // Tip centers: (xScreen, yTopScreen) for upper, (xScreen, yBottomScreen) for lower
    int topX = xScreen - m_arrowW / 2;
    int topY = yTopScreen - m_arrowH;
    int botX = xScreen - m_arrowW / 2;
    int botY = yBottomScreen;
    int stpX = xScreen - m_stripeW / 2;
    int stpY = yTopScreen;

    SetWindowPos(m_hStripe, HWND_TOPMOST, stpX, stpY, m_stripeW, stripeH,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SetWindowPos(m_hTop,    HWND_TOPMOST, topX, topY, m_arrowW, m_arrowH,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SetWindowPos(m_hBottom, HWND_TOPMOST, botX, botY, m_arrowW, m_arrowH,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void HeaderDragOverlay::Hide() {
    if (m_hTop)    ShowWindow(m_hTop,    SW_HIDE);
    if (m_hBottom) ShowWindow(m_hBottom, SW_HIDE);
    if (m_hStripe) ShowWindow(m_hStripe, SW_HIDE);
}
