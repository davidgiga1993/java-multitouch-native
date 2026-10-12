/* Header for class org_devcore_linux_Multitouch */
#pragma once
#include <jni.h>

#ifndef _Included_org_devcore_linux_Multitouch
#define _Included_org_devcore_linux_Multitouch
#ifdef __cplusplus
extern "C" {
#endif
#undef org_devcore_linux_Multitouch_POINTER_DOWN
#define org_devcore_linux_Multitouch_POINTER_DOWN 0L
#undef org_devcore_linux_Multitouch_POINTER_UP
#define org_devcore_linux_Multitouch_POINTER_UP 1L
#undef org_devcore_linux_Multitouch_POINTER_MOVE
#define org_devcore_linux_Multitouch_POINTER_MOVE 2L
#undef org_devcore_linux_Multitouch_BUTTON_1
#define org_devcore_linux_Multitouch_BUTTON_1 0L
#undef org_devcore_linux_Multitouch_BUTTON_2
#define org_devcore_linux_Multitouch_BUTTON_2 1L
#undef org_devcore_linux_Multitouch_BUTTON_3
#define org_devcore_linux_Multitouch_BUTTON_3 2L
#undef org_devcore_linux_Multitouch_BUTTON_4
#define org_devcore_linux_Multitouch_BUTTON_4 3L
#undef org_devcore_linux_Multitouch_BUTTON_5
#define org_devcore_linux_Multitouch_BUTTON_5 4L
	/*
	 * Class:     org_devcore_linux_Multitouch
	 * Method:    initTouch
	 * Signature: (J)I
	 */
	JNIEXPORT jint JNICALL Java_org_devcore_touch_linux_X11Multitouch_initTouch
	(JNIEnv *, jobject, jlong);

	/*
	 * Class:     org_devcore_linux_Multitouch
	 * Method:    dispose
	 * Signature: (J)V
	 */
	JNIEXPORT void JNICALL Java_org_devcore_touch_linux_X11Multitouch_dispose
	(JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
