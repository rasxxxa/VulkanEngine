#include "VulkanRenderer.h"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


void VulkanRenderer::CreateInstance()
{
    // check for enableValidationLayers
    
    if (enableValidationLayers && !CheckValidationLayerSupport())
        throw std::runtime_error("Validation layers requested, but not available");

    // Information about the application
    // Most data here doesn't affect problem
    // and it is for developer convenience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan app";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;


    // Creation information for vkInstance (Vulkan instance)
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    // Create list to hold instance extensions
    std::vector<const char*> instanceExtensions = std::vector<const char*>();

    // set up extensions to use
    uint32_t glfwExtensionCount = 0; // may require multiply extensions

    const char** glfwExtensions;     // extensions pust as an array of cstrings to pointers

    // Get GLFW extensions
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // add glfw extensions to list of extensions 
    for (size_t i = 0; i < glfwExtensionCount; i++)
    {
        instanceExtensions.push_back(glfwExtensions[i]);
    }

    if (enableValidationLayers)
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // Check are instance exception supported 
    if (!CheckInstanceExtensionSupport(&instanceExtensions))
        throw std::runtime_error("vkInstance does not support required extension");

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    instance = nullptr;
    // Create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vulkan init");
    }
}

void VulkanRenderer::CreateLogicalDevice()
{
    // Get the queue family indices for the chosen physical device
    QueueFamilyIndices indices = GetQueueFamilies(mainDevice.physicalDevice);

    // for queue creation information, and set for family indices
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices{ indices.graphicsFamily, indices.presentationFamily };

    // Queues the logical device needs to create and info to do so
    for (int queueFamilyIndex : queueFamilyIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1; // number of queues to create
        float priority = 1.0f;
        queueCreateInfo.pQueuePriorities = &priority; // vulkans needs to know how to handle multiply queues so decide priority
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // ifnormation to create logical device 
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    VkPhysicalDeviceFeatures physicalFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &physicalFeatures; // shaders, geometry...

    VkResult vkResult = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
    if (vkResult != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to initialze logical device");
    }

    // Queues are created at the same time as the device
    // so we want hangle to queues
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

void VulkanRenderer::SetupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanRenderer::CreateSwapChain()
{
    // Get swap chain details so we can pick best settings
    SwapChainDetails details = GetSwapChainDetails(mainDevice.physicalDevice);

    // 1. Choose best surface format
    // 2. Choose best presentation mode
    // 3. Choose swap chain image resolution
    VkSurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat(details.formats);
    VkPresentModeKHR presentMode = ChooseBestPresentationMode(details.presentationModes);
    VkExtent2D extent = ChooseSwapExtent(details.surfaceCapabilities);

    // how many images are in swapchain. Get 1 more than minimum to allow tripple buffering
	uint32_t imageCount = details.surfaceCapabilities.minImageCount + 1;

	// If imageCount higher than max, then clamp down to max
	// If 0, then limitless
	if (details.surfaceCapabilities.maxImageCount > 0
		&& details.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = details.surfaceCapabilities.maxImageCount;
	}

    // Creation information for swap chain
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;														// Swapchain surface
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.preTransform = details.surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // blending
    swapChainCreateInfo.clipped = VK_TRUE;
    
    // Get queue family indices 
    QueueFamilyIndices indices = GetQueueFamilies(mainDevice.physicalDevice);

    // if graphics and presentation are different then swapchain must let images be 
    // shared between families
    if (indices.graphicsFamily != indices.presentationFamily)
    {
        uint32_t queueFamilyIndices[] =
        {
            (uint32_t)indices.graphicsFamily,
            (uint32_t)indices.presentationFamily
        };

        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;                // numbers of queues that share images
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; // array of queues to share between
    }
    else
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;

    }
    // if old swap chain been destroyed and this one replaces it
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create swap chain
    VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a swapchain");
    }

    // Store for later reference
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    uint32_t swapChainImageCount = 0;
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, nullptr);

    std::vector<VkImage> images(swapChainImageCount);
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, images.data());

    for (auto image : images)
    {
        SwapChainImage swapImage = {};
        swapImage.image = image;

        // CREATE IMAGE VIEW
        swapImage.imageView = CreateImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        swapChainImages.push_back(swapImage);
    } 
}

void VulkanRenderer::CreateDepthBufferImage()
{
    // Get supported format for depth buffer
    depthFormat = ChooseSupportedFormat(
        { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Create depth buffer image
    depthBufferImage = CreateImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory);

    // Create depth buffer image view
    depthBufferImageView = CreateImageView(depthBufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void VulkanRenderer::CreateFramebuffers()
{
    swapChainFrameBuffers.resize(swapChainImages.size());

    // create a framebuffer for each swap chain image
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        std::array<VkImageView, 2> attachment =
        {
            swapChainImages[i].imageView,
            depthBufferImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;                                     // Render pass layout the framebuffer will be used with
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
        framebufferInfo.pAttachments = attachment.data();                           // List of attachments (1:1) with render pass
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;                                                 // framebuffer layers
        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Frame buffer cannot be created");
        }
    }
}

void VulkanRenderer::CreateCommandPool()
{

    // get indices of queue family from device
    QueueFamilyIndices indices = GetQueueFamilies(mainDevice.physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily; // Queue family type that buffers from this command pool will use

    // Create a graphics queue family command pool
    VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo, nullptr, &graphicsCommandPool);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool");
    }

}

void VulkanRenderer::CreateCommandBuffers()
{
    // Resize command buffer 
    commandBuffers.resize(swapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo cbAllocInfo = {};
    cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.commandPool = graphicsCommandPool;
    cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // vk cmd execute commands / buffer you submit directly to queue. Cant be called by other buffers
                                                         // secondary               / cant be called directly. Can be called from other
    cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    // allocate command buffers and place handles in array of buffers
    VkResult result  = vkAllocateCommandBuffers(mainDevice.logicalDevice, &cbAllocInfo, commandBuffers.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void VulkanRenderer::Draw()
{

    vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);

    // -- Get Next image --
    // Get index of next image to be drawn to, and signal semaphore when ready to be drawn to
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mainDevice.logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    RecordCommands(imageIndex);

    UpdateUniformBuffer(imageIndex);

    // -- Submit command buffer to render
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1; // Number of semaphores to wait
    submitInfo.pWaitSemaphores = &imageAvailable[currentFrame]; // list of semaphores to wait on
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.pWaitDstStageMask = waitStages; // stage to check semaphores at
    submitInfo.commandBufferCount = 1;         // Number of command buffers to submit
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; // command buffer to submit
    submitInfo.signalSemaphoreCount = 1; // Numbers of semaphores to signal
    submitInfo.pSignalSemaphores = &renderFinished[currentFrame]; // semaphores to signals when command buffer finishes

    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer to queue!");
    }

    // -- PRESENT ReNDERED IMAGE TO SCREEN --
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1; // Number of semaphores to wait
    presentInfo.pWaitSemaphores = &renderFinished[currentFrame]; // semaphore to wait
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;   // index of images in swapchains to present

    result = vkQueuePresentKHR(presentationQueue, &presentInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

VkFormat VulkanRenderer::ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
    // Loop through options and find compatible one
    for (const VkFormat& format : formats)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(mainDevice.physicalDevice, format, &properties);

        // Depending on tiling choice
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find a matching format!");

}

bool VulkanRenderer::CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
    // Need to get number of extensions to create array of current size to hold extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // Create a list of vkextensionproperties using count

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // check if given extensions are in list of avaliable extensions
    for (const auto& checkExtension : *checkExtensions)
    {
        bool hasExtension = false;
        for (const auto& extension : extensions)
        {
            if (strcmp(checkExtension, extension.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }
        if (!hasExtension)
            return false;
    }
    return true;
}

bool VulkanRenderer::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layerName : validationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    // get device extensions count
    uint32_t extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

    if (!extensionsCount)
        return false;

    // populate list of extensions
    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensions.data());

    // check for extension
    for (const auto& deviceExtension : deviceExtensions)
    {
        bool hasExtension = false;
        for (const auto& extension : extensions)
        {
            if (strcmp(deviceExtension, extension.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }

        if (!hasExtension)
        {
            return false;
        }
    }
    return true;
}

void VulkanRenderer::GetPhysicalDevice()
{
    // Enumerate physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if (deviceCount == 0)
    {
        throw std::runtime_error("Can't find GPU's that support vulkan instances");
    }

    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());
    
    // Pick first device
    for (const auto& device : deviceList)
    {
        if (CheckDeviceSuitable(device))
        {
            mainDevice.physicalDevice = device;
            break;
        }
    }



    //Get properties of new device
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &deviceProperties);

    //minUniformBUfferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;

}

void VulkanRenderer::AllocateDynamicBuffer()
{
    //// Calculate alignment of model data
    //modelUniformAlignment = (sizeof(Model) + minUniformBUfferOffset - 1) & ~(minUniformBUfferOffset - 1);

    //// Create space in memory to hold dynamic buffer that is aligned to our requred alignment and holds max_objects
    //modelTransferSpace = (Model*)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);

}

bool VulkanRenderer::CheckDeviceSuitable(VkPhysicalDevice device)
{

    /*
    // information about the device itself (id,  name, type, vendor)
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    */

    // check for queues
    auto indices = GetQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainVaild = false;

    if (extensionsSupported)
    {
        SwapChainDetails details = GetSwapChainDetails(device);
        swapChainVaild = !details.presentationModes.empty() && !details.formats.empty();
    }

    return indices.isValid() && extensionsSupported && swapChainVaild;
}

void VulkanRenderer::CleanUp()
{
    // Wait untill no action is run on device
    vkDeviceWaitIdle(mainDevice.logicalDevice);


    for (size_t i = 0; i < textureImages.size(); i++)
    {
        vkDestroyImageView(mainDevice.logicalDevice, textureImageViews[i], nullptr);
        vkDestroyImage(mainDevice.logicalDevice, textureImages[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, textureImageMemory[i], nullptr);
    }


    vkDestroyImageView(mainDevice.logicalDevice, depthBufferImageView, nullptr);
    vkDestroyImage(mainDevice.logicalDevice, depthBufferImage, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, depthBufferImageMemory, nullptr);

    //_aligned_free(modelTransferSpace);

    vkDestroyDescriptorPool(mainDevice.logicalDevice, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, descriptorSetLayout, nullptr);

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vkDestroyBuffer(mainDevice.logicalDevice, uniformBuffer[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, uniformBufferMemory[i], nullptr);
        //vkDestroyBuffer(mainDevice.logicalDevice, modelUniformBuffer[i], nullptr);
        //vkFreeMemory(mainDevice.logicalDevice, modelUniformBufferMemory[i], nullptr);
    }

    for (size_t i = 0; i < meshList.size(); i++)
        meshList[i].DestroyBuffer();

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
        vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
    }
    vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);

    for (auto framebuffer : swapChainFrameBuffers)
    {
        vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);
    for (const auto& image : swapChainImages)
    {
        vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);
    }
    vkDestroySwapchainKHR(mainDevice.logicalDevice, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::UpdateModel(glm::mat4 newModel, int modelId)
{
    if (static_cast<size_t>(modelId) >= meshList.size())
        return;

    meshList[modelId].SetModel(newModel);

}

QueueFamilyIndices VulkanRenderer::GetQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    // get queue family info for given devices
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

    // go through each queue family and check if it has one of required types of queue
    int i = 0;
    for (const auto& queueFamily : queueFamilyList)
    {
        // first check if queue family has at least one queue in that family
        // Queue can be multiple times define through bitfield
        // 
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i; // if queue is valid then get index

        }

        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);

        // Check if queue is presentation type can be graphics and presentation
        if (queueFamily.queueCount > 0 && presentationSupport == VK_TRUE)
        {
            indices.presentationFamily = i;
        }

        if (indices.isValid())
        {
            break;
        }
        i++;
    }

    return indices;
}

void VulkanRenderer::CreateSurface()
{
    // Create surface (creates a surface create info struct, runs the create surface function
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create a surface");
    }
}

SwapChainDetails VulkanRenderer::GetSwapChainDetails(VkPhysicalDevice device)
{
    SwapChainDetails swapChainDetails;

    // -- CAPABILITIES
    // Get the surface capabilites for the given surface for the given physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);


    // -- FORMATS
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount)
    {
        swapChainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
    }

    // -- PRESENTATION MODES
    uint32_t presentationCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

    if (presentationCount)
    {
        swapChainDetails.presentationModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
    }

    return swapChainDetails;
}

void VulkanRenderer::RecordCommands(uint32_t currentImage)
{
    // Information about how to begin each command buffer
    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   // bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Buffer can be resubmitted when it has already been submited and waiting execution
    
    // Information about how to begin a render pass (only needed for graphical aplication)

    VkRenderPassBeginInfo renderpassBeginInfo = {};
    renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBeginInfo.renderPass = renderPass;
    renderpassBeginInfo.renderArea.offset = { 0,0 };                     // start point of render pass in pixel
    renderpassBeginInfo.renderArea.extent = swapChainExtent;             // size of region to run render pass on (starting at offset)

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.6f, 0.65f, 0.4f, 1.0f };
    clearValues[1].depthStencil.depth = 1.0f;


    renderpassBeginInfo.pClearValues = clearValues.data();                     // list of clear values 
    renderpassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

    renderpassBeginInfo.framebuffer = swapChainFrameBuffers[currentImage];

    // start recording commands to command buffer!
    VkResult result = vkBeginCommandBuffer(commandBuffers[currentImage], &bufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to start recording command buffer!");
    }

    // Begin render pass
    vkCmdBeginRenderPass(commandBuffers[currentImage], &renderpassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline to be used in render pass
    vkCmdBindPipeline(commandBuffers[currentImage], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    for (size_t j = 0; j < meshList.size(); j++)
    {
        VkBuffer vertexBuffers[] = { meshList[j].GetVertexBuffer() }; // buffers to bind
        VkDeviceSize offsets[] = { 0 };  // offsets into buffers being bound
        vkCmdBindVertexBuffers(commandBuffers[currentImage], 0, 1, vertexBuffers, offsets); // command to bind vertex buffer before drawing to them

        // Bind mesh index buffer with 0 offset and using the uin32 type
        vkCmdBindIndexBuffer(commandBuffers[currentImage], meshList[j].GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

        // Dynamic offset ammount 
        //uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * j;

        vkCmdPushConstants(
            commandBuffers[currentImage],
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(Model),
            (&meshList[j].GetModel()));

        // Bind descriptor sets
        vkCmdBindDescriptorSets(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
            0, 1, &descriptorSets[currentImage], 0, nullptr);

        // execute pipeline
        vkCmdDrawIndexed(commandBuffers[currentImage], meshList[j].GetIndexCount(), 1, 0, 0, 0);
    }

    // End render pass
    vkCmdEndRenderPass(commandBuffers[currentImage]);

    result = vkEndCommandBuffer(commandBuffers[currentImage]);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to stop recording a command buffer");
    }
}

// Best format is subjective
// Format     : VK_FORMAT_R8G8B8A8_UNORM or VK_FORMAT_B8G8R8A8_UNORM
// colorspace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRenderer::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    // if size = 1 all formats are avaliable 
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        return { VkFormat::VK_FORMAT_R8G8B8A8_UNORM, VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    // try to find optimal format
    for (const auto& format : formats)
    {
        if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && 
            format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    // default return first
    return formats[0];
}

VkPresentModeKHR VulkanRenderer::ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentModes)
{
    // Look for mailbox presentation mode
    for (const auto& presentationMode : presentModes)
    {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentationMode;
    }

    // if cant find mailbox use default fifo
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return surfaceCapabilities.currentExtent;
    }
    else
    {
        // if value can vary need to set manually
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        // create new extent with window size
        VkExtent2D extent = {};
        extent.width = static_cast<uint32_t>(width);
        extent.height = static_cast<uint32_t>(height);

        // Surface also defines max and min, so make sure within boundaries by clamping value
        extent.width = std::max(surfaceCapabilities.minImageExtent.width,std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
        return extent;
    }
}

VkImage VulkanRenderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceMemory* imageMemory)
{
    // Create image

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1; 
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = useFlags;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image;
    VkResult result = vkCreateImage(mainDevice.logicalDevice, &imageCreateInfo, nullptr, &image);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an image");
    }

    // Create memory for image

    // Get memory requirements for a type of image
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mainDevice.logicalDevice, image, &memoryRequirements);

    // Allocate memory using requirements
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, propertyFlags, mainDevice.physicalDevice);

    result = vkAllocateMemory(mainDevice.logicalDevice, &memoryAllocateInfo, nullptr, imageMemory);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate memory for image");
    }

    // Connect memory to image
    vkBindImageMemory(mainDevice.logicalDevice, image, *imageMemory, 0);

    return image;
}

VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // allows remmaping of rgba componnets to other values
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // Subresources allow the view to view only a part of an image
    imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags; // Which asspect of image to view (e.g COLOR_BIT for viewwing color
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;         // Start mipmap level to view from 
    imageViewCreateInfo.subresourceRange.levelCount = 1; // number of mipmap level to view
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0; // start array level to view from
    imageViewCreateInfo.subresourceRange.layerCount = 1; // number of array levels to view

    // Create image view 
    VkImageView imageView;
    VkResult result = vkCreateImageView(mainDevice.logicalDevice, &imageViewCreateInfo, nullptr, &imageView);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an image view");
    }
    return imageView;
}

void VulkanRenderer::CreateRenderPass()
{
    // Color attachment of render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;                   // format to use for attachment
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                 // number of samples to write for multisampling
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;            // what to do with attachemnt before rendering
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;          // Describes what  to do with attachment after rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; 
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Framebuffer data will be stored as an image, but images can be given different data layouts
    // to give optimal use for certain operations
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;       // image data layout before render pass starts
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // image data layout after render pass

    // Depth attachment of render pass
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = ChooseSupportedFormat(
        { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Attachment reference uses an attachment index that refers to index in the attachment list passed to renderpasscreateinfo
    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0; // index not a number
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment reference
    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1; // index not a number
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    // Information about a particual subpass the render pass is using
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;     // Pipeline type subpass is to be bound to
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    // Need to determine when layout transition occur using subpass dependencies
    std::array<VkSubpassDependency, 2> subpassDependencies;

    // Conversion from VK_IMAGE_LAYOUT_UNDEFINIED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    // Transition must happen after ...
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;                                                              // Subpass index 
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;                                           // Pipeline stage
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;                                                     // stage access mask

    // But must happen before...
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0;


    // Conversion from VK_IMAGE_LAYOUT_UNDEFINIED to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    // Transition must happen after ...
    subpassDependencies[1].srcSubpass = 0;                                                    
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;      
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // But must happen before...
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0;
    
    std::array<VkAttachmentDescription, 2> renderPassAttachments = { colorAttachment, depthAttachment };

    // create info for rendere pass
    VkRenderPassCreateInfo renderpassCreateInfo = {};
    renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
    renderpassCreateInfo.pAttachments = renderPassAttachments.data();
    renderpassCreateInfo.subpassCount = 1;
    renderpassCreateInfo.pSubpasses = &subpass;
    renderpassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderpassCreateInfo.pDependencies = subpassDependencies.data();

    VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderpassCreateInfo, nullptr, &renderPass);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a render pass");
    }

}

void VulkanRenderer::CreateGraphicsPipeline()
{
    // read spir-v code of shaders
    auto vertexShaderCode = readFile("Shaders/vert.spv");
    auto fragmentShaderCode = readFile("Shaders/frag.spv");

    // Build shader modules to link to graphics pipeline 
    auto moduleVertex = CreateShaderModule(vertexShaderCode);
    auto moduleFragment = CreateShaderModule(fragmentShaderCode);

    // SHADER STAGE CREATION INFORMATION

    // VERTEX STAGE CREATION INFORMATION

    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;   // shader stage name
    vertexShaderCreateInfo.module = moduleVertex;                // shader module to be used by stage
    vertexShaderCreateInfo.pName = "main";                       // entry point into shader


    // FRAGMENT STAGE CREATION INFORMATION
    
    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;   // shader stage name
    fragmentShaderCreateInfo.module = moduleFragment;                // shader module to be used by stage
    fragmentShaderCreateInfo.pName = "main";                       // entry point into shader

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vertexShaderCreateInfo,
        fragmentShaderCreateInfo
    };

    // CREATE PIPELINE

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0; // Can bind multiple streams of data, this defines which one
    bindingDescription.stride = sizeof(Vertex); // size of a single vertex object
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // How to move between data after each vertex
                                                                // VK_VERTEX_INPUT_RATE_VERTEX    : Move on to the next vertex
                                                                // VK_VERTEX_INPUT_RATE_INSTANCE  : Move to a vertex for the next instance


    // How the data for an attribute is defined within a vertex
    std::array<VkVertexInputAttributeDescription, 2> attributeDescription;

    attributeDescription[0].binding = 0;  // Which binding the data is at (should be same as above)
    attributeDescription[0].location = 0; // Location in shader where data will be read from
    attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Format the data will take (also helps define size of data)
    attributeDescription[0].offset = offsetof(Vertex, m_position); // Where this attribute is defined in the data for a single vertex

    // Color attribute

    attributeDescription[1].binding = 0;
    attributeDescription[1].location = 1;
    attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription[1].offset = offsetof(Vertex, m_color);

    // VERTEX INPUT 
    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo = {};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexStateCreateInfo.pVertexBindingDescriptions = &bindingDescription; // List of vertex binding description (data spacing, strive information)
    vertexStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexStateCreateInfo.pVertexAttributeDescriptions = attributeDescription.data(); // List of vertex attribute description (data format, where to bind.. )

    // -- INPUT ASSEMBLY --
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateinfo = {};
    inputAssemblyCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateinfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Primitive type to assemble vertices at
    inputAssemblyCreateinfo.primitiveRestartEnable = VK_FALSE; // Continue to draw with earlier primitives (vertices), allow overridng of 'strip' topology to start new primitve

    // -- VIEWPORT & SCISSOR --
    // Create a viewport info struct
    VkViewport viewPort = {};
    viewPort.x = 0.0f;                               // x start coordinate
    viewPort.y = 0.0f;                               // y start coordinate
    viewPort.width = (float)swapChainExtent.width;   // width of viewport
    viewPort.height = (float)swapChainExtent.height; // height of viewport
    viewPort.minDepth = 0.0f;                        // min framebuffer depth
    viewPort.maxDepth = 1.0f;                        // max framebuffer depth

    // Create a scissor info struct
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };                       // offset to use region from
    scissor.extent = swapChainExtent;                // extent to describe region to use

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewPort;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    // -- DYNAMIC STATES --
    // Dynamic states to enable

    //std::vector<VkDynamicState> dynamicStateEnables;
    //dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT); // Dynamic viewport : can resize in command buffer with vkCmdSetViewPort(commandBuffer, 0, 1
    //dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);  // Dynamic scissor can resize in commandBuffer with vkSetScissor(command buffer, i, 1, scissor)

    //// Dynamic state creation info
    //VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    //dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    //dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    //dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

    // -- RASTERIZER --
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;                 // change if fragments beyond near/far planes are clipped (default) or clapped to plane
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;          // whether to discard data and skip rasterzier. Never creates fragments, only suitable for pipeline without framebuffer output
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;          // how to handle filling points between vertices
    rasterizerCreateInfo.lineWidth = 1.0f;                            // how thick lines should be when drawn
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;            // which face of a tri to cull
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // winding to determine which side is front
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;                  // whether to add depth bias to fragments (good for stopping 'shadow acne')


    // -- MULTISAMPLING --
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
    multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE; // enable multisampling shading
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // number of samples to use per fragment

    // -- BLENDING -- 
    // Blending decides how to blend a new color being writter to a fragment, with the old value

    // Blend attachment state
    VkPipelineColorBlendAttachmentState colorState = {};
    colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorState.blendEnable = VK_TRUE; // enable blending

    // blending uses equation (srcColorBlendFactor * new Color) colorBlendOp(destColorBlendFactor * oldColor)
    colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorState.colorBlendOp = VK_BLEND_OP_ADD;

    // Sumarized (VK_BLEND_FACTOR_SRC_ALPHA * newCOLOR) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * oldColor)
    // (new Color alpha * new color) + (1 - newColorAlpha)*(old color)

    colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorState.alphaBlendOp = VK_BLEND_OP_ADD;

    // sumarized (1 * newAlpha) + (0 * oldAplha) = newAlpha

    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
    colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingCreateInfo.logicOpEnable = VK_FALSE; // alternative to calculation is to use logical operation
    //colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingCreateInfo.attachmentCount = 1;
    colorBlendingCreateInfo.pAttachments = &colorState;

    // -- PIPELINE LAYOUT 

    VkPipelineLayoutCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineCreateInfo.setLayoutCount = 1;
    pipelineCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineCreateInfo.pushConstantRangeCount = 1;
    pipelineCreateInfo.pPushConstantRanges = &pushConstantRange;

    // Create pipeline layout
    VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineCreateInfo, nullptr, &pipelineLayout);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    // -- DEPTH STENCIL TESTING -- 
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;  // Enable checking depth to determine fragment write
    depthStencilInfo.depthWriteEnable = VK_TRUE; // Enable writing to depth buffer (to replace old values)
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; // Comparisson operation that allows overwrite (is in front)
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE; // Depth bound test: does the depth value exist between two bounds
    depthStencilInfo.stencilTestEnable = VK_FALSE; // Enable stencil test

    VkGraphicsPipelineCreateInfo pipelineGraphicsCreateInfo = {};
    pipelineGraphicsCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineGraphicsCreateInfo.stageCount = 2;
    pipelineGraphicsCreateInfo.pStages = shaderStages;
    pipelineGraphicsCreateInfo.pVertexInputState = &vertexStateCreateInfo;
    pipelineGraphicsCreateInfo.pInputAssemblyState = &inputAssemblyCreateinfo;  // all the fixed function pipeline states
    pipelineGraphicsCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineGraphicsCreateInfo.pDynamicState = nullptr;
    pipelineGraphicsCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineGraphicsCreateInfo.pMultisampleState = &multisamplingCreateInfo;
    pipelineGraphicsCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
    pipelineGraphicsCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineGraphicsCreateInfo.layout = pipelineLayout;                          // pipeline layout pipeline should use
    pipelineGraphicsCreateInfo.renderPass = renderPass;                          // renderpass description the pipeline is compatible with
    pipelineGraphicsCreateInfo.subpass = 0;                                      // subpass of render pass to use with pipeline

    // Pipeline derivatives : can create multiple pipelines that derive from one another for optimisation
    pipelineGraphicsCreateInfo.basePipelineHandle = VK_NULL_HANDLE;              // existing pipeline to derive from
    pipelineGraphicsCreateInfo.basePipelineIndex = -1;                           // or index of pipeline being created to derive

    // create graphics pipeline
    result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineGraphicsCreateInfo, nullptr, &graphicsPipeline);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create graphics pipeline");
    }

    // DESTROY SHADER MODULES NOT NEEDED AFTER PIPELINE CREATED
    vkDestroyShaderModule(mainDevice.logicalDevice, moduleFragment, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice, moduleVertex, nullptr);

}

VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code)
{
    // shader module create informations
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &createInfo, nullptr, &shaderModule);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a shader module");
    }

    return shaderModule;
}

void VulkanRenderer::CreateSynchronisation()
{
    imageAvailable.resize(MAX_FRAME_DRAWS);
    renderFinished.resize(MAX_FRAME_DRAWS);
    drawFences.resize(MAX_FRAME_DRAWS);


    // Semaphore creation information
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        if (vkCreateSemaphore(mainDevice.logicalDevice, &createInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mainDevice.logicalDevice, &createInfo, nullptr, &renderFinished[i]) != VK_SUCCESS ||
            vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create a semaphore");
        }
    }
}

void VulkanRenderer::CreateDescriptorSetLayout()
{
    // View projection Binding info
    
    VkDescriptorSetLayoutBinding vpLayoutBinding = {};
    vpLayoutBinding.binding = 0;                                                 // Binding point in shader designated by binding number in shader
    vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;          // Type of descriptor
    vpLayoutBinding.descriptorCount = 1;                                         // Number of descriptor for binding
    vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;                     // Shader stage to bind to
    vpLayoutBinding.pImmutableSamplers = nullptr;                                // For texture: Can make sampler data unchangeable

    //// Model binding info
    //VkDescriptorSetLayoutBinding modelLayoutBinding = {};
    //modelLayoutBinding.binding = 1;
    //modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    //modelLayoutBinding.descriptorCount = 1;
    //modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //modelLayoutBinding.pImmutableSamplers = nullptr;
    //
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { vpLayoutBinding/*, modelLayoutBinding */};

    // Create descriptor set layout with given bindings
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());                                            // Number of binding infos
    layoutCreateInfo.pBindings = layoutBindings.data();                               // Array of binding infos

    // Create descriptor set layout
    VkResult result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

}

void VulkanRenderer::CreatePushConstantRange()
{
    // Define push constant values (no 'create' needed!)
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Shader stage push constall will go to
    pushConstantRange.offset = 0;                              // Offset info given data to pass to push constant
    pushConstantRange.size = sizeof(Model);                    // Size of data being passed
}

void VulkanRenderer::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(ViewProjection);

    //VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

    // One uniform buffer for each image (and by extension, command buffer)
    uniformBuffer.resize(swapChainImages.size());
    uniformBufferMemory.resize(swapChainImages.size());
    //modelUniformBuffer.resize(swapChainImages.size());
    //modelUniformBufferMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        CreateBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer[i], &uniformBufferMemory[i]);

        //CreateBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        //    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelUniformBuffer[i], &modelUniformBufferMemory[i]);

    }
}

void VulkanRenderer::CreateDescriptorPool()
{

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(uniformBuffer.size());


    //VkDescriptorPoolSize modelPoolSize = {};
    //modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    //modelPoolSize.descriptorCount = static_cast<uint32_t>(modelUniformBuffer.size());

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { poolSize/*, modelPoolSize*/ };

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());     // Maximum number of descriptor sets that can be created from pool
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());                                         // Ammount of pool sizes being passed
    poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();                                    // Pool sizes to create pool with



    VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor pool");
    }

}

void VulkanRenderer::CreateDescriptorSets()
{
    descriptorSets.resize(swapChainImages.size());

    std::vector<VkDescriptorSetLayout> setLayouts(swapChainImages.size(), descriptorSetLayout);

    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = descriptorPool;
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    setAllocInfo.pSetLayouts = setLayouts.data();


    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, descriptorSets.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // Update all of descriptor set buffer bindings
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        // Buffer info and data offset info
        VkDescriptorBufferInfo descriptorInfo = {};
        descriptorInfo.buffer = uniformBuffer[i];
        descriptorInfo.offset = 0;
        descriptorInfo.range = sizeof(ViewProjection);

        // Data about connection between binding and buffer
        VkWriteDescriptorSet vpSetWrite = {};
        vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpSetWrite.dstSet = descriptorSets[i];
        vpSetWrite.dstBinding = 0;                                     // Binding to update (matches to binding in shader)
        vpSetWrite.dstArrayElement = 0;
        vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vpSetWrite.descriptorCount = 1;
        vpSetWrite.pBufferInfo = &descriptorInfo;

        // Model descriptor
        // Model buffer binding info 

        //VkDescriptorBufferInfo descriptorBufferInfo = {};
        //descriptorBufferInfo.buffer = modelUniformBuffer[i];
        //descriptorBufferInfo.offset = 0;
        //descriptorBufferInfo.range = modelUniformAlignment;

        //VkWriteDescriptorSet modelSetWrite = {};
        //modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //modelSetWrite.dstSet = descriptorSets[i];
        //modelSetWrite.dstBinding = 1;
        //modelSetWrite.dstArrayElement = 0;
        //modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        //modelSetWrite.descriptorCount = 1;
        //modelSetWrite.pBufferInfo = &descriptorBufferInfo;

        std::vector<VkWriteDescriptorSet> descriptorSetsWrites = { vpSetWrite/*, modelSetWrite*/ };

        // Update the descriptor sets with new buffer/binding info
        vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(descriptorSetsWrites.size()), descriptorSetsWrites.data(), 0, nullptr);
    }

}

int VulkanRenderer::CreateTextureImage(std::string fileName)
{
    int width, height;
    VkDeviceSize imageSize;
    auto imageData = LoadTextureFile(fileName, &width, &height, &imageSize);

    // Creating staging buffer to hold loaded data
    VkBuffer imageStagingBuffer;
    VkDeviceMemory imageStagingBufferMemory;
    CreateBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);

    // Copy image data to staging buffer
    void* data;
    vkMapMemory(mainDevice.logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vkUnmapMemory(mainDevice.logicalDevice, imageStagingBufferMemory);

    // Free original data
    stbi_image_free(imageData);

    // Create image to hold final texture
    VkImage texImage;
    VkDeviceMemory texImageMemory;

    texImage = CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

    // Trainsition image to be dst for copy operation
    TransitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // copy image to data
    CopyImageBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, imageStagingBuffer, texImage, width, height);

    // Trasnition image to be shader readable
    TransitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    // Add texture data to vector for reference
    textureImages.push_back(texImage);
    textureImageMemory.push_back(texImageMemory);

    // Destroy stagin buffers
    vkDestroyBuffer(mainDevice.logicalDevice, imageStagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, imageStagingBufferMemory, nullptr);

    // Return index of new texture image
    return textureImages.size() - 1;
}

int VulkanRenderer::CreateTexture(std::string fileName)
{
    // Create TextureImage and get its location in array
    int textureImageLoc = CreateTextureImage(fileName);

    VkImageView imageView = CreateImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    textureImageViews.push_back(imageView);

    // TODO: Create descriptor set here

    return 0;
}

void VulkanRenderer::UpdateUniformBuffer(uint32_t imageIndex)
{
    void* data;
    vkMapMemory(mainDevice.logicalDevice, uniformBufferMemory[imageIndex], 0, sizeof(ViewProjection), 0, &data);
    memcpy(data, &modelviewprojection, sizeof(ViewProjection));
    vkUnmapMemory(mainDevice.logicalDevice, uniformBufferMemory[imageIndex]);

    //for (size_t i = 0; i < meshList.size(); i++)
    //{
    //    Model* thisModel = (Model*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment));
    //    *thisModel = meshList[i].GetModel();
    //}

    //vkMapMemory(mainDevice.logicalDevice, modelUniformBufferMemory[imageIndex], 0, modelUniformAlignment * meshList.size(), 0, &data);
    //memcpy(data, modelTransferSpace, modelUniformAlignment * meshList.size());
    //vkUnmapMemory(mainDevice.logicalDevice, modelUniformBufferMemory[imageIndex]);

}

stbi_uc* VulkanRenderer::LoadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
{
    // Number of channels image uses
    int channels;

    // load pixel data for image
    std::string fileLoc = "Textures/" + fileName;
    stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        throw std::runtime_error("Failed to load an image: " + fileName);
    }


    *imageSize = static_cast<long>(*width) * static_cast<long>(*height) * 4;

    return image;
}

VulkanRenderer::VulkanRenderer()
{

}

VulkanRenderer::~VulkanRenderer()
{

}

int VulkanRenderer::Init(GLFWwindow* newWindow)
{
    window = newWindow;

    try
    {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        GetPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreatePushConstantRange();
        CreateGraphicsPipeline();
        CreateDepthBufferImage();
        CreateFramebuffers();
        CreateCommandPool();
        int firstTexture = CreateTexture("emoji.png");

        modelviewprojection.m_projection = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
        modelviewprojection.m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        modelviewprojection.m_projection[1][1] *= -1;

        // Vertex data
        std::vector<Vertex> meshVertices = {
            { { -0.4, 0.4, 0.0 },{ 1.0f, 0.0f, 0.0f } },	// 0
            { { -0.4, -0.4, 0.0 },{ 1.0f, 0.0f, 0.0f } },	    // 1
            { { 0.4, -0.4, 0.0 },{ 1.0f, 0.0f, 0.0f } },    // 2
            { { 0.4, 0.4, 0.0 },{ 1.0f, 0.0f, 0.0f } },   // 3
        };

        std::vector<Vertex> meshVertices2 = {
            { { -0.25, 0.6, 0.0 },{ 0.0f, 0.0f, 1.0f } },	// 0
            { { -0.25, -0.6, 0.0 },{ 0.0f, 0.0f, 1.0f } },	    // 1
            { { 0.25, -0.6, 0.0 },{ 0.0f, 0.0f, 1.0f } },    // 2
            { { 0.25, 0.6, 0.0 },{ 0.0f, 0.0f, 1.0f } },   // 3
        };
        // Index data

        std::vector<uint32_t> meshIndices =
        {
            0, 1, 2,
            2, 3, 0
        };
        Mesh firstMesh = Mesh(mainDevice.physicalDevice, mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, &meshVertices, &meshIndices);
        Mesh firstMesh2 = Mesh(mainDevice.physicalDevice, mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, &meshVertices2, &meshIndices);

        meshList.push_back(firstMesh);
        meshList.push_back(firstMesh2);

        glm::mat4 meshModelMatrix = meshList[0].GetModel().m_model;
        meshModelMatrix = glm::rotate(meshModelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        meshList[0].SetModel(meshModelMatrix);

        CreateCommandBuffers();
    //    AllocateDynamicBuffer();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateSynchronisation();
    }
    catch (const std::runtime_error& exeption)
    {
        std::cout << "ERROR: " << exeption.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
