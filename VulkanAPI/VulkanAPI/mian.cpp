//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/mat4x4.hpp>
//#include <glm/mat4x4.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
    
    float angle = 0.0f;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    //Loop until close
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        float now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        angle += 10.0f * deltaTime;
        if (angle > 360.f) { angle -= 360.0f; };

        glm::mat4 firstModel(1.0f);
        glm::mat4 secondModel(1.0f);
        firstModel = glm::translate(firstModel, glm::vec3(0.0f, 0.0f, -2.f));
        firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        secondModel = glm::translate(secondModel, glm::vec3(0.0f, 0.0f, -2.01f));
        secondModel = glm::rotate(secondModel, glm::radians(angle * -10), glm::vec3(0.0f, 0.0f, 1.0f));
        vulkanRender.updateModel(0, firstModel);
        vulkanRender.updateModel(1, secondModel);

        //vulkanRender.updateModel(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)));
      

      
        vulkanRender.draw();
    }

    vulkanRender.cleanup();
    //Detory GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
