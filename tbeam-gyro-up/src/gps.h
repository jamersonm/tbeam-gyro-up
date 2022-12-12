#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial GPS(1);
unsigned long timerGPS = millis();

void configureGPS()
{
    GPS.begin(9600, SERIAL_8N1, 34, 12);
}

void loopGPS()
{
    if((millis() - timerGPS) < 10)
        gps.encode(GPS.read());
    if((millis() - timerGPS) > 20)
        timerGPS = millis();
}

float getLatitudeBinary(){return (((gps.location.lat() + 90) / 180) * 16777215);}
float getLongitudeBinary(){return (((gps.location.lng() + 180) / 360) * 16777215);}
float getSpeed(){return (gps.speed.kmph()*10);}
float getAltitude(){return (gps.altitude.meters()*10);}
float getHdops(){return (gps.hdop.hdop() * 10);}
#endif