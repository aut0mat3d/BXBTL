#ifndef CANFunctions_h
#define CANFunctions_h

//Dont know if the Linker is intelligent enough to do not calc every time we use portTICK_PERIOD_MS
//#define portTICK_PERIOD_MS			( ( TickType_t ) 1000 / configTICK_RATE_HZ )
//configTICK_RATE_HZ is 1000 here
//So... we have 1ms here - no need to calc every time CAN_cfg.rx_queue is called ;)
#define myportTICK_PERIOD_MS			( ( TickType_t ) 1 )//1ms to stay in xQueueReceive


#define WHEEL_CIRCUMSTANCE 2200
#define FACTOR_MOTOR_STATUS_SPEED 1.200012 // (WHEEL_CIRCUMSTANCE/1000)*0,54546
#define FACTOR_MOTOR_ASSIST_LEVEL 1.5625
#define FACTOR_MOTOR_TORQUE_GAUGE_VALUE 1.5625
#define FACTOR_MOTOR_STATUS_POWER_METER 1.5625
#define FACTOR_MOTOR_STATUS_POWER_VOLTAGE 1000
#define FACTOR_BATTERY_STATUS_POWER_VOLTAGE 1000
#define FACTOR_BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED 0.416667
#define OFFSET_BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED 20.8333
#define FACTOR_BATTERY_STATUS_CHARGE_LEVEL 6.6667



/*
ID Motor: 0x20
--------------------------------------------------------------------------------------------------------------------
Register 0x09
200400090040        REG_MOTOR_ASSIST_LEVEL                              [unit:%, range:-100..100, factor:1.5625] !!! signed !!!
Register wird nur beschrieben!
Besonderheit: wird immer 2x in Folge geschrieben!
Beispiele:
200400090039    0x0039 = DEZ 57 * 1,5625 = 89,0625%
20040009FFD8    0xFFD8 = DEZ -40 * 1,5625 = -62,5%
--------------------------------------------------------------------------------------------------------------------
Register 0x11
20020011            REG_MOTOR_STATUS_SPEED                              [unit:rpm, factor:9.091]
08040011000A        0x0A = DEZ 10 * 9,091 = 90,91 rpm
Berechnung km/h:    90,91 * 2,2 (Radumfang in m) = 200,02 m/min oder 12,00012 km/h

Oder.. 
Radumfang: 2200mm
bei n = 1 upm:   1* (2200/1000000)*60 = 0,132 km/h per upm

oder: Registerwert * 1,200012
Faktor wäre dann Radumfang(mm) * 0,00054546 Berechnung (mit max Genauigkeit): (Radumfang/1000) * 0,54546
--------------------------------------------------------------------------------------------------------------------
Register 0x14       REG_MOTOR_STATUS_POWER_METER                        [unit:%, factor:1.5625]
20020014
080400140040        0x40 = DEZ 64 * 1,5625 = 100%   Hinweis:    Es gibt keine negativen Werte!!!!!
                                                                Bei der Anzeige sollte also in Betracht gezogen werden ob rekukperiert wird!!
--------------------------------------------------------------------------------------------------------------------
Register 0x21       
20020021            REG_MOTOR_TORQUE_GAUGE_VALUE                        [unit:%, range:0?..100, factor:1.5625]
080400210009        0x09 = DEZ 9 * 1,5625 = 14,0625%
08040021002E        0x2E = DEZ 46 * 1,5625  = 100%
--------------------------------------------------------------------------------------------------------------------
Register 0x70       REG_MOTOR_STATUS_POWER_VOLTAGE_HI                   [unit:V, factor:0.001] oder div1000
20020070
08040070009B        0x9B00 = DEZ 39680 * 0,001 = 39,68V
                    Hinweis: Die Console fragt nur das HIGH Register ab - warum auch immer
--------------------------------------------------------------------------------------------------------------------
Register 0x16       REG_MOTOR_STATUS_TEMPERATURE                        [unit:C]
20020016
080400160024        0x24 = DEZ 36 = 36°C
--------------------------------------------------------------------------------------------------------------------
Register 0x12       UNBEKANNT                                           Werte von 0x00 bis 0x04 aktuell in einem Log
20020012
080400120004
--------------------------------------------------------------------------------------------------------------------


--------------------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
--------------------------------------------------------------------------------------------------------------------
ID Battery: 0x10
--------------------------------------------------------------------------------------------------------------------
Register 0xAA       REG_BATTERY_STATUS_POWER_VOLTAGE_HI                 [unit:V, factor:0.001]
100200AA
080400AA009F        0x9F00 = DEZ 39424 /1000 = 39,424V                 
--------------------------------------------------------------------------------------------------------------------
Register 0x33       REG_BATTERY_STATISTIC_BATTERY_AVGVOLTAGE_NORMALIZED [unit:%, factor:0.416667, offset:20.8333]
                    Average battery voltage read during 50s based on battery.status.vBatt, 
                    in percentage of its nominal voltage    -hmm? Wird interessant im Fahrbetrieb - Mitteln über 
                    einen längeren Zeitraum macht ja Sinn, vor Allem wo wir auch rekuperieren und wechselnde Lasten
                    haben. Hier nimmt uns der Akku also einiges an Berechnung ab - dieses Register könnte das Initial
                    für die Leistungsreduzierung sein wenn Trip to Empty = 0
10020033
0804003300C0        0xC0 = DEZ  192 * 0,416667 + 20,8333 = 100,833364%                    
--------------------------------------------------------------------------------------------------------------------
Register 0x61       REG_BATTERY_STATUS_CHARGE_LEVEL                     [unit:%, factor:6.6667]
10020061                                                                Wir haben hier also 0-15
08040061000E        0x0E = DEZ 14 * 6,6667 = 93,3338%         
                    Wir haben hier also 0-15 = 16 Stufen für eine Kapazitätsanzeige ;)
--------------------------------------------------------------------------------------------------------------------
Register 0x30       REG_BATTERY_STATUS_ESTIMATED_SOC                    [unit:%]
                    Return an estimated value of SOC based on battery voltage. Only works with LiIon battery
10020030
080400300028        0x28 = DEZ 40%    ?? Ladestand Akku war beim Log auf 93%?? Berechnung SOC in 1h?? oder was auch immer
--------------------------------------------------------------------------------------------------------------------
!!!!!!!!!!!! FOLGENDE REGISTER NOCH NICHT EINGEBUNDEN!!!!!
--------------------------------------------------------------------------------------------------------------------
Zitat temp zu 16bit Registern die über doppelte Abfrage von ein und demselben Register erfolgen: 
Ähnlich wird die 0xff bei Gauge-Voltage sein. Da das ein Wert ist der über 2 getrennte CAN-Nachrichten kommt 
muss man hier aber probieren, da das nirgends beschrieben steht. 
Ein Programmierer der solche volatile Werte in 2 Häppchen übertragen will, muss die Reihenfolge festlegen. 
Oder, beim ersten Request wird der Wert zwischengespeichert und die Hälfte übertragen, 
der 2. Request liefert den Rest aus dem Zwischenspeicher. 
Damit ist es nicht egal ob zuerst das High- oder Low-Byte abgeholt wird. 
Wie es richtig ist kann ich dir jetzt nicht sagen, aber im Zweifel wenn unstimmige Werte kommen sollte man mal Probieren zu drehen. 
Kann gut sein, dass du bei deinen Versuchen in Ruhestellung ein Pendeln um 0 beobachtest.
*/
//extern void onReceive();
//extern void setup_can_driver();



extern void Canreceive();

#endif