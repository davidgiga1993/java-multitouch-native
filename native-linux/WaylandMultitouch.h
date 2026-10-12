/* Header for class org_devcore_touch_linux_WaylandMultitouch */
#pragma once
#include <jni.h>

#ifndef _Included_org_devcore_touch_linux_WaylandMultitouch
#define _Included_org_devcore_touch_linux_WaylandMultitouch
#ifdef __cplusplus
extern "C" {
#endif
	/*
	 * Class:     org_devcore_touch_linux_WaylandMultitouch
	 * Method:    initTouch
	 * Signature: (J)I
	 */
	JNIEXPORT jint JNICALL Java_org_devcore_touch_linux_WaylandMultitouch_initTouch
	(JNIEnv *, jobject, jlong);

	/*
	 * Class:     org_devcore_touch_linux_WaylandMultitouch
	 * Method:    dispose
	 * Signature: (J)V
	 */
	JNIEXPORT void JNICALL Java_org_devcore_touch_linux_WaylandMultitouch_dispose
	(JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
