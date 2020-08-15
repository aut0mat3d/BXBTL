#include <Arduino.h>
#include <Defines.h>
#include <Globalvariables.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

const char XLBPreamble[] = "CM";

uint32_t currentMillis = 0;
uint32_t RTlastMillis = 0;
uint8_t _RThh = 0;
uint8_t _RTmm = 0;
uint8_t _RTss = 0;

uint32_t BXID = 0;
uint8_t BXlen = 0;
uint8_t BXreg = 0x00;
CAN_frame_t rx_frame;
unsigned long rxtime = 0;


volatile int Speedintegration = 0;
uint8_t Speedintegrationcnt = 0;

uint8_t Logfiltercnt = 0;
volatile int Torqueintegration = 0;
uint8_t Torqueintegrationcnt = 0;
volatile int AssistLevelintegration = 0;
volatile int PowerMeterintegration = 0;

volatile int MOTOR_ASSIST_LEVEL = 0.0; //-100% to 100%
volatile int MOTOR_STATUS_POWER_METER = 0; //0 to 100%
volatile int MOTOR_TORQUE_GAUGE_VALUE = 0; //0 to 100%
volatile float MOTOR_STATUS_POWER_VOLTAGE = 0.0;
uint8_t MOTOR_STATUS_TEMPERATURE = 0;
volatile float SPEED = 0.0;
volatile int MOTOR_UNKNOWN_0x12 = 0;
volatile float BATTERY_STATUS_POWER_VOLTAGE = 0.0;
uint8_t BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED = 0; //0-100%
uint8_t BATTERY_STATUS_CHARGE_LEVEL_PERCENT = 0; //0-100%
uint8_t BATTERY_STATUS_CHARGE_LEVEL = 0; //0-15
uint8_t BATTERY_STATUS_ESTIMATED_SOC = 0; //0-100%



bool errorprinted = false;
unsigned long lastreceive = 0;

bool btconnected = false;

uint8_t SerialBTpntr = 0;

uint32_t SerialBTMillis = 0;
uint32_t previousMillis = 0;
uint32_t interval = 1000;

uint8_t Logmode = 0;
uint32_t Loginterval = 0;