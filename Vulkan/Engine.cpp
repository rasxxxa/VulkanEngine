#include "Engine.h"

void Engine::InitImGui()
{
    m_renderer->SetupImgui(&m_MainWindowData);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;

    ImGui::StyleColorsDark();
    VkResult err;
    m_renderer->InitForVulkan();


    VkCommandPool command_pool = m_MainWindowData.Frames[m_MainWindowData.FrameIndex].CommandPool;
    VkCommandBuffer command_buffer = m_MainWindowData.Frames[m_MainWindowData.FrameIndex].CommandBuffer;

    err = vkResetCommandPool(m_renderer->GetLogicalDevice(), command_pool, 0);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    err = vkEndCommandBuffer(command_buffer);
    err = vkQueueSubmit(m_renderer->GetGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE);
    err = vkDeviceWaitIdle(m_renderer->GetLogicalDevice());
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    if (err != VK_SUCCESS)
    {
        std::cout << "Error while setting font for IMGUI" << std::endl;
        assert(false);
    }

}

bool Engine::CreateWindow(const int width, const int height)
{
    // Initialize GLFW
    glfwInit();

    // Set glfw to not work with opengl
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Set to be resizible
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(width, height, "Game window", nullptr, nullptr);

    if (!glfwInit())
    {
        std::cout << "Could not init glfw window" << std::endl;
        assert(false);
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Setup Vulkan
    if (!glfwVulkanSupported())
    {
        std::cout << "GLFW: Vulkan Not Supported" << std::endl;;
        assert(false);
        return false;
    }
}

bool Engine::DestroyWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
    return true;
}

void Engine::InitProgram(int width, int height)
{
    CreateWindow(width, height);
    CreateRenderer();
    InitImGui();
    RunWindow();
    ShutdownApplication();
}

Engine& Engine::GetInstance()
{
    static Engine eng = {};
    return eng;
}

void Engine::RunWindow()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            auto size = m_renderer->ReturnSceneObject().size();
            static std::vector<float> sizes(size, 0);
            static std::vector<std::array<float, 3>> poses(size, { 0.0f, 0.0f, 0.0f });

            if (sizes.size() != size)
            {
                sizes.resize(size);
                poses.resize(size);
            }

            if (PRINT_OBJECTS)
            {
                for (size_t i = 0; i < size; i++)
                {
                    std::string itemMenu = "item Menu";
                    itemMenu += std::to_string(i);
                    if (ImGui::BeginMenu(itemMenu.c_str()))
                    {
                        std::string r = "rotation";
                        r += std::to_string(i);
                        ImGui::SliderFloat(r.c_str(), &sizes[i], 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                        glm::mat4 firstModel(1.0f);
                        firstModel = glm::rotate(firstModel, glm::radians(sizes[i]), glm::vec3(0.0f, 0.0f, 1.0f));


                        std::string translationX = "translationX";
                        std::string translationY = "translationY";
                        std::string translationZ = "translationZ";
                        translationX += std::to_string(i);
                        translationY += std::to_string(i);
                        translationZ += std::to_string(i);
                        ImGui::SliderFloat(translationX.c_str(), &poses[i][0], -2.0f, 2.0f);
                        ImGui::SliderFloat(translationY.c_str(), &poses[i][1], -2.0f, 2.0f);
                        ImGui::SliderFloat(translationZ.c_str(), &poses[i][2], -2.0f, 2.0f);
                    }
                }
            }

            ImGui::SameLine();
            ImGui::Text("Number of objects = %u", m_renderer->ReturnSceneObject().size());

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Memory used: %.3f MB", m_renderer->GetDeviceMemory() / 1024.0f / 1024.0f);
        }

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            m_renderer->Draw();
        }
    }
}

void Engine::CreateRenderer()
{
    m_renderer = std::unique_ptr<VulkanRenderer>(new VulkanRenderer());
    if (m_renderer->Init(m_window) == EXIT_FAILURE)
    {
        std::cout << "Error while initializing renderer" << std::endl;
        assert(false);
    };
}

void Engine::ShutdownApplication()
{
    m_renderer->CleanUp();
    m_renderer.reset();
    DestroyWindow();
}

