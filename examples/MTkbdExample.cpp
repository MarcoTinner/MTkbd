#include <Arduino.h>
#include <MTkbd.h>

#define Console Serial

MTkbd kbd;

void setup()
{
  Console.begin(115200);
  delay(100);
  
  Console.println(F("KeyBoard Library"));

  kbd.begin();

  Console.printf("KeyCode for pin  0 is %i\r\n", kbd.getKeyCodeOfPin(0));
  Console.printf("KeyCode for pin  2 is %i\r\n", kbd.getKeyCodeOfPin(2));
  Console.printf("KeyCode for pin  4 is %i\r\n", kbd.getKeyCodeOfPin(4));
  Console.printf("KeyCode for pin 36 is %i\r\n", kbd.getKeyCodeOfPin(36));
  Console.printf("KeyCode for pin 39 is %i\r\n", kbd.getKeyCodeOfPin(39));
}

void loop()
{
  kbd.loop();
  if (kbd.available())
  {
    if (kbd.IsPattern())
      Console.printf("-> handle Kbd Pattern %s\r\n", kbd.Pattern().c_str());
    else if (kbd.Repeat() > 0)
      Console.printf("-> handle Kbd KeyCode %i %i repeats withing duration %i ms\r\n",
                     kbd.KeyCode(), kbd.Repeat(), kbd.Duration());
    else
      Console.printf("-> handle Kbd KeyCode %i duration %i ms\r\n",
                     kbd.KeyCode(), kbd.Duration());
    kbd.handled();
  }
}
