#include <Wire.h>
#include <SD.h>
#include "Canbus.h"
#include "defaults.h"
#include "global.h"
#include "mcp2515.h"
#include "mcp2515_defs.h"

#define BYTES 24 //Bytes expected to be received from I2C

int counter; // counter used in some loop functions
byte buff[BYTES]; //buffer for receiving data from I2C
const int chipSelect = 9; //Chip Select pin is tied to pin 9 on the SparkFun CAN-Bus Shield
tCAN message; //CAN message holder
int interval = 1000; //time interval for CANbus data requests
int speed; //newest speed received from CANbus
int rpm; //newest rpm received from CANbus
unsigned long previousMillis = 0; //variable used in millis() timer
//char buffer[456];  //Data will be temporarily stored to this buffer before being written to the file (CAN related)

void eraseBuffer()
{
  //Erasing the I2C buffer
  for(int i=0;i<BYTES;i++){
    buff[i]='\0';
  }
}

void printFromBuffer()
{
  //Printing data from I2C buffer to serial
  for(int i=0;i<BYTES;i++){
    Serial.print((char)buff[i]);
  }
}

void canErrorBlink(int blinkTime)
{
  eraseBuffer();

  //Five short blinks - CAN Bus initialisation error
  for(;;)
  {
    digitalWrite(7, HIGH);
    delay(blinkTime);
    digitalWrite(7, LOW);
    delay(blinkTime);
    digitalWrite(7, HIGH);
    delay(blinkTime);
    digitalWrite(7, LOW);
    delay(blinkTime);
    digitalWrite(7, HIGH);
    delay(blinkTime);
    digitalWrite(7, LOW);
    delay(blinkTime);
    digitalWrite(7, HIGH);
    delay(blinkTime);
    digitalWrite(7, LOW);
    delay(blinkTime);
    digitalWrite(7, HIGH);
    delay(blinkTime);
    digitalWrite(7, LOW);
    Serial.print("\n");
    delay(4000);
  }
  
}

void errorBlink(int blinkTime)
{
  eraseBuffer();

  //Three short blinks - I2C Error
  //Three long blinks - SD Error
  digitalWrite(7, HIGH);
  delay(blinkTime);
  digitalWrite(7, LOW);
  delay(blinkTime);
  digitalWrite(7, HIGH);
  delay(blinkTime);
  digitalWrite(7, LOW);
  delay(blinkTime);
  digitalWrite(7, HIGH);
  delay(blinkTime);
  digitalWrite(7, LOW);
  Serial.print("\n");
  delay(4000);
}

void checkAnswers(){
  if (mcp2515_check_message()) 
  {
    if (mcp2515_get_message(&message)) 
    {
      if(message.id == 0x7E8)  //uncomment when you want to filter
      {
        if(message.data[2] == 0x0C)
        {
          Serial.print(F("RPM"));
          rpm = ((256 * message.data[3]) + message.data[4])/4;
          Serial.print(rpm);
          Serial.print(F(" rpm"));
        }
        else
        {
          Serial.print(F("SPEED"));
          speed = message.data[3];
          Serial.print(message.data[3], DEC);
          Serial.print(F(" km/h"));
        }
        Serial.print(F("\n"));
      }
    }
  }
}

void setup()
{
  delay(1000);
  Serial.begin(9600);
  Serial.println();
  Serial.print(F("-------------------------------------\n"));
  Serial.print(F("Driving Style Recorder (DSR) - Arduino UNO Firmware\n"));
  Serial.print(F("Made by Dominik Ciesiolkiewicz (2022)\n"));
  Serial.print(F("Version 0.0.3\n"));
  Serial.print(F("-------------------------------------\n"));
  Serial.print(F("Beginning system startup.\n"));

  pinMode(7,OUTPUT);//Error LED
  pinMode(8,OUTPUT);//Transmission Acknowledge LED

  Serial.print(F("Initializing CANbus communication.\n"));
  if(Canbus.init(CANSPEED_500))  /* Initialise MCP2515 CAN controller at the specified speed */
  {
    Serial.print(F("CANbus communication initialized correctly.\n")); 
  } 
  else
  {
    Serial.print(F("CANbus communication could not be initialised!\n"));
    canErrorBlink(100);
  }  
  delay(1000); 

  Serial.print(F("Initializing I2C bus as master.\n"));
  Wire.begin();
  //Wire.setTimeout(250); //setTimeout nie może być używany z biblioteką wire.
  Serial.print(F("I2C bus initialization complete.\n")); 

  pinMode(chipSelect, OUTPUT);

  counter = 0;
  Serial.print(F("System startup done.\n"));
  Serial.print(F("-------------------------------------\n"));
}

void loop()
{
  //REQUEST NUCLEO GPS DATA --------------------------------------
  Serial.print("Sending I2C data request...\n");
  int ret = Wire.requestFrom(0x02, BYTES);

  //Checking data sent by slave
  counter = 0;
  while (Wire.available()) {
    char c = Wire.read(); //receive a byte as character
    buff[counter] = c; //add the byte to a buffer
    counter++;
  }

  //Checking if all of the characters in the buffer are null
  counter = 0;
  for(int j=0;j<BYTES;j++){
    if(buff[j] == '\0')
    {
      counter++;      
    }
    else
    {
      break;
    }
  }

  //If yes, send an error
  if(counter == BYTES)
  {
    Serial.print(F("Data was not retrieved from I2C correctly!"));
    counter = 0;
    eraseBuffer(); //erasing the buffer for next usage
    errorBlink(100);//show I2C error
    return;
  }
  //or continue normally
  Serial.print(F("Data received successfully.\n"));
  counter = 0;

  //CAN REQUESTS -------------------------------------------------
  Serial.print(F("Requesting RPM data from CAN...\n"));
  rpm = 0;
  speed = 0;
  unsigned long currentMillis = millis();

  //Sending CAN RPM Request
  Canbus.ecu_req(ENGINE_RPM);

  //Reading RPM
  previousMillis = currentMillis;
  currentMillis = millis();

  while((unsigned long)(currentMillis - previousMillis) < interval) {
    checkAnswers();
    currentMillis = millis();
  }
  previousMillis = currentMillis;

  Serial.print(F("Finished.\nRequesting speed data from CAN...\n"));
  //Sending CAN Speed Request
  Canbus.ecu_req(VEHICLE_SPEED);
  
  //Reading Speed
  previousMillis = currentMillis;
  currentMillis = millis();
  
  while((unsigned long)(currentMillis - previousMillis) < interval) {
    checkAnswers();
    currentMillis = millis();
  }
  previousMillis = currentMillis;
  Serial.print(F("Finished.\n"));

  //SAVING TO SD -------------------------------------------------

  Serial.print(F("Trying to save data on the micro SD card.\n"));

  //Open file on the SD card
  if(!SD.begin(chipSelect)) {
    Serial.println(F("ERROR: Card failed, or not present!"));
    errorBlink(400);
    return;
  }
  File dataFile = SD.open("DSR_GPS.txt", FILE_WRITE);  

  //If the file is available, write to it:
  if (dataFile){
    //Printing data from I2C buffer to serial
    for(int i=0;i<BYTES;i++){
      Serial.print((char)buff[i]);
    }
    Serial.print(",");
    Serial.print(speed);
    Serial.print(",");
    Serial.print(rpm);
    Serial.print(";");
    Serial.println();

    //Save data to SD card
    for(int i=0;i<BYTES;i++){
      dataFile.print((char)buff[i]);
    }
    dataFile.print(",");
    dataFile.print(speed);
    dataFile.print(",");
    dataFile.print(rpm);
    dataFile.print(";");
    dataFile.println();
    
    Serial.print(F("Data saved to SD card successfully.\n"));
    dataFile.close();
  }
  else
  {
    //If the file isn't open, show an error:
    Serial.println(F("Error copying GPS and CAN data to DSRData.txt."));
    dataFile.close();
    errorBlink(400);
        
    return;
  }
  eraseBuffer(); // erasing the buffer for next usage

  //ACKNOWLEDGE --------------------------------------------------
  //Show acknowledge through a fast LED blink
  digitalWrite(8, HIGH);
  delay(200);
  digitalWrite(8, LOW);

  Serial.println();
  delay(4800);
}
