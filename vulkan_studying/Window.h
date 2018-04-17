#pragma once

#include "Headers.h"

#include <SDL.h>
#include <SDL_syswm.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

class Window
{
public:
	Window();
	Window(const char* title, int x, int y, int w, int h, Uint32 flags) : 
		m_title(title),
		m_x(x),
		m_y(y),
		m_screenWidth(w),
		m_screenHeight(h),
		m_flags(flags)
	{}
	~Window();

	SDL_Window* getWindow() { return m_window; }

	int getWidth() { return m_screenWidth; }
	int getHeight() { return m_screenHeight; }

private:
	SDL_Window* m_window;

	const char* m_title = "vulkan application";
	int m_x = SDL_WINDOWPOS_CENTERED;
	int m_y = SDL_WINDOWPOS_CENTERED;
	int m_screenWidth = 800;
	int m_screenHeight = 600;
	Uint32 m_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
};

