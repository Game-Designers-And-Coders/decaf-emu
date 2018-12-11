#include "inputsettings.h"
#include "inputeventfilter.h"
#include "sdleventloop.h"

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

InputSettings::InputSettings(QWidget *parent, Qt::WindowFlags f) :
   QDialog(parent, f)
{
   mUi.setupUi(this);
   mInputEventFilter = new InputEventFilter(this);
   mSdlEventLoop = new SdlEventLoop(this);
   qApp->installEventFilter(mInputEventFilter);
   connect(mInputEventFilter, &InputEventFilter::caughtKeyPress, this, &InputSettings::caughtKeyPress);
   connect(mSdlEventLoop, &SdlEventLoop::joystickConnected, this, &InputSettings::joystickConnected);
   connect(mSdlEventLoop, &SdlEventLoop::joystickDisconnected, this, &InputSettings::joystickDisconnected);
   connect(mSdlEventLoop, &SdlEventLoop::joystickButtonDown, this, &InputSettings::joystickButton);
   connect(mSdlEventLoop, &SdlEventLoop::joystickAxisMotion, this, &InputSettings::joystickAxisMotion);
   connect(mSdlEventLoop, &SdlEventLoop::joystickHatMotion, this, &InputSettings::joystickHatMotion);
}

InputSettings::~InputSettings()
{
}

void InputSettings::addController()
{
   auto item = new QListWidgetItem(QString("New Controller %1").arg(mControllers.size()));
   mUi.controllerList->addItem(item);
   mUi.controllerList->setCurrentItem(item);
   mControllers.push_back({});
}

void InputSettings::removeController()
{
   mControllers.removeAt(mUi.controllerList->currentRow());
}

void InputSettings::editController(int index)
{
   auto &controller = mControllers[index];

   mUi.controllerType->setUpdatesEnabled(false);
   mUi.controllerType->clear();
   mUi.controllerType->addItem("Wii U Gamepad", QVariant::fromValue(static_cast<int>(Controller::Gamepad)));
   mUi.controllerType->addItem("Wiimote", QVariant::fromValue(static_cast<int>(Controller::WiiMote)));
   mUi.controllerType->addItem("Classic Controller", QVariant::fromValue(static_cast<int>(Controller::ClassicController)));
   mUi.controllerType->addItem("Pro Controller", QVariant::fromValue(static_cast<int>(Controller::ProController)));
   mUi.controllerType->setCurrentIndex(0);
   mUi.controllerType->setUpdatesEnabled(true);
}

void InputSettings::assignButton(QPushButton *button, ButtonType type)
{
   if (button->isChecked()) {
      if (mAssignButton && mAssignButton != button) {
         mAssignButton->setChecked(false);
      }

      mInputEventFilter->enable();
      mSdlEventLoop->enableButtonEvents();
      mAssignButtonType = type;
      mAssignButton = button;
   } else if (mAssignButton == button) {
      mAssignButtonType = type;
      mAssignButton = nullptr;
      mInputEventFilter->disable();
      mSdlEventLoop->disableButtonEvents();
   }
}

void InputSettings::caughtKeyPress(int key)
{
   // TODO: Assign mAssignButtonType to key!
   if (mAssignButton) {
      mAssignButton->setText(QString("Keyboard Key %1").arg(QKeySequence(key).toString()));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::joystickConnected(SDL_JoystickID id, SDL_JoystickGUID guid, const char *name)
{
   mJoysticks.push_back({ id, guid, name });
}

void InputSettings::joystickDisconnected(SDL_JoystickID id)
{
}

void InputSettings::joystickButton(SDL_JoystickID id, int button)
{
   // TODO: Assign mAssignButtonType to key!
   if (mAssignButton) {
      mAssignButton->setText(QString("Joystick %1 button %2").arg(id).arg(button));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::joystickAxisMotion(SDL_JoystickID id, int axis, float value)
{
   if (value > -0.5f && value < 0.5) {
      // Ignore until axis has a decent value
      return;
   }

   // TODO: Assign mAssignButtonType to key!
   if (mAssignButton) {
      mAssignButton->setText(QString("Joystick %1 axis %2 %3").arg(id).arg(axis).arg(value < 0 ? "negative" : "positive"));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::joystickHatMotion(SDL_JoystickID id, int hat, int value)
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
      return;
   }

   // TODO: Assign mAssignButtonType to key!
   if (mAssignButton) {
      mAssignButton->setText(QString("Joystick %1 hat %2 %3").arg(id).arg(hat).arg(direction));
      mAssignButton->setChecked(false);
   }
}

void InputSettings::controllerTypeChanged(int index)
{
   mUi.buttonList->setUpdatesEnabled(false);
   qDeleteAll(mUi.buttonList->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
   mUi.buttonList->setUpdatesEnabled(true);

   if (index == -1) {
      return;
   }

   auto type = static_cast<Controller::Type>(mUi.controllerType->itemData(index).toInt());
   mUi.buttonList->setUpdatesEnabled(false);

   if (type == Controller::Gamepad) {
      auto buttonLayout = reinterpret_cast<QFormLayout *>(mUi.buttonList->layout());
      auto addButton = [&](ButtonType type)
      {
         auto button = new QPushButton("");
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
