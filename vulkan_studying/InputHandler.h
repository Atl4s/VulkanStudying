#pragma once
#include "Headers.h"
#include "glm/glm.hpp"

typedef enum
{ 
	LEFT = 0, 
	MIDDLE = 1,
	RIGHT = 2 
}mouse_buttons;

typedef enum
{
	LEFT_STICK = 1,
	RIGHT_STICK = 2
}stick;

class InputHandler
{
public:
	static InputHandler* Instance()
	{
		if (s_pInstance == 0)
		{
			s_pInstance = new InputHandler();
		}
		return s_pInstance;
	}

	int xvalue(stick stick);
	int yvalue(stick stick);

	void update();
	void clean();

	void initialiseJoysticks();

	bool joysticksInitialised() { return m_bGameControllerInitialised; }

	bool getLeftTriggerState(){ return m_axesStates.first; }
	bool getRightTriggerState(){ return m_axesStates.second; }

	bool getButtonState(int buttonNumber){ return m_buttonStates[buttonNumber]; }

	bool getMouseButtonState(int buttonNumber) { return m_mouseButtonStates[buttonNumber]; }

	glm::vec2 getMousePosition() { 
		int x, y;
		SDL_GetMouseState(&x, &y);
		m_mousePosition.x = x;
		m_mousePosition.y = y;
		return m_mousePosition; }
	glm::vec2 getRelativeMousePosition() { 
		int x, y;
		SDL_GetRelativeMouseState(&x, &y);
		m_relativeMousePosition.x = x;
		m_relativeMousePosition.y = y;
		return m_relativeMousePosition; }

	bool isKeyDown(SDL_Scancode key);

private:
	InputHandler();
	~InputHandler();

	static InputHandler* s_pInstance;

	SDL_GameController* m_GameControllers;

	bool m_bGameControllerInitialised;

	std::pair<glm::vec2, glm::vec2> m_joystickValues;

	const int m_joystickDeadZone = 10000;

	std::vector<bool> m_buttonStates;

	std::pair<bool, bool> m_axesStates;

	std::vector<bool> m_mouseButtonStates;

	glm::vec2 m_mousePosition;
	glm::vec2 m_relativeMousePosition;

	const Uint8* m_keystates;

	//handle keyboard events
	void onKeyUp();
	void onKeyDown();

	//handle mouse events
	void onMouseMove(SDL_Event& event);
	void onMouseButtonDown(SDL_Event& event);
	void onMouseButtonUp(SDL_Event& event);

	//handle
	void onControllerAxisMove(SDL_Event& event);
	void onControllerButtonDown(SDL_Event& event);
	void onControllerButtonUp(SDL_Event& event);
};

