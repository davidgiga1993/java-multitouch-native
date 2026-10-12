#pragma once
#include <jni.h>
#include <wayland-client.h>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <cstdint>
#include <utility>
#include "MtSurface.h"

class SeatManager
{
public:
	SeatManager();
	~SeatManager();

	/*
	Adds a multitouch listener to the given surface
	*/
	void addWindow(JNIEnv *jniEnv, jobject object, struct wl_surface *surface);

	/*
	Removes the listener for the given surface
	*/
	void removeWindow(struct wl_surface *surface);

	// Registered as wl_registry/wl_seat/wl_touch listener callbacks; public so the
	// file-scope listener struct initializers in SeatManager.cpp can reference them.
	static void registryGlobal(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
	static void registryGlobalRemove(void *data, struct wl_registry *registry, uint32_t name);
	static void seatCapabilities(void *data, struct wl_seat *seat, uint32_t capabilities);
	static void seatName(void *data, struct wl_seat *seat, const char *name);
	static void touchDown(void *data, struct wl_touch *touch, uint32_t serial, uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
	static void touchUp(void *data, struct wl_touch *touch, uint32_t serial, uint32_t time, int32_t id);
	static void touchMotion(void *data, struct wl_touch *touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
	static void touchFrame(void *data, struct wl_touch *touch);
	static void touchCancel(void *data, struct wl_touch *touch);

private:
	struct PendingEvent
	{
		struct wl_surface *surface;
		double x;
		double y;
		int32_t touchId;
		int mode;
	};

	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_seat *seat;
	struct wl_touch *touch;
	int wakeupPipe[2];
	bool running;
	std::thread eventThread;

	std::mutex windowsMutex;
	std::vector<MtSurface *> windows;

	// touch ids that are currently down, and the surface/position they were last seen at
	std::unordered_map<int32_t, struct wl_surface *> activeTouches;
	std::unordered_map<int32_t, std::pair<double, double>> lastPosition;
	std::vector<PendingEvent> pendingEvents;

	void connect();
	void eventLoop();
	void wakeupEventLoop();
	void stopEventLoop();
	MtSurface *findWindow(struct wl_surface *surface);

	void handleTouchDown(struct wl_surface *surface, int32_t touchId, double x, double y);
	void handleTouchMotion(int32_t touchId, double x, double y);
	void handleTouchUp(int32_t touchId);
	void handleTouchCancel();
	void handleTouchFrame();
};
