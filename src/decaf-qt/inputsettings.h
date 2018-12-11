#pragma once
#include "ui_inputsettings.h"

#include <SDL_joystick.h>
#include <QDialog>
#include <QVector>
#include <QMap>

enum class ButtonType
{
   A,
   B,
   X,
   Y,
   R,
   L,
   ZR,
   ZL,
   Plus,
   Minus,
   Home,
   Sync,
   DpadUp,
   DpadDown,
   DpadLeft,
   DpadRight,
   LeftStickPress,
   LeftStickUp,
   LeftStickDown,
   LeftStickLeft,
   LeftStickRight,
   RightStickPress,
   RightStickUp,
   RightStickDown,
   RightStickLeft,
   RightStickRight,
};

struct Controller
{
   enum Type
   {
      Invalid,
      Gamepad,
      WiiMote,
      ProController,
      ClassicController,
   };

   Type type = Invalid;
};

class InputEventFilter;
class SdlEventLoop;

struct JoystickInfo
{
   SDL_JoystickID id = -1;
   SDL_JoystickGUID guid;
   const char *name = nullptr;
};

class InputSettings : public QDialog
{
   Q_OBJECT

public:
   InputSettings(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~InputSettings();

private slots:
   void addController();
   void removeController();
   void editController(int index);
   void controllerTypeChanged(int index);

   void assignButton(QPushButton *button, ButtonType type);
   void caughtKeyPress(int key);

   void joystickConnected(SDL_JoystickID id, SDL_JoystickGUID guid, const char *name);
   void joystickDisconnected(SDL_JoystickID guid);
   void joystickButton(SDL_JoystickID guid, int key);
   void joystickAxisMotion(SDL_JoystickID id, int axis, float value);
   void joystickHatMotion(SDL_JoystickID id, int hat, int value);

private:
   Ui::InputSettings mUi;
   QVector<Controller> mControllers;
   QVector<JoystickInfo> mJoysticks;
   InputEventFilter *mInputEventFilter;

   ButtonType mAssignButtonType;
   QPushButton *mAssignButton = nullptr;
   SdlEventLoop *mSdlEventLoop = nullptr;
};
