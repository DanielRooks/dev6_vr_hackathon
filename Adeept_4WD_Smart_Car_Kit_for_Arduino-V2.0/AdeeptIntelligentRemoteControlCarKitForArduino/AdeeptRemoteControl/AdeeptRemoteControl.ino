/***********************************************************
File name:  AdeeptRemoteControl.ino
Description:  

Website: www.adeept.com
E-mail: support@adeept.com
Author: Tom
Date: 2018/6/8 
***********************************************************/
#include <SPI.h>
#include "RF24.h"
#include <Wire.h>
#define DEVICE (0x53)      //ADXL345 device address
#define TO_READ (6)        //num of bytes we are going to read each time (two bytes for each axis)
RF24 radio(9, 10);            // define the object to control NRF24L01
byte addresses[][6] = {"007","001"};
int data[9];                  // define array used to save the communication data
int radar[2];
int mode = 1;
char onlyOne=0;
const int R6Pin = 7;        // define R6
const int R1Pin = 6;        // define R1
const int led1Pin = 6;        // define pin for LED1 which is close to NRF24L01 and used to indicate the state of NRF24L01
const int led2Pin = 7;        // define pin for LED2 which is the mode is displayed in the car remote control mode  
const int led3Pin = 8;        // define pin for LED3 which is the mode is displayed in the car auto mode
const int APin = 2;           // define pin for D2
const int BPin = 3;           // define pin for D3
const int CPin = 4;           // define pin for D4
const int DPin = 5;           // define pin for D5
const int dcMotorPin = 1;       // define pin for direction Y of joystick U1
const int dirPin = 2;       // define pin for direction X of joystick U2
const int ultrasonicPin = 3;  // define pin for direction Y of joystick U2
byte buff[TO_READ] ;        //6 bytes buffer for saving data read from the device
char str[512];              //string buffer to transform data before sending it to the serial port
int regAddress = 0x32;      //first axis-acceleration-data register on the ADXL345
int x, y, z;                        //three axis acceleration data
double roll = 0.00, pitch = 0.00;       //Roll & Pitch are the angles which rotate by the axis X and y 
//in the sequence of R(x-y-z),more info visit
void setup() {
  Serial.begin(9600);
  Wire.begin();         // join i2c bus (address optional for master)
  radio.begin();                      // initialize RF24
  radio.setRetries(0, 15);            // set retries times
  radio.setPALevel(RF24_PA_LOW);      // set power
  radio.openWritingPipe(addresses[1]);  // open delivery channel
  radio.openReadingPipe(1,addresses[0]);
  radio.stopListening();              // stop monitoring
 //radio.startListening();// start monitoring
  pinMode(led1Pin, OUTPUT);   // set led1Pin to output mode
  pinMode(led2Pin, OUTPUT);   // set led2Pin to output mode
  pinMode(led3Pin, OUTPUT);   // set led3Pin to output mode
  pinMode(APin, INPUT_PULLUP);   // set APin to output mode
  pinMode(BPin, INPUT_PULLUP);   // set BPin to output mode
  pinMode(CPin, INPUT_PULLUP);   // set CPin to output mode
  pinMode(DPin, INPUT_PULLUP);   // set DPin to output mode  
  //Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);      
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
  data[2] = 1;
}
void loop() {
  // put the values of rocker, switch and potentiometer into the array
  data[0] = analogRead(dirPin);
  data[1] = analogRead(dcMotorPin);
  if(digitalRead(APin)==LOW){  
    delay(20);
    if(digitalRead(APin)==LOW){
      mode = 1;
      data[2] = 1;
      radio.stopListening();              // stop monitoring
    }
  }
  if(digitalRead(BPin)==LOW){  
    delay(20);
    if(digitalRead(BPin)==LOW){
      mode = 2;
      data[2] = 2;
      radio.stopListening();              // stop monitoring
    }
  }
  if(digitalRead(CPin)==LOW){  
    delay(20);
    if(digitalRead(CPin)==LOW){
      mode = 3;
      data[2] = 3;
      radio.stopListening();              // stop monitoring
    }
  }
  if(digitalRead(DPin)==LOW){  
    delay(20);
    while(!(digitalRead(DPin)==LOW));
      mode = 4;
      data[2] = 4;
      radio.stopListening();
     
      // send array data. If the sending succeeds, open signal LED
      radio.write( data, sizeof(data));
      radio.startListening();// start monitoring
  } 
  data[3] = analogRead(R1Pin);
  data[4] = analogRead(R6Pin);
  data[5] = analogRead(ultrasonicPin);
  if(mode == 3){
    readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345
                                                //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
                                                //thus we are converting both bytes in to one int
    x = (((int)buff[1]) << 8) | buff[0];
    y = (((int)buff[3]) << 8) | buff[2];
    z = (((int)buff[5]) << 8) | buff[4];
    data[0] = map(x,-300,300,0,1023);
    data[1] = map(y,-300,300,0,1023);
  } 

  if(mode!=4){
    // send array data. If the sending succeeds, open signal LED
    if (radio.write( data, sizeof(data) ))
    digitalWrite(led1Pin,HIGH);
    // delay for a period of time, then turn off the signal LED for next sending
    delay(2);
    digitalWrite(led1Pin,LOW);
  }  
  if(mode==4){
     
    if ( radio.available()) {             // if receive the data
      while (radio.available()) {         // read all the data
        radio.read( data, sizeof(data) ); // read data
      }     
      digitalWrite(led1Pin,HIGH);
     }else{digitalWrite(led1Pin,LOW);}
     Serial.print(data[6]); // Sends the current degree into the Serial Port
     Serial.print(","); // Sends addition character right next to the previous value needed later in the Processing IDE for indexing
     Serial.print(data[7]); // Sends the distance value into the Serial Port
     Serial.print("."); // Sends addition character right next to the previous value needed later in the Processing IDE for indexing
  }

  if(mode==1){
    digitalWrite(led2Pin,HIGH);
    digitalWrite(led3Pin,LOW);
  }
  if(mode==2){
    digitalWrite(led2Pin,LOW);
    digitalWrite(led3Pin,HIGH);
  }
  if(mode==3){
    digitalWrite(led2Pin,HIGH);
    digitalWrite(led3Pin,HIGH);
  }
  if(mode==4){
    digitalWrite(led2Pin,LOW);
    digitalWrite(led3Pin,LOW);
  }
}
//---------------- ADXL345 Functions
//Writes val to address register on device
void writeTo(int device, byte address, byte val) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        // send register address
  Wire.write(val);        // send value to write
  Wire.endTransmission(); //end transmission
}
//reads num bytes starting from address register on device in to buff array
void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}
//calculate the Roll&Pitch
void RP_calculate(){
  double x_Buff = float(x);
  double y_Buff = float(y);
  double z_Buff = float(z);
  roll = atan2(y_Buff , z_Buff) * 57.3;
  pitch = atan2((- x_Buff) , sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * 57.3;
}
