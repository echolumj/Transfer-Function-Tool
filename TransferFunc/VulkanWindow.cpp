#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanWindow.h"
#include "shared/trianglerenderer.h"

QVulkanWindowRenderer *VulkanWindow::createRenderer()
{
    m_renderer = new TriangleRenderer(this);
    return m_renderer;
}

void VulkanWindow::volumeData_renew()
{
    m_renderer->volumeDataToBuffer();
}

void VulkanWindow::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_W:
        m_renderer->ubo.model *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.02));
        break;
    case Qt::Key_S:
        m_renderer->ubo.model *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.02));
        break;
    case Qt::Key_Up:
        m_renderer->ubo.model *= glm::rotate(glm::mat4(1.0f), glm::radians(0.4f), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case Qt::Key_Down:
        m_renderer->ubo.model *= glm::rotate(glm::mat4(1.0f), glm::radians(-0.4f), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case Qt::Key_Left:
        m_renderer->ubo.model *= glm::rotate(glm::mat4(1.0f), glm::radians(0.4f), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case Qt::Key_Right:
        m_renderer->ubo.model *= glm::rotate(glm::mat4(1.0f), glm::radians(-0.4f), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    default:
        break;
    }
    m_renderer->ubo.invModelView = m_renderer->ubo.model;
}

void VulkanWindow::imageData_renew(QImage img)
{
    m_renderer->writeImage(img);
}
