#ifndef globalvariables_h
#define globalvariables_h
#include <ESP32CAN.h>
#include <CAN_config.h>


extern const char XLBPreamble[];

extern uint32_t currentMillis;
extern uint32_t RTlastMillis;
extern uint8_t _RThh;
extern uint8_t _RTmm;
extern uint8_t _RTss;

extern uint32_t BXID;
extern uint8_t BXlen;
extern uint8_t BXreg;
extern CAN_frame_t rx_frame;
extern unsigned long rxtime;

extern volatile int Speedintegration;
extern uint8_t Speedintegrationcnt;

extern uint8_t Logfiltercnt;
extern volatile int Torqueintegration;
extern uint8_t Torqueintegrationcnt;
extern volatile int AssistLevelintegration;
extern volatile int PowerMeterintegration;

extern volatile int MOTOR_ASSIST_LEVEL; //-100% to 100%
extern volatile int MOTOR_STATUS_POWER_METER; //0 to 100%
extern volatile int MOTOR_TORQUE_GAUGE_VALUE; //0 to 100%
extern volatile float MOTOR_STATUS_POWER_VOLTAGE;
extern uint8_t MOTOR_STATUS_TEMPERATURE;
extern volatile float SPEED;
extern volatile int MOTOR_UNKNOWN_0x12;
extern volatile float BATTERY_STATUS_POWER_VOLTAGE;
extern uint8_t BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED; //0-100%
extern uint8_t BATTERY_STATUS_CHARGE_LEVEL_PERCENT; //0-100%
extern uint8_t BATTERY_STATUS_CHARGE_LEVEL; //0-15
extern uint8_t BATTERY_STATUS_ESTIMATED_SOC; //0-100%


struct XLBCANMsg
{
  INT32U Id;
  INT8U Len;
  INT8U Data[8];
};
extern XLBCANMsg Id; 
extern XLBCANMsg Len; 
extern XLBCANMsg Data[]; 



extern bool errorprinted;

extern unsigned long lastreceive;
extern bool btconnected;

extern uint8_t SerialBTpntr;

extern uint32_t SerialBTMillis;
extern uint32_t previousMillis;
extern uint32_t interval;

extern uint8_t Logmode;
extern uint32_t Loginterval;

#endif