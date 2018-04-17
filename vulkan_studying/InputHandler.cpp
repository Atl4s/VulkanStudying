#include "InputHandler.h"

InputHandler* InputHandler::s_pInstance = 0;

InputHandler::InputHandler()
{
	for (int i = 0; i < 3; i++) m_mouseButtonStates.push_back(false);
	m_mousePosition.x = 0;
	m_mousePosition.y = 0;
}


InputHandler::~InputHandler()
{
}

void InputHandler::initialiseJoysticks() {
	if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == 0) 
	{ 
		SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
	}
	SDL_GameController* GameControllerID = nullptr;
	if (SDL_NumJoysticks() > 0)  
	{
		for (int i = 0; i < SDL_NumJoysticks(); i++)
		{
			if (SDL_IsGameController(i)) GameControllerID = SDL_GameControllerOpen(i);
			std::cout << "controller name: " << SDL_GameControllerName(GameControllerID) << std::endl;
			break;
		}
			

			if (SDL_GameControllerGetAttached(GameControllerID))
			{
				m_joystickValues = std::make_pair(glm::vec2(0,0), glm::vec2(0, 0)); // add pair 
				for (int j = 0; j < 16; j++)
				{
					m_buttonStates.push_back(false);
				}
				
				m_axesStates = std::make_pair(false, false);
			}
			else  
			{
				std::cout << SDL_GetError(); 
			}
		SDL_GameControllerEventState(SDL_ENABLE);  
		m_bGameControllerInitialised = true;
	}
	else  { m_bGameControllerInitialised = false; }
}

void InputHandler::clean()
{ 
	if (m_bGameControllerInitialised)
	{
		SDL_GameControllerClose(m_GameControllers);
	} 
}

void InputHandler::update() {
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		m_keystates = SDL_GetKeyboardState(NULL);
		switch (event.type)
		{
		case SDL_QUIT:
			//Game::Instance()->clean();
			break;
		case SDL_CONTROLLERAXISMOTION:
			onControllerAxisMove(event);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			onControllerButtonDown(event);
			break;
		case SDL_CONTROLLERBUTTONUP:
			onControllerButtonUp(event);
			break;
		case SDL_MOUSEMOTION:
			onMouseMove(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			onMouseButtonDown(event);
			break;
		case SDL_MOUSEBUTTONUP:
			onMouseButtonUp(event);
			break;
		case SDL_KEYDOWN:
			onKeyDown();
			break;
		case SDL_KEYUP:
			onKeyUp();
			break;
		default:
			break;
		}
	}
}

void InputHandler::onControllerAxisMove(SDL_Event& event)
{
	if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) //left stick moves horizontal
	{
		if (event.caxis.value > m_joystickDeadZone)
		{
			m_joystickValues.first.x = 1;
		}
		else if (event.caxis.value < -m_joystickDeadZone)
		{
			m_joystickValues.first.x = -1;
		}
		else
		{
			m_joystickValues.first.x = 0;
		}
	}

	if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) //left stick moves vertical
	{
		if (event.caxis.value > m_joystickDeadZone)
		{
			m_joystickValues.first.y = 1;
		}
		else if (event.caxis.value < -m_joystickDeadZone)
		{
			m_joystickValues.first.y = -1;
		}
		else
		{
			m_joystickValues.first.y = 0;
		}
	}
	
	if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) //right stick moves horizontal
	{
		if (event.caxis.value > m_joystickDeadZone)
		{
			m_joystickValues.second.x = 1;
		}
		else if (event.caxis.value < -m_joystickDeadZone)
		{
			m_joystickValues.second.x = -1;
		}
		else
		{
			m_joystickValues.second.x = 0;
		}
	}

	if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) //right stick moves vertical
	{
		if (event.caxis.value > m_joystickDeadZone)
		{
			m_joystickValues.second.y = 1;
		}
		else if (event.caxis.value < -m_joystickDeadZone)
		{
			m_joystickValues.second.y = -1;
		}
		else
		{
			m_joystickValues.second.y = 0;
		}
	}

	if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) //left trigger click
	{
		if (event.caxis.value > m_joystickDeadZone)
		{
			m_axesStates.first = true;
		}
		else
		{
			m_axesStates.first = false;
		}
	}

	if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) //right trigger click
	{
		if (event.caxis.value > m_joystickDeadZone)
		{
			m_axesStates.second = true;
		}
		else
		{
			m_axesStates.second = false;
		}
	}
}

void InputHandler::onControllerButtonDown(SDL_Event& event)
{
	m_buttonStates[event.cbutton.button] = true;
}

void InputHandler::onControllerButtonUp(SDL_Event& event)
{
	m_buttonStates[event.cbutton.button] = false;
}

void InputHandler::onMouseMove(SDL_Event& event)
{
	//m_mousePosition.x = event.motion.x;
	//m_mousePosition.y = event.motion.y;
}

void InputHandler::onMouseButtonDown(SDL_Event& event)
{
	if (event.button.button == SDL_BUTTON_LEFT) { m_mouseButtonStates[LEFT] = true; std::cout << "key pressed \n"; }
	if (event.button.button == SDL_BUTTON_MIDDLE)  { m_mouseButtonStates[MIDDLE] = true; }
	if (event.button.button == SDL_BUTTON_RIGHT)  { m_mouseButtonStates[RIGHT] = true; }
}

void InputHandler::onMouseButtonUp(SDL_Event& event)
{
	if (event.button.button == SDL_BUTTON_LEFT)  { m_mouseButtonStates[LEFT] = false; std::cout << "key released \n"; }
	if (event.button.button == SDL_BUTTON_MIDDLE)  { m_mouseButtonStates[MIDDLE] = false; }
	if (event.button.button == SDL_BUTTON_RIGHT)  { m_mouseButtonStates[RIGHT] = false; }
}

void InputHandler::onKeyDown()
{

}

void InputHandler::onKeyUp()
{

}

int InputHandler::xvalue(stick stick)
{
		if (stick == 1) // for left stick
		{
			return m_joystickValues.first.x;
		}
		else if (stick == 2) // for right stick
		{
			return m_joystickValues.second.x;
		}
	return 0;
}

int InputHandler::yvalue(stick stick)
{

		if (stick == 1) // for left stick
		{
			return m_joystickValues.first.y;
		}
		else if (stick == 2) // for right stick
		{
			return m_joystickValues.second.y;
		}
	return 0;
}

bool InputHandler::isKeyDown(SDL_Scancode key) 
{
	if (m_keystates != 0) 
	{ 
		if (m_keystates[key] == 1)   { return true; } 
	else   { return false; } 
	}
	return false;
}