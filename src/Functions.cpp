#include <Arduino.h>
#include <Defines.h>
#include <Globalvariables.h>
#include <CAN-Registers.h>
#include <Functions.h>
#include <CAN_Functions.h>



BluetoothSerial SerialBT;

void printTimeBT(unsigned long time, bool dhh, bool dmm)
{
  unsigned int ms = time %1000;
  if (dhh == true)
  {
    if (_RThh < 10)
    {
        SerialBT.print(0);
    }
    SerialBT.print(_RThh);
    SerialBT.print(":");
  }
  if (dmm == true)
  {
    if (_RTmm < 10)
    {
        SerialBT.print(0);
    }
    SerialBT.print(_RTmm);
    SerialBT.print(":");
  }
  if (_RTss < 10)
  {
      SerialBT.print(0);
  }
  SerialBT.print(_RTss);
  SerialBT.print(".");
  if(ms < 100)
  {
    SerialBT.print(0);
  }
  if(ms < 10)
  {
    SerialBT.print(0);
  }
  SerialBT.print(ms);
  
  
}

void printHEX(uint8_t n, bool PAen, uint8_t SEP) //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
{

    if (PAen == true)
    {
      Serial.print("0x");
    }
    if (n<0x10)
    {
      Serial.print("0");
    }
    Serial.print(n,HEX);
    if(SEP == 1)
    {
      Serial.print(",");
    }
    else if(SEP == 2)
    {
      Serial.print(" ");
    }
    //Serial.print("");

}

void printHEXBT(uint8_t n, bool PAen, uint8_t SEP) //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
{
    if (PAen == true)
    {
      SerialBT.print("0x");
    }
    if (n<0x10)
    {
      SerialBT.print("0");
    }
    SerialBT.print(n,HEX);
    if(SEP == 1)
    {
      SerialBT.print(",");
    }
    else if(SEP == 2)
    {
      SerialBT.print(" ");
    }
    //Serial.print("");
    
}



/******************** Serial Handling  **************************/


bool PrintHelpBT()
{
  SerialBT.println ( F("--BXBTL--") );
  SerialBT.println ( F("(c) Automated 2020") );
  //SerialBT.println ( F("D: Drive for Standard Operation")); 
  //SerialBT.println ( F("-Control your Bike via App") );
  SerialBT.println ( F("") );
  SerialBT.println ( F("L[m][t]: Log[mode][time(s*5)]") );
  SerialBT.println ( F("[m]odes:") );
  SerialBT.println ( F("A: All,") );
  SerialBT.println ( F("t: Torque specific(filtered),") );
  SerialBT.println ( F("T: Torque specific(unfiltered),") );
  SerialBT.println ( F("X: Exit ") );
  SerialBT.println ( F("") );
  
  return true;
}

bool handleserialBT()
{
  bool res = false;
  SerialBTMillis = millis();
  char inchar = SerialBT.read();
  SerialBTpntr ++;
  /*
  Serial.print(SerialBTpntr);
  Serial.print(", IC:");
  Serial.print(inchar);
  Serial.print(",");
  Serial.println(inchar,HEX);
  */
  if ((inchar == 0x0D) || (inchar == 0x0A))   // 0x0D = '\r' carriage return), 0x0A = Linefeed
  {
    SerialBTpntr = 0;
    return true;
  }

  if (SerialBTpntr == 1)//Handle first received Char (Mode Command)
  {
    switch(inchar)
    {
      case '?':
      Logmode = LOG_DISABLED;
      res =PrintHelpBT();
      break;

      case 'L'://Logging Mode
      {
        Logmode = LOG_INIT;
      }
      break;

      default:
      SerialBT.println("Unknown Command - ? for Help");
      break;
    }
  }

  if (Logmode > LOG_DISABLED)
  {
    if (SerialBTpntr == 2)//Handle second received Char (Mode Command)
    {
      switch(inchar)
      {
        case 'A':
        Logmode = LOG_ALL;
        break;

        case 't':
        Logmode = LOG_TORQUE_FILTERED;
        break;

        case 'T':
        Logmode = LOG_TORQUE;
        break;

        case 'X':
        default:
        SerialBT.println("Logging DISABLED");
        Logmode = LOG_DISABLED;
        break;
      }
    }
    else if(SerialBTpntr == 3)//Handle third received Char (Log Time)
    {
      if ((inchar > 0x29) && (inchar < 0x40) && (Logmode > LOG_DISABLED)) //Check if the received Value is a ASCII numerical
      {
        uint8_t Logtime = inchar - 0x30;
        SerialBT.print("Logging ");
        switch (Logmode)
        {
          case LOG_ALL:
          SerialBT.print("ALL");
          break;
          case LOG_TORQUE_FILTERED:
          SerialBT.print("TORQUE_FILTERED");
          break;
          case LOG_TORQUE:
          SerialBT.print("TORQUE");
          break;
        }
        SerialBT.print(" for ");
        if(Logtime > 0)
        {   
          Logtime = Logtime*5;
          SerialBT.print(Logtime);
          SerialBT.println("s");
          Loginterval = 1000*Logtime;
          Loginterval = Loginterval + millis();
        }
        else
        {
          Loginterval = 0;
          SerialBT.print("ever");
        }
        SerialBT.println("");

        if (Logmode == LOG_TORQUE_FILTERED)//print cvs header
        {    
        /**
             * S: float SPEED = 0.0;
             * t: uint8_t MOTOR_STATUS_TEMPERATURE = 0;
             * G: int TORQUE_GAUGE_VALUE integrated = 0; //0 to 100%
             * A: int MOTOR_ASSIST_LEVEL integrated = 0; //-100% to 100%
             * P: int MOTOR_STATUS_POWER_METER integrated = 0; //0 to 100%
             * U: int MOTOR_UNKNOWN_0x12 
             * BV:float BATTERY_STATUS_POWER_VOLTAGE
             * BA:uint8_t BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED = 0; //0-100%
             * BP:uint8_t BATTERY_STATUS_CHARGE_LEVEL_PERCENT = 0; //0-100%
             * BS:uint8_t BATTERY_STATUS_CHARGE_LEVEL = 0; //0-15
             * BE:uint8_t BATTERY_STATUS_ESTIMATED_SOC = 0; //0-100%
             * 
             * 
             * Output Order: S,t,G,A,P,U,BV,BA,BP,BS,BE
             */
          SerialBT.println(".csv");
          SerialBT.print("SPEED");
          SerialBT.print(",");
          SerialBT.print("Temp °C");
          SerialBT.print(",");
          SerialBT.print("GaugeI %");
          SerialBT.print(",");
          SerialBT.print("AssistI %");
          SerialBT.print(",");
          SerialBT.print("PowermI %");
          SerialBT.print(",");
          SerialBT.print("U:0x12");
          SerialBT.print(",");
          SerialBT.print("Batt Voltage V");
          SerialBT.print(",");
          SerialBT.print("Batt AVGV_Norm %");
          SerialBT.print(",");
          SerialBT.print("Batt CHGE %");
          SerialBT.print(",");
          SerialBT.print("Batt CHGE");
          SerialBT.print(",");
          SerialBT.println("Batt Est SOC %");
        }
        else if (Logmode == LOG_TORQUE)//print cvs header
        {
            /**
             * S: float SPEED = 0.0;
             * t: uint8_t MOTOR_STATUS_TEMPERATURE = 0;
             * V: float MOTOR_STATUS_POWER_VOLTAGE = 0.0;
             * G: int MOTOR_TORQUE_GAUGE_VALUE = 0; //0 to 100%
             * A: int MOTOR_ASSIST_LEVEL = 0.0; //-100% to 100%
             * P: int MOTOR_STATUS_POWER_METER = 0; //0 to 100%
             * 
             * Output Order: S,t,V,G,A,P
             */
          SerialBT.println(".csv");
          SerialBT.print("SPEED");
          SerialBT.print(",");
          SerialBT.print("Temp °C");
          SerialBT.print(",");
          SerialBT.print("POWER Voltage V");
          SerialBT.print(",");
          SerialBT.print("Gauge %");
          SerialBT.print(",");
          SerialBT.print("Assist %");
          SerialBT.print(",");
          SerialBT.println("Powerm %");
      
        }


      }
      else
      {
        SerialBT.println("Logging DISABLED");
        Logmode = LOG_DISABLED;
      }
      //SerialBT.flush();
      SerialBTpntr = 0;
      
    }
  
  }

  return res;
}