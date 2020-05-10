/*
Made by: Christopher Milian
Purpose: To display distance from ultrasonic sensor onto LCD
Started: 5/6/2020
Completed: 5/7/2020
*/

#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <pcf8574.h>
#include <lcd.h>
#include <time.h>
#include <sys/time.h>

// For Ultrasonic Sensor
#define trigPin 4       
#define echoPin 5
#define MAX_DISTANCE 220        // Define the maximum measured distance
#define timeOut MAX_DISTANCE*60 // Calculate timeout according to the maximum measured distance

// For LCD Screen. Uncomment the address of your device
#define pcf8574_address 0x27    // default I2C address of PCF8574
//#define pcf8574_address 0x3F  // default I2C address of PCF8574A
#define BASE 64                 // BASE any number above 64

//Define the output pins of the PCF8574, which are directly connected to the LCD1602 pin.
#define RS      BASE+0
#define RW      BASE+1
#define EN      BASE+2
#define LED     BASE+3
#define D4      BASE+4
#define D5      BASE+5
#define D6      BASE+6
#define D7      BASE+7

int lcdhd;  // Used to handle LCD

// Functions for Ultrasonic Sensor
int pulseIn(int pin, int level, int timeout)  // Function pulseIn: obtain pulse time of a pin
{
   struct timeval tn, t0, t1;
   long micros;
   gettimeofday(&t0, NULL);
   micros = 0;
   while (digitalRead(pin) != level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(pin) == level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros = micros + (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros = micros + (tn.tv_usec - t1.tv_usec);
   return micros;
}

float getSonar(){  // Get the measurement result of ultrasonic module with unit: cm

    long pingTime;
    float distance;
    digitalWrite(trigPin,HIGH); // Send 10us high level to trigPin 
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pingTime = pulseIn(echoPin,HIGH,timeOut);           // Read plus time of echoPin
    distance = (float)pingTime * 340.0 / 2.0 / 10000.0; // Calculate distance with sound speed 340m/s
    return distance;
}

// Function that displays distance from sensor onto LCD
void printDistance(){
    float distance = 0;
    distance = getSonar();
    lcdPosition(lcdhd,0,0);  // Set the LCD cursor position to (0,0)
    lcdPrintf(lcdhd,"Dist: %.2fcm", distance);  //Display system time on LCD
    printf("Dist: %.2fcm\n", distance);
}

// Main Code for Sensor and LCD
int main(){

    printf("Sensor and Screen are Initializing.\n\n");
    wiringPiSetup();

    int i;
    pcf8574Setup(BASE,pcf8574_address);  // Initialize PCF8574
    for(i=0;i<8;i++){
        pinMode(BASE+i,OUTPUT);  // Set PCF8574 port to output mode
    } 

    digitalWrite(LED,HIGH);  // Turn on LCD backlight
    digitalWrite(RW,LOW);    // Allow writing to LCD
	lcdhd = lcdInit(2,16,4,RS,EN,D4,D5,D6,D7,0,0,0,0);  // Initialize LCD and return “handle” used to handle LCD

    
    pinMode(trigPin,OUTPUT);
    pinMode(echoPin,INPUT);

    if(lcdhd == -1){
        printf("LCD could not be initialized. Exiting.");
        return 1;
    }
    while(1){

        printDistance(); 
        delay(500);
    }
    return 1;
}