#include "AnimationLoader.h"

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

#ifndef USE_MULTITHREADS

    for (size_t tex = rangeMin; tex < rangeMax; tex++)
    {
        auto size = 0.5f;

        auto posX = distributionX(mtRand);
        auto posY = distributionY(mtRand);
        //auto posZ = distributionX(mtRand) + 0.5f;

        std::vector<Vertex> meshVertices =
        {
                { { posX, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f}, 1.0f},	// 0
                { { posX, posY - size, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f}, 1.0f},	    // 1
                { { posX + size, posY - size, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f}, 1.0f },    // 2
                { { posX + size, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f}, 1.0f  },   // 3
        };

        meshesLoaded.emplace_back(renderer->mainDevice.physicalDevice, renderer->mainDevice.logicalDevice, renderer->graphicsQueue, renderer->graphicsCommandPool, &meshVertices , &MESH_INDICES, renderer->CreateTexture(pathsToImages[tex]));
    }

#else

#endif // !THREADS

    return meshesLoaded;
}
