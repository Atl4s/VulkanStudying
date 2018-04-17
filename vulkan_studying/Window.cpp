#include "Window.h"


Window::Window()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	m_window = SDL_CreateWindow(
		m_title,
		m_x,
		m_y,
		m_screenWidth,
		m_screenHeight,
		m_flags);
}

Window::~Window()
{
	SDL_DestroyWindow(m_window);
}
