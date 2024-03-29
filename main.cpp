#include "DHT11.h"
#include "Servo.h"
#include "TextLCD.h"
#include "buzzer.h"
#include "ds1307.h"
#include "mbed.h"
#include <vector>

Serial pc(USBTX, USBRX);
DigitalOut led(p12);    // Automatic lights for building
DS1307 rtc(p9, p10);    // Real-time clock module (sda, scl)
DHT11 humidAndTemp(p6); // Humidity and temperature sensor (pin)
TextLCD lcd(p15, p16, p17, p18, p19, p20); // LCD module: rs, e, d4-d7
// Beep buzzer(); // Active buzzer module
PwmOut fanSwitch(p21);

int sec = 0;
int mins = 0;
int hour = 0;
int day = 0;
int date = 0;
int month = 0;
int year = 0;

// Control lights based on the time of day
void automateLights() 
{
  rtc.gettime(&sec, &mins, &hour, &day, &date, &month, &year);
  pc.printf("%.2i : %.2i : %.2i\n\r", hour, mins, sec);

  // Turn on LED between 8:00 am and 4:00 pm
  if (hour >= 8 && hour <= 16)
    led = 1;
  else
    led = 0;
}

// Control fan speed and power based on temperature and humidity
void regulateTemperatureAndHumidity() 
{
    int temperature = humidAndTemp.readTemperature();
    int humidity = humidAndTemp.readHumidity();

    lcd.cls();
    lcd.locate(0, 0);
    lcd.printf("Temperature: %i C", temperature);
    pc.printf("Temperature: %i C\n\r", temperature);
    lcd.locate(0, 1);
    lcd.printf("Humidity: %i %%", humidity);
    pc.printf("Humidity: %i %%\n\r", humidity);

    // Three speed fans
    if (temperature < 25 || humidity < 12) 
    {
        fanSwitch = 0;
    }

    else if (temperature < 26 || humidity < 15) 
    {
        fanSwitch = 0.33;
    }

    else if (temperature < 27 || humidity < 17) 
    {
        fanSwitch = 0.66;
    }

    else 
    {
        fanSwitch = 1;
    }
}

int main() 
{
    fanSwitch.period(0.1);
    int status = humidAndTemp.readData(); // Read the status of sensor

    while (status != DHT11::OK) 
    {
        status = humidAndTemp.readData();
    }

    while (1) 
    {
        automateLights();

        regulateTemperatureAndHumidity();

        wait(1);
    }
}

