#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <array>

struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributedescriptions = {};

		attributedescriptions[0].binding = 0;
		attributedescriptions[0].location = 0;
		attributedescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributedescriptions[0].offset = offsetof(Vertex, position);

		attributedescriptions[1].binding = 0;
		attributedescriptions[1].location = 1;
		attributedescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributedescriptions[1].offset = offsetof(Vertex, color);

		return attributedescriptions;
	}
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};