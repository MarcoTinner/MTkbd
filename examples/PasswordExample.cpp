#include <Arduino.h>
#include <MTkbd.h>

#define Console Serial

MTkbd kbd;

bool _advancedMode = false;

bool checkPassword(String password, uint8_t maxTry);

void setup()
{
  Console.begin(115200);
  delay(1000);

  Console.println(F("KeyBoard Library"));

  // begin keyboard with active low, key io pins 0 2 4 and 36
  kbd.Begin(true, 4, new uint8_t[4]{0, 2, 4, 36});

  // set pattern KeyCode for pin 0 and 2 pressed together
  kbd.SetPatternKeyCode(kbd.GetKeyCodeOfPin(0) | kbd.GetKeyCodeOfPin(2));

  Console.printf("KeyCode for pin  0 is %i\r\n", kbd.GetKeyCodeOfPin(0));
  Console.printf("KeyCode for pin  2 is %i\r\n", kbd.GetKeyCodeOfPin(2));
  Console.printf("KeyCode for pin  4 is %i\r\n", kbd.GetKeyCodeOfPin(4));
  Console.printf("KeyCode for pin 36 is %i\r\n", kbd.GetKeyCodeOfPin(36));
  Console.printf("KeyCode for pin 39 is %i\r\n", kbd.GetKeyCodeOfPin(39));

  // password check function
  _advancedMode = checkPassword("12488421", 3);
  Console.printf("%s you entered the %s password!\r\n\r\n", _advancedMode ? "Thanks" : "Sorry", _advancedMode ? "correct" : "wrong");
}

void loop()
{
  kbd.Loop();
  if (kbd.Available())
  {
    if (_advancedMode)
    {
      String hexKeyCode = String(kbd.KeyCode(), HEX);
      String binKeyCode = String(kbd.KeyCode(), BIN);
      if (kbd.IsPattern())
        Console.printf("-> handle Kbd Pattern %s\r\n", kbd.Pattern().c_str());
      else if (kbd.Repeat() > 0)
        Console.printf("-> handle Kbd KeyCode %i 0x%s 0b%s %i repeats withing duration %i ms\r\n",
                       kbd.KeyCode(), hexKeyCode.c_str(), binKeyCode.c_str(), kbd.Repeat(), kbd.Duration());
      else
        Console.printf("-> handle Kbd KeyCode %i 0x%s 0b%s duration %i ms\r\n",
                       kbd.KeyCode(), hexKeyCode.c_str(), binKeyCode.c_str(), kbd.Duration());
    }
    else
    {
      if (kbd.IsPattern())
        Console.printf("-> handle Kbd Pattern %s\r\n", kbd.Pattern().c_str());
      else if (kbd.Repeat() > 0)
        Console.printf("-> handle Kbd KeyCode %i %i repeats withing duration %i ms\r\n",
                       kbd.KeyCode(), kbd.Repeat(), kbd.Duration());
      else
        Console.printf("-> handle Kbd KeyCode %i duration %i ms\r\n",
                       kbd.KeyCode(), kbd.Duration());
    }
    kbd.Handled();
  }
}

bool checkPassword(String password, uint8_t maxTry)
{
  uint32_t curPatternTimeout = kbd.GetPatternTimeout();
  kbd.outputEnabled = false;
  uint8_t retry = 0;
  bool pwdMatch = false;
  do
  {
    Console.printf("Enter the password (press Key between %3.1f and %3.1f sec or wait 10 sec when done)\r\n",
                   (float)(kbd.GetPatternMinMS() / 1000), (float)(kbd.GetPatternMaxMS() / 1000));
    kbd.StartPasswordMode(10);
    while (!kbd.Available())
    {
      kbd.Loop();
    }
    if (kbd.Available())
    {
      pwdMatch = kbd.Pattern() == password;
      Console.printf("Password '%s' entered is %s\r\n\r\n", kbd.Pattern(), pwdMatch ? "correct!" : "wrong !?!");
      kbd.Handled();
      retry++;
    }
  } while (!pwdMatch && retry < maxTry);

  kbd.outputEnabled = true;
  kbd.SetPatternTimeout(curPatternTimeout);
  return pwdMatch;
}