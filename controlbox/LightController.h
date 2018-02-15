#ifndef _LIGHTCONTROLLER_h
#define _LIGHTCONTROLLER_h

#include "arduino.h"
#include "nrf.h"

#define LIGHT_PWM_TOP 1023
#define PWM_PIN       23
#define PWM_HIGH_PIN  24
#define MOTION_PIN    2

#define MODE_MASK   0x03
#define MODE_OFF    0x00
#define MODE_ON     0x01
#define MODE_MOTION 0x02

#define LDR_MASK     0x04
#define LDR_PIN      29
#define LDR_HIGH_PIN 28

#define STATE_LOW     0
#define STATE_TRANSIT 1
#define STATE_HIGH    2
#define STATE_OFF     3

// coefficients derived by using vandermonde matrix
// and interpolation
// with N0 being replaced by config's minPWM (or STATE_LOW PWM setting)
#define N4 0.00004
#define N3 -0.00614
#define N2 0.38088
#define N1 -5.75833

#define THRESHOLD_DAY   800
#define THRESHOLD_NIGHT 200
#define MOTION_TIMEOUT  300

struct __attribute__((__packed__)) LightConfig {
  uint8_t  mode;
  uint16_t maxPWM;
  uint16_t minPWM;
};

static uint16_t light_seq[4] = {bit(15),bit(15),bit(15),bit(15)};

class LightControllerClass {
  private:
    LightConfig conf;
    
    void pwm(uint16_t);

    void runStateMachine();
    void stateLow();
    void stateTransit();
    void stateHigh();
    void stateOff();

    uint8_t currState;

    uint32_t currMillis;
    uint32_t transitMillis;
    uint32_t motionMillis;

  public:
    void  begin();
    void  think();
    const LightConfig * getConfig(void);
    void  setConfig(LightConfig *);
};

extern LightControllerClass LightController;

#endif
