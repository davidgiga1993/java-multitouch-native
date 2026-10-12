#pragma once
#include <jni.h>
#include <Windows.h>

class MtWindow
{
public:
	HWND hwdn;

	MtWindow(JNIEnv *jniEnv, jobject object, HWND hwnd);
	~MtWindow();;

	LRESULT CALLBACK handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool handlePointerEvent(WPARAM wParam);

private:
	jobject object;
	jmethodID callbackMethod;
	JNIEnv *jniEnv;
	LONG_PTR originalWndProc;
};

