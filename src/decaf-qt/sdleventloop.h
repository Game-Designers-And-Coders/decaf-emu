#pragma once
#include <QObject>
#include <QVector>
#include <SDL_joystick.h>

class SdlEventLoop : public QObject
{
   Q_OBJECT

public:
   SdlEventLoop(QObject *parent = nullptr);
   ~SdlEventLoop();

   void enableButtonEvents() { mButtonEventsEnabled = true; }
   void disableButtonEvents() { mButtonEventsEnabled = false; }

signals:
   void joystickConnected(SDL_JoystickID id, SDL_JoystickGUID guid, const char *name);
   void joystickDisconnected(SDL_JoystickID id);
   void joystickButtonDown(SDL_JoystickID id, int button);
   void joystickAxisMotion(SDL_JoystickID id, int axis, float value);
   void joystickHatMotion(SDL_JoystickID id, int hat, int value);

private slots:
   void update();

private:
   bool mButtonEventsEnabled = false;
};
