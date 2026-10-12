package org.devcore.touch;

/**
 * Listener for multitouch events
 */
public interface MultitouchListener
{

	/**
	 * Gets called on a touch event
	 *
	 * @param x       X pos
	 * @param y       Y pos
	 * @param pointer Touch id
	 * @param mode    Touch mode
	 * @param button  Button id
	 */
	void onTouch(int x, int y, int pointer, int mode, int button);
}
