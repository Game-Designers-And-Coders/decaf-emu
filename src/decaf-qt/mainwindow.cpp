#include "mainwindow.h"

#include "inputsettings.h"
#include "vulkanwindow.h"


MainWindow::MainWindow(QVulkanInstance *vulkanInstance, QWidget* parent) :
   QMainWindow(parent)
{
   mVulkanWindow = new VulkanWindow(vulkanInstance);

   QWidget* wrapper = QWidget::createWindowContainer((QWindow*)mVulkanWindow);
   wrapper->setFocusPolicy(Qt::StrongFocus);
   wrapper->setFocus();
   setCentralWidget(QWidget::createWindowContainer(mVulkanWindow));

   mUi.setupUi(this);
}

void
MainWindow::openInputSettings()
{
   InputSettings dialog(this);
   dialog.exec();
}
