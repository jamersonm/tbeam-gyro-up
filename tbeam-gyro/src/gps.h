#include <TinyGPS++.h>

uint32_t LatitudeBinary, LongitudeBinary;
uint16_t altitudeGps;
uint16_t speedGPS;
uint8_t hdopGps;

TinyGPSPlus gps;
HardwareSerial GPS(1);

void configureGPS()
{
    GPS.begin(9600, SERIAL_8N1, 34, 12);
}

void buildTxBuffer(uint8_t txBuffer[9])
{
    LatitudeBinary = ((gps.location.lat() + 90) / 180) * 16777215;
    LongitudeBinary = ((gps.location.lng() + 180) / 360) * 16777215;

    txBuffer[0] = (LatitudeBinary >> 16) & 0xFF;
    txBuffer[1] = (LatitudeBinary >> 8) & 0xFF;
    txBuffer[2] = LatitudeBinary & 0xFF;

    txBuffer[3] = (LongitudeBinary >> 16) & 0xFF;
    txBuffer[4] = (LongitudeBinary >> 8) & 0xFF;
    txBuffer[5] = LongitudeBinary & 0xFF;

    //speedGPS = gps.speed.kmph()*10;
    //txBuffer[6] = (speedGPS >> 8) & 0xFF;
    //txBuffer[7] = speedGPS & 0xFF;

    altitudeGps = gps.altitude.meters()*10;
    txBuffer[6] = (altitudeGps >> 8) & 0xFF;
    txBuffer[7] = altitudeGps & 0xFF;

    #ifndef USE_COMPASS
    hdopGps = gps.hdop.hdop() * 10;
    txBuffer[8] = hdopGps & 0xFF;
    #endif
}

void printGPS(uint8_t txBuffer[9])
{
    Serial.println(*txBuffer, HEX);
    Serial.println(gps.location.lat(), 5);
    Serial.println(gps.location.lng(), 4);
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print("Speed  : ");
    Serial.println(gps.speed.kmph(), 1);
    Serial.print("Altitude  : ");
    Serial.println(gps.altitude.meters());
}