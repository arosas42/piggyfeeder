#ifndef _CHARGECONTROLLER_h
#define _CHARGECONTROLLER_h

#define BATTERY_PIN        31
#define CHARGE_PUMP_PIN1   30 
#define CHARGE_PUMP_PIN2   14
#define CHARGE_PUMP_PWM    20
#define CALIBRATION_FACTOR 1040
#define ADC_TOTAL_SAMPLES  8
#define CHARGE_PWM_TOP     255

#include "arduino.h"
#include "nrf.h"

// check voltage every 5 ms
#define CHARGE_DELAY 5

class ChargeControllerClass
{
 protected:
     const 
     uint16_t setPoint = 13.5 * 20 / (20 + 82) * 1024 / 3.3; // voltage divider R1 82k, R2 20k
     int      measurement;
     uint16_t pulseWidth;
     int      difference;
     int      stepSize;
     uint16_t pwm_seq[4];
     uint32_t lastMillis;
     void     pwm(uint16_t);

 public:
    void     begin();
    void     think();
    int      getPulseWidth();
    uint16_t getADCValue(void);
    float    getVoltage();
};

extern ChargeControllerClass ChargeController;

#endif
