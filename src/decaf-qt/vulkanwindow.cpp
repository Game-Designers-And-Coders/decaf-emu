#include "vulkanwindow.h"

#include <common-sdl/decafsdl_vulkan_shaders.h>
#include <libgpu/gpu_graphicsdriver.h>
#include <libgpu/gpu_vulkandriver.h>
#include <libdecaf/decaf.h>
#include <libdecaf/decaf_debugger.h>

#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QPlatformSurfaceEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTouchEvent>

VulkanWindow::VulkanWindow(QVulkanInstance *instance, DecafInterface *decafInterface) :
   mDecafInterface(decafInterface)
{
   setVulkanInstance(instance);

   VkPhysicalDeviceFeatures devFeatures = { };
   devFeatures.depthClamp = true;
   devFeatures.geometryShader = true;
   devFeatures.textureCompressionBC = true;
   devFeatures.independentBlend = true;
   devFeatures.fillModeNonSolid = true;
   devFeatures.samplerAnisotropy = true;
   devFeatures.wideLines = true;
   devFeatures.logicOp = true;
   setPhysicalDeviceFeatures(devFeatures);

   VkPhysicalDeviceTransformFeedbackFeaturesEXT devXfbFeatures = { };
   devXfbFeatures.transformFeedback = true;
   devXfbFeatures.geometryStreams = true;
   setPhysicalDeviceTransformFeedbackFeatures(devXfbFeatures);

   setPreferredColorFormats({
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_FORMAT_B8G8R8A8_SRGB
   });

   setDeviceExtensions({
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME,
      VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME,
      VK_KHR_MAINTENANCE1_EXTENSION_NAME,
      VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,
      VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
      VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
   });
}
/*

// Stuff for the debugger

using ClipboardTextGetCallback = const char *(*)(void *);
using ClipboardTextSetCallback = void (*)(void *, const char*);

void
setClipboardTextCallbacks(ClipboardTextGetCallback getter,
                          ClipboardTextSetCallback setter);
*/

static decaf::input::KeyboardKey
translateKeyCode(QKeyEvent *ev)
{
   auto key = ev->key();
   switch (key) {
   case Qt::Key_Tab:
      return decaf::input::KeyboardKey::Tab;
   case Qt::Key_Left:
      return decaf::input::KeyboardKey::LeftArrow;
   case Qt::Key_Right:
      return decaf::input::KeyboardKey::RightArrow;
   case Qt::Key_Up:
      return decaf::input::KeyboardKey::UpArrow;
   case Qt::Key_Down:
      return decaf::input::KeyboardKey::DownArrow;
   case Qt::Key_PageUp:
      return decaf::input::KeyboardKey::PageUp;
   case Qt::Key_PageDown:
      return decaf::input::KeyboardKey::PageDown;
   case Qt::Key_Home:
      return decaf::input::KeyboardKey::Home;
   case Qt::Key_End:
      return decaf::input::KeyboardKey::End;
   case Qt::Key_Delete:
      return decaf::input::KeyboardKey::Delete;
   case Qt::Key_Backspace:
      return decaf::input::KeyboardKey::Backspace;
   case Qt::Key_Return:
      return decaf::input::KeyboardKey::Enter;
   case Qt::Key_Escape:
      return decaf::input::KeyboardKey::Escape;
   case Qt::Key_Control:
      return decaf::input::KeyboardKey::LeftControl;
   case Qt::Key_Shift:
      return decaf::input::KeyboardKey::LeftShift;
   case Qt::Key_Alt:
      return decaf::input::KeyboardKey::LeftAlt;
   case Qt::Key_AltGr:
      return decaf::input::KeyboardKey::RightAlt;
   case Qt::Key_Super_L:
      return decaf::input::KeyboardKey::LeftSuper;
   case Qt::Key_Super_R:
      return decaf::input::KeyboardKey::RightSuper;
   default:
      if (key >= Qt::Key_A && key <= Qt::Key_Z) {
         auto id = (key - Qt::Key_A) + static_cast<int>(decaf::input::KeyboardKey::A);
         return static_cast<decaf::input::KeyboardKey>(id);
      } else if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
         auto id = (key - Qt::Key_F1) + static_cast<int>(decaf::input::KeyboardKey::F1);
         return static_cast<decaf::input::KeyboardKey>(id);
      }

      return decaf::input::KeyboardKey::Unknown;
   }
}

void VulkanWindow::keyPressEvent(QKeyEvent *ev)
{
   decaf::injectKeyInput(translateKeyCode(ev), decaf::input::KeyboardAction::Press);

   if (auto text = ev->text(); !text.isEmpty()) {
      decaf::injectTextInput(text.toUtf8().data());
   }
}

void VulkanWindow::keyReleaseEvent(QKeyEvent *ev)
{
   decaf::injectKeyInput(translateKeyCode(ev), decaf::input::KeyboardAction::Release);
}

void VulkanWindow::mousePressEvent(QMouseEvent *ev)
{
   auto button = decaf::input::MouseButton::Left;

   if (ev->button() == Qt::MouseButton::RightButton) {
      button = decaf::input::MouseButton::Right;
   } else if (ev->button() == Qt::MouseButton::MiddleButton) {
      button = decaf::input::MouseButton::Middle;
   }

   decaf::injectMouseButtonInput(button, decaf::input::MouseAction::Press);
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent *ev)
{
   auto button = decaf::input::MouseButton::Left;

   if (ev->button() == Qt::MouseButton::RightButton) {
      button = decaf::input::MouseButton::Right;
   } else if (ev->button() == Qt::MouseButton::MiddleButton) {
      button = decaf::input::MouseButton::Middle;
   }

   decaf::injectMouseButtonInput(button, decaf::input::MouseAction::Release);
}

void VulkanWindow::mouseMoveEvent(QMouseEvent *ev)
{
   auto pos = ev->pos();
   decaf::injectMousePos(static_cast<float>(pos.x()),
                         static_cast<float>(pos.y()));
}

void VulkanWindow::wheelEvent(QWheelEvent *ev)
{
   auto delta = ev->angleDelta();
   decaf::injectScrollInput(delta.x() / 120.0f, delta.y() / 120.0f);
}

void VulkanWindow::touchEvent(QTouchEvent *ev)
{
   // TODO: Translate touch events?
}

QVulkanWindowRenderer *VulkanWindow::createRenderer()
{
   return new VulkanRenderer(this, mDecafInterface);
}

VulkanRenderer::VulkanRenderer(QVulkanWindow2 *w, DecafInterface *decafInterface) :
   m_window(w),
   mDecafInterface(decafInterface)
{
   QObject::connect(mDecafInterface, &DecafInterface::titleLoaded,
                    this, &VulkanRenderer::titleLoaded);
   QObject::connect(this, &VulkanRenderer::graphicsReady,
                    mDecafInterface, &DecafInterface::graphicsReady);
}

void VulkanRenderer::initResources()
{
   qDebug("initResources");

   mDevice = m_window->device();
   m_devFuncs = m_window->vulkanInstance()->deviceFunctions(m_window->device());

   // Initialise trivial sampler
   vk::SamplerCreateInfo baseSamplerDesc;
   baseSamplerDesc.magFilter = vk::Filter::eLinear;
   baseSamplerDesc.minFilter = vk::Filter::eLinear;
   baseSamplerDesc.mipmapMode = vk::SamplerMipmapMode::eNearest;
   baseSamplerDesc.addressModeU = vk::SamplerAddressMode::eRepeat;
   baseSamplerDesc.addressModeV = vk::SamplerAddressMode::eRepeat;
   baseSamplerDesc.addressModeW = vk::SamplerAddressMode::eRepeat;
   baseSamplerDesc.mipLodBias = 0.0f;
   baseSamplerDesc.anisotropyEnable = false;
   baseSamplerDesc.maxAnisotropy = 0.0f;
   baseSamplerDesc.compareEnable = false;
   baseSamplerDesc.compareOp = vk::CompareOp::eAlways;
   baseSamplerDesc.minLod = 0.0f;
   baseSamplerDesc.maxLod = 0.0f;
   baseSamplerDesc.borderColor = vk::BorderColor::eFloatTransparentBlack;
   baseSamplerDesc.unnormalizedCoordinates = false;
   mTrivialSampler = mDevice.createSampler(baseSamplerDesc);

   // Initialise descriptor set layout
   std::array<vk::Sampler, 1> immutableSamplers = { mTrivialSampler };
   std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
      vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, immutableSamplers.data()),
      vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eSampledImage, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
   };
   vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutDesc;
   descriptorSetLayoutDesc.bindingCount = static_cast<uint32_t>(bindings.size());
   descriptorSetLayoutDesc.pBindings = bindings.data();
   mDescriptorSetLayout = mDevice.createDescriptorSetLayout(descriptorSetLayoutDesc);

   // Initialise pipeline layout
   std::array<vk::DescriptorSetLayout, 1> layoutBindings = { mDescriptorSetLayout };
   vk::PipelineLayoutCreateInfo pipelineLayoutDesc;
   pipelineLayoutDesc.setLayoutCount = static_cast<uint32_t>(layoutBindings.size());
   pipelineLayoutDesc.pSetLayouts = layoutBindings.data();
   pipelineLayoutDesc.pushConstantRangeCount = 0;
   pipelineLayoutDesc.pPushConstantRanges = nullptr;
   mPipelineLayout = mDevice.createPipelineLayout(pipelineLayoutDesc);

   // Initialise descriptor pool
   std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
      vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 100),
      vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 100),
      vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
   };

   vk::DescriptorPoolCreateInfo descriptorPoolInfo;
   descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
   descriptorPoolInfo.pPoolSizes = descriptorPoolSizes.data();
   descriptorPoolInfo.maxSets = static_cast<uint32_t>(descriptorPoolSizes.size() * 100);
   mDescriptorPool = mDevice.createDescriptorPool(descriptorPoolInfo);

   // Initialise descriptor sets
   for (auto &frameResources : mFrameResources) {
      vk::DescriptorSetAllocateInfo descriptorSetAllocDesc;
      descriptorSetAllocDesc.descriptorPool = mDescriptorPool;
      descriptorSetAllocDesc.descriptorSetCount = 1;
      descriptorSetAllocDesc.pSetLayouts = &mDescriptorSetLayout;
      frameResources.tvDesc = mDevice.allocateDescriptorSets(descriptorSetAllocDesc)[0];
      frameResources.drcDesc = mDevice.allocateDescriptorSets(descriptorSetAllocDesc)[0];
   }

   // Initialise buffers
   auto findMemoryType = [](vk::PhysicalDevice physDevice, uint32_t typeFilter, vk::MemoryPropertyFlags props)
   {
      auto memProps = physDevice.getMemoryProperties();

      for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
         if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
         }
      }

      throw std::runtime_error("failed to find suitable memory type!");
   };

   static const std::array<float, 24> vertices = {
      -1.0f,  1.0f,  0.0f,  1.0f,
       1.0f,  1.0f,  1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,  0.0f,

       1.0f, -1.0f,  1.0f,  0.0f,
      -1.0f, -1.0f,  0.0f,  0.0f,
      -1.0f,  1.0f,  0.0f,  1.0f,
   };

   uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(m_window->graphicsQueueFamilyIdx()) };

   vk::BufferCreateInfo bufferDesc;
   bufferDesc.size = static_cast<uint32_t>(sizeof(float) * vertices.size());
   bufferDesc.usage = vk::BufferUsageFlagBits::eVertexBuffer;
   bufferDesc.sharingMode = vk::SharingMode::eExclusive;
   bufferDesc.queueFamilyIndexCount = 1;
   bufferDesc.pQueueFamilyIndices = queueFamilyIndices;
   mVertexBuffer = mDevice.createBuffer(bufferDesc);

   auto bufferMemReqs = mDevice.getBufferMemoryRequirements(mVertexBuffer);

   vk::MemoryAllocateInfo allocDesc;
   allocDesc.allocationSize = bufferMemReqs.size;
   allocDesc.memoryTypeIndex = findMemoryType(m_window->physicalDevice(), bufferMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
   auto bufferMem = mDevice.allocateMemory(allocDesc);

   mDevice.bindBufferMemory(mVertexBuffer, bufferMem, 0);
   auto hostMem = mDevice.mapMemory(bufferMem, 0, VK_WHOLE_SIZE);
   memcpy(hostMem, vertices.data(), bufferMemReqs.size);
   mDevice.flushMappedMemoryRanges({ vk::MappedMemoryRange(bufferMem, 0, VK_WHOLE_SIZE) });
   mDevice.unmapMemory(bufferMem);

   // Create libgpu driver
   mDecafDriver = reinterpret_cast<gpu::VulkanDriver*>(gpu::createVulkanDriver());
   mDecafDriver->initialise(m_window->physicalDevice(), mDevice, m_window->graphicsDriverQueue(), m_window->graphicsQueueFamilyIdx());

   // Create debug ui renderer
   mDebugUiRenderer = reinterpret_cast<decaf::VulkanUiRenderer *>(decaf::createDebugVulkanRenderer());
   decaf::VulkanUiRendererInitInfo uiInitInfo;
   uiInitInfo.physDevice = m_window->physicalDevice();
   uiInitInfo.device = m_window->device();
   uiInitInfo.queue = m_window->graphicsQueue();
   uiInitInfo.descriptorPool = mDescriptorPool;
   uiInitInfo.renderPass = m_window->defaultRenderPass();
   uiInitInfo.commandPool = m_window->graphicsCommandPool();
   mDebugUiRenderer->initialise(&uiInitInfo);

   mGraphicsThread = std::thread { [this]() { mDecafDriver->run(); } };

   emit graphicsReady(mDecafDriver, mDebugUiRenderer);
}

void VulkanRenderer::initSwapChainResources()
{
   qDebug("initSwapChainResources");
   createRenderPipeline();
}

bool VulkanRenderer::createRenderPipeline()
{
   auto swapChainImageSize = m_window->swapChainImageSize();

   auto scanbufferVertBytesSize = sizeof(scanbufferVertBytes) / sizeof(scanbufferVertBytes[0]);
   auto scanbufferVertModule = mDevice.createShaderModuleUnique(vk::ShaderModuleCreateInfo({}, scanbufferVertBytesSize, reinterpret_cast<const uint32_t*>(scanbufferVertBytes)));

   auto scanbufferFragBytesSize = sizeof(scanbufferFragBytes) / sizeof(scanbufferFragBytes[0]);
   auto scanbufferFragModule = mDevice.createShaderModuleUnique(vk::ShaderModuleCreateInfo({}, scanbufferFragBytesSize, reinterpret_cast<const uint32_t*>(scanbufferFragBytes)));

   std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
      vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, scanbufferVertModule.get(), "main", nullptr),
      vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, scanbufferFragModule.get(), "main", nullptr)
   };

   std::array<vk::VertexInputBindingDescription, 1> vtxBindings = {
      vk::VertexInputBindingDescription(0, 16, vk::VertexInputRate::eVertex)
   };

   std::array<vk::VertexInputAttributeDescription, 2> vtxAttribs = {
      vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, 0),
      vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 8),
   };

   // Vertex input stage, we store all our vertices in the actual shaders
   vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
   vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vtxBindings.size());
   vertexInputInfo.pVertexBindingDescriptions = vtxBindings.data();
   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vtxAttribs.size());
   vertexInputInfo.pVertexAttributeDescriptions = vtxAttribs.data();

   vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
   inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
   inputAssembly.primitiveRestartEnable = false;

   vk::Viewport viewport(0.0f, 0.0f,
                         static_cast<float>(swapChainImageSize.width()),
                         static_cast<float>(swapChainImageSize.height()),
                         0.0f, 0.0f);
   vk::Rect2D scissor({ 0,0 }, { static_cast<uint32_t>(swapChainImageSize.width()), static_cast<uint32_t>(swapChainImageSize.height()) });
   vk::PipelineViewportStateCreateInfo viewportState;
   viewportState.viewportCount = 1;
   viewportState.pViewports = &viewport;
   viewportState.scissorCount = 1;
   viewportState.pScissors = &scissor;

   vk::PipelineRasterizationStateCreateInfo rasterizer;
   rasterizer.depthClampEnable = false;
   rasterizer.rasterizerDiscardEnable = false;
   rasterizer.polygonMode = vk::PolygonMode::eFill;
   rasterizer.cullMode = vk::CullModeFlagBits::eNone;
   rasterizer.frontFace = vk::FrontFace::eClockwise;
   rasterizer.depthBiasEnable = false;
   rasterizer.depthBiasConstantFactor = 0.0f;
   rasterizer.depthBiasClamp = 0.0f;
   rasterizer.depthBiasSlopeFactor = 0.0f;
   rasterizer.lineWidth = 1.0f;

   vk::PipelineMultisampleStateCreateInfo multisampling;
   multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
   multisampling.sampleShadingEnable = false;
   multisampling.minSampleShading = 1.0f;
   multisampling.pSampleMask = nullptr;
   multisampling.alphaToCoverageEnable = false;
   multisampling.alphaToOneEnable = false;

   vk::PipelineColorBlendAttachmentState colorBlendAttachement0;
   colorBlendAttachement0.blendEnable = false;
   colorBlendAttachement0.srcColorBlendFactor = vk::BlendFactor::eOne;
   colorBlendAttachement0.dstColorBlendFactor = vk::BlendFactor::eZero;
   colorBlendAttachement0.colorBlendOp = vk::BlendOp::eAdd;
   colorBlendAttachement0.srcAlphaBlendFactor = vk::BlendFactor::eOne;
   colorBlendAttachement0.dstAlphaBlendFactor = vk::BlendFactor::eZero;
   colorBlendAttachement0.alphaBlendOp = vk::BlendOp::eAdd;
   colorBlendAttachement0.colorWriteMask =
      vk::ColorComponentFlagBits::eR |
      vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB |
      vk::ColorComponentFlagBits::eA;

   std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments = {
      colorBlendAttachement0
   };

   vk::PipelineColorBlendStateCreateInfo colorBlendState;
   colorBlendState.logicOpEnable = false;
   colorBlendState.logicOp = vk::LogicOp::eCopy;
   colorBlendState.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
   colorBlendState.pAttachments = colorBlendAttachments.data();
   colorBlendState.blendConstants[0] = 0.0f;
   colorBlendState.blendConstants[1] = 0.0f;
   colorBlendState.blendConstants[2] = 0.0f;
   colorBlendState.blendConstants[3] = 0.0f;

   std::vector<vk::DynamicState> dynamicStates = {
      vk::DynamicState::eViewport
   };

   vk::PipelineDynamicStateCreateInfo dynamicState;
   dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
   dynamicState.pDynamicStates = dynamicStates.data();

   vk::PipelineDepthStencilStateCreateInfo dsState;

   vk::GraphicsPipelineCreateInfo pipelineInfo;
   pipelineInfo.pStages = shaderStages.data();
   pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pTessellationState = nullptr;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = &dsState;
   pipelineInfo.pColorBlendState = &colorBlendState;
   pipelineInfo.pDynamicState = &dynamicState;
   pipelineInfo.layout = mPipelineLayout;
   pipelineInfo.renderPass = m_window->defaultRenderPass();
   pipelineInfo.subpass = 0;
   pipelineInfo.basePipelineHandle = vk::Pipeline();
   pipelineInfo.basePipelineIndex = -1;
   mGraphicsPipeline = mDevice.createGraphicsPipeline(vk::PipelineCache(), pipelineInfo);
   return true;
}


void VulkanRenderer::releaseSwapChainResources()
{
   qDebug("releaseSwapChainResources");

   if (mGraphicsPipeline) {
      mDevice.destroyPipeline(mGraphicsPipeline);
      mGraphicsPipeline = nullptr;
   }
}

void VulkanRenderer::releaseResources()
{
   qDebug("releaseResources");

   if (mVertexBuffer) {
      mDevice.destroyBuffer(mVertexBuffer);
      mVertexBuffer = nullptr;
   }

#if 0 // Apparently we do not need to free descriptor sets?
   for (auto &frameResources : mFrameResources) {
      if (frameResources.tvDesc) {
         mDevice.freeDescriptorSets(mDescriptorPool, { frameResources.tvDesc });
         frameResources.tvDesc = nullptr;
      }

      if (frameResources.drcDesc) {
         mDevice.freeDescriptorSets(mDescriptorPool, { frameResources.drcDesc });
         frameResources.drcDesc = nullptr;
      }
   }
#endif

   if (mDescriptorPool) {
      mDevice.destroyDescriptorPool(mDescriptorPool);
      mDescriptorPool = nullptr;
   }

   if (mPipelineLayout) {
      mDevice.destroyPipelineLayout(mPipelineLayout);
      mPipelineLayout = nullptr;
   }

   if (mDescriptorSetLayout) {
      mDevice.destroyDescriptorSetLayout(mDescriptorSetLayout);
      mDescriptorSetLayout = nullptr;
   }

   if (mTrivialSampler) {
      mDevice.destroySampler(mTrivialSampler);
      mTrivialSampler = nullptr;
   }

   // Shut down the debugger ui driver
   if (mDebugUiRenderer) {
      mDebugUiRenderer->shutdown();
   }

   // Stop the GPU
   mDecafDriver->stop();
   mGraphicsThread.join();

   // Shut down the gpu driver
   mDecafDriver->shutdown();
}

void
VulkanRenderer::calculateScreenViewports(Viewport &tv, Viewport &drc)
{
   int TvWidth = 1280;
   int TvHeight = 720;

   int DrcWidth = 854;
   int DrcHeight = 480;

   int OuterBorder = 0;
   int ScreenSeparation = 5;

   auto swapChainImageSize = m_window->swapChainImageSize();
   int windowWidth = swapChainImageSize.width();
   int windowHeight = swapChainImageSize.height();
   int nativeHeight, nativeWidth;
   int tvLeft, tvBottom, tvTop, tvRight;
   int drcLeft, drcBottom, drcTop, drcRight;

   auto tvVisible = true;
   auto drcVisible = true;


   if (false /*config::display::layout == config::display::Toggle*/) {
      // For toggle mode only one screen is visible at a time, so we calculate the
      // screen position as if only the TV exists here
      nativeHeight = TvHeight;
      nativeWidth = TvWidth;
      DrcWidth = 0;
      DrcHeight = 0;
      ScreenSeparation = 0;
   } else {
      nativeHeight = DrcHeight + TvHeight + ScreenSeparation + 2 * OuterBorder;
      nativeWidth = std::max(DrcWidth, TvWidth) + 2 * OuterBorder;
   }

   if (windowWidth * nativeHeight >= windowHeight * nativeWidth) {
      // Align to height
      int drcBorder = (windowWidth * nativeHeight - windowHeight * DrcWidth + nativeHeight) / nativeHeight / 2;
      int tvBorder = false /*config::display::stretch*/ ? 0 : (windowWidth * nativeHeight - windowHeight * TvWidth + nativeHeight) / nativeHeight / 2;

      drcBottom = OuterBorder;
      drcTop = OuterBorder + (DrcHeight * windowHeight + nativeHeight / 2) / nativeHeight;
      drcLeft = drcBorder;
      drcRight = windowWidth - drcBorder;

      tvBottom = windowHeight - OuterBorder - (TvHeight * windowHeight + nativeHeight / 2) / nativeHeight;
      tvTop = windowHeight - OuterBorder;
      tvLeft = tvBorder;
      tvRight = windowWidth - tvBorder;
   } else {
      // Align to width
      int heightBorder = (windowHeight * nativeWidth - windowWidth * (DrcHeight + TvHeight + ScreenSeparation) + nativeWidth) / nativeWidth / 2;
      int drcBorder = (windowWidth - DrcWidth * windowWidth / nativeWidth + 1) / 2;
      int tvBorder = (windowWidth - TvWidth * windowWidth / nativeWidth + 1) / 2;

      drcBottom = heightBorder;
      drcTop = heightBorder + (DrcHeight * windowWidth + nativeWidth / 2) / nativeWidth;
      drcLeft = drcBorder;
      drcRight = windowWidth - drcBorder;

      tvTop = windowHeight - heightBorder;
      tvBottom = windowHeight - heightBorder - (TvHeight * windowWidth + nativeWidth / 2) / nativeWidth;
      tvLeft = tvBorder;
      tvRight = windowWidth - tvBorder;
   }

   if (false /*config::display::layout == config::display::Toggle*/) {
      // In toggle mode, DRC and TV size are the same
      drcLeft = tvLeft;
      drcRight = tvRight;
      drcTop = tvTop;
      drcBottom = tvBottom;

      if (false /*mToggleDRC*/) {
         drcVisible = true;
         tvVisible = false;
      } else {
         drcVisible = false;
         tvVisible = true;
      }
   }

   if (drcVisible) {
      drc.x = static_cast<float>(drcLeft);
      drc.y = static_cast<float>(drcBottom);
      drc.width = static_cast<float>(drcRight - drcLeft);
      drc.height = static_cast<float>(drcTop - drcBottom);
   } else {
      drc.x = 0.0f;
      drc.y = 0.0f;
      drc.width = 0.0f;
      drc.height = 0.0f;
   }

   if (tvVisible) {
      tv.x = static_cast<float>(tvLeft);
      tv.y = static_cast<float>(tvBottom);
      tv.width = static_cast<float>(tvRight - tvLeft);
      tv.height = static_cast<float>(tvTop - tvBottom);
   } else {
      tv.x = 0.0f;
      tv.y = 0.0f;
      tv.width = 0.0f;
      tv.height = 0.0f;
   }

   decaf_check(drc.x >= 0);
   decaf_check(drc.y >= 0);
   decaf_check(drc.x + drc.width <= windowWidth);
   decaf_check(drc.y + drc.height <= windowHeight);
   decaf_check(tv.x >= 0);
   decaf_check(tv.y >= 0);
   decaf_check(tv.x + tv.width <= windowWidth);
   decaf_check(tv.y + tv.height <= windowHeight);
}


void
VulkanRenderer::acquireScanBuffer(vk::CommandBuffer cmdBuffer, vk::DescriptorSet descriptorSet, vk::Image image, vk::ImageView imageView)
{
   vk::ImageMemoryBarrier imageBarrier;
   imageBarrier.srcAccessMask = vk::AccessFlags();
   imageBarrier.dstAccessMask = vk::AccessFlags();
   imageBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
   imageBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
   imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   imageBarrier.image = image;
   imageBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
   imageBarrier.subresourceRange.baseMipLevel = 0;
   imageBarrier.subresourceRange.levelCount = 1;
   imageBarrier.subresourceRange.baseArrayLayer = 0;
   imageBarrier.subresourceRange.layerCount = 1;

   cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllGraphics,
                             vk::PipelineStageFlagBits::eAllGraphics,
                             vk::DependencyFlagBits::eByRegion,
                             {},
                             {},
                             { imageBarrier });

   vk::DescriptorImageInfo imageInfo;
   imageInfo.imageView = imageView;
   imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

   vk::WriteDescriptorSet descWriteDesc;
   descWriteDesc.dstSet = descriptorSet;
   descWriteDesc.dstBinding = 1;
   descWriteDesc.dstArrayElement = 0;
   descWriteDesc.descriptorCount = 1;
   descWriteDesc.descriptorType = vk::DescriptorType::eSampledImage;
   descWriteDesc.pImageInfo = &imageInfo;
   mDevice.updateDescriptorSets({ descWriteDesc }, {});
}

void
VulkanRenderer::renderScanBuffer(vk::Viewport viewport, vk::CommandBuffer cmdBuffer, vk::DescriptorSet descriptorSet, vk::Image image, vk::ImageView imageView)
{
   cmdBuffer.setViewport(0, { viewport });
   cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, { descriptorSet }, {});
   cmdBuffer.bindVertexBuffers(0, { mVertexBuffer }, { 0 });
   cmdBuffer.draw(6, 1, 0, 0);
}

void
VulkanRenderer::releaseScanBuffer(vk::CommandBuffer cmdBuffer, vk::DescriptorSet descriptorSet, vk::Image image, vk::ImageView imageView)
{
   vk::ImageMemoryBarrier imageBarrier;
   imageBarrier.srcAccessMask = vk::AccessFlags();
   imageBarrier.dstAccessMask = vk::AccessFlags();
   imageBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
   imageBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
   imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   imageBarrier.image = image;
   imageBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
   imageBarrier.subresourceRange.baseMipLevel = 0;
   imageBarrier.subresourceRange.levelCount = 1;
   imageBarrier.subresourceRange.baseArrayLayer = 0;
   imageBarrier.subresourceRange.layerCount = 1;

   cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllGraphics,
                             vk::PipelineStageFlagBits::eAllGraphics,
                             vk::DependencyFlagBits::eByRegion,
                             {},
                             {},
                             { imageBarrier });
}

void
VulkanRenderer::renderFrame(Viewport &tv, Viewport &drc)
{
}

void VulkanRenderer::titleLoaded(quint64 id, const QString &name)
{
   mGameLoaded = true;
}

void VulkanRenderer::startNextFrame()
{
   const QSize sz = m_window->swapChainImageSize();
   auto renderCmdBuf = vk::CommandBuffer { m_window->currentCommandBuffer() };
   renderCmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);

   vk::ClearColorValue mBackgroundColour;
   // Setup background colour
   mBackgroundColour.float32[0] = 128.0f / 255.0f;
   mBackgroundColour.float32[1] = 128.0f / 255.0f;
   mBackgroundColour.float32[2] = 128.0f / 255.0f;

   // Apply some gamma correction
   mBackgroundColour.float32[0] = pow(mBackgroundColour.float32[0], 2.2f);
   mBackgroundColour.float32[1] = pow(mBackgroundColour.float32[1], 2.2f);
   mBackgroundColour.float32[2] = pow(mBackgroundColour.float32[2], 2.2f);

   vk::RenderPassBeginInfo renderPassBeginInfo;
   renderPassBeginInfo.renderPass = m_window->defaultRenderPass();
   renderPassBeginInfo.framebuffer = m_window->currentFramebuffer();
   renderPassBeginInfo.renderArea = vk::Rect2D({ 0, 0 }, { static_cast<uint32_t>(sz.width()), static_cast<uint32_t>(sz.height()) });
   vk::ClearValue clearValues[] = {
      mBackgroundColour, vk::ClearDepthStencilValue { }
   };
   renderPassBeginInfo.clearValueCount = 2;
   renderPassBeginInfo.pClearValues = clearValues;

   vk::Image tvImage, drcImage;
   vk::ImageView tvView, drcView;

   // Select some descriptors to use
   auto &frameResources = mFrameResources[mFrameIndex];
   auto descriptorSetTv = frameResources.tvDesc;
   auto descriptorSetDrc = frameResources.drcDesc;

   if (mGameLoaded) {
      Viewport tv, drc;
      calculateScreenViewports(tv, drc);

      // Grab the scan buffers...
      mDecafDriver->getSwapBuffers(tvImage, tvView, drcImage, drcView);

      if (tvImage) {
         acquireScanBuffer(renderCmdBuf, descriptorSetTv, tvImage, tvView);
      }
      if (drcImage) {
         acquireScanBuffer(renderCmdBuf, descriptorSetDrc, drcImage, drcView);
      }
      renderCmdBuf.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

      // TODO: Technically, we are generating these coordinates upside down, and then
      //  'correcting' it here.  We should probably generate these accurately, and then
      //  flip them for OpenGL, which is the API with the unintuitive origin.
      vk::Viewport tvViewport(tv.x, tv.y, tv.width, tv.height);
      tvViewport.y = sz.height() - (tvViewport.y + tvViewport.height);

      vk::Viewport drcViewport(drc.x, drc.y, drc.width, drc.height);
      drcViewport.y = sz.height() - (drcViewport.y + drcViewport.height);

      if (tvImage) {
         renderScanBuffer(tvViewport, renderCmdBuf, descriptorSetTv, tvImage, tvView);
      }
      if (drcImage) {
         renderScanBuffer(drcViewport, renderCmdBuf, descriptorSetDrc, drcImage, drcView);
      }

      // Draw the debug UI
      if (mDebugUiRenderer) {
         mDebugUiRenderer->draw(sz.width(), sz.height(), renderCmdBuf);
      }
   } else {
      renderCmdBuf.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
   }

   renderCmdBuf.endRenderPass();

   if (tvImage) {
      releaseScanBuffer(renderCmdBuf, descriptorSetTv, tvImage, tvView);
   }
   if (drcImage) {
      releaseScanBuffer(renderCmdBuf, descriptorSetDrc, drcImage, drcView);
   }

   // Increment our frame index counter
   mFrameIndex = (mFrameIndex + 1) % mFrameResources.size();

   m_window->frameReady();
   m_window->requestUpdate(); // render continuously, throttled by the presentation rate
}
