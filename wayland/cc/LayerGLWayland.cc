#include <algorithm>
#include <iostream>
#include <jni.h>
#include "impl/Library.hh"
#include "impl/RefCounted.hh"
#include "WindowWayland.hh"
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GL/gl.h>
#include "ILayerWayland.hh"

namespace jwm {

    class LayerGL: public RefCounted, public ILayerWayland {
    public:
        WindowWayland* fWindow;
        wl_egl_window* _eglWindow = nullptr;
        wl_region* _region = nullptr;
        EGLContext _context = nullptr;
        EGLDisplay _display = nullptr;
        EGLSurface _surface = nullptr;
        EGLConfig _config = nullptr;

        LayerGL() = default;
        virtual ~LayerGL() = default;

        void attach(WindowWayland* window) {
            fWindow = jwm::ref(window);
            fWindow->setLayer(this);
            // HACK: reopen
            bool visible = fWindow->_visible;
            if (visible) {
              fWindow->hide();
            }
            if (_display == nullptr) {
              _display = eglGetDisplay(window->_windowManager.display);

              eglInitialize(_display, nullptr, nullptr);

            }
            if ( eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
              throw new std::runtime_error("Cannot bind EGL Api");
            }
            if (_context == nullptr) {
                EGLint attrList[] = {
                  EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                  EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                  EGL_BLUE_SIZE, 8,
                  EGL_GREEN_SIZE, 8,
                  EGL_RED_SIZE, 8,
                  EGL_NONE
                };
                EGLint contextAttr[] = {
                  EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE
                };
                EGLint numConfig;
                if ( ( eglGetConfigs(_display, nullptr, 0, &numConfig) != EGL_TRUE) || (numConfig == 0) ) {
                    throw std::runtime_error("No configuration");
                }
                if ( ( eglChooseConfig(_display, attrList, &_config, 1, &numConfig) != EGL_TRUE) || (numConfig != 1)) {
                    throw std::runtime_error("No/Amibguous configuration");
                }
                // :troll:
                _context = eglCreateContext(_display,
                                            _config,
                                            EGL_NO_CONTEXT,
                                            contextAttr);
                if ( _context == EGL_NO_CONTEXT ) {
                    throw std::runtime_error("Couldn't make context");
                }
                // Don't block on swap
                eglSwapInterval(_display, 0); 
            }
            if (visible)
              fWindow->show();
            makeCurrentForced();
        }

        void setVsyncMode(VSync v) override {
          // vsync? what vsync?
        }

        void resize(int width, int height) override {
            // Make current to avoid artifacts in other windows
            makeCurrentForced();
            glClearStencil(0);
            glClearColor(0, 0, 0, 255);
            glStencilMask(0xffffffff);
            glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            // ???
            glViewport(0, 0, width, height);
            // God is dead if _eglWindow is null 
            if (_eglWindow)
              wl_egl_window_resize(_eglWindow, width, height, 0, 0);
        }

        void swapBuffers() override {
            printf("before swap\n");
            eglSwapBuffers(_display, _surface);
            printf("after swap\n");
        }

        void close() override {
            detach();
            eglDestroyContext(_display, _context);
            eglTerminate(_display);

            jwm::unref(&fWindow);
        }

        void makeCurrentForced() override {
            ILayer::makeCurrentForced();
            eglMakeCurrent(_display,
                          _surface,
                          _surface,
                          _context);
        }
        void attachBuffer() override {
          if (fWindow && fWindow->_waylandWindow) {
            if (!_eglWindow) {
              // _region = wl_compositor_create_region(fWindow->_windowManager.compositor);
              // wl_region_add(_region, 0, 0, fWindow->getWidth(), fWindow->getHeight());
              // wl_surface_set_opaque_region(fWindow->_waylandWindow, _region);
              _eglWindow = wl_egl_window_create(fWindow->_waylandWindow, fWindow->getWidth(), fWindow->getHeight());
              _surface = eglCreateWindowSurface(_display, _config, _eglWindow, nullptr);
             
              if ( _eglWindow == EGL_NO_SURFACE ) {
                  throw std::runtime_error("couldn't get surface");
              } 
            }
            makeCurrentForced();
          }
        }
        void detach() override {
            eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (_surface) {
              eglDestroySurface(_display, _surface);
            }
            _surface = nullptr;
            if (_eglWindow) {
              wl_egl_window_destroy(_eglWindow);
            }
            _eglWindow = nullptr;
        }
    };

} // namespace jwm

// JNI

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_LayerGL__1nMake
  (JNIEnv* env, jclass jclass) {
    jwm::LayerGL* instance = new jwm::LayerGL();
    return reinterpret_cast<jlong>(instance);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nAttach
  (JNIEnv* env, jobject obj, jobject windowObj) {
    try {
        jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
        jwm::WindowWayland* window = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, windowObj));
        instance->attach(window);
    } catch (const std::exception& e) {
        printf("%s\n", e.what());
        jwm::classes::Throwable::throwLayerNotSupportedException(env, "Failed to init OpenGL");
    }
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nReconfigure
  (JNIEnv* env, jobject obj, jint width, jint height) {
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nResize
  (JNIEnv* env, jobject obj, jint width, jint height) {
    jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
    instance->resize(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nMakeCurrent
  (JNIEnv* env, jobject obj) {
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nSwapBuffers
  (JNIEnv* env, jobject obj) {
    jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
    instance->swapBuffers();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nClose
  (JNIEnv* env, jobject obj) {
    jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
    instance->close();
}