#include "mbed.h"
#include "DHT11.h"
#include "Keypad.h"
#include "ds1307.h"
#include "DS1820.h"
#include "Servo.h"
#include "TextLCD.h"
#include "buzzer.h"

Serial pc(USBTX, USBRX);
DigitalOut led(p12); // Automatic lights for building  
DS1307 rtc(p9, p10); // Real-time clock module (sda, scl)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       );
// DHT11 humidAndTemp(); // Humidity and temperature sensor (pin)
// DS1820 thermometer(); // Digital thermometer
// Keypad keypad(); // 4x4 keypad
// Servo servo(); // Servo motor
// TextLCD lcd(); // LCD module
// Beep buzzer(); // Active buzzer module

int sec = 0;
int min = 0;
int hour = 0;
int day = 0;
int date = 0;
int month = 0;
int year = 0;

int main() 
{
    led = 0;
    wait(4);
    
    while(1) 
    { 
        rtc.gettime(&sec, &min, &hour, &day, &date, &month, &year);
        pc.printf("%.2D : %.2D : %.2D", hour, min, sec);
        
        // Turn on LED between 8:00 am and 4:00 pm
        if(hour >= 8 && hour <= 16)
            led = 1;
        else
            led = 0;

        wait(2);
    }
}

