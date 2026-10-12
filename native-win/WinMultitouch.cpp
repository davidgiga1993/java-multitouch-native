#include "stdafx.h"
#include "WinMultitouch.h"
#include <iostream>
#include "WindowManager.h"

using namespace std;

WindowManager windowManager = WindowManager();

JNIEXPORT jint JNICALL Java_org_devcore_touch_win_WinMultitouch_initTouch(JNIEnv *env, jobject object, jlong hwndLong)
{
	try
	{
		windowManager.addWindow(env, object, (HWND)hwndLong);
	}
	catch (std::exception) {
		return -1;
	}
	return 0;
}

JNIEXPORT void JNICALL Java_org_devcore_touch_win_WinMultitouch_dispose(JNIEnv *, jobject object, jlong hwndLong)
{
	windowManager.removeWindow((HWND)hwndLong);
}