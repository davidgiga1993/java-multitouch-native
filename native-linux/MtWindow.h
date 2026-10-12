#pragma once
#include <jni.h>
#include <X11/Xlib.h>

class MtWindow
{
public:
	Window window;

	MtWindow(JNIEnv *jniEnv, jobject object, Display *display, Window window);
	~MtWindow();

	void handleTouchEvent(int mode, double x, double y, int touchId);

private:
	jobject object;
	jmethodID callbackMethod;
	JavaVM *jvm;
};
