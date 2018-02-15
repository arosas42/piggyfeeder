#include "LightController.h"

void LightControllerClass::begin() {
    pinMode(PWM_PIN, OUTPUT);
    pinMode(PWM_HIGH_PIN, OUTPUT);
    pinMode(MOTION_PIN, INPUT);
    pinMode(LDR_HIGH_PIN, OUTPUT);

    digitalWrite(PWM_PIN, LOW);
    digitalWrite(PWM_HIGH_PIN, HIGH);
    digitalWrite(LDR_HIGH_PIN, HIGH);

    NRF_PWM1->PSEL.OUT[0] = (PWM_PIN << PWM_PSEL_OUT_PIN_Pos) | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);                        
    NRF_PWM1->ENABLE      = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
    NRF_PWM1->MODE        = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
    NRF_PWM1->PRESCALER   = (PWM_PRESCALER_PRESCALER_DIV_1 <<  PWM_PRESCALER_PRESCALER_Pos);
    NRF_PWM1->COUNTERTOP  = (LIGHT_PWM_TOP << PWM_COUNTERTOP_COUNTERTOP_Pos); //1 msec
    NRF_PWM1->LOOP        = 0;
    NRF_PWM1->DECODER     = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM1->SEQ[0].PTR  = ((uint32_t)(light_seq) << PWM_SEQ_PTR_PTR_Pos);
    NRF_PWM1->SEQ[0].CNT  = 4;
    NRF_PWM1->SEQ[0].REFRESH  = 0;
    NRF_PWM1->SEQ[0].ENDDELAY = 0;
    NRF_PWM1->TASKS_SEQSTART[0] = 1;

    currState = STATE_LOW;
    conf.mode = MODE_OFF | LDR_MASK;
    conf.maxPWM = LIGHT_PWM_TOP;
    conf.minPWM = 0;
}

void LightControllerClass::pwm(uint16_t val) {
    val |= bit(15);
    if (light_seq[0] != val) {
        light_seq[0] = val;
        NRF_PWM1->TASKS_SEQSTART[0] = 1;
    }
}

void LightControllerClass::think() {
    static uint32_t lastMillis = 0;
    currMillis = millis();

    switch(conf.mode & MODE_MASK) {
        case MODE_OFF:
            pwm(0);
            break;

        case MODE_ON:
            if (conf.mode & LDR_MASK && analogRead(LDR_PIN) >= THRESHOLD_DAY)
              pwm(0);
            else
              pwm(conf.maxPWM);
            break;

        case MODE_MOTION:
        default:
            runStateMachine();
    }
}

void LightControllerClass::runStateMachine() {
    if (digitalRead(MOTION_PIN) == HIGH) {
        motionMillis = currMillis;
    }

    if (currState != STATE_OFF &&
        conf.mode & LDR_MASK &&
        analogRead(LDR_PIN) >= THRESHOLD_DAY) {
      currState = STATE_OFF;
    }

    switch(currState) {
        case STATE_LOW:
            stateLow();
            break;
    
        case STATE_TRANSIT:
            stateTransit();
            break;

        case STATE_HIGH:
            stateHigh();
            break;

        case STATE_OFF:
            stateOff();
            break;

        default:
            stateLow();
    }
}

void LightControllerClass::stateOff() {
  if (conf.mode & LDR_MASK && analogRead(LDR_PIN) <= THRESHOLD_NIGHT) {
    currState = STATE_LOW;
    return;
  }
  pwm(0);
}

void LightControllerClass::stateLow() {
    if (motionMillis == currMillis) {
        currState = STATE_TRANSIT;
        transitMillis = currMillis;
    }
    else
        pwm(conf.minPWM);
}

void LightControllerClass::stateTransit() {
    static uint32_t printMillis = 0;

    if ((currMillis - motionMillis)/1000 > MOTION_TIMEOUT) {
        currState = STATE_LOW;
        return;
    }

    float time_lapse = (currMillis - transitMillis) / 1000.0;
    int16_t pwmValue = N4 * pow(time_lapse, 4)
                     + N3 * pow(time_lapse, 3)
                     + N2 * pow(time_lapse, 2)
                     + N1 * pow(time_lapse, 1)
                     + conf.minPWM;
                     
    if (pwmValue < conf.minPWM)
        pwmValue = conf.minPWM;

    if (pwmValue >= conf.maxPWM) {
        pwmValue  = conf.maxPWM;
        currState = STATE_HIGH;
    }
    else
        pwm(pwmValue);
}

void LightControllerClass::stateHigh() {
    if ((currMillis - motionMillis) / 1000 > MOTION_TIMEOUT) {
        currState = STATE_LOW;
        return;
    }
    pwm(conf.maxPWM);
}

const LightConfig * LightControllerClass::getConfig() {
    return &conf;
}

void LightControllerClass::setConfig(LightConfig * cnf) {
    if ((conf.mode & MODE_MASK) != (cnf->mode & MODE_MASK)) {
      currState = STATE_LOW;
      transitMillis = 0;
    }
    memcpy(&conf, cnf, sizeof(LightConfig));
}

LightControllerClass LightController;
