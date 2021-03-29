#include "Buzzer.h"

#define BUZZER_CHANNEL 0

void startBuzz()
{
  tone(PIN_BUZZER, 500, NULL, 0);
}
