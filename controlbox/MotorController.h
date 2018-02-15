#ifndef _MOTORCONTROLLER_h
#define _MOTORCONTROLLER_h

#include "arduino.h"
#include "nrf.h"
#include "RTClib.h"

#define MOTOR_PIN    25
#define I2C_LOW_PIN  15
#define I2C_HIGH_PIN 19

#define MAX_MOTOR_EVENTS 10

#define STATE_LOW   0
#define STATE_HIGH  1
#define DELAY_LOW   500
#define DELAY_HIGH  1000
#define DAY_SECONDS 86400

static uint16_t motor_seq[4] = {bit(15),bit(15),bit(15),bit(15)};

struct __attribute__((__packed__)) MotorEvent {
  uint8_t dow;
  uint8_t hour;
  uint8_t minute;
  uint8_t duration;
  uint8_t power;
};

struct __attribute__((__packed__)) MotorEnvelope {
  uint8_t    idx;
  MotorEvent event;
};

class MotorControllerClass {
  private:
  void pwm(uint16_t);
  void lowState();
  void highState();

  MotorEvent schedule[MAX_MOTOR_EVENTS];
  RTC_DS3231 rtc;
  uint8_t    duration;
  uint8_t    power;
  uint8_t    currState;
  uint32_t   currMillis;
  
  public:
  void begin();
  void think();
  void runMotor(uint8_t, uint8_t);

  // the thought is:
  // given the easily tractable size of the "schedule"
  // the nrf chip doesn't care about the order of the schedule
  // just give the smart device full access to the array
  // and let IT handle the ordering/sorting for it's own purposes
  const MotorEvent * readSchedule(uint8_t);
  void writeSchedule(uint8_t, MotorEvent);
};

extern MotorControllerClass MotorController;
#endif
