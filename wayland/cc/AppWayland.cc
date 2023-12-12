#include <jni.h>
#include "AppWayland.hh"
#include <cstdlib>
#include <wayland-client.h>
#include <cassert>
jwm::AppWayland jwm::app;


void jwm::AppWayland::init(JNIEnv* jniEnv) {
    jint rs = jniEnv->GetJavaVM(&_javaVM);
    assert(rs == JNI_OK);
}

void jwm::AppWayland::start() {
    wm.runLoop();
}

void jwm::AppWayland::terminate() {
    wm.terminate();
}

JNIEnv* jwm::AppWayland::getJniEnv() {
    JNIEnv* env;
    // no-op on an already attached thread, so fast?
    // makes it thread-safe (?)
    jint rs = _javaVM->AttachCurrentThread((void**)&env, nullptr);
    assert(rs == JNI_OK);
    return env;
}
// JNI

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_App__1nStart(JNIEnv* env, jclass jclass, jobject launcher) {
    jwm::app.init(env);
    jwm::classes::Runnable::run(env, launcher);
    jwm::app.start();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_App__1nTerminate(JNIEnv* env, jclass jclass) {
    jwm::app.terminate();
}

extern"C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    return JNI_VERSION_1_2;
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_App__1nRunOnUIThread
        (JNIEnv* env, jclass cls, jobject callback) {
    jobject callbackRef = env->NewGlobalRef(callback);
    jwm::app.getWindowManager().enqueueTask([callbackRef] {
        jwm::classes::Runnable::run(jwm::app.getJniEnv(), callbackRef);
        jwm::app.getJniEnv()->DeleteGlobalRef(callbackRef);
    });
}

// how awful
extern "C" JNIEXPORT jobjectArray JNICALL Java_io_github_humbleui_jwm_App__1nGetScreens(JNIEnv* env, jobject cls) noexcept {


    jobjectArray array = env->NewObjectArray(jwm::app.wm.outputs.size(), jwm::classes::Screen::kCls, 0);
    size_t index = 0;

    for (auto& i : jwm::app.wm.outputs) {
        env->SetObjectArrayElement(array, index++, i->getScreenInfo().asJavaObject(env));
    }


    return array;
}