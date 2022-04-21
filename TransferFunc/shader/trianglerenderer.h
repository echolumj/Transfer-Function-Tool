/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#define GLM_FORCE_RADIANS
#include <QVulkanWindow>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <datainfo.h>
#include <datainfo.h>

//vert shader input attribute
#define ATTRIBUTE_NUM 2
//#define THREAD_NUM 8

extern glm::vec4 axisRange;

namespace VolumeStructure{

struct UniformBufferObject {
    glm::mat4x4 model = glm::mat4(1.0f);
    glm::mat4x4 view;
    glm::mat4x4 proj;
    glm::mat4x4 invModelView;
    glm::vec3 posCamera = glm::vec3(0.0, 0.0, 2.0);
    alignas(16)
    glm::vec2 xAxisRange;
    glm::vec2 yAxisRange;
    //alignas(4)
    //float aspect;
    //float cotHalfFov;
};

struct PhongModel {
    glm::vec3 lightPos;
    float ka;
    glm::vec3 lightColor;
    float kd;
    float ks;
    float shininess;
};

struct PushConstantObject {
    glm::vec2 xAxisRange;
    glm::vec2 yAxisRange;
};


struct Vertex {
        glm::vec2 position;
        glm::vec2 texCoord;
        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription  vertexBinding{};
            vertexBinding.binding = 0;
            vertexBinding.stride = sizeof(Vertex);
            vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return vertexBinding;
        }

        static std::array<VkVertexInputAttributeDescription, ATTRIBUTE_NUM> getAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, ATTRIBUTE_NUM> vertexAttribute = {};

            //position
            vertexAttribute[0].binding = 0;
            vertexAttribute[0].location = 0;
            vertexAttribute[0].format = VK_FORMAT_R32G32_SFLOAT; //vec2
            vertexAttribute[0].offset = offsetof(Vertex, position);

            //texCoord
            vertexAttribute[1].binding = 0;
            vertexAttribute[1].location = 1;
            vertexAttribute[1].format = VK_FORMAT_R32G32_SFLOAT;
            vertexAttribute[1].offset = offsetof(Vertex, texCoord);

            return vertexAttribute;
        }
    };
}

class TriangleRenderer : public QObject, public QVulkanWindowRenderer
{
     Q_OBJECT
public:
    TriangleRenderer(QVulkanWindow *w);

    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;
    void writeImage(QImage img);
    void volumeDataToBuffer(void);

public:
    VolumeStructure::UniformBufferObject ubo{};
    VolumeStructure::PushConstantObject pushConstant{};

private:
    bool createTexture(QImage img);
    bool createTexture(const QString &name);
    bool createTextureImage(const QSize &size, uint32_t depth, VkImageType imageType,VkFormat format,
                            VkImage &image, VkDeviceMemory &mem, VkImageTiling tiling, VkImageUsageFlags usage, uint32_t memIndex);
    bool writeLinearImage(const QImage &img, VkImage image, VkDeviceMemory memory);
    void ensureTexture();
    void textureSampler_create(void);

    //texture 3D

    void threadSpeed(int start, int end, Vec3i dim, vtkPointData* data, float* scalar, float* vector);
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, uint32_t memIndex, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    VkCommandBuffer beginSingleTimeCommands(void);

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth);

    bool createTexture3D(void);
    void textureSampler3D_create(void);

protected:
    VkShaderModule createShader(const QString &name);

    QVulkanWindow *m_window;
    QVulkanDeviceFunctions *m_devFuncs;

    VkDeviceMemory m_bufMem = VK_NULL_HANDLE;
    VkBuffer m_buf = VK_NULL_HANDLE;
    VkDescriptorBufferInfo m_uniformBufInfo[QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT];

    VkDescriptorPool m_descPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet[QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT];

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    VkSampler m_sampler = VK_NULL_HANDLE;
    VkImage m_texImage = VK_NULL_HANDLE;
    VkDeviceMemory m_texMem = VK_NULL_HANDLE;
    bool m_texLayoutPending = false;
    VkImageView m_texView = VK_NULL_HANDLE;
    VkImage m_texStaging = VK_NULL_HANDLE;
    VkDeviceMemory m_texStagingMem = VK_NULL_HANDLE;
    bool m_texStagingPending = false;
    QSize m_texSize;
    VkFormat m_texFormat;

    VkSampler m_sampler3d = VK_NULL_HANDLE;
    VkImage m_tex3dImage = VK_NULL_HANDLE;
    VkDeviceMemory m_tex3dMem = VK_NULL_HANDLE;
    VkImageView m_tex3dView = VK_NULL_HANDLE;
    QSize m_tex3dSize;
    VkFormat m_tex3dFormat;

    QMatrix4x4 m_proj;
    float m_rotation = 0.0f;
};
