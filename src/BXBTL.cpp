/**
{ Copyright (C) 2020 Rainer Oberegger                                    }
{     https://github.com/aut0mat3d                                       }
{                                                                        }
{ This program is free software; you can redistribute it and/or modify   }
{ it under the terms of the GNU General Public License as published by   }
{ the Free Software Foundation; either version 2 of the License, or      }
{ (at your option) any later version.                                    }
{                                                                        }
{ This program is distributed in the hope that it will be useful,        }
{ but WITHOUT ANY WARRANTY; without even the implied warranty of         }
{ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          }
{ GNU General Public License for more details.                           }
{                                                                        }
{ You should have received a copy of the GNU General Public License      }
{ along with this program; if not, write to the Free Software            }
{ Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              }
{                                                                        }
{ CAN-Registers.h is ported from BionX.pas v0.90 (WinGui Bikeinfo)       }
{ published by Thorsten Schmidt (tschmidt@ts-soft.de)                    } 
*/



#include <Arduino.h>

#include <ESP32CAN.h>
#include <CAN_config.h>

#include <SPI.h>

#include <Defines.h>
#include <Globalvariables.h>
#include <Functions.h>

#include "BluetoothSerial.h"
#include <CAN_Functions.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif



CAN_device_t CAN_cfg;               // CAN Config
const int rx_queue_size = 32;       // Receive Queue size


/******************** Setup Routine **************************/
void setup()
{

  // init serial port
  Serial.begin(115200);
  //Serial.begin(460800);
   
  SerialBT.begin("BXBTL"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  

  CAN_cfg.speed=CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_22;
  CAN_cfg.rx_pin_id = GPIO_NUM_21;
  //CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  //start CAN Module
  ESP32Can.CANInit();

}



/******************** main program loop **************************/
void loop()
{
  currentMillis = millis();
  if ((currentMillis - RTlastMillis) > 1000)//Simple building a Runtime in hh:mm:ss format
  {
    RTlastMillis = currentMillis;
    _RTss++;
    if(_RTss > 59)
    {
      _RTss = 0;
      _RTmm ++;
    }
    if(_RTmm > 59)
    {
      _RTmm = 0;
      _RThh ++;
    }
/*
    uint16_t val = 0; //Leaving that for Documentation - needed that to catch a Problem with uint16_t

    val = 0xAB;   
    val = val <<8;//Shift left
    Serial.println(val);
    Serial.println(val,HEX);
    */
  }
  Canreceive();//Receive CAN Packets and parse them
  /*
  if (SerialBT.connected()) {
    //dosomething
  }
  else
  {
    //dosomething
  }
  */
  
  if (SerialBT.hasClient())
  {
       while (SerialBT.available())//Data in FIFO
       {
         //char val = SerialBT.read();
         //Serial.print(val,HEX);
         bool retval = handleserialBT(); 
       }
       //Serial.println("");
       //SerialBT.flush();

    if (btconnected == false)
    {
      btconnected = true;
      Serial.println("BT Client connected");
    }
  }
  else//No BT Client
  {
    if (btconnected == true)
    {
      btconnected = false;
      SerialBT.flush();
      //Serial.println("BT Client disconnected!");
    }
  }
  uint32_t gap = currentMillis - SerialBTMillis;//Reset Pointer for Serial Input and flush Buffer (if any)
  if ((gap > 1000) && (SerialBTpntr > 0))
  {
    Serial.println("Timeout");
    SerialBTpntr = 0;
    //SerialBT.flush();
    while (SerialBT.available())
       {
         SerialBT.read();
       }
  }

  if((Logmode > LOG_DISABLED) && (Loginterval > 0))
  {
    if (currentMillis > Loginterval)
    {
      Logmode = LOG_DISABLED;
      Loginterval = 0;
      SerialBT.println("Logging DISABLED");
    }

  }


  
}

/*********************************************************************************************************
*********************************************************************************************************/
