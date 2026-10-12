#include "LinuxMultitouch.h"
#include "WindowManager.h"
#include <iostream>

static WindowManager windowManager;

JNIEXPORT jint JNICALL Java_org_devcore_touch_linux_X11Multitouch_initTouch(JNIEnv *env, jobject object, jlong windowId)
{
	try
	{
		windowManager.addWindow(env, object, (Window)windowId);
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Failed to register touch handler: " << ex.what() << std::endl;
		return -1;
	}
	return 0;
}

JNIEXPORT void JNICALL Java_org_devcore_touch_linux_X11Multitouch_dispose(JNIEnv *, jobject, jlong windowId)
{
	windowManager.removeWindow((Window)windowId);
}
