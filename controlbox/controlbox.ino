#include <SPI.h>
#include <BLEPeripheral.h>

#define DEBUG
#define BLE_CONNECTED_PIN 7

#include "LightController.h"
#include "ChargeController.h"
#include "MotorController.h"
#include "RTClib.h"

BLEPeripheral                 ble     = BLEPeripheral();
BLEService                    service = BLEService("fff0");
BLEFixedLengthCharacteristic  led     = BLEFixedLengthCharacteristic("fff1", BLERead | BLEWrite, sizeof(LightConfig));
BLEFixedLengthCharacteristic  motor   = BLEFixedLengthCharacteristic("fff2", BLENotify | BLERead | BLEWrite, sizeof(MotorEnvelope));
BLEUnsignedLongCharacteristic clk     = BLEUnsignedLongCharacteristic("fff3", BLENotify | BLEWrite );
BLEFloatCharacteristic        tmp     = BLEFloatCharacteristic("fff4", BLENotify);

bool clkSub = false;
bool tmpSub = false;
static uint16_t seq[4] = {bit(15), bit(15), 0, 0};

RTC_DS3231 rtc;
DateTime now;

void setup() {

#ifdef DEBUG
  Serial.begin(115200);
#endif

  ble.setLocalName("PIGGY FEEDER");
  ble.setAdvertisedServiceUuid(service.uuid());
  ble.setAdvertisingInterval(200);

  ble.addAttribute(service);
  ble.addAttribute(motor);
  ble.addAttribute(led);
  ble.addAttribute(clk);
  ble.addAttribute(tmp);

  ble.setEventHandler(BLEConnected,    connectCallback);
  ble.setEventHandler(BLEDisconnected, disconnectCallback);

  led.setEventHandler(BLEWritten, ledCallback);

  motor.setEventHandler(BLEWritten,    motorWriteCallback);
  motor.setEventHandler(BLESubscribed, motorSubscribeCallback);

  clk.setEventHandler(BLESubscribed,   clkSubscribeCallback);
  clk.setEventHandler(BLEUnsubscribed, clkUnsubscribeCallback);
  clk.setEventHandler(BLEWritten,      clkWriteCallback);

  tmp.setEventHandler(BLESubscribed,   tmpSubscribeCallback);
  tmp.setEventHandler(BLEUnsubscribed, tmpUnsubscribeCallback);

  ble.begin();
  ble.setTxPower(16); // does this (16) really work? it returns true *shrugs*
  
  ChargeController.begin();
  LightController.begin();
  MotorController.begin();

  // BLE connection indicator light
  pinMode(BLE_CONNECTED_PIN, OUTPUT);
  digitalWrite(BLE_CONNECTED_PIN, LOW);
}

void loop() {
  static uint32_t lastMillis = millis();
  uint32_t curMillis = millis();

  ble.poll();
  ChargeController.think();
  LightController.think();
  MotorController.think();

  if (curMillis - lastMillis > 1000) {
    now = rtc.now();

    if (clkSub) {
      clk.setValue((uint32_t)now.unixtime());
    }

    if (tmpSub) {
      tmp.setValue((float)rtc.getTemperature());
    }

    lastMillis = curMillis;
  }
}

void ledCallback(BLECentral & central, BLECharacteristic & characteristic) {
    LightConfig * val = (LightConfig *)led.value();
    LightController.setConfig(val);
}


void motorWriteCallback(BLECentral & central, BLECharacteristic & characteristic) {
  static MotorEnvelope readEnvelope;
  MotorEnvelope * env = (MotorEnvelope *)motor.value();

  // depending on the fields for the received MotorEnvelope
  // perform different actions

  // RUN MOTOR IMMEDIATELY
  if (env->idx == 0xFF || 
      env->event.hour == 0xFF || 
      env->event.minute == 0xFF) {

#ifdef DEBUG    
    Serial.print(F("running motor for: "));
    Serial.print(env->event.duration);
    Serial.println(F(" seconds"));
#endif

    MotorController.runMotor(env->event.duration, env->event.power);
  }

  // SCHEDULE ACTIONS
  else if (env->idx < MAX_MOTOR_EVENTS) {

    // READ FROM SCHEDULE AT IDX
    if (!env->event.dow && 
        !env->event.hour && 
        !env->event.minute && 
        !env->event.duration && 
         env->event.power == 0xFF) {

      const MotorEvent * event = MotorController.readSchedule(env->idx);
      readEnvelope.idx = env->idx;
      memcpy(&(readEnvelope.event), (char *)event, sizeof(MotorEvent));
      characteristic.setValue((const unsigned char *)&readEnvelope, sizeof(readEnvelope));

#ifdef DEBUG
      Serial.print(F("retrieving event at index: "));
      Serial.println(env->idx);
      Serial.print(F("dow: "));
      Serial.println(readEnvelope.event.dow, HEX);
      Serial.print(F("hour: "));
      Serial.println(readEnvelope.event.hour, HEX);
      Serial.print(F("minute: "));
      Serial.println(readEnvelope.event.minute, HEX);
      Serial.print(F("duration: "));
      Serial.println(readEnvelope.event.duration, HEX);
      Serial.print(F("power: "));
      Serial.println(readEnvelope.event.power, HEX);
#endif

    }

    // WRITE TO SCHEDULE AT IDX
    else {
      MotorController.writeSchedule(env->idx, env->event);

#ifdef DEBUG
      Serial.print(F("saving event at index: "));
      Serial.println(env->idx);
      Serial.print(F("dow: "));
      Serial.println(env->event.dow, HEX);
      Serial.print(F("hour: "));
      Serial.println(env->event.hour, HEX);
      Serial.print(F("minute: "));
      Serial.println(env->event.minute, HEX);
      Serial.print(F("duration: "));
      Serial.println(env->event.duration, HEX);
      Serial.print(F("power: "));
      Serial.println(env->event.power, HEX);
#endif

    }
  }
}

void motorSubscribeCallback(BLECentral & central, BLECharacteristic & characteristic) {
  static MotorEnvelope env;

  // READ ENTIRE SCHEDULE ALL AT ONCE
  for (int i = 0; i < MAX_MOTOR_EVENTS; i++) {
    const MotorEvent * event = MotorController.readSchedule(i);

    if (!event)
      continue;
      
    env.idx = i;
    memcpy(&(env.event), (char *)event, sizeof(MotorEvent));
    characteristic.setValue((const unsigned char *)&env, sizeof(env));

#ifdef DEBUG    
    Serial.print(F("retrieving event at index: "));
    Serial.println(env.idx);
    Serial.print(F("dow: "));
    Serial.println(env.event.dow, HEX);
    Serial.print(F("hour: "));
    Serial.println(env.event.hour, HEX);
    Serial.print(F("minute: "));
    Serial.println(env.event.minute, HEX);
    Serial.print(F("duration: "));
    Serial.println(env.event.duration, HEX);
    Serial.print(F("power: "));
    Serial.println(env.event.power, HEX);
#endif

    // may need to tune this while building the mobile app
    delay(10);
  }
}

void clkSubscribeCallback(BLECentral & central, BLECharacteristic & characteristic) {
  clkSub = true;
}


void clkUnsubscribeCallback(BLECentral & central, BLECharacteristic & characteristic) {
  clkSub = false;
}

void clkWriteCallback(BLECentral & central, BLECharacteristic & characteristic) {
  uint32_t * ptr = (uint32_t *)characteristic.value();
  rtc.adjust(DateTime(*ptr));
}

void tmpSubscribeCallback(BLECentral & central, BLECharacteristic & characteristic) {
  tmpSub = true;
}

void tmpUnsubscribeCallback(BLECentral & central, BLECharacteristic & characteristic) {
  tmpSub = false;
}

void connectCallback(BLECentral & central) {
  led.setValue((char *)LightController.getConfig());
  digitalWrite(BLE_CONNECTED_PIN, HIGH);
}

void disconnectCallback(BLECentral & central) {
  digitalWrite(BLE_CONNECTED_PIN, LOW);
}

