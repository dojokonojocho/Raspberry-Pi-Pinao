#include <stdio.h>
#include <wiringPi.h>

// define wiringPi pins for LEDs
#define LED0 0
#define LED1 2
#define LED2 3
#define LED3 4

int main(void) {
    // Setup WiringPi
    if (wiringPiSetup() == -1)
      return 1;

    // Set output pins
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
	digitalWrite(LED0, 0);
	digitalWrite(LED1, 0);
	digitalWrite(LED2, 0);
	digitalWrite(LED3, 0);
    return 0;
  }

