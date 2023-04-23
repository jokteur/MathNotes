#pragma once

#include <string>

#include <tempo.h>

/**
 * This function helps understand if at current mouse position, some other window/popup is display in front
 *
 * Helpful for determining if once can interact with a widget without something else blocking
 * in front
*/
bool isOnTop(const std::string& window_name);