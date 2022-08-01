#include <iostream>
#include <OBJ_Loader.h>
#include <string>
#include <fstream>
#include <vector>
#include "../engine/render/vulkan_manager.h"
#include <GLFW/glfw3.h>

class Model{
public:
    int render_id;
public:
    void loadModel(const std::string& filename){
        objl::Loader loader;
        loader.LoadFile(filename);

        // Go through each loaded mesh and out its contents
        for (int i = 0; i < loader.LoadedMeshes.size(); i++)
        {
            // Copy one of the loaded meshes to be our current mesh
            objl::Mesh curMesh = loader.LoadedMeshes[i];
            _vertcies.reserve(curMesh.Vertices.size() * 6);
            for (int j = 0; j < curMesh.Vertices.size(); j++)
            {
                _vertcies.push_back(curMesh.Vertices[j].Position.X);
                _vertcies.push_back(curMesh.Vertices[j].Position.Y);
                _vertcies.push_back(curMesh.Vertices[j].Position.Z);

                _vertcies.push_back(curMesh.Vertices[j].Normal.X);
                _vertcies.push_back(curMesh.Vertices[j].Normal.Y);
                _vertcies.push_back(curMesh.Vertices[j].Normal.Z);
            }

            _indices = curMesh.Indices;
        }
    }
public:
    std::vector<float> _vertcies;
    std::vector<uint32_t> _indices;
};

using namespace galaxysailing;
const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;
int main(void){
    std::cout<<"hello vulkan render test!\n";
    Model model;
    model.loadModel("../workdir/models/bunny/bunny.obj");


    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window;
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    // glfwSetWindowUserPointer(window, this);
    // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    VulkanManager vkrender;
    vkrender.initialize(window);
    model.render_id = vkrender.addMesh(model._indices, model._vertcies);
     while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        vkrender.renderFrame(model.render_id);
    }

    printf("------------------------------------------------------------------------\n");
    printf("window close!\n");
    printf("------------------------------------------------------------------------\n");

    return 0;
}