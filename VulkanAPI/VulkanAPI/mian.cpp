//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/mat4x4.hpp>
//#include <glm/mat4x4.hpp>

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include<stdexcept>
#include<vector>
#include <iostream>
#include "VulkanRender.h"

GLFWwindow* window;
VulkanRender vulkanRender;

void initWindow(std::string wName="Test Window", const int width=800,const int height=600)
{
    //Initialise GLFW
    glfwInit();

    //Set GLFW to NOT work with OpenGL
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);

    window = glfwCreateWindow(width, height, wName.c_str(),nullptr,nullptr);
}

int main() {

    //create window
    initWindow("Vulkan Test Window",800,600);

    //create Vulkan Render instance
    if (vulkanRender.init(window) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    

    //Loop until close
    while (!glfwWindowShouldClose(window))
    {
        glfwWaitEvents();
        vulkanRender.draw();
    }

    vulkanRender.cleanup();
    //Detory GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
