#include "trianglerenderer.h"
#include <QVulkanFunctions>
#include <QFile>
#include <QCoreApplication>
#include <thread>

using namespace VolumeStructure;

glm::vec4 axisRange;
// Note that the vertex data and the projection matrix assume OpenGL. With
// Vulkan Y is negated in clip space and the near/far plane is at 0/1 instead
// of -1/1. These will be corrected for by an extra transformation when
// calculating the modelview-projection matrix.
/*
static float vertexData[] = { // Y up, front = CCW
     0.0f,   0.5f,   1.0f, 0.0f, 0.0f,
    -0.5f,  -0.5f,   0.0f, 1.0f, 0.0f,
     0.5f,  -0.5f,   0.0f, 0.0f, 1.0f
};
*/
//the align problem of texture image
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f},{0.0f,0.0f}},
    {{0.5f, -0.5f},{1.0f,0.0f}},
    {{0.5f, 0.5f},{1.0f,1.0f}},
    {{-0.5f, 0.5f},{0.0f,1.0f}}
};

//index
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

VkDeviceSize vertexAllocSize;//as offset
uint32_t indicesSize = sizeof (uint16_t) * indices.size();
//static const int UNIFORM_DATA_SIZE = 16 * sizeof(float);
VkDeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();

const VkFormat scalarTempGradFormat = VK_FORMAT_R32G32_SFLOAT;
const VkFormat GradientFormat = VK_FORMAT_R32G32B32_SFLOAT;

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

TriangleRenderer::TriangleRenderer(QVulkanWindow *w)
    : m_window(w)
{
    //init range
   // ubo.xAxisRange = glm::vec2(0.0f, 100000.0f);
   // ubo.yAxisRange = glm::vec2(0.0f, 100000.0f);
}

VkShaderModule TriangleRenderer::createShader(const QString &name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Failed to read shader %s", qPrintable(name));
        return VK_NULL_HANDLE;
    }
    QByteArray blob = file.readAll();
    file.close();

    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = blob.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(blob.constData());
    VkShaderModule shaderModule;
    VkResult err = m_devFuncs->vkCreateShaderModule(m_window->device(), &shaderInfo, nullptr, &shaderModule);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create shader module: %d", err);
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

void TriangleRenderer::initResources()
{
    qDebug("initResources");

    VkDevice dev = m_window->device();
    m_devFuncs = m_window->vulkanInstance()->deviceFunctions(dev);

    const int concurrentFrameCount = m_window->concurrentFrameCount();
    const VkPhysicalDeviceLimits *pdevLimits = &m_window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;
    qDebug("uniform buffer offset alignment is %u", (uint) uniAlign);

    VkBufferCreateInfo bufInfo{};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // Our internal layout is vertex, uniform, uniform, ... with each uniform buffer start offset aligned to uniAlign.
    vertexAllocSize = vertexSize;//aligned(sizeof(vertices), uniAlign);
    const VkDeviceSize uniformAllocSize = aligned(sizeof(UniformBufferObject), uniAlign);
    const VkDeviceSize AllocSize = aligned(indicesSize + vertexSize, uniAlign);
    bufInfo.size = AllocSize + concurrentFrameCount * uniformAllocSize;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_buf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create buffer: %d", err);

    VkMemoryRequirements memReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_buf, &memReq);

    VkMemoryAllocateInfo memAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReq.size,
        m_window->hostVisibleMemoryIndex()
    };

    err = m_devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &m_bufMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    err = m_devFuncs->vkBindBufferMemory(dev, m_buf, m_bufMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind buffer memory: %d", err);

    quint8 *p;
    err = m_devFuncs->vkMapMemory(dev, m_bufMem, 0, memReq.size, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);
    memcpy(p, vertices.data(), vertexSize);
    memcpy(p + vertexAllocSize, indices.data(), indicesSize);

    memset(m_uniformBufInfo, 0, sizeof(m_uniformBufInfo));
    for (int i = 0; i < concurrentFrameCount; ++i) {
        const VkDeviceSize offset = AllocSize + i * uniformAllocSize;
        memcpy(p + offset, &ubo, sizeof(UniformBufferObject));
        m_uniformBufInfo[i].buffer = m_buf;
        m_uniformBufInfo[i].offset = offset;
        m_uniformBufInfo[i].range = uniformAllocSize;
    }
    m_devFuncs->vkUnmapMemory(dev, m_bufMem);

    VkVertexInputBindingDescription vertexBindingDesc = VolumeStructure::Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, ATTRIBUTE_NUM> vertexAttrDesc = VolumeStructure::Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = ATTRIBUTE_NUM;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

    textureSampler3D_create();
    textureSampler_create();
    if (!createTexture(QString::fromUtf8(":/tf/resources/tf/tf.png")))
        qFatal("Failed to create texture");
    if(!createTexture3D())
        qFatal("Failed to create 3D texture");

    //Set up descriptor set and its layout.
    //uniform buffer
    //texture2d
    VkDescriptorPoolSize descPoolSizes[3] =
    { {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t(concurrentFrameCount)},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount)},
       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount)}
    };

    VkDescriptorPoolCreateInfo descPoolInfo{};
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.maxSets = concurrentFrameCount;
    descPoolInfo.poolSizeCount = 3;
    descPoolInfo.pPoolSizes = descPoolSizes;
    err = m_devFuncs->vkCreateDescriptorPool(dev, &descPoolInfo, nullptr, &m_descPool);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor pool: %d", err);

    //vertex shader stage:proj model...
    VkDescriptorSetLayoutBinding layoutBinding[3] = {
        {
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        },
        {
            1, // binding
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1, // descriptorCount
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        },
        {
            2, // binding
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1, // descriptorCount
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        3,
        layoutBinding
    };
    err = m_devFuncs->vkCreateDescriptorSetLayout(dev, &descLayoutInfo, nullptr, &m_descSetLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor set layout: %d", err);

    //descriptorSets_create
    for (int i = 0; i < concurrentFrameCount; ++i) {
        VkDescriptorSetAllocateInfo descSetAllocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            m_descPool,
            1,
            &m_descSetLayout
        };
        err = m_devFuncs->vkAllocateDescriptorSets(dev, &descSetAllocInfo, &m_descSet[i]);
        if (err != VK_SUCCESS)
            qFatal("Failed to allocate descriptor set: %d", err);

        VkWriteDescriptorSet descWrite[3]={};
        descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite[0].dstSet = m_descSet[i];
        descWrite[0].dstBinding = 0;
        descWrite[0].descriptorCount = 1;
        descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrite[0].pBufferInfo = &m_uniformBufInfo[i];

        VkDescriptorImageInfo  descImageInfo = {
            m_sampler,
            m_texView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite[1].dstSet = m_descSet[i];
        descWrite[1].dstBinding = 1;
        descWrite[1].descriptorCount = 1;
        descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite[1].pImageInfo = &descImageInfo;

        VkDescriptorImageInfo descImage3DInfo = {
            m_sampler3d,
            m_tex3dView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        descWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite[2].dstSet = m_descSet[i];
        descWrite[2].dstBinding = 2;
        descWrite[2].descriptorCount = 1;
        descWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite[2].pImageInfo = &descImage3DInfo;

        m_devFuncs->vkUpdateDescriptorSets(dev, 3, descWrite, 0, nullptr);
    }

    // Pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheInfo{};
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = m_devFuncs->vkCreatePipelineCache(dev, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline cache: %d", err);

    //push constant
    VkPhysicalDeviceProperties properties;
    QVulkanFunctions *f = m_window->vulkanInstance()->functions();
    f->vkGetPhysicalDeviceProperties(m_window->physicalDevice(), &properties);
    uint32_t maxPushConstantSize(properties.limits.maxPushConstantsSize);
    if (sizeof(PushConstantObject) > maxPushConstantSize)
    {
        qFatal("Requested push constant size is greater than supported push constant size!");
    }

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;//visible in fragment stage
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantObject);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descSetLayout;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;
    //pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    err = m_devFuncs->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", err);

    // Shaders
    VkShaderModule vertShaderModule = createShader(QStringLiteral(":/vert.spv"));
    VkShaderModule fragShaderModule = createShader(QStringLiteral(":/frag.spv"));

    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShaderModule,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShaderModule,
            "main",
            nullptr
        }
    };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.pInputAssemblyState = &ia;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport/Scissor.
    // This way the pipeline does not need to be touched when resizing the window.
    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT; // we want the back face as well ★change
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rs;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // Enable multisampling.
    ms.sampleShadingEnable = VK_FALSE;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &ms;

    //★need remove?

    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.pDepthStencilState = &ds;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 0xF;// no blend, write out all of rgba
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;

    pipelineInfo.pColorBlendState = &colorBlendState;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn{};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;


    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();

    err = m_devFuncs->vkCreateGraphicsPipelines(dev, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (err != VK_SUCCESS)
        qFatal("Failed to create graphics pipeline: %d", err);

    if (vertShaderModule)
        m_devFuncs->vkDestroyShaderModule(dev, vertShaderModule, nullptr);
    if (fragShaderModule)
        m_devFuncs->vkDestroyShaderModule(dev, fragShaderModule, nullptr);
}

void TriangleRenderer::initSwapChainResources()
{
    qDebug("initSwapChainResources");
    m_proj = m_window->clipCorrectionMatrix(); // adjust for Vulkan-OpenGL clip space differences
    for(uint8_t i = 0; i < 4; i++)
        for(uint8_t j = 0; j < 4; j++)
        {
            ubo.proj[i][j] = *(m_proj.data() + i * 4 + j);
        }
    //ubo.proj = m_proj.data();
    const QSize sz = m_window->swapChainImageSize();
    ubo.proj *= glm::perspective(45.0f, sz.width() / (float) sz.height(), 0.01f, 100.0f);
    ubo.proj = glm::translate(ubo.proj , glm::vec3(0, 0, -2));

    ubo.xAxisRange = glm::vec2(axisRange.x, axisRange.y);
    ubo.yAxisRange = glm::vec2(axisRange.z, axisRange.w);
}

void TriangleRenderer::releaseSwapChainResources()
{
    qDebug("releaseSwapChainResources");
}

void TriangleRenderer::releaseResources()
{
    qDebug("releaseResources");

    VkDevice dev = m_window->device();
    if (m_sampler) {
        m_devFuncs->vkDestroySampler(dev, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    if(m_sampler3d)
    {
        m_devFuncs->vkDestroySampler(dev, m_sampler3d, nullptr);
        m_sampler3d = VK_NULL_HANDLE;
    }

    if (m_texStaging) {
        m_devFuncs->vkDestroyImage(dev, m_texStaging, nullptr);
        m_texStaging = VK_NULL_HANDLE;
    }

    if (m_texStagingMem) {
        m_devFuncs->vkFreeMemory(dev, m_texStagingMem, nullptr);
        m_texStagingMem = VK_NULL_HANDLE;
    }

    if(m_tex3dView)
    {
        m_devFuncs->vkDestroyImageView(dev, m_tex3dView, nullptr);
        m_tex3dView = VK_NULL_HANDLE;
    }

    if (m_texView) {
        m_devFuncs->vkDestroyImageView(dev, m_texView, nullptr);
        m_texView = VK_NULL_HANDLE;
    }

    if(m_tex3dImage)
    {
        m_devFuncs->vkDestroyImage(dev, m_tex3dImage, nullptr);
        m_tex3dImage = VK_NULL_HANDLE;
    }
    if (m_texImage) {
        m_devFuncs->vkDestroyImage(dev, m_texImage, nullptr);
        m_texImage = VK_NULL_HANDLE;
    }

    if(m_tex3dMem)
    {
        m_devFuncs->vkFreeMemory(dev, m_tex3dMem, nullptr);
        m_tex3dMem = VK_NULL_HANDLE;
    }

    if (m_texMem) {
        m_devFuncs->vkFreeMemory(dev, m_texMem, nullptr);
        m_texMem = VK_NULL_HANDLE;
    }

    if (m_pipeline) {
        m_devFuncs->vkDestroyPipeline(dev, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout) {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache) {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_descSetLayout) {
        m_devFuncs->vkDestroyDescriptorSetLayout(dev, m_descSetLayout, nullptr);
        m_descSetLayout = VK_NULL_HANDLE;
    }

    if (m_descPool) {
        m_devFuncs->vkDestroyDescriptorPool(dev, m_descPool, nullptr);
        m_descPool = VK_NULL_HANDLE;
    }

    if (m_buf) {
        m_devFuncs->vkDestroyBuffer(dev, m_buf, nullptr);
        m_buf = VK_NULL_HANDLE;
    }

    if (m_bufMem) {
        m_devFuncs->vkFreeMemory(dev, m_bufMem, nullptr);
        m_bufMem = VK_NULL_HANDLE;
    }
}

void TriangleRenderer::startNextFrame()
{
    VkDevice dev = m_window->device();
    VkCommandBuffer cb = m_window->currentCommandBuffer();
    const QSize sz = m_window->swapChainImageSize();

   // ensureTexture();

    VkClearColorValue clearColor = {{ 0, 0, 0, 1 }};
    VkClearDepthStencilValue clearDS = { 1, 0 };
    VkClearValue clearValues[3]={};
    clearValues[0].color = clearValues[2].color = clearColor;
    clearValues[1].depthStencil = clearDS;

    VkRenderPassBeginInfo rpBeginInfo{};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = m_window->defaultRenderPass();
    rpBeginInfo.framebuffer = m_window->currentFramebuffer();
    rpBeginInfo.renderArea.extent.width = sz.width();
    rpBeginInfo.renderArea.extent.height = sz.height();
    rpBeginInfo.clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
    rpBeginInfo.pClearValues = clearValues;
    m_devFuncs->vkCmdBeginRenderPass(cb, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    quint8 *p;
    VkResult err = m_devFuncs->vkMapMemory(dev, m_bufMem, m_uniformBufInfo[m_window->currentFrame()].offset,
            sizeof(VolumeStructure::UniformBufferObject), 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);

    //ubo.proj = m_proj;
   // m.rotate(m_rotation, 0, 0, 1);
     memcpy(p, &ubo, sizeof(ubo));
   // memcpy(p+32*sizeof(float), &m, sizeof (m));
    m_devFuncs->vkUnmapMemory(dev, m_bufMem);

    // Not exactly a real animation system, just advance on every frame for now.
    //m_rotation += 1.0f;

    m_devFuncs->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    m_devFuncs->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                               &m_descSet[m_window->currentFrame()], 0, nullptr);
    VkDeviceSize vbOffset = 0;
    m_devFuncs->vkCmdBindVertexBuffers(cb, 0, 1, &m_buf, &vbOffset);
    m_devFuncs->vkCmdBindIndexBuffer(cb, m_buf, vertexAllocSize, VK_INDEX_TYPE_UINT16);

    VkViewport viewport;
    viewport.x = viewport.y = 0;
    viewport.width = sz.width();
    viewport.height = sz.height();
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    m_devFuncs->vkCmdSetViewport(cb, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    m_devFuncs->vkCmdSetScissor(cb, 0, 1, &scissor);

    //m_devFuncs->vkCmdPushConstants(cb, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantObject), &pushConstant);
    m_devFuncs->vkCmdDrawIndexed(cb, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    m_devFuncs->vkCmdEndRenderPass(cb);

    m_window->frameReady();
    m_window->requestUpdate(); // render continuously, throttled by the presentation rate
}

void TriangleRenderer::threadSpeed(int start, int end, Vec3i dim, vtkPointData* data, float* scalar, float* vector)
{
    int Dimxy = dim.x * dim.y;
    for (int k = start; k <= end; k++)
    {
        for (int j = 0; j < dim.y; j++)
        {
            for (int i = 0; i < dim.x; i++)
            {
                vtkIdType id = k * (Dimxy) + j * dim.x + i;

                //temporal gradient + gradient
                scalar[id * 2 + 0] = static_cast<float>(data->GetArray(scalarName.c_str())->GetComponent(id, 0));
                scalar[id * 2 + 1] = static_cast<float>(data->GetArray(temporalName.c_str())->GetComponent(id, 0));
           }
        }
    }
}

//input texture 3d volume data
void TriangleRenderer::volumeDataToBuffer()
{
    if(globalData->data == nullptr)
        return;

    VkDevice dev = m_window->device();
    auto dim = globalData->getDim();

    int part = dim.z / THREAD_NUM;
    std::thread thread[THREAD_NUM];

    auto scalarTempMem = sizeof(float)*dim.x* dim.y * dim.z * 2;
   // auto gradientMem = sizeof(float)*dim.x* dim.y * dim.z * 3;

    float * scalarTempTex = (float*)malloc(scalarTempMem);
    float * gradientTex = (float*)malloc(4);

    vtkPointData *pointdata   = STRUCTDATA(globalData->data)->GetPointData();

    thread[0] = std::thread(&TriangleRenderer::threadSpeed, this, 0, part-1, dim, pointdata, scalarTempTex, gradientTex);
    for (size_t index = 1; index < THREAD_NUM - 1; index++)
    {
        thread[index] = std::thread(&TriangleRenderer::threadSpeed, this, part * index, part * (index + 1)-1, dim, pointdata, scalarTempTex, gradientTex);
    }
    thread[THREAD_NUM - 1] = std::thread(&TriangleRenderer::threadSpeed, this, part * (THREAD_NUM-1), dim.z-1, dim, pointdata, scalarTempTex, gradientTex);

    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        thread[i].join();
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(scalarTempMem, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, m_window->hostVisibleMemoryIndex(), stagingBuffer, stagingBufferMemory);

    float *mapped;
    m_devFuncs->vkMapMemory(dev, stagingBufferMemory, 0, scalarTempMem, 0, (void **)(&mapped));
    memcpy(mapped, scalarTempTex, scalarTempMem);
    m_devFuncs->vkUnmapMemory(dev, stagingBufferMemory);

    transitionImageLayout(m_tex3dImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, m_tex3dImage, static_cast<uint32_t>(dim.x), static_cast<uint32_t>(dim.y), static_cast<uint32_t>(dim.z));
    transitionImageLayout(m_tex3dImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

/*
    VkImageCopy copyInfo{};
    copyInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyInfo.srcSubresource.layerCount = 1;
    copyInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyInfo.dstSubresource.layerCount = 1;
    copyInfo.extent.width = static_cast<uint32_t>(dim.x);
    copyInfo.extent.height = static_cast<uint32_t>(dim.y);
    copyInfo.extent.depth = static_cast<uint32_t>(dim.z);
    m_devFuncs->vkCmdCopyImage(cb, m_texStaging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      m_tex3dImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

    transitionImageLayout(m_tex3dImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
*/
    delete[] scalarTempTex;
    delete[] gradientTex;

    m_devFuncs->vkDestroyBuffer(dev, stagingBuffer, nullptr);
    m_devFuncs->vkFreeMemory(dev, stagingBufferMemory, nullptr);
}

void TriangleRenderer::writeImage(QImage img)
{
   writeLinearImage(img, m_texImage, m_texMem);
}

//input texture 2d image
bool TriangleRenderer::writeLinearImage(const QImage &img, VkImage image, VkDeviceMemory memory)
{
    VkDevice dev = m_window->device();

    VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, // mip level
        0
    };
    VkSubresourceLayout layout;
    m_devFuncs->vkGetImageSubresourceLayout(dev, image, &subres, &layout);

    uchar *p;
    VkResult err = m_devFuncs->vkMapMemory(dev, memory, layout.offset, layout.size, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS) {
        qWarning("Failed to map memory for linear image: %d", err);
        return false;
    }

    for (int y = 0; y < img.height(); ++y) {
        const uchar *line = img.constScanLine(y);
        memcpy(p, line, img.width() * 4);
        p += layout.rowPitch;
    }

    m_devFuncs->vkUnmapMemory(dev, memory);
    return true;
}



bool TriangleRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, uint32_t memIndex, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkDevice dev = m_window->device();

    //1.create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult err =  m_devFuncs->vkCreateBuffer(dev, &bufferInfo, nullptr, &buffer);
    if (err != VK_SUCCESS)
    {
        qWarning("failed to create buffer: %d", err);
        return false;
    }

    //2.memory requirement
    VkMemoryRequirements requirements;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, buffer, &requirements);

    //3.memory allocate
    VkMemoryAllocateInfo memAllocateInfo{};
    memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocateInfo.allocationSize = requirements.size;
    memAllocateInfo.memoryTypeIndex = memIndex;

    err = m_devFuncs->vkAllocateMemory(dev, &memAllocateInfo, nullptr, &bufferMemory);
    if (err != VK_SUCCESS)
    {
        qWarning("failed to allocate memory: %d", err);
        return false;
    }

    //4.bind memory with buffer
    m_devFuncs->vkBindBufferMemory(dev, buffer, bufferMemory, 0);

    return  true;
}

void TriangleRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth)
{
    VkCommandBuffer cb = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        depth
    };

    m_devFuncs->vkCmdCopyBufferToImage(
        cb,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(cb);
}

bool TriangleRenderer::createTexture3D(void)
{
    auto dim = globalData->getDim();
    QVulkanFunctions *f = m_window->vulkanInstance()->functions();
    VkDevice dev = m_window->device();


    VkPhysicalDeviceProperties properties;
    f->vkGetPhysicalDeviceProperties(m_window->physicalDevice(), &properties);
    uint32_t maxImageDimension3D(properties.limits.maxImageDimension3D);

    if (dim.x > maxImageDimension3D || dim.y> maxImageDimension3D || dim.z > maxImageDimension3D)
    {
        throw std::runtime_error("Requested texture dimensions is greater than supported 3D texture dimension!");
    }

    /*
    if(!createTextureImage(QSize(dim.x, dim.y) ,dim.z, VK_IMAGE_TYPE_3D, scalarTempGradFormat, m_tex3dImage, m_tex3dMem, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, m_window->deviceLocalMemoryIndex()))
    {
        qWarning("create texture image failed!");
    }

    m_devFuncs->vkDestroyImage(dev, m_texStaging, nullptr);
    m_devFuncs->vkFreeMemory(dev, m_texStagingMem, nullptr);


    if(!createTextureImage(QSize(dim.x, dim.y) ,dim.z, VK_IMAGE_TYPE_3D, scalarTempGradFormat, m_texStaging, m_texStagingMem,
                                    VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                    m_window->hostVisibleMemoryIndex()))
    {
        qWarning("create stage image failed!");
    }
*/
    if(!createTextureImage(QSize(dim.x, dim.y) ,dim.z, VK_IMAGE_TYPE_3D, scalarTempGradFormat, m_tex3dImage, m_tex3dMem, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_window->deviceLocalMemoryIndex()))
    {
        qWarning("create texture image failed!");
    }


    volumeDataToBuffer();

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_tex3dImage;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    createInfo.format = scalarTempGradFormat;

    //表示用途，这里只做颜色使用，也可以做深度使用
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.levelCount = 1;

    VkResult err = m_devFuncs->vkCreateImageView(dev, &createInfo, nullptr, &m_tex3dView);
    if (err != VK_SUCCESS)
    {
        qWarning("failed to create image view: %d", err);
        return false;
    }

    return true;
}

void TriangleRenderer::textureSampler3D_create(void)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;//■
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;//■
    samplerInfo.unnormalizedCoordinates = VK_FALSE;//[0,1) change
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;//■
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult err = m_devFuncs->vkCreateSampler(m_window->device(), &samplerInfo, nullptr, &m_sampler3d);
    if (err != VK_SUCCESS)
        qFatal("Failed to create sampler: %d", err);
}

bool TriangleRenderer::createTexture(QImage img)
{
    // Convert to byte ordered RGBA8. Use premultiplied alpha, see pColorBlendState in the pipeline.
    img = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);

    QVulkanFunctions *f = m_window->vulkanInstance()->functions();
    VkDevice dev = m_window->device();

    const bool srgb = QCoreApplication::arguments().contains(QStringLiteral("--srgb"));
    if (srgb)
        qDebug("sRGB swapchain was requested, making texture sRGB too");

    m_texFormat = srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;//VK_FORMAT_B8G8R8A8_SRGB;//

    // Now we can either map and copy the image data directly, or have to go
    // through a staging buffer to copy and convert into the internal optimal
    // tiling format.
    VkFormatProperties props;
    f->vkGetPhysicalDeviceFormatProperties(m_window->physicalDevice(), m_texFormat, &props);
    const bool canSampleLinear = (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    const bool canSampleOptimal = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (!canSampleLinear && !canSampleOptimal) {
        qWarning("Neither linear nor optimal image sampling is supported for RGBA8");
        return false;
    }

    static bool alwaysStage = qEnvironmentVariableIntValue("QT_VK_FORCE_STAGE_TEX");

    if (canSampleLinear && !alwaysStage) {
        if (!createTextureImage(img.size(), 1, VK_IMAGE_TYPE_2D, m_texFormat, m_texImage, m_texMem,
                                VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT,
                                m_window->hostVisibleMemoryIndex()))
            return false;

        if (!writeLinearImage(img, m_texImage, m_texMem))
            return false;

        m_texLayoutPending = true;
    } else {
        if (!createTextureImage(img.size(),1, VK_IMAGE_TYPE_2D, m_texFormat, m_texStaging, m_texStagingMem,
                                VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                m_window->hostVisibleMemoryIndex()))
            return false;

        if (!createTextureImage(img.size(), 1, VK_IMAGE_TYPE_2D, m_texFormat, m_texImage, m_texMem,
                                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                m_window->deviceLocalMemoryIndex()))
            return false;

        if (!writeLinearImage(img, m_texStaging, m_texStagingMem))
            return false;

        m_texStagingPending = true;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_texImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_texFormat;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = viewInfo.subresourceRange.layerCount = 1;

    VkResult err = m_devFuncs->vkCreateImageView(dev, &viewInfo, nullptr, &m_texView);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create image view for texture: %d", err);
        return false;
    }

    m_texSize = img.size();

    return true;
}

bool TriangleRenderer::createTexture(const QString &name)
{
    QImage img(name);
    if (img.isNull()) {
        qWarning("Failed to load image %s", qPrintable(name));
        return false;
    }

   return createTexture(img);
}


bool TriangleRenderer::createTextureImage(const QSize &size, uint32_t depth, VkImageType imageType,VkFormat format, VkImage &image, VkDeviceMemory &mem, VkImageTiling tiling, VkImageUsageFlags usage, uint32_t memIndex)
{
    VkDevice dev = m_window->device();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;
    imageInfo.format = format;
    imageInfo.extent.width = size.width();
    imageInfo.extent.height = size.height();
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;//VK_IMAGE_LAYOUT_PREINITIALIZED;

    VkResult err = m_devFuncs->vkCreateImage(dev, &imageInfo, nullptr, &image);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create linear image for texture: %d", err);
        return false;
    }

    VkMemoryRequirements memReq;
    m_devFuncs->vkGetImageMemoryRequirements(dev, image, &memReq);

    if (!(memReq.memoryTypeBits & (1 << memIndex))) {
        VkPhysicalDeviceMemoryProperties physDevMemProps;
        m_window->vulkanInstance()->functions()->vkGetPhysicalDeviceMemoryProperties(m_window->physicalDevice(), &physDevMemProps);
        for (uint32_t i = 0; i < physDevMemProps.memoryTypeCount; ++i) {
            if (!(memReq.memoryTypeBits & (1 << i)))
                continue;
            memIndex = i;
        }
    }

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReq.size,
        memIndex
    };
    qDebug("allocating %u bytes for texture image", uint32_t(memReq.size));

    err = m_devFuncs->vkAllocateMemory(dev, &allocInfo, nullptr, &mem);
    if (err != VK_SUCCESS) {
        qWarning("Failed to allocate memory for linear image: %d", err);
        return false;
    }

    err = m_devFuncs->vkBindImageMemory(dev, image, mem, 0);
    if (err != VK_SUCCESS) {
        qWarning("Failed to bind linear image memory: %d", err);
        return false;
    }

    return true;
}

void TriangleRenderer::textureSampler_create(void)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;//[0,1)
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult err = m_devFuncs->vkCreateSampler(m_window->device(), &samplerInfo, nullptr, &m_sampler);
    if (err != VK_SUCCESS)
        qFatal("Failed to create sampler: %d", err);
}

void TriangleRenderer::ensureTexture(void)
{
    if (!m_texLayoutPending && !m_texStagingPending)
        return;

    Q_ASSERT(m_texLayoutPending != m_texStagingPending);
    VkCommandBuffer cb = m_window->currentCommandBuffer();

    VkImageMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1;

    if (m_texLayoutPending) {
        m_texLayoutPending = false;

        barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.image = m_texImage;

        m_devFuncs->vkCmdPipelineBarrier(cb,
                                VK_PIPELINE_STAGE_HOST_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                0, 0, nullptr, 0, nullptr,
                                1, &barrier);
    } else {
        m_texStagingPending = false;

        barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.image = m_texStaging;
        m_devFuncs->vkCmdPipelineBarrier(cb,
                                VK_PIPELINE_STAGE_HOST_BIT,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                0, 0, nullptr, 0, nullptr,
                                1, &barrier);

        barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.image = m_texImage;
        m_devFuncs->vkCmdPipelineBarrier(cb,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                0, 0, nullptr, 0, nullptr,
                                1, &barrier);

        VkImageCopy copyInfo;
        memset(&copyInfo, 0, sizeof(copyInfo));
        copyInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyInfo.srcSubresource.layerCount = 1;
        copyInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyInfo.dstSubresource.layerCount = 1;
        copyInfo.extent.width = m_texSize.width();
        copyInfo.extent.height = m_texSize.height();
        copyInfo.extent.depth = 1;
        m_devFuncs->vkCmdCopyImage(cb, m_texStaging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          m_texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.image = m_texImage;
        m_devFuncs->vkCmdPipelineBarrier(cb,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                0, 0, nullptr, 0, nullptr,
                                1, &barrier);
    }
}

VkCommandBuffer TriangleRenderer::beginSingleTimeCommands(void)
{
    auto dev = m_window->device();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_window->graphicsCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    m_devFuncs->vkAllocateCommandBuffers(dev, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    m_devFuncs->vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void TriangleRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    m_devFuncs->vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    m_devFuncs->vkQueueSubmit(m_window->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    m_devFuncs->vkQueueWaitIdle(m_window->graphicsQueue());

    m_devFuncs->vkFreeCommandBuffers(m_window->device(), m_window->graphicsCommandPool(), 1, &commandBuffer);
}


void TriangleRenderer::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer cb = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1;

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    else if(oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    else if(oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    m_devFuncs->vkCmdPipelineBarrier(cb,
                            sourceStage,
                            destinationStage,
                            0, 0, nullptr, 0, nullptr,
                            1, &barrier);

    endSingleTimeCommands(cb);
}
