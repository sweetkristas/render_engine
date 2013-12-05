#include "WindowManager.hpp"

int main()
{
	graphics::WindowManagerPtr main_wnd = graphics::WindowManager::factory("SDL", "opengl");
	return 0;
}