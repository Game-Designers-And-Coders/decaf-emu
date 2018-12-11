#include "sdleventloop.h"
#include <QTimer>
#include <SDL.h>
#include <SDL_gamecontroller.h>

SdlEventLoop::SdlEventLoop(QObject *parent) :
   QObject(parent)
{
   SDL_Init(SDL_INIT_HAPTIC | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
   QTimer::singleShot(100, Qt::PreciseTimer, this, SLOT(update()));
}

SdlEventLoop::~SdlEventLoop()
{
   SDL_Quit();
}

static inline float translateAxisValue(Sint16 value)
{
   if (value < 0) {
      return value / 32768.0f;
   } else {
      return value / 32767.0f;
   }
}

void SdlEventLoop::update()
{
   SDL_Event event;

   while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_JOYDEVICEADDED:
      {
         auto joystick = SDL_JoystickOpen(event.jdevice.which);
         emit joystickConnected(SDL_JoystickInstanceID(joystick),
                                SDL_JoystickGetGUID(joystick),
                                SDL_JoystickName(joystick));
         break;
      }
      case SDL_JOYDEVICEREMOVED:
      {
         auto joystick = SDL_JoystickFromInstanceID(event.jdevice.which);
         if (joystick) {
            joystickDisconnected(SDL_JoystickInstanceID(joystick));
            SDL_JoystickClose(joystick);
         }
         break;
      }
      case SDL_JOYBUTTONDOWN:
      {
         if (mButtonEventsEnabled) {
            joystickButtonDown(event.jbutton.which, event.jbutton.button);
         }
         break;
      }
      case SDL_JOYAXISMOTION:
      {
         if (mButtonEventsEnabled) {
            joystickAxisMotion(event.jaxis.which, event.jaxis.axis, translateAxisValue(event.jaxis.value));
         }
         break;
      }
      case SDL_JOYHATMOTION:
      {
         if (mButtonEventsEnabled) {
            joystickHatMotion(event.jhat.which, event.jhat.hat, event.jhat.value);
         }
         break;
      }

      // I don't think we actually care about game controllers?
      case SDL_CONTROLLERDEVICEADDED:
         qDebug("SDL_CONTROLLERDEVICEADDED");
         break;
      case SDL_CONTROLLERDEVICEREMAPPED:
         qDebug("SDL_CONTROLLERDEVICEREMAPPED");
         break;
      case SDL_CONTROLLERDEVICEREMOVED:
         qDebug("SDL_CONTROLLERDEVICEREMOVED");
         break;
      case SDL_CONTROLLERBUTTONDOWN:
         qDebug("SDL_CONTROLLERBUTTONDOWN");
         break;
      }
   }

   QTimer::singleShot(10, Qt::PreciseTimer, this, SLOT(update()));
}
