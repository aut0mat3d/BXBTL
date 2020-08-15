#include <Arduino.h>

#include <driver/can.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"


#include <Defines.h>
#include <Globalvariables.h>
#include <Functions.h>
#include <CAN-Registers.h>
#include <CAN_Functions.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

void Canreceive()
{
    //CAN_frame_t rx_frame;
    //if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE)//Original Method of the used Example   
    //if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 1*portTICK_PERIOD_MS)==pdTRUE)
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, myportTICK_PERIOD_MS)==pdTRUE)
    //while (xQueueReceive(CAN_cfg.rx_queue,&rx_frame, myportTICK_PERIOD_MS)==pdTRUE) 
    {
      if(rx_frame.FIR.B.FF==CAN_frame_std)
      {
        //printf("New standard frame");
      }
      else
      {
        //printf("New extended frame");
      }

      if(rx_frame.FIR.B.RTR==CAN_RTR)
      {
        //printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      }
      else//Standard Frame Data
      {
        rxtime = millis();
        
        BXID = rx_frame.MsgID;
        BXlen = rx_frame.FIR.B.DLC;
        
        if ((BXlen != 0x02) && (BXlen != 0x04))//We only accept Frames with Length 2 and 4!
        {
          return;
        }

        bool gotData = false;
        if(Logmode == LOG_ALL)
        {
            printTimeBT(rxtime,false,false);//printTimeBT(unsigned long time, bool dhh, bool dmm)
            SerialBT.print(",");
            //Serial.print("MsgID:");
            printHEXBT(BXID,false,1); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
            //Serial.print(",Length:");
            printHEXBT(BXlen,false,1); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
            for(int i = 0; i < BXlen; i++)//No CSV as we must avoid to many Data over BT
            {
              printHEXBT(rx_frame.data.u8[i],false,0); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '  
            }
            SerialBT.println();
        }
        else //Werte je nach Gerät (soweit ich es implementiert habe) in die entsprechenden Variablen einlesen
        {
          BXreg = rx_frame.data.u8[1];//Das BionX-Register ist bei allen Frames dankenswerter Weise am selben Platz ;)
          
          if((BXID == ID_MOTOR) && (BXlen == 0x04))//Befehle an den Motor - sendet normalerweise nur die Konsole
          {
            switch(BXreg)
            {
              case REG_MOTOR_ASSIST_LEVEL:
              {
                int16_t val = rx_frame.data.u8[2];   
                val = val <<8;//Shift left
                val = val | rx_frame.data.u8[3];//Bitweises OR - val sollte nun den 16 bit Wert beinhalten
                MOTOR_ASSIST_LEVEL = val * FACTOR_MOTOR_ASSIST_LEVEL;//Der Wert in %
              }
              break;

              default:
              break;
            }


          }
          if((BXID == ID_CONSOLE) && (BXlen == 0x04))//Antworten Peripherie zur Konsole
          {
            switch(BXreg)
            {
              case REG_MOTOR_STATUS_SPEED:
              {
                uint8_t val = rx_frame.data.u8[3];
                Speedintegration = Speedintegration + val;
                Speedintegrationcnt++;
                if(Speedintegrationcnt >= Speedintegrationmax)
                {
                  float fval = Speedintegration/(float)Speedintegrationmax;
                  SPEED = fval * FACTOR_MOTOR_STATUS_SPEED;
                  Speedintegrationcnt = 0;
                  Speedintegration = 0;
                }
              }
              break;

              case REG_MOTOR_STATUS_POWER_METER://This also indicates the End of a std Cycle
              {
                //Auswertung ist hier üertrieben! an sich sollte nur das LSB bedient werden (nur positive Werte)
                int16_t val = rx_frame.data.u8[2];   
                val = val <<8;//Shift left
                val = val | rx_frame.data.u8[3];//Bitweises OR - val sollte nun den 16 bit Wert beinhalten
                MOTOR_STATUS_POWER_METER = val * FACTOR_MOTOR_STATUS_POWER_METER;//Der Wert in %
                gotData = true;
              }
              break;

              case REG_MOTOR_TORQUE_GAUGE_VALUE:
              {
                //Auswertung ist hier üertrieben! an sich sollte nur das LSB bedient werden (nur positive Werte)
                int16_t val = rx_frame.data.u8[2];   
                val = val <<8;//Shift left
                val = val | rx_frame.data.u8[3];//Bitweises OR - val sollte nun den 16 bit Wert beinhalten
                MOTOR_TORQUE_GAUGE_VALUE = val * FACTOR_MOTOR_TORQUE_GAUGE_VALUE;//Der Wert in %
              }
              break;
  
              case REG_MOTOR_STATUS_POWER_VOLTAGE_HI:
              {
                //Auswertung ist hier üertrieben! an sich sollte nur das LSB bedient werden (nur positive Werte)
                uint16_t val = 0;
                /*
                printHEXBT(rx_frame.data.u8[2],true,1); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
                printHEXBT(rx_frame.data.u8[3],false,0); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
                SerialBT.println("");
                */
                val = rx_frame.data.u8[3];   
                val = val <<8;//Shift left
                //SerialBT.println(val);
                //SerialBT.println(val,HEX);
                MOTOR_STATUS_POWER_VOLTAGE = val / (float)FACTOR_MOTOR_STATUS_POWER_VOLTAGE;//Der Wert in V
                //Serial.println(MOTOR_STATUS_POWER_VOLTAGE);
              }
              break;

              case REG_MOTOR_STATUS_TEMPERATURE:
              {
                MOTOR_STATUS_TEMPERATURE = rx_frame.data.u8[3]; 
              }
              break;

              /**
              --------------------------------------------------------------------------------------------------------------------
              Register 0x12       UNBEKANNT                                           Werte von 0x00 bis 0x04 aktuell in einem Log
              20020012
              080400120004
               */
              case 0x12:
              {
                int16_t val = rx_frame.data.u8[2];   
                val = val <<8;//Shift left
                val = val | rx_frame.data.u8[3];//Bitweises OR - val sollte nun den 16 bit Wert beinhalten
                MOTOR_UNKNOWN_0x12 = val;
              }
              break;

              case REG_BATTERY_STATUS_POWER_VOLTAGE_HI:
              {
                uint16_t val = 0;

                val = rx_frame.data.u8[3];   
                val = val <<8;//Shift left

                BATTERY_STATUS_POWER_VOLTAGE = val / (float)FACTOR_BATTERY_STATUS_POWER_VOLTAGE;//Der Wert in V
              }
              break;

              case REG_BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED:
              {
                uint8_t val = rx_frame.data.u8[3];
                BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED = ((float)val * FACTOR_BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED)+OFFSET_BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED;
              }
              break;

              case REG_BATTERY_STATUS_CHARGE_LEVEL:
              {
                BATTERY_STATUS_CHARGE_LEVEL = rx_frame.data.u8[3];
                BATTERY_STATUS_CHARGE_LEVEL_PERCENT = (float)BATTERY_STATUS_CHARGE_LEVEL * FACTOR_BATTERY_STATUS_CHARGE_LEVEL;
              }
              break;

              case REG_BATTERY_STATUS_ESTIMATED_SOC:
              {
                BATTERY_STATUS_ESTIMATED_SOC = rx_frame.data.u8[3]; 
              }
              break;


              default:
              break;
            }

          }
        }

        if(Logmode == LOG_TORQUE) //each ~100ms
        {
          if (gotData == true)
          {
            gotData = false;
            //return;
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

            SerialBT.print(SPEED);
            SerialBT.print(",");//Delimiter
            SerialBT.print(MOTOR_STATUS_TEMPERATURE);
            SerialBT.print(",");
            SerialBT.print(MOTOR_STATUS_POWER_VOLTAGE);
            SerialBT.print(",");
            SerialBT.print(MOTOR_TORQUE_GAUGE_VALUE);
            SerialBT.print(",");
            SerialBT.print(MOTOR_ASSIST_LEVEL);
            SerialBT.print(",");
            SerialBT.println(MOTOR_STATUS_POWER_METER);
            //Newline is our Terminator, so that could be parsed easily
          }

        }
        else if (Logmode == LOG_TORQUE_FILTERED)
        {
          if (gotData == true)
          {
            gotData = false;
            Logfiltercnt++;
            Torqueintegration = Torqueintegration + MOTOR_TORQUE_GAUGE_VALUE;
            AssistLevelintegration = AssistLevelintegration + MOTOR_ASSIST_LEVEL;
            PowerMeterintegration = PowerMeterintegration + MOTOR_STATUS_POWER_METER;

            if (Logfiltercnt > 39)//~4s
            {
              Logfiltercnt = 0;
              //return;
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
            SerialBT.print(SPEED);
            SerialBT.print(",");//Delimiter
            SerialBT.print(MOTOR_STATUS_TEMPERATURE);
            SerialBT.print(",");
            //SerialBT.print(MOTOR_STATUS_POWER_VOLTAGE);
            //SerialBT.print(",");
            SerialBT.print(Torqueintegration/40);
            SerialBT.print(",");
            SerialBT.print(AssistLevelintegration/40);
            SerialBT.print(",");
            SerialBT.print(PowerMeterintegration/40);
            SerialBT.print(",");
            SerialBT.print(MOTOR_UNKNOWN_0x12);
            SerialBT.print(",");
            SerialBT.print(BATTERY_STATUS_POWER_VOLTAGE);
            SerialBT.print(",");
            SerialBT.print(BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED);
            SerialBT.print(",");
            SerialBT.print(BATTERY_STATUS_CHARGE_LEVEL_PERCENT);
            SerialBT.print(",");
            SerialBT.print(BATTERY_STATUS_CHARGE_LEVEL);
            SerialBT.print(",");
            SerialBT.println(BATTERY_STATUS_ESTIMATED_SOC);



            Torqueintegration = MOTOR_TORQUE_GAUGE_VALUE;
            AssistLevelintegration = MOTOR_ASSIST_LEVEL;
            PowerMeterintegration = MOTOR_STATUS_POWER_METER;

            }
          }

        }
        
      }
    }

    /*
    else //Left for Documentation if we want to send via CAN - completely untested
    {
      
      rx_frame.FIR.B.FF = CAN_frame_std;
      rx_frame.MsgID = 1;
      rx_frame.FIR.B.DLC = 8;
      rx_frame.data.u8[0] = 'h';
      rx_frame.data.u8[1] = 'e';
      rx_frame.data.u8[2] = 'l';
      rx_frame.data.u8[3] = 'l';
      rx_frame.data.u8[4] = 'o';
      rx_frame.data.u8[5] = 'c';
      rx_frame.data.u8[6] = 'a';
      rx_frame.data.u8[7] = 'n';
        
      
      //ESP32Can.CANWriteFrame(&rx_frame);
    }
    */
    

}


