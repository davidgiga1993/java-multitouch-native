#include "WaylandMultitouch.h"
#include "SeatManager.h"
#include <iostream>

static SeatManager seatManager;

JNIEXPORT jint JNICALL Java_org_devcore_touch_linux_WaylandMultitouch_initTouch(JNIEnv *env, jobject object, jlong surfacePtr)
{
	try
	{
		seatManager.addWindow(env, object, reinterpret_cast<struct wl_surface *>(surfacePtr));
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Failed to register touch handler: " << ex.what() << std::endl;
		return -1;
	}
	return 0;
}

JNIEXPORT void JNICALL Java_org_devcore_touch_linux_WaylandMultitouch_dispose(JNIEnv *, jobject, jlong surfacePtr)
{
	seatManager.removeWindow(reinterpret_cast<struct wl_surface *>(surfacePtr));
}
