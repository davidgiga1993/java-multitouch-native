package org.devcore.touch.linux;

import org.devcore.touch.Multitouch;
import org.devcore.touch.MultitouchListener;
import org.devcore.touch.util.NativeLibraryLoader;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Handles multitouch events for one or more X11 windows.
 * The coordinates used are always relative to the client area of the window
 */
public class X11Multitouch implements Multitouch
{
	private final List<Long> activeWindows = new ArrayList<>();
	private final Map<Long, MultitouchListener> callbacks = new HashMap<>();

	private native int initTouch(long window);

	private native void dispose(long window);

	public X11Multitouch()
	{
		NativeLibraryLoader.loadRaw("libMtLinux64.so");
	}

	@Override
	public void dispose()
	{
		for (long window : activeWindows)
		{
			dispose(window);
		}
		activeWindows.clear();
	}

	@Override
	public void removeWindow(long window)
	{
		dispose(window);
		activeWindows.remove(window);
	}

	@Override
	public void addWindow(long window, MultitouchListener listener)
	{
		if (initTouch(window) != 0)
		{
			throw new RuntimeException("Registering the handler failed");
		}

		activeWindows.add(window);
		callbacks.put(window, listener);
	}

	/**
	 * This method will be called by the native callback
	 *
	 * @param window  X11 window id
	 * @param x       X pos
	 * @param y       Y pos
	 * @param pointer Touch id
	 * @param mode    Touch mode
	 * @param button  Button
	 */
	@SuppressWarnings("unused")
	protected void nativeOnTouchCallback(long window, int x, int y, int pointer, int mode, int button)
	{
		MultitouchListener processor = callbacks.get(window);
		if (processor != null)
		{
			processor.onTouch(x, y, pointer, mode, button);
		}
	}
}
