// If using PlatformIO, see the platformio.ini file for instructions
// on how to pick which sample to run

#include "Fsm.h"

// State machine variables
#define FLIP_LIGHT_SWITCH 1

State state_light_on(on_light_on_enter, NULL, &on_light_on_exit);
State state_light_off(on_light_off_enter, NULL, &on_light_off_exit);
Fsm fsm(&state_light_off);

// Transition callback functions
void on_light_on_enter()
{
  Serial.println(F("Entering LIGHT_ON"));
}

void on_light_on_exit()
{
  Serial.println(F("Exiting LIGHT_ON"));
}

void on_light_off_enter()
{
  Serial.println(F("Entering LIGHT_OFF"));
}

void on_light_off_exit()
{
  Serial.println(F("Exiting LIGHT_OFF"));
}

void on_trans_light_on_light_off()
{
  Serial.println(F("Transitioning from LIGHT_ON to LIGHT_OFF"));
}

void on_trans_light_off_light_on()
{
  Serial.println(F("Transitioning from LIGHT_OFF to LIGHT_ON"));
}

// standard arduino functions
void setup()
{
  Serial.begin(9600);
  while (!Serial && !Serial.available()) {
  }
  delay(1000);

  Serial.println(F("starting light_switch example"));

  fsm.add_transition(&state_light_on, &state_light_off,
                     FLIP_LIGHT_SWITCH,
                     &on_trans_light_on_light_off);
  fsm.add_transition(&state_light_off, &state_light_on,
                     FLIP_LIGHT_SWITCH,
                     &on_trans_light_off_light_on);
}

void loop()
{
  // "fsm.run_machine()" call needed because Fsm needs to be initialized
  fsm.run_machine();
  delay(2000);
  fsm.trigger(FLIP_LIGHT_SWITCH);
  delay(2000);
  fsm.trigger(FLIP_LIGHT_SWITCH);
}
