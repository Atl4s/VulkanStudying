// vulkan_studying.cpp : Defines the entry point for the console application.
//

#include "stdio.h"

#include "BaseApplication.h"

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <functional>



int main(int argc, char** argv)
{
	BaseApplication app;

	try {
		app.run();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}

