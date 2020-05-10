/*
Made by: Christopher Milian 
Purpose: To sound an alarm when on object gets too close, then display "Too Close! on the LCD. 
Started: 5/9/2020
Completed: 
*/

#pragma region Includes
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pcf8574.h>
#include <lcd.h>
#include <softTone.h>
#include <math.h>
#include <time.h> 
#include <sys/time.h>
#pragma endregion Includes

#pragma region Defines
// Defines for Buzzer
#define buzzerPin    0  // Define the buzzerPin
// Defines For Ultrasonic Sensor
#define trigPin 4       
#define echoPin 5
#define MAX_DISTANCE 220        // Define the maximum measured distance
#define timeOut MAX_DISTANCE*60 // Calculate timeout according to the maximum measured distance
// Defines For LCD Screen. 
#define pcf8574_address 0x27    // default I2C address of PCF8574
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
#pragma endregion Defines

int lcdhd;  // Used to handle LCD

void startAlertor(int pin){ // Turning on Buzzer
	int x;
	double sinVal, toneVal;
	for(x=0;x<360;x++){ // frequency of the alertor is consistent with the sine wave 
		sinVal = sin(x * (M_PI / 180));		//Calculate the sine value
		toneVal = 2000 + sinVal * 500;		//Add the resonant frequency and weighted sine value 
		softToneWrite(pin,toneVal);			//output corresponding PWM
		delay(1);
	}
}

void stopAlertor(int pin){ // Turning off buzzer
	softToneWrite(pin,0);
}

int pulseIn(int pin, int level, int timeout)  // Obtain pulse time of a pin
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

float getSonar(){  // Calculate distance of ultrasonic module in centimeters

    long pingTime;
    float distance;
    digitalWrite(trigPin,HIGH); // Send 10us high level to trigPin 
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pingTime = pulseIn(echoPin,HIGH,timeOut);           // Read plus time of echoPin
    distance = (float)pingTime * 340.0 / 2.0 / 10000.0; // Calculate distance with sound speed 340m/s
    return distance;
}

void printTooClose(){  // Displays "Too Close!" on LCD
    float distance = 0;
    distance = getSonar();
    lcdPosition(lcdhd,0,0);  // Set the LCD cursor position to (0,0)
    lcdPrintf(lcdhd,"Too Close!");  // Display on LCD
    printf("You are %.2fcm from the Pi.\n", distance);
}

void printGuarding(){  // Displays "Guarding.." on LCD
    float distance = 0;
    distance = getSonar();
    lcdPosition(lcdhd,0,1);  // Set the LCD cursor position to (0,1)
    lcdPrintf(lcdhd,"Guarding..");  // Display on LCD
    printf("You are %.2fcm from the Pi.\n", distance);
}

int main(void){
    
    wiringPiSetup(); 

    #pragma region Buzzer Setup
    // Buzzer Setup
    pinMode(buzzerPin, OUTPUT); 
	pinMode(buttonPin, INPUT);
    softToneCreate(buzzerPin); //set buzzerPin
	    //pullUpDnControl(buttonPin, PUD_UP);  //pull up to HIGH level
    #pragma endregion Buzzer Setup

    #pragma region LCD Setup
    // LCD Setup
    int i;
    pcf8574Setup(BASE,pcf8574_address);  // Initialize PCF8574
    for(i=0;i<8;i++){
        pinMode(BASE+i,OUTPUT);  // Set PCF8574 port to output mode
    } 
    digitalWrite(LED,HIGH);  // Turn on LCD backlight
    digitalWrite(RW,LOW);    // Allow writing to LCD
	lcdhd = lcdInit(2,16,4,RS,EN,D4,D5,D6,D7,0,0,0,0);  // Initialize LCD and return “handle” used to handle LCD
    #pragma endregion LCD Setup

    #pragma region HS-SR04 Setup
    // HS-SR04 Setup
     pinMode(trigPin,OUTPUT);
    pinMode(echoPin,INPUT);
    #pragma endregion HS-SR04 Setup
    
    // Checking to see if LCD is connected
    if(lcdhd == -1){ 
        printf("LCD could not be initialized. Exiting.");
        return 1;
    }

    while (1){

        if(distance < 20){ // Object too close
			startAlertor(buzzerPin);   // Turn on buzzer
			printTooClose();
		}
		else {  // No object detected
			stopAlertor(buzzerPin);   // Turn off buzzer
			printGuarding();
		}
    }
    return 0;
}