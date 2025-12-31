/**
 * Titan Platform Header
 * 
 * Platform abstraction layer
 */

#ifndef TITAN_PLATFORM_HPP
#define TITAN_PLATFORM_HPP

#include "Titan_Core.hpp"
#include "Titan_Input.hpp"

namespace Titan::Platform {

/**
 * Initialize the platform (create window, etc.)
 * @param Width - Window width
 * @param Height - Window height
 * @param Title - Window title
 */
void Init(uint32 Width, uint32 Height, const char* Title);

/**
 * Shutdown platform and cleanup
 */
void Shutdown();

/**
 * Process window messages
 * @return true if the application should continue, false to quit
 */
bool PumpMessages();

/**
 * Swap buffers (present frame)
 */
void SwapBuffers();

/**
 * Check if a key is currently down
 */
bool GetKeyDown(KeyCode Key);

/**
 * Get mouse position relative to window
 */
Math::Vec2 GetMousePos();

/**
 * Get high-resolution time in seconds since start
 */
float GetTime();

/**
 * Get time since last frame
 */
float GetDeltaTime();

/**
 * Get window width
 */
uint32 GetWindowWidth();

/**
 * Get window height
 */
uint32 GetWindowHeight();

/**
 * Set window title
 */
void SetWindowTitle(const char* Title);

/**
 * Show/hide cursor
 */
void SetCursorVisible(bool bVisible);

/**
 * Lock cursor to window center
 */
void SetCursorLocked(bool bLocked);

} // namespace Titan::Platform

#endif // TITAN_PLATFORM_HPP
