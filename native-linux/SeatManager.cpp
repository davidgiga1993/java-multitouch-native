#include "SeatManager.h"
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

namespace
{
	const struct wl_registry_listener registryListener = {
		SeatManager::registryGlobal,
		SeatManager::registryGlobalRemove,
	};

	const struct wl_seat_listener seatListener = {
		SeatManager::seatCapabilities,
		SeatManager::seatName,
	};

	const struct wl_touch_listener touchListener = {
		SeatManager::touchDown,
		SeatManager::touchUp,
		SeatManager::touchMotion,
		SeatManager::touchFrame,
		SeatManager::touchCancel,
	};
}

SeatManager::SeatManager()
	: display(nullptr), registry(nullptr), seat(nullptr), touch(nullptr), running(false)
{
	this->wakeupPipe[0] = -1;
	this->wakeupPipe[1] = -1;
}

SeatManager::~SeatManager()
{
	this->stopEventLoop();

	if (this->touch != nullptr)
	{
		wl_touch_destroy(this->touch);
	}
	if (this->seat != nullptr)
	{
		wl_seat_destroy(this->seat);
	}
	if (this->registry != nullptr)
	{
		wl_registry_destroy(this->registry);
	}
	if (this->display != nullptr)
	{
		wl_display_disconnect(this->display);
	}
}

void SeatManager::connect()
{
	this->display = wl_display_connect(nullptr);
	if (this->display == nullptr)
	{
		throw std::runtime_error("Unable to connect to the Wayland display");
	}

	this->registry = wl_display_get_registry(this->display);
	wl_registry_add_listener(this->registry, &registryListener, this);

	// First round trip: receive the registry's global advertisements (binds wl_seat).
	wl_display_roundtrip(this->display);

	if (this->seat == nullptr)
	{
		wl_display_disconnect(this->display);
		this->display = nullptr;
		throw std::runtime_error("Compositor did not advertise a wl_seat");
	}

	// Second round trip: receive the seat's capabilities event (creates wl_touch).
	wl_display_roundtrip(this->display);

	if (this->touch == nullptr)
	{
		wl_display_disconnect(this->display);
		this->display = nullptr;
		this->seat = nullptr;
		throw std::runtime_error("Seat does not report touch capability");
	}

	if (pipe(this->wakeupPipe) != 0)
	{
		throw std::runtime_error("Unable to create wakeup pipe");
	}
	fcntl(this->wakeupPipe[0], F_SETFL, O_NONBLOCK);
}

void SeatManager::addWindow(JNIEnv *jniEnv, jobject object, struct wl_surface *surface)
{
	if (this->display == nullptr)
	{
		this->connect();
	}

	auto mtSurface = new MtSurface(jniEnv, object, surface);
	{
		std::lock_guard<std::mutex> lock(this->windowsMutex);
		this->windows.push_back(mtSurface);
	}

	if (!this->running)
	{
		this->running = true;
		this->eventThread = std::thread(&SeatManager::eventLoop, this);
	}
}

void SeatManager::removeWindow(struct wl_surface *surface)
{
	bool empty;
	{
		std::lock_guard<std::mutex> lock(this->windowsMutex);
		for (size_t i = 0; i < this->windows.size(); i++)
		{
			if (this->windows[i]->surface != surface)
			{
				continue;
			}
			delete this->windows[i];
			this->windows.erase(this->windows.begin() + i);
			break;
		}
		empty = this->windows.empty();
	}

	if (empty)
	{
		this->stopEventLoop();
	}
}

void SeatManager::wakeupEventLoop()
{
	if (this->wakeupPipe[1] == -1)
	{
		return;
	}
	char byte = 0;
	ssize_t result = write(this->wakeupPipe[1], &byte, 1);
	(void)result;
}

void SeatManager::stopEventLoop()
{
	if (!this->running)
	{
		return;
	}

	this->running = false;
	this->wakeupEventLoop();
	if (this->eventThread.joinable())
	{
		this->eventThread.join();
	}

	if (this->wakeupPipe[0] != -1)
	{
		close(this->wakeupPipe[0]);
		close(this->wakeupPipe[1]);
		this->wakeupPipe[0] = -1;
		this->wakeupPipe[1] = -1;
	}
}

MtSurface *SeatManager::findWindow(struct wl_surface *surface)
{
	std::lock_guard<std::mutex> lock(this->windowsMutex);
	for (auto mtSurface : this->windows)
	{
		if (mtSurface->surface == surface)
		{
			return mtSurface;
		}
	}
	return nullptr;
}

void SeatManager::eventLoop()
{
	while (this->running)
	{
		while (wl_display_prepare_read(this->display) != 0)
		{
			wl_display_dispatch_pending(this->display);
		}
		wl_display_flush(this->display);

		struct pollfd fds[2];
		fds[0].fd = wl_display_get_fd(this->display);
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		fds[1].fd = this->wakeupPipe[0];
		fds[1].events = POLLIN;
		fds[1].revents = 0;

		poll(fds, 2, -1);

		if (fds[1].revents & POLLIN)
		{
			wl_display_cancel_read(this->display);
			char buffer[16];
			while (read(this->wakeupPipe[0], buffer, sizeof(buffer)) > 0)
			{
			}
			break;
		}

		if (fds[0].revents & POLLIN)
		{
			wl_display_read_events(this->display);
		}
		else
		{
			wl_display_cancel_read(this->display);
			continue;
		}

		if (!this->running)
		{
			break;
		}

		wl_display_dispatch_pending(this->display);
	}
}

void SeatManager::handleTouchDown(struct wl_surface *surface, int32_t touchId, double x, double y)
{
	this->activeTouches[touchId] = surface;
	this->lastPosition[touchId] = {x, y};
	this->pendingEvents.push_back({surface, x, y, touchId, 0}); // POINTER_DOWN
}

void SeatManager::handleTouchMotion(int32_t touchId, double x, double y)
{
	auto it = this->activeTouches.find(touchId);
	if (it == this->activeTouches.end())
	{
		return;
	}
	this->lastPosition[touchId] = {x, y};
	this->pendingEvents.push_back({it->second, x, y, touchId, 2}); // POINTER_MOVE
}

void SeatManager::handleTouchUp(int32_t touchId)
{
	auto it = this->activeTouches.find(touchId);
	if (it == this->activeTouches.end())
	{
		return;
	}
	auto pos = this->lastPosition[touchId];
	this->pendingEvents.push_back({it->second, pos.first, pos.second, touchId, 1}); // POINTER_UP
	this->activeTouches.erase(touchId);
	this->lastPosition.erase(touchId);
}

void SeatManager::handleTouchCancel()
{
	this->pendingEvents.clear();
	this->activeTouches.clear();
	this->lastPosition.clear();
}

void SeatManager::handleTouchFrame()
{
	for (const auto &event : this->pendingEvents)
	{
		MtSurface *mtSurface = this->findWindow(event.surface);
		if (mtSurface != nullptr)
		{
			mtSurface->handleTouchEvent(event.mode, event.x, event.y, event.touchId);
		}
	}
	this->pendingEvents.clear();
}

void SeatManager::registryGlobal(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	auto self = static_cast<SeatManager *>(data);
	if (std::string(interface) == wl_seat_interface.name)
	{
		self->seat = static_cast<struct wl_seat *>(
			wl_registry_bind(registry, name, &wl_seat_interface, version < 5 ? version : 5));
		wl_seat_add_listener(self->seat, &seatListener, self);
	}
}

void SeatManager::registryGlobalRemove(void *, struct wl_registry *, uint32_t)
{
}

void SeatManager::seatCapabilities(void *data, struct wl_seat *seat, uint32_t capabilities)
{
	auto self = static_cast<SeatManager *>(data);
	bool hasTouch = (capabilities & WL_SEAT_CAPABILITY_TOUCH) != 0;
	if (hasTouch && self->touch == nullptr)
	{
		self->touch = wl_seat_get_touch(seat);
		wl_touch_add_listener(self->touch, &touchListener, self);
	}
	else if (!hasTouch && self->touch != nullptr)
	{
		wl_touch_destroy(self->touch);
		self->touch = nullptr;
	}
}

void SeatManager::seatName(void *, struct wl_seat *, const char *)
{
}

void SeatManager::touchDown(void *data, struct wl_touch *, uint32_t, uint32_t, struct wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	static_cast<SeatManager *>(data)->handleTouchDown(surface, id, wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void SeatManager::touchUp(void *data, struct wl_touch *, uint32_t, uint32_t, int32_t id)
{
	static_cast<SeatManager *>(data)->handleTouchUp(id);
}

void SeatManager::touchMotion(void *data, struct wl_touch *, uint32_t, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	static_cast<SeatManager *>(data)->handleTouchMotion(id, wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void SeatManager::touchFrame(void *data, struct wl_touch *)
{
	static_cast<SeatManager *>(data)->handleTouchFrame();
}

void SeatManager::touchCancel(void *data, struct wl_touch *)
{
	static_cast<SeatManager *>(data)->handleTouchCancel();
}
