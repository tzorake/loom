#include "tzwin32window.hpp"
#include "tzwin32window_p.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <mutex>
#include <stdexcept>

static const wchar_t *kWindowClass = L"TzWindow";
static std::once_flag gClassRegistration;

static void registerWindowClass()
{
    std::call_once(gClassRegistration, []() {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = TzWin32WindowPrivate::wndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wc.lpszClassName = kWindowClass;
        RegisterClassExW(&wc);
    });
}

static Key vkToKey(WPARAM vk)
{
    switch (vk) {
    case VK_ESCAPE:
        return Key::Escape;
    case VK_RETURN:
        return Key::Enter;
    case VK_TAB:
        return Key::Tab;
    case VK_BACK:
        return Key::Backspace;
    case VK_UP:
        return Key::Up;
    case VK_DOWN:
        return Key::Down;
    case VK_LEFT:
        return Key::Left;
    case VK_RIGHT:
        return Key::Right;
    case VK_HOME:
        return Key::Home;
    case VK_END:
        return Key::End;
    case VK_PRIOR:
        return Key::PageUp;
    case VK_NEXT:
        return Key::PageDown;
    case VK_F1:
        return Key::F1;
    case VK_F2:
        return Key::F2;
    case VK_F3:
        return Key::F3;
    case VK_F4:
        return Key::F4;
    case VK_F5:
        return Key::F5;
    case VK_F6:
        return Key::F6;
    case VK_F7:
        return Key::F7;
    case VK_F8:
        return Key::F8;
    case VK_F9:
        return Key::F9;
    case VK_F10:
        return Key::F10;
    case VK_F11:
        return Key::F11;
    case VK_F12:
        return Key::F12;
    default:
        return Key::Unknown;
    }
}

static KeyModifiers currentModifiers()
{
    KeyModifiers mods;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= KeyModifier::Shift;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= KeyModifier::Ctrl;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= KeyModifier::Alt;
    return mods;
}

static KeyModifiers mouseButtonModifiers(WPARAM wParam)
{
    KeyModifiers mods;
    if (wParam & MK_SHIFT)
        mods |= KeyModifier::Shift;
    if (wParam & MK_CONTROL)
        mods |= KeyModifier::Ctrl;
    return mods;
}

LRESULT CALLBACK TzWin32WindowPrivate::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TzWin32WindowPrivate *d = nullptr;

    if (msg == WM_NCCREATE) {
        auto *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
        d = static_cast<TzWin32WindowPrivate *>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(d));
        d->hwnd = hwnd;
    } else {
        d = reinterpret_cast<TzWin32WindowPrivate *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!d)
        return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        d->onPaint();
        return 0;

    case WM_CLOSE:
        d->onClose();
        return 0;

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
            d->onSize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        d->onKey(wParam, true, lParam);
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        d->onKey(wParam, false, lParam);
        return 0;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        d->onMouseButton(msg, wParam, lParam);
        return 0;

    case WM_MOUSEMOVE:
        d->onMouseMove(lParam, wParam);
        return 0;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        d->onMouseWheel(msg, wParam, lParam);
        return 0;

    case WM_DESTROY:
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        return 0;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

TzWin32WindowPrivate::TzWin32WindowPrivate(int width, int height, TzPlatformWindow *owner)
    : owner(owner)
    , windowWidth(width)
    , windowHeight(height)
{
    registerWindowClass();

    DWORD style = WS_OVERLAPPEDWINDOW;

    // Calculate total window size from desired client area
    RECT rc = {0, 0, width, height};
    AdjustWindowRect(&rc, style, FALSE);
    int totalWidth = rc.right - rc.left;
    int totalHeight = rc.bottom - rc.top;

    // Center on primary monitor
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - totalWidth) / 2;
    int y = (screenH - totalHeight) / 2;

    hwnd = CreateWindowExW(0, kWindowClass, L"", style, x, y, totalWidth, totalHeight, nullptr,
                           nullptr, GetModuleHandle(nullptr), this);

    if (!hwnd)
        throw std::runtime_error("CreateWindowExW failed");
}

TzWin32WindowPrivate::~TzWin32WindowPrivate()
{
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void TzWin32WindowPrivate::setTitle(const std::string &title)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, title.data(), static_cast<int>(title.size()), nullptr,
                                  0);
    if (len <= 0) {
        SetWindowTextW(hwnd, L"");
        return;
    }
    std::wstring wTitle(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, title.data(), static_cast<int>(title.size()), wTitle.data(),
                        len);
    SetWindowTextW(hwnd, wTitle.data());
}

void TzWin32WindowPrivate::show()
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void TzWin32WindowPrivate::hide()
{
    ShowWindow(hwnd, SW_HIDE);
}

void TzWin32WindowPrivate::render(const std::vector<uint32_t> &newPixels, int width, int height)
{
    {
        std::lock_guard lock(pixelMutex);
        pixels = newPixels;
        pixelWidth = width;
        pixelHeight = height;
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}

void TzWin32WindowPrivate::onClose()
{
    TzCloseEvent event;
    TzCoreApplication::sendEvent(owner, &event);
    if (event.isAccepted())
        ShowWindow(hwnd, SW_HIDE);
}

void TzWin32WindowPrivate::onSize(int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    // Use sendEvent (synchronous) so the scene repaints before WM_PAINT fires.
    // postEvent would delay until the event loop resumes, which never happens
    // during Windows's live-resize modal loop.
    TzResizeEvent re(windowWidth, windowHeight);
    TzCoreApplication::sendEvent(owner, &re);
}

void TzWin32WindowPrivate::onPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    {
        std::lock_guard lock(pixelMutex);

        if (!pixels.empty() && pixelWidth > 0 && pixelHeight > 0) {
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = pixelWidth;
            // Negative height = top-down DIB, matching the painter's top-down storage
            bmi.bmiHeader.biHeight = -pixelHeight;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            StretchDIBits(hdc, 0, 0, windowWidth, windowHeight, 0, 0, pixelWidth, pixelHeight,
                          pixels.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
        } else {
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        }
    }

    EndPaint(hwnd, &ps);
}

void TzWin32WindowPrivate::onKey(WPARAM vk, bool pressed, LPARAM lParam)
{
    Key key = vkToKey(vk);
    KeyModifiers mods = currentModifiers();

    std::string utf8;
    if (key == Key::Unknown) {
        BYTE keyState[256];
        GetKeyboardState(keyState);
        WCHAR buf[4] = {};
        UINT scanCode = static_cast<UINT>((lParam >> 16) & 0xFF);
        int n = ToUnicodeEx(static_cast<UINT>(vk), scanCode, keyState, buf, 4, 0,
                            GetKeyboardLayout(0));
        if (n > 0) {
            int len = WideCharToMultiByte(CP_UTF8, 0, buf, n, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                utf8.resize(len);
                WideCharToMultiByte(CP_UTF8, 0, buf, n, utf8.data(), len, nullptr, nullptr);
            }
        }
    }

    TzCoreApplication::postEvent(owner,
                                 new TzKeyEvent(pressed ? TzEvent::KeyPress : TzEvent::KeyRelease,
                                                key, mods, std::move(utf8)));
}

void TzWin32WindowPrivate::onMouseButton(UINT msg, WPARAM wParam, LPARAM lParam)
{
    double x = static_cast<double>(GET_X_LPARAM(lParam));
    double y = static_cast<double>(GET_Y_LPARAM(lParam));

    MouseButton button = MouseButton::None;
    TzEvent::Type type = TzEvent::MouseButtonPress;

    switch (msg) {
    case WM_LBUTTONDOWN:
        button = MouseButton::Left;
        type = TzEvent::MouseButtonPress;
        break;
    case WM_LBUTTONUP:
        button = MouseButton::Left;
        type = TzEvent::MouseButtonRelease;
        break;
    case WM_RBUTTONDOWN:
        button = MouseButton::Right;
        type = TzEvent::MouseButtonPress;
        break;
    case WM_RBUTTONUP:
        button = MouseButton::Right;
        type = TzEvent::MouseButtonRelease;
        break;
    case WM_MBUTTONDOWN:
        button = MouseButton::Middle;
        type = TzEvent::MouseButtonPress;
        break;
    case WM_MBUTTONUP:
        button = MouseButton::Middle;
        type = TzEvent::MouseButtonRelease;
        break;
    default:
        break;
    }

    TzCoreApplication::postEvent(owner, new TzMouseEvent(type, button, x, y,
                                                         mouseButtonModifiers(wParam)));
}

void TzWin32WindowPrivate::onMouseMove(LPARAM lParam, WPARAM wParam)
{
    double x = static_cast<double>(GET_X_LPARAM(lParam));
    double y = static_cast<double>(GET_Y_LPARAM(lParam));
    TzCoreApplication::postEvent(owner, new TzMouseEvent(TzEvent::MouseMove, MouseButton::None, x,
                                                         y, mouseButtonModifiers(wParam)));
}

void TzWin32WindowPrivate::onMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam)
{
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(hwnd, &pt);

    double delta = static_cast<double>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
    double scrollDx = (msg == WM_MOUSEHWHEEL) ? delta : 0.0;
    double scrollDy = (msg == WM_MOUSEWHEEL) ? delta : 0.0;

    TzCoreApplication::postEvent(owner, new TzMouseEvent(TzEvent::MouseScroll, MouseButton::None,
                                                         static_cast<double>(pt.x),
                                                         static_cast<double>(pt.y),
                                                         mouseButtonModifiers(wParam), scrollDx,
                                                         scrollDy));
}

// ── TzWin32Window public API ────────────────────────────────────────────────

TzWin32Window::TzWin32Window(int width, int height)
    : d_ptr(new TzWin32WindowPrivate(width, height, this))
{}

TzWin32Window::~TzWin32Window() = default;

void TzWin32Window::setTitle(const std::string &title)
{
    d_ptr->setTitle(title);
}

void TzWin32Window::show()
{
    d_ptr->show();
    TzResizeEvent re(d_ptr->windowWidth, d_ptr->windowHeight);
    TzCoreApplication::sendEvent(this, &re);
}

void TzWin32Window::hide()
{
    d_ptr->hide();
}

void TzWin32Window::render(const std::vector<uint32_t> &pixels, int width, int height)
{
    d_ptr->render(pixels, width, height);
}

TzNativeWindowHandle TzWin32Window::nativeWindowHandle() const
{
    TzNativeWindowHandle h;
    h.hwnd      = static_cast<void *>(d_ptr->hwnd);
    h.hinstance = nullptr; // caller can use GetModuleHandle(nullptr) if needed
    return h;
}
