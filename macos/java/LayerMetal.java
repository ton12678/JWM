package org.jetbrains.jwm;

import java.util.function.*;
import lombok.*;
import org.jetbrains.annotations.*;
import org.jetbrains.jwm.*;
import org.jetbrains.jwm.impl.*;

public class LayerMetal extends RefCounted implements Layer {
    @ApiStatus.Internal public Window _window;
    @ApiStatus.Internal public int _width;
    @ApiStatus.Internal public int _height;

    public LayerMetal() {
        super(_nMake());
    }

    @Override
    public void attach(Window window) {
        _window = window;
        _nAttach(window);
    }

    @Override
    public void reconfigure() {
        _nReconfigure();
    }

    @Override
    public void resize(int width, int height) {
        _width = width;
        _height = height;
        _nResize(width, height);
    }

    @Override
    public int getWidth() {
        return _width;
    }

    @Override
    public int getHeight() {
        return _height;
    }

    @Override
    public void swapBuffers() {
        _nSwapBuffers();
    }

    @Override
    public void close() {
        _nClose();
        _window = null;
        super.close();
    }

    public native long getDevicePtr();

    public native long getQueuePtr();

    public native long nextDrawableTexturePtr();

    @ApiStatus.Internal public static native long _nMake();
    @ApiStatus.Internal public native void _nAttach(Window window);
    @ApiStatus.Internal public native void _nReconfigure();
    @ApiStatus.Internal public native void _nResize(int width, int height);
    @ApiStatus.Internal public native void _nSwapBuffers();
    @ApiStatus.Internal public native void _nClose();
}