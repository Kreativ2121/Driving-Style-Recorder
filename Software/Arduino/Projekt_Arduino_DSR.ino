#include <Arduino.h>

//Can Shield Libraries:
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

//Micro SD Card Saving Library:
#include <SD.h>
#include <SPI.h>

//GPS Libraries:
#include <SoftwareSerial.h>
#include "src/L76X/DEV_Config.h"
#include "src/L76X/L76X.h"
GNRMC GPS1;

//Czesc komunikacji leci po 9600 a czesc po 115200 baud, sprawdzic czemu
void setup() {
  //GPS Setup:
  DEV_Delay_ms(1000);
  Serial.println("Setting serial speed to 115200.");
  Serial.begin(115200);

  Serial.println("Setting GPS serial speed to 115200.");
  DEV_Set_Baudrate(115200);
  
  Serial.println("Setting NMEA output frequencies.");
  L76X_Send_Command(SET_NMEA_OUTPUT);  
  Serial.println("Setting GPS serial speed to 115200.");
  L76X_Send_Command(SET_NMEA_BAUDRATE_115200); 
  Serial.println("Setting message interval to 1 second.");
  L76X_Send_Command(SET_POS_FIX_1S);
  
  DEV_Delay_ms(500);
  Serial.println("GPS startup done correctly.");

  //TODO
  //CAN-Bus Read Setup:
  //  Serial.begin(9600); // For debug use
  //Serial.println("CAN Read - Testing receival of CAN Bus message.");  
  //delay(1000);

  //if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
  //  Serial.println("CAN init ok.");
  //else
  //  Serial.println("Can't init CAN");
  //  
  //delay(1000);

  Serial.println("Beginning transmission.");
}

void loop() {

//GPS Loop
  GPS1 = L76X_Gat_GNRMC();
  Serial.print("\r\n");
  Serial.print("Time:");
  Serial.print(GPS1.Time_H);
  Serial.print(":");
  Serial.print(GPS1.Time_M); 
  Serial.print(":");
  Serial.print(GPS1.Time_S);
  Serial.print("\r\n");

  //CAN-Bus loop:
  //tCAN message;
  //
  //if (mcp2515_check_message()) 
  //  {
  //    if (mcp2515_get_message(&message)) 
  //  {
  //        //if(message.id == 0x620 and message.data[2] == 0xFF)  //uncomment when you want to filter
  //             //{
  //               
  //               Serial.print("ID: ");
  //               Serial.print(message.id,HEX);
  //               Serial.print(", ");
  //               Serial.print("Data: ");
  //               Serial.print(message.header.length,DEC);
  //               for(int i=0;i<message.header.length;i++) 
  //                { 
  //                  Serial.print(message.data[i],HEX);
  //                  Serial.print(" ");
  //                }
  //               Serial.println("");
  //             //}
  //           }}

}
