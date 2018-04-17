#pragma once

#include "Window.h"
#include <iostream>
#include <stdexcept>
#include <functional>

template <typename T>
class Cleaner
{
public:
	Cleaner() : Cleaner([](T, VkAllocationCallbacks*) {}) {};

	Cleaner(std::function<void(T, VkAllocationCallbacks*)> deletef)
	{
		m_cleaner = [=](T obj) { deletef(obj, nullptr); };
	}

	Cleaner(const Cleaner<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
	{
		m_cleaner = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
	}

	Cleaner(const Cleaner<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
	{
		m_cleaner = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
	}

	~Cleaner() 
	{
		cleanup();
	}

	const T* operator &() const
	{
		return &m_object;
	}

	T* data()
	{
		cleanup();
		return &m_object;
	}

	operator T() const
	{
		return m_object;
	}

	void operator=(T rhs)
	{
		cleanup();
		m_object = rhs;
	}

	template<typename V>
	bool operator==(V rhs)
	{
		return m_object == T(rhs);
	}

private:
	T m_object{ VK_NULL_HANDLE };
	std::function<void(T)> m_cleaner;

	void cleanup()
	{
		if (m_object != VK_NULL_HANDLE)
		{
			m_cleaner(m_object);
		}
		m_object = VK_NULL_HANDLE;
	}
};

