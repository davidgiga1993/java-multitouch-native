#include "MtSurface.h"
#include <stdexcept>

#define POINTER_BUTTON_FIRST 0

MtSurface::MtSurface(JNIEnv *jniEnv, jobject object, struct wl_surface *surface)
{
	this->surface = surface;

	jclass cls = jniEnv->GetObjectClass(object);
	this->callbackMethod = jniEnv->GetMethodID(cls, "nativeOnTouchCallback", "(JIIIII)V");
	if (this->callbackMethod == nullptr)
	{
		throw std::runtime_error("Could not get id for callback method");
	}

	if (jniEnv->GetJavaVM(&this->jvm) != 0)
	{
		throw std::runtime_error("Could not get JavaVM");
	}

	this->object = jniEnv->NewGlobalRef(object);
}

MtSurface::~MtSurface()
{
	if (this->object != nullptr)
	{
		JNIEnv *env;
		if (this->jvm->GetEnv((void **)&env, JNI_VERSION_1_6) == JNI_OK)
		{
			env->DeleteGlobalRef(this->object);
		}
	}
}

/*
Dispatched from the SeatManager's background event thread, which is not the thread that
created the JVM, so the JNIEnv has to be (re-)attached before it can be used.
*/
void MtSurface::handleTouchEvent(int mode, double x, double y, int touchId)
{
	JNIEnv *env;
	bool attached = false;
	int getEnvResult = this->jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
	if (getEnvResult == JNI_EDETACHED)
	{
		if (this->jvm->AttachCurrentThread((void **)&env, nullptr) != 0)
		{
			return;
		}
		attached = true;
	}
	else if (getEnvResult != JNI_OK)
	{
		return;
	}

	env->CallVoidMethod(this->object, this->callbackMethod, (jlong)this->surface,
		(jint)x, (jint)y, (jint)touchId, (jint)mode, (jint)POINTER_BUTTON_FIRST);

	if (attached)
	{
		this->jvm->DetachCurrentThread();
	}
}
