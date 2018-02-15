#include "MotorController.h"

void MotorControllerClass::begin() {
    pinMode(I2C_LOW_PIN, OUTPUT);
    pinMode(I2C_HIGH_PIN, OUTPUT);
    digitalWrite(I2C_LOW_PIN, LOW);
    digitalWrite(I2C_HIGH_PIN, HIGH);
  
    NRF_PWM2->PSEL.OUT[0] = (MOTOR_PIN << PWM_PSEL_OUT_PIN_Pos) | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);                        
    NRF_PWM2->ENABLE      = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
    NRF_PWM2->MODE        = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
    NRF_PWM2->PRESCALER   = (PWM_PRESCALER_PRESCALER_DIV_1 <<  PWM_PRESCALER_PRESCALER_Pos);
    NRF_PWM2->COUNTERTOP  = (255 << PWM_COUNTERTOP_COUNTERTOP_Pos); //1 msec
    NRF_PWM2->LOOP        = 0;
    NRF_PWM2->DECODER     = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM2->SEQ[0].PTR  = ((uint32_t)(motor_seq) << PWM_SEQ_PTR_PTR_Pos);
    NRF_PWM2->SEQ[0].CNT  = 4;
    NRF_PWM2->SEQ[0].REFRESH  = 0;
    NRF_PWM2->SEQ[0].ENDDELAY = 0;
    NRF_PWM2->TASKS_SEQSTART[0] = 1;

    rtc.begin();
    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    memset(schedule, 0, sizeof(MotorEvent) * MAX_MOTOR_EVENTS);
}

void MotorControllerClass::think() {
  currMillis = millis();

  switch(currState) {
    case STATE_HIGH:
      highState();
      break;
      
    case STATE_LOW:
    default:
      lowState();
      break;
  }
}

void MotorControllerClass::lowState() {
  static uint32_t lastUnix = 0;
  static uint32_t lastMillis = 0;
  
  if( currMillis - lastMillis > DELAY_LOW ) {
    DateTime now = rtc.now();
    lastMillis = currMillis;
    
    if(lastUnix != now.unixtime()) {
      lastUnix = now.unixtime();

      // only check schedule once a minute on the minute
      if(!(lastUnix % 60)) { 
        for( int i = 0; i < MAX_MOTOR_EVENTS; i++ ) {
          uint32_t sched_seconds = (schedule[i].hour * 60 + schedule[i].minute) * 60;
          uint8_t  day_mask      = 1 << now.dayOfTheWeek();

          // we have a match! run motor
          if (schedule[i].dow & day_mask && (lastUnix % DAY_SECONDS) == sched_seconds) {
            runMotor(schedule[i].duration, schedule[i].power);
            return;        
          }
        }
      }
    }
  }
  pwm(0);
}

void MotorControllerClass::highState() {
  static uint8_t count = 0;
  static uint32_t lastMillis = 0;

  if ( currMillis - lastMillis > DELAY_HIGH ) {
    lastMillis = currMillis;
    
    if (count < duration) {
      count++;
    }
    else {
      count = 0;
      currState = STATE_LOW;
    }
  }
  pwm(power);
}

void MotorControllerClass::runMotor(uint8_t duration, uint8_t power) {
  this->duration = duration;
  this->power = power;
  this->currState = STATE_HIGH;
}

void MotorControllerClass::pwm(uint16_t val) {
  val |= bit(15);
  if(motor_seq[0] != val) {
    motor_seq[0] = val;
    NRF_PWM2->TASKS_SEQSTART[0] = 1;
  }
}

const MotorEvent * MotorControllerClass::readSchedule(uint8_t idx) {
  if(idx < MAX_MOTOR_EVENTS) {
    return &schedule[idx];
  }
  return (MotorEvent *)NULL;
}

void MotorControllerClass::writeSchedule(uint8_t idx, MotorEvent event) {
  if(idx < MAX_MOTOR_EVENTS) {
    memcpy((char *)&schedule[idx], (char *)&event, sizeof(MotorEvent));
  }
}

MotorControllerClass MotorController;
