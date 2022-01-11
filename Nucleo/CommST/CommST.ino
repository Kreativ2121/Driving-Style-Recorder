#include <Wire.h>
#include <SoftwareSerial.h>
#include "DEV_Config.h"
#include "L76X.h"
#include <SPI.h>
#include <SD.h>
GNRMC GPS1;

String LatDmmToDd(double latitude)
{
  String lat;
  String latDec;
  int deg;
  float tempR;
  String temp1, temp2;
  float temp3, temp4;
  
  latDec = String(latitude-(int)latitude,6);
  latDec.remove(0,2);

  deg = (int)latitude;
  temp1 = latDec.substring(0,2);
  temp3 = temp1.toInt();
  temp2 = latDec.substring(2);
  temp4 = temp2.toInt();
  temp4 = temp4/10000.0;
  
  tempR = temp3 + temp4;
  tempR = tempR/60.0;
  lat = String(deg + tempR,6);

  //Serial.println(lat);
  return lat;
}

String LonDmmToDd(double longitude)
{
  String lon;
  String lonDec;
  int deg;
  float tempR;
  String temp1, temp2;
  float temp3, temp4;

  lonDec=String(longitude-(int)longitude,6);
  lonDec.remove(0,2);
  
  deg = (int)longitude;
  temp1 = lonDec.substring(0,2);
  temp3 = temp1.toInt();
  temp2 = lonDec.substring(2);
  temp4 = temp2.toInt();
  temp4 = temp4/10000.0;
  
  tempR = temp3 + temp4;
  tempR = tempR/60.0;
  lon = String(deg + tempR,6);

  //Serial.println(lon);
  return lon;
}

void setup()
{
  delay(1000);
  Serial.begin(9600);
  Serial.println();
  Serial.print(F("-------------------------------------\n"));
  Serial.print(F("Driving Style Recorder (DSR) - Nucleo F303RE Firmware\n"));
  Serial.print(F("Made by Dominik Ciesiolkiewicz (2022)\n"));
  Serial.print(F("Version 0.2.1\n"));
  Serial.print(F("-------------------------------------\n"));
  Serial.print(F("Beginning system startup.\n"));

  pinMode(PA5,OUTPUT);

  Serial.print(F("Beginning GPS initialization.\n"));
  DEV_Set_Baudrate(9600);
  L76X_Send_Command(SET_NMEA_OUTPUT);
  L76X_Send_Command(SET_NMEA_BAUDRATE_9600);
  L76X_Send_Command(SET_POS_FIX_1S);
  L76X_Send_Command(SET_SYNC_PPS_NMEA_OFF);
  L76X_Send_Command(SET_PERIODIC_MODE);
  DEV_Delay_ms(500);
  DEV_Set_Baudrate(9600);
  DEV_Delay_ms(500);
  Serial.print(F("GPS initialization finished.\n"));

  Serial.print(F("Fetching first batch of GPS data...\n"));
  GPS1 = L76X_Gat_GNRMC();
  Serial.print(F("First GPS data fetch complete.\n"));

  Serial.print(F("Initializing I2C bus as slave.\n"));
  Wire.begin(0x02);
  Wire.onRequest(sendMessage);
  Serial.print(F("I2C bus initialization complete.\n"));

  Serial.print(F("System startup done.\n"));
  Serial.print(F("-------------------------------------\n"));
}

void loop()
{
  //Gather data from GPS
  GPS1 = L76X_Gat_GNRMC();
}

void sendMessage()
{
  Serial.println("Request received!");
  
  //Print data to serial
  String Status;
  String Lat;
  String Lon;
  Status.reserve(1);
  Lat.reserve(10);
  Lon.reserve(11);
  Status = String(GPS1.Status);
  Lat = LatDmmToDd(GPS1.Lat);
  Lon = LonDmmToDd(GPS1.Lon);
  
  //Making sure that latitude is exactly 10 bytes long for sending it through I2C
  while(strlen(Lat.c_str())<10)
  {
    if(Lat[0]=='-')
    {
      Lat.remove(1,0);
      Lat = '-' + '0' + Lat;
    }
    else
    {
      Lat = '0' + Lat;
    }
  }

  //Making sure that longitude is exactly 11 bytes long for sending it through I2C
  while(strlen(Lon.c_str())<11)
  {
    if(Lon[0]=='-')
    {
      Lon.remove(1,0);
      Lon = '-' + '0' + Lon;
    }
    else
    {
      Lon = '0' + Lon;
    }
  }

  //Turn on the bulit-in LED to show the user that data is being sent
  digitalWrite(PA5, HIGH);

  //Send data through I2C
  Wire.write(Status.c_str());
  Wire.write(",");
  Wire.write(Lat.c_str());
  Wire.write(",");
  Wire.write(Lon.c_str());

  //Confirm sent data to Serial (DEBUG)
  Serial.print(F("Data sent via I2C to master: "));
  Serial.print(Status);
  Serial.print(",");
  Serial.print(Lat.c_str());
  Serial.print(",");
  Serial.print(Lon.c_str());
  Serial.print("\n");

  //Turn off LED after some time
  delay(200);
  digitalWrite(PA5, LOW);


  //Wire.write("1,53.42399799263681,14.53600240436017700");
}
