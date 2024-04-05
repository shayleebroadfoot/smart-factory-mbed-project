#include "mbed.h"
#include "DHT11.h"
#include "Servo.h"
#include "TextLCD.h"
#include "buzzer.h"
#include "ds1307.h"
#include <vector>
#include <stdio.h>
#include <string>
using namespace std;

Serial pc(USBTX, USBRX);
Serial bluetooth(p13, p14);
DigitalOut led(p12);    // Automatic lights for building
DS1307 rtc(p9, p10);    // Real-time clock module (sda, scl)
DHT11 humidAndTemp(p6); // Humidity and temperature sensor (pin)
TextLCD lcd(p15, p16, p23, p24, p25, p26); // LCD module: rs, e, d4-d7
Beep buzzer(p22); // Active buzzer module
PwmOut fanSwitch(p21); // DC Fan Motor
AnalogIn waterSensor(p19); // Water level sensor

int sec = 0;
int mins = 0;
int hour = 0;
int day = 0;
int date = 0;
int month = 0;
int year = 0;

int hourStart = 8;
int hourEnd = 16;
char hourStartString[] = "08 AM";
char hourEndString[] = "16 PM";

int status;
int temperature = 0;
int humidity = 0;
int tempMin = 22, tempMed = 26, tempMax = 30;
char tempString[3] = {0};
string minMedOrMax = "";

float waterVal;

void convert24HourTo12Hour(int hour_24, int hourStartOrHourEnd) // Pass 0 for start or 1 for end
{
    const char* meridian = "AM";
    if (hour_24 == 0) 
    {
        hour_24 = 12; // Midnight is 12 AM
    }
    else if (hour_24 == 12) 
    {
        meridian = "PM"; // Noon is 12 PM
    } 
    else if (hour_24 > 12) 
    {
        hour_24 -= 12;
        meridian = "PM";
    }

    if (hourStartOrHourEnd == 0)
        std::sprintf(hourStartString, "%i %s", hour_24, meridian);
    else if (hourStartOrHourEnd == 1)
        std::sprintf(hourEndString, "%i %s", hour_24, meridian);
}

// Control lights based on the time of day
void automateLights() 
{
  rtc.gettime(&sec, &mins, &hour, &day, &date, &month, &year);
  pc.printf("Time = %.2i : %.2i : %.2i\n\r", hour, mins, sec);

    // Turn on LED Monday to Friday
    if (day > 0 && day < 6)
    {
        // Turn on LED between 8:00 am and 4:00 pm standard
        if (hour >= hourStart && hour < hourEnd)
            led = 1;
        else
        {
            if (hourEnd < hourStart)
            {
                led = 1;
            }
            
            led = 0;  
        } 
    }
    else 
    {
        led = 0;
    }
}

// Control fan speed and power based on temperature and humidity
void regulateTemperatureAndHumidity() 
{
    status = humidAndTemp.readData(); // Read the status of sensor
    if (status != DHT11::OK) 
    {  // If not okay
        pc.printf("Device not ready\r\n");
    } 
    else 
    { // If the status is okay, read the values
        temperature = humidAndTemp.readTemperature();
        humidity = humidAndTemp.readHumidity();
        pc.printf("Temperature: %d C\r\n", temperature);
        pc.printf("Humidity: %d %%\r\n", humidity);
    }

    lcd.cls();
    lcd.locate(0, 0);
    lcd.printf("Temperature: %i C", temperature);
    pc.printf("Temperature: %i C\n\r", temperature);
    lcd.locate(0, 1);
    lcd.printf("Humidity: %i %%", humidity);
    pc.printf("Humidity: %i %%\n\r", humidity);

    // Three speed fans
    if (temperature < tempMin || humidity < 12) 
    {
        fanSwitch = 0;
    }

    else if (temperature < tempMed || humidity < 15) 
    {
        fanSwitch = 0.33;
    }

    else if (temperature < tempMax || humidity < 17) 
    {
        fanSwitch = 0.66;
    }

    else 
    {
        fanSwitch = 1;
    }
}

void waterDetectionAlarm()
{
    waterVal = waterSensor;
    pc.printf("Water level = %f \n\r", waterVal);

    if (waterSensor > 0.01)
    {
        buzzer.beep(400, 1);
    }

    else 
    {
        buzzer.nobeep();
    }
}

void setLightOnStartTime()
{
    int index;
    bool isMorning = true;

    for (int i = 0; i < 6; i++)
    {
        if (hourStartString[i] == 'M')
        {
            index = i;

            if (hourStartString[index - 1] == 'A')
                isMorning = true;

            else if (hourStartString[index - 1] == 'P')
                isMorning = false;
            
            break;
        }
    }

    char temp[3] = {0};
    for (int i = 0; i < index - 2; i++)
    {
        temp[i] = hourStartString[i];
    }

    if (isMorning)
        hourStart = atoi(temp);
    else
    {
        hourStart = atoi(temp) + 12;
        hourStart = (hourStart == 24 ? 0 : hourStart);
    }
}

void setLightOnEndTime()
{
    int index;
    bool isMorning = false;

     for (int i = 0; i < 6; i++)
    {
        if (hourEndString[i] == 'M')
        {
            index = i;

            if (hourEndString[index - 1] == 'A')
                isMorning = true;

            else if (hourEndString[index - 1] == 'P')
                isMorning = false;
            
            break;
        }
    }

    char temp[3] = {0};
    for (int i = 0; i < index - 2; i++)
    {
        temp[i] = hourEndString[i];
    }

    if (isMorning)
    {
        hourEnd = atoi(temp);
        hourEnd = (hourEnd == 12 ? 0 : hourEnd);
    }
    else
    {
        hourEnd = atoi(temp);
        pc.printf("PM SUM: %i\n", hourEnd);
        hourEnd = (hourEnd == 12 ? hourEnd : hourEnd + 12);
    }
}

void setFanTemperatures(string minMedOrMax)
{
    if (minMedOrMax == "minimum")
    {
        tempMin = atoi(tempString);
    }

    else if (minMedOrMax == "medium")
    {
        tempMed = atoi(tempString);
    }

    if (minMedOrMax == "maximum")
    {
        tempMax = atoi(tempString);
    }
}

int main() 
{
    bluetooth.baud(9600);
    fanSwitch.period(0.1);
    
    char buffer[4] = {0};
    char symbol = 248; // degree symbol
    status = humidAndTemp.readData(); // Read the status of sensor

    while (status != DHT11::OK)
    {
        status = humidAndTemp.readData();
    }

    while (1) 
    {
        if(bluetooth.readable()) 
        {
            bluetooth.gets(buffer, sizeof(buffer));
            pc.printf("%s\n\r", buffer);

            if (strcmp(buffer, "000") == 0)
            {
                bluetooth.gets(hourStartString, sizeof(hourStartString));

                pc.printf("%s\n\r", hourStartString);

                setLightOnStartTime();
            }

            if (strcmp(buffer, "001") == 0)
            {
                bluetooth.gets(hourEndString, sizeof(hourEndString));
                pc.printf("%s\n\r", hourEndString);

                setLightOnEndTime();
            }

            if (strcmp(buffer, "010") == 0)
            {
                minMedOrMax = "minimum";
                bluetooth.gets(tempString, sizeof(tempString));
                pc.printf("%s\n\r", tempString);

                setFanTemperatures(minMedOrMax);
            }

            if (strcmp(buffer, "011") == 0)
            {
                minMedOrMax = "medium";
                bluetooth.gets(tempString, sizeof(tempString));
                pc.printf("%s\n\r", tempString);

                setFanTemperatures(minMedOrMax);
            }

            if (strcmp(buffer, "100") == 0)
            {
                minMedOrMax = "maximum";
                bluetooth.gets(tempString, sizeof(tempString));
                pc.printf("%s\n\r", tempString);

                setFanTemperatures(minMedOrMax);
            }
        }

        automateLights();

        regulateTemperatureAndHumidity();

        waterDetectionAlarm();

        convert24HourTo12Hour(hourStart, 0);

        convert24HourTo12Hour(hourEnd, 1);

        string lightStatus = (led == 1 ? "ON" : "OFF");
        string leakStatus = (waterVal > 0.01 ? "Leak detected!" : "No leaks detected");

        bluetooth.printf("%s|%i C|%i %%|%s|%s|%s|%i C|%i C|%i C", lightStatus, temperature, humidity, leakStatus, hourStartString, hourEndString, tempMin, tempMed, tempMax);
        pc.printf("%s|%i C|%i %%|%s|%i|%i|%i|%i|%i\n\r", lightStatus, temperature, humidity, leakStatus, hourStart, hourEnd, tempMin, tempMed, tempMax);
        pc.printf("Current Time = %i : %i\n\r", hour, mins);
        wait(2);
    }
}