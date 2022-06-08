#include "AnimationLoader.h"
#include <chrono>

std::recursive_mutex AnimationLoader::m_lock;

AnimationLoader::AnimationLoader(const std::string& path, VulkanRenderer* r)
{
    auto animPath = std::filesystem::current_path();
    animPath.append(path);
    m_path = animPath;
    renderer = r;
    mtRand = std::mt19937(randomDevice());
    distributionX = std::uniform_real_distribution<float>(-0.5f, 0.1f);
    distributionY = std::uniform_real_distribution<float>(0.0f, 0.4f);
    rangeMin = -1;
    rangeMax = -1;
}

AnimationLoader::AnimationLoader(const std::string& path, unsigned int rangedMin, unsigned int rangeMax, VulkanRenderer* r)
{
    auto animPath = std::filesystem::current_path();
    animPath.append(path);
    m_path = animPath;
    renderer = r;
    mtRand = std::mt19937(randomDevice());
    distributionX = std::uniform_real_distribution<float>(-0.5f, 0.1f);
    distributionY = std::uniform_real_distribution<float>(0.0f, 0.4f);
    rangeMin = rangedMin;
    this->rangeMax = rangeMax;
}

void ThreadHelperFunc(std::vector<std::string>& images, size_t lowerBound, size_t upperBound, VulkanRenderer* renderer)
{
    for (size_t it = lowerBound; it < upperBound; it++)
    {
        VulkanRenderer::imagesID[images[it]] = renderer->CreateTexture(images[it]);
    }
    std::cout << "Thread finished: " << std::this_thread::get_id() << std::endl;
}

std::vector<Mesh> AnimationLoader::Load()
{
    std::vector<Mesh> meshesLoaded;
    std::vector<std::string> pathsToImages;
    if (std::filesystem::is_directory(m_path))
    {
        for (const auto& dirEntry : std::filesystem::directory_iterator(m_path))
            pathsToImages.push_back(dirEntry.path().generic_string());
    }

    if (rangeMin == -1 || rangeMax == -1)
    {
        rangeMin = 0;
        rangeMax = pathsToImages.size();
    }
    auto start = std::chrono::high_resolution_clock::now();
    if (USE_THREAD_LOADING)
    {

        size_t numberOfElementsPerThread = (rangeMax - rangeMin) / NUMBER_OF_THREADS;
        std::vector<std::thread> threads;

        for (size_t thread = 0; thread < NUMBER_OF_THREADS; thread++)
        {
            threads.emplace_back(ThreadHelperFunc, std::ref(pathsToImages), thread * numberOfElementsPerThread, ((thread == NUMBER_OF_THREADS - 1) ? rangeMax : (thread + 1) * numberOfElementsPerThread), renderer);
        }

        for (size_t thread = 0; thread < NUMBER_OF_THREADS; thread++)
            threads[thread].join();

        for (size_t tex = rangeMin; tex < rangeMax; tex++)
        {
            auto size = 0.5f;

            auto posX = distributionX(mtRand);
            auto posY = distributionY(mtRand);
            //auto posZ = distributionX(mtRand) + 0.5f;

            // Take random position for testing
            std::vector<Vertex> meshVertices =
            {
                    { { posX, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f}, 1.0f},	// 0
                    { { posX, posY - size, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f}, 1.0f},	    // 1
                    { { posX + size, posY - size, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f}, 1.0f },    // 2
                    { { posX + size, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f}, 1.0f  },   // 3
            };

            meshesLoaded.emplace_back(renderer->mainDevice.physicalDevice, renderer->mainDevice.logicalDevice, renderer->graphicsQueue, renderer->graphicsCommandPool, &meshVertices, &MESH_INDICES, VulkanRenderer::imagesID[pathsToImages[tex]]);
        }
    }
    else
    {
        for (size_t tex = rangeMin; tex < rangeMax; tex++)
        {
            auto size = 0.5f;

            auto posX = distributionX(mtRand);
            auto posY = distributionY(mtRand);
            //auto posZ = distributionX(mtRand) + 0.5f;

            // Take random position for testing
            std::vector<Vertex> meshVertices =
            {
                    { { posX, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f}, 1.0f},	// 0
                    { { posX, posY - size, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f}, 1.0f},	    // 1
                    { { posX + size, posY - size, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f}, 1.0f },    // 2
                    { { posX + size, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f}, 1.0f  },   // 3
            };

            meshesLoaded.emplace_back(renderer->mainDevice.physicalDevice, renderer->mainDevice.logicalDevice, renderer->graphicsQueue, renderer->graphicsCommandPool, &meshVertices, &MESH_INDICES, renderer->CreateTexture(pathsToImages[tex]));
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    double difference = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Loading time: " << difference << std::endl;

    return meshesLoaded;
}

