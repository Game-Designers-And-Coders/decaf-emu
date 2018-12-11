#include "mainwindow.h"
#include "vulkanwindow.h"

#include <QApplication>
#include <QVulkanInstance>

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QVulkanInstance inst;

   inst.setApiVersion({ 1, 0, 0 });
   inst.setLayers({
      "VK_LAYER_LUNARG_standard_validation"
   });
   inst.setExtensions({
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
      VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
   });

   if (!inst.create()) {
      qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
   }

   MainWindow mainWindow(&inst);
   mainWindow.resize(1024, 768);
   mainWindow.show();
   return app.exec();
}
