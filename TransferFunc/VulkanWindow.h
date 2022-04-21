#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H
#include <QVulkanWindow>
#include <QKeyEvent>
#include <Vec.h>

class TriangleRenderer;

class VulkanWindow :public QVulkanWindow
{
     Q_OBJECT
public:
    QVulkanWindowRenderer *createRenderer() override;

    TriangleRenderer* m_renderer = nullptr;

protected:
    void keyPressEvent(QKeyEvent *ev) override;


public slots:
    void volumeData_renew(void);
    void imageData_renew(QImage);

};

#endif // VULKANWINDOW_H
