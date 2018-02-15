#include "ChargeController.h"

void ChargeControllerClass::begin()
{
    // setup the M in EasyDMA
    pwm_seq[0] = 137 | bit(15);
    pwm_seq[1] = 117; // inverted polarity
    pwm_seq[2] = 0 | bit(15);

    // these registers are documented
    // in the nrf52 sdk (I used sdk 11/SoftDevice 132)
    
    NRF_PWM0->PSEL.OUT[0] = (CHARGE_PUMP_PIN1 << PWM_PSEL_OUT_PIN_Pos) | 
                            (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
    NRF_PWM0->PSEL.OUT[1] = (CHARGE_PUMP_PIN2 << PWM_PSEL_OUT_PIN_Pos) | 
                            (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
    NRF_PWM0->PSEL.OUT[2] = (CHARGE_PUMP_PWM << PWM_PSEL_OUT_PIN_Pos) | 
                            (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
    NRF_PWM0->ENABLE      = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
    NRF_PWM0->MODE        = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
    NRF_PWM0->PRESCALER   = (PWM_PRESCALER_PRESCALER_DIV_1 <<  PWM_PRESCALER_PRESCALER_Pos);
    NRF_PWM0->COUNTERTOP  = (CHARGE_PWM_TOP << PWM_COUNTERTOP_COUNTERTOP_Pos); //1 msec
    NRF_PWM0->LOOP        = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);
    NRF_PWM0->DECODER     = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->SEQ[0].PTR  = ((uint32_t)(pwm_seq) << PWM_SEQ_PTR_PTR_Pos);
    NRF_PWM0->SEQ[0].CNT  = 4;
    NRF_PWM0->SEQ[0].REFRESH  = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;

    // use VDD as reference pin
    analogReference(AR_VDD4);

    measurement = 0;
    pulseWidth  = 0;
    difference  = 0;
    stepSize    = 0;
}

void ChargeControllerClass::think()
{
    static uint32_t lastPrintMillis  = 0;
    static uint32_t lastSampleMillis = 0;
           uint32_t currMillis       = millis();
    
    if (currMillis - lastMillis > CHARGE_DELAY)
    {
        measurement = (int)getADCValue();
        difference  = (int)setPoint - (int)measurement;

        // if there's a considerable swing in voltage 
        // ex: sudden load placed/released on/from battery
        // adjust pwm in large increments
        if (abs(difference) > 100) {
          pulseWidth += difference;
        }

        // otherwise take a slower pace
        else if (difference > 0) {
          pulseWidth++;
        }
        else {
          pulseWidth--;
        }
        
        if (pulseWidth > CHARGE_PWM_TOP)
            pulseWidth = CHARGE_PWM_TOP;

        if (pulseWidth < 0)
            pulseWidth = 0;

        pwm(pulseWidth);
        lastMillis = currMillis;
    }
}

int ChargeControllerClass::getPulseWidth() {
  return pulseWidth;
}

void ChargeControllerClass::pwm(uint16_t val) {
    val |= bit(15);
    if (pwm_seq[2] == val) {
        return;
    }
    
    pwm_seq[2] = val;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;  
}

uint16_t ChargeControllerClass::getADCValue() {
  return (analogRead(BATTERY_PIN) * CALIBRATION_FACTOR)/1024;
}

float ChargeControllerClass::getVoltage() {
    return (getADCValue() * 102 * 3.3)/(1024*20);
}

ChargeControllerClass ChargeController;
