#pragma once
#include <jni.h>
#include <wayland-client.h>

class MtSurface
{
public:
	struct wl_surface *surface;

	MtSurface(JNIEnv *jniEnv, jobject object, struct wl_surface *surface);
	~MtSurface();

	void handleTouchEvent(int mode, double x, double y, int touchId);

private:
	jobject object;
	jmethodID callbackMethod;
	JavaVM *jvm;
};
