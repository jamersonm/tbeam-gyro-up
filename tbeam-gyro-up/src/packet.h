#ifndef PACKET_H
#define PACKET_H

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "gps.h"
#include "settings.h"

void buildPacket(uint8_t packet[9])
{
    uint32_t LatitudeBinary = getLatitudeBinary();
    packet[0] = (LatitudeBinary >> 16) & 0xFF;
    packet[1] = (LatitudeBinary >> 8) & 0xFF;
    packet[2] = LatitudeBinary & 0xFF;

    uint32_t LongitudeBinary = getLongitudeBinary();
    packet[3] = (LongitudeBinary >> 16) & 0xFF;
    packet[4] = (LongitudeBinary >> 8) & 0xFF;
    packet[5] = LongitudeBinary & 0xFF;

    #ifdef USE_SPEED
    uint16_t Speed = getSpeed();
    packet[6] = (Speed >> 8) & 0xFF;
    packet[7] = Speed & 0xFF;
    #endif

    #ifdef USE_ALTITUDE
    uint16_t Altitude = getAltitude();
    packet[6] = (Altitude >> 8) & 0xFF;
    packet[7] = Altitude & 0xFF;
    #endif

    uint8_t Hdops = getHdops();
    packet[8] = Hdops & 0xFF;
}

void printPacket(uint8_t packet[9])
{
    Serial.print("Latitude: ");
    Serial.print(packet[0]);
    Serial.print(packet[1]);
    Serial.println(packet[2]);

    Serial.print("Longitude: ");
    Serial.print(packet[3]);
    Serial.print(packet[4]);
    Serial.println(packet[5]);
    
    #ifdef USE_SPEED
    Serial.print("Speed  : ");
    #endif
    
    #ifdef USE_ALTITUDE
    Serial.print("Altitude  : ");  
    #endif  
    
    Serial.print(packet[6]);
    Serial.println(packet[7]); 

    Serial.print("Satellites (hdop): ");
    Serial.println(packet[8]);
}

void sendPacket(uint8_t packet[9])
{

}

#endif