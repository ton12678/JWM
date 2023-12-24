#pragma once

#include <wayland-client.h>
#include <map>
#include <wayland-cursor.h>
#include "MouseCursor.hh"
#include <wayland-pointer-constraints-unstable-v1-client-protocol.h>

namespace jwm {
    class WindowManagerWayland;
    class WindowWayland;
    class Pointer {
    public:
        Pointer(wl_pointer* pointer, jwm::WindowManagerWayland* wm);
        ~Pointer();

        wl_pointer* _pointer = nullptr;
        wl_pointer* getPointer() const {
            return _pointer;
        }

        uint32_t _serial = 0;
        uint32_t getSerial() {
            return _serial;
        }

        wl_surface* _surface = nullptr;
        wl_surface* getSurface() const {
            return _surface;
        }

        WindowWayland* _focusedSurface = nullptr;
        WindowWayland* getFocusedSurface() const {
            return _focusedSurface;
        }

        zwp_locked_pointer_v1* _lock = nullptr;
        bool _locked = false;
        void lock();
        void unlock();
        bool isLocked() {
            return _locked;
        }

        uint32_t _lastMouseX;
        uint32_t _lastMouseY;
        float _dX = 0.0;
        float _dY = 0.0;

        int _mouseMask = 0;
        jwm::WindowManagerWayland& _wm;

        static wl_pointer_listener _pointerListener;

        void mouseUpdate(uint32_t x, uint32_t y, uint32_t mask);
        void mouseUpdateUnscaled(uint32_t x, uint32_t y, uint32_t mask);

        void updateHotspot(int x, int y);

        std::map<int, wl_cursor_theme*> _cursorThemes;

        wl_cursor_theme* _makeCursors(int scale);
        wl_cursor_theme* getThemeFor(int scale);
        wl_cursor* getCursorFor(int scale, jwm::MouseCursor cursor);

        static bool ownPointer(wl_pointer* pointer);


    private:
        Pointer(const Pointer& other) = delete;
        Pointer(Pointer&&) = delete;
        Pointer& operator=(const Pointer& other) = delete;
        Pointer& operator=(Pointer&&) = delete;
    };
}
