#pragma optimize("", off)
#include "inputsettings.h"
#include "inputeventfilter.h"
#include "inputdriver.h"

#include <QStandardItemModel>

#include <libdecaf/decaf_input.h>
#include <functional>

const char *
getButtonName(ButtonType type)
{
   switch (type) {
   case ButtonType::A:
      return "A";
   case ButtonType::B:
      return "B";
   case ButtonType::X:
      return "X";
   case ButtonType::Y:
      return "Y";
   case ButtonType::R:
      return "R";
   case ButtonType::L:
      return "L";
   case ButtonType::ZR:
      return "ZR";
   case ButtonType::ZL:
      return "ZL";
   case ButtonType::Plus:
      return "Plus";
   case ButtonType::Minus:
      return "Minus";
   case ButtonType::Home:
      return "Home";
   case ButtonType::Sync:
      return "Sync";
   case ButtonType::DpadUp:
      return "DpadUp";
   case ButtonType::DpadDown:
      return "DpadDown";
   case ButtonType::DpadLeft:
      return "DpadLeft";
   case ButtonType::DpadRight:
      return "DpadRight";
   case ButtonType::LeftStickPress:
      return "LeftStickPress";
   case ButtonType::LeftStickUp:
      return "LeftStickUp";
   case ButtonType::LeftStickDown:
      return "LeftStickDown";
   case ButtonType::LeftStickLeft:
      return "LeftStickLeft";
   case ButtonType::LeftStickRight:
      return "LeftStickRight";
   case ButtonType::RightStickPress:
      return "RightStickPress";
   case ButtonType::RightStickUp:
      return "RightStickUp";
   case ButtonType::RightStickDown:
      return "RightStickDown";
   case ButtonType::RightStickLeft:
      return "RightStickLeft";
   case ButtonType::RightStickRight:
      return "RightStickRight";
   default:
      return "";
   }
}

InputSettings::InputSettings(InputDriver *inputDriver, QWidget *parent, Qt::WindowFlags f) :
   QDialog(parent, f),
   mInputDriver(inputDriver)
{
   mUi.setupUi(this);
   mInputEventFilter = new InputEventFilter(this);
   qApp->installEventFilter(mInputEventFilter);
   connect(mInputEventFilter, &InputEventFilter::caughtKeyPress, this, &InputSettings::caughtKeyPress);
   connect(mInputDriver, &InputDriver::joystickConnected, this, &InputSettings::joystickConnected);
   connect(mInputDriver, &InputDriver::joystickDisconnected, this, &InputSettings::joystickDisconnected);
   connect(mInputDriver, &InputDriver::joystickButtonDown, this, &InputSettings::joystickButton);
   connect(mInputDriver, &InputDriver::joystickAxisMotion, this, &InputSettings::joystickAxisMotion);
   connect(mInputDriver, &InputDriver::joystickHatMotion, this, &InputSettings::joystickHatMotion);
}

InputSettings::~InputSettings()
{
}

void InputSettings::showEvent(QShowEvent *ev)
{
   mInputConfiguration = mInputDriver->getInputConfiguration();
   updateControllerListFromConfiguration();
   QDialog::showEvent(ev);
}

void InputSettings::addController()
{
   auto id = mInputConfiguration.controllers.size();
   mInputConfiguration.controllers.push_back({});

   auto item = new QListWidgetItem(QString("New Controller %1").arg(id));
   mUi.controllerList->addItem(item);
   mUi.controllerList->setCurrentItem(item);
}

void InputSettings::removeController()
{
   mInputConfiguration.controllers.erase(mInputConfiguration.controllers.begin() + mUi.controllerList->currentRow());
   delete mUi.controllerList->takeItem(mUi.controllerList->currentRow());
}

void InputSettings::updateControllerListFromConfiguration()
{
   mUi.controllerList->setUpdatesEnabled(false);
   mUi.controllerList->clear();

   auto index = 0;
   auto gamepad = 0;
   auto wiimote = 0;
   auto pro = 0;
   auto classic = 0;

   for (auto &controller : mInputConfiguration.controllers) {
      switch (controller.type) {
      case ControllerType::Invalid:
         mUi.controllerList->addItem(QString("New Controller %1").arg(index));
         break;
      case ControllerType::Gamepad:
         mUi.controllerList->addItem(QString("Gamepad %1").arg(gamepad++));
         break;
      case ControllerType::WiiMote:
         mUi.controllerList->addItem(QString("Wiimote %1").arg(wiimote++));
         break;
      case ControllerType::ProController:
         mUi.controllerList->addItem(QString("Pro Controller %1").arg(pro++));
         break;
      case ControllerType::ClassicController:
         mUi.controllerList->addItem(QString("Classic Controller %1").arg(classic++));
         break;
      }

      ++index;
   }

   mUi.controllerList->setUpdatesEnabled(true);
}

void InputSettings::editController(int index)
{
   if (index == -1) {
      mUi.controllerType->clear();
      return;
   }

   auto type = mInputConfiguration.controllers[index].type;
   mUi.controllerType->setUpdatesEnabled(false);
   mUi.controllerType->clear();
   mUi.controllerType->addItem("Unconfigured", QVariant::fromValue(static_cast<int>(ControllerType::Invalid)));
   mUi.controllerType->addItem("Wii U Gamepad", QVariant::fromValue(static_cast<int>(ControllerType::Gamepad)));
   mUi.controllerType->addItem("Wiimote", QVariant::fromValue(static_cast<int>(ControllerType::WiiMote)));
   mUi.controllerType->addItem("Pro Controller", QVariant::fromValue(static_cast<int>(ControllerType::ProController)));
   mUi.controllerType->addItem("Classic Controller", QVariant::fromValue(static_cast<int>(ControllerType::ClassicController)));
   mUi.controllerType->setCurrentIndex(mUi.controllerType->findData(QVariant::fromValue(static_cast<int>(type))));
   mUi.controllerType->setUpdatesEnabled(true);
}

void InputSettings::assignButton(QPushButton *button, ButtonType type)
{
   if (button->isChecked()) {
      if (mAssignButton && mAssignButton != button) {
         mAssignButton->setChecked(false);
      }

      mInputEventFilter->enable();
      mInputDriver->enableButtonEvents();
      mAssignButtonType = type;
      mAssignButton = button;
   } else if (mAssignButton == button) {
      mAssignButtonType = type;
      mAssignButton = nullptr;
      mInputEventFilter->disable();
      mInputDriver->disableButtonEvents();
   }
}

static QString
inputToText(const InputConfiguration::Input &input)
{
   switch (input.source) {
   case InputConfiguration::Input::KeyboardKey:
      return QString("Keyboard key %1")
         .arg(QKeySequence(input.id).toString());
   case InputConfiguration::Input::JoystickButton:
      return QString("Joystick %1 button %2")
         .arg(input.joystickInstanceId)
         .arg(input.id);
   case InputConfiguration::Input::JoystickAxis:
      return QString("Joystick %1 axis %2 %3")
         .arg(input.joystickInstanceId)
         .arg(input.id)
         .arg(input.invert ? "negative" : "positive");
   case InputConfiguration::Input::JoystickHat:
   {
      const char *direction = "";
      if (input.hatValue == SDL_HAT_UP) {
         direction = "up";
      } else if (input.hatValue == SDL_HAT_LEFT) {
         direction = "left";
      } else if (input.hatValue == SDL_HAT_RIGHT) {
         direction = "right";
      } else if (input.hatValue == SDL_HAT_DOWN) {
         direction = "down";
      }

      return QString("Joystick %1 Hat %2 %3")
         .arg(input.joystickInstanceId)
         .arg(input.id)
         .arg(direction);
   }
   default:
      return {};
   }
}

void InputSettings::caughtKeyPress(int key)
{
   // TODO: Assign mAssignButtonType to key!
   if (mAssignButton) {
      auto index = mUi.controllerList->currentIndex().row();
      auto &controller = mInputConfiguration.controllers[index];
      auto &input = controller.inputs[static_cast<size_t>(mAssignButtonType)];
      input.source = InputConfiguration::Input::KeyboardKey;
      input.id = key;

      mAssignButton->setText(inputToText(input));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::joystickConnected(SDL_JoystickID id, SDL_JoystickGUID guid, const char *name)
{
   mJoysticks.push_back({ id, guid, name });
}

void InputSettings::joystickDisconnected(SDL_JoystickID id, SDL_JoystickGUID guid)
{
}

void InputSettings::joystickButton(SDL_JoystickID id, SDL_JoystickGUID guid, int button)
{
   if (mAssignButton) {
      auto index = mUi.controllerList->currentIndex().row();
      auto &controller = mInputConfiguration.controllers[index];
      auto &input = controller.inputs[static_cast<size_t>(mAssignButtonType)];
      input.source = InputConfiguration::Input::JoystickButton;
      input.id = button;
      input.joystickInstanceId = id;
      input.joystickGuid = guid;

      mAssignButton->setText(inputToText(input));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::joystickAxisMotion(SDL_JoystickID id, SDL_JoystickGUID guid, int axis, float value)
{
   if (value > -0.5f && value < 0.5) {
      // Ignore until axis has a decent value
      return;
   }

   if (mAssignButton) {
      auto index = mUi.controllerList->currentIndex().row();
      auto &controller = mInputConfiguration.controllers[index];
      auto &input = controller.inputs[static_cast<size_t>(mAssignButtonType)];
      input.source = InputConfiguration::Input::JoystickAxis;
      input.id = axis;
      input.joystickInstanceId = id;
      input.joystickGuid = guid;
      input.invert = (value < 0);

      mAssignButton->setText(inputToText(input));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::joystickHatMotion(SDL_JoystickID id, SDL_JoystickGUID guid, int hat, int value)
{
   const char *direction = nullptr;
   if (value == SDL_HAT_UP) {
      direction = "up";
   } else if (value == SDL_HAT_LEFT) {
      direction = "left";
   } else if (value == SDL_HAT_RIGHT) {
      direction = "right";
   } else if (value == SDL_HAT_DOWN) {
      direction = "down";
   } else {
      // Only allow one direction
      return;
   }

   if (mAssignButton) {
      auto index = mUi.controllerList->currentIndex().row();
      auto &controller = mInputConfiguration.controllers[index];
      auto &input = controller.inputs[static_cast<size_t>(mAssignButtonType)];
      input.source = InputConfiguration::Input::JoystickHat;
      input.id = hat;
      input.hatValue = value;
      input.joystickInstanceId = id;
      input.joystickGuid = guid;

      mAssignButton->setText(inputToText(input));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::controllerTypeChanged(int typeIndex)
{
   mUi.buttonList->setUpdatesEnabled(false);
   qDeleteAll(mUi.buttonList->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
   mUi.buttonList->setUpdatesEnabled(true);

   if (typeIndex == -1) {
      return;
   }

   auto type = static_cast<ControllerType>(mUi.controllerType->itemData(typeIndex).toInt());
   auto index = mUi.controllerList->currentIndex().row();
   auto &controller = mInputConfiguration.controllers[index];
   controller.type = type;
   mUi.buttonList->setUpdatesEnabled(false);

   if (type == ControllerType::Gamepad) {
      auto buttonLayout = reinterpret_cast<QFormLayout *>(mUi.buttonList->layout());
      auto addButton = [&](ButtonType type)
      {
         auto &input = controller.inputs[static_cast<size_t>(type)];
         auto button = new QPushButton(inputToText(input));
         button->setCheckable(true);
         connect(button, &QPushButton::toggled, std::bind(&InputSettings::assignButton, this, button, type));
         buttonLayout->addRow(getButtonName(type), button);
      };

      addButton(ButtonType::A);
      addButton(ButtonType::B);
      addButton(ButtonType::X);
      addButton(ButtonType::Y);
      addButton(ButtonType::R);
      addButton(ButtonType::L);
      addButton(ButtonType::ZR);
      addButton(ButtonType::ZL);
      addButton(ButtonType::Plus);
      addButton(ButtonType::Minus);
      addButton(ButtonType::Home);
      addButton(ButtonType::Sync);
      addButton(ButtonType::DpadUp);
      addButton(ButtonType::DpadDown);
      addButton(ButtonType::DpadLeft);
      addButton(ButtonType::DpadRight);
      addButton(ButtonType::LeftStickPress);
      addButton(ButtonType::LeftStickUp);
      addButton(ButtonType::LeftStickDown);
      addButton(ButtonType::LeftStickLeft);
      addButton(ButtonType::LeftStickRight);
      addButton(ButtonType::RightStickPress);
      addButton(ButtonType::RightStickUp);
      addButton(ButtonType::RightStickDown);
      addButton(ButtonType::RightStickLeft);
      addButton(ButtonType::RightStickRight);
   }

   mUi.buttonList->setUpdatesEnabled(true);
}

void InputSettings::buttonBoxClicked(QAbstractButton *button)
{
   auto role = mUi.buttonBox->buttonRole(button);
   if (role == QDialogButtonBox::AcceptRole ||
       role == QDialogButtonBox::ApplyRole) {
      mInputDriver->updateInputConfiguration(mInputConfiguration);
   }

   if (role == QDialogButtonBox::ResetRole) {
      mInputConfiguration = mInputDriver->getInputConfiguration();
      updateControllerListFromConfiguration();
   }

   if (role == QDialogButtonBox::AcceptRole) {
      accept();
   } else if (role == QDialogButtonBox::RejectRole) {
      reject();
   }
}