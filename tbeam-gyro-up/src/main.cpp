#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "axp.h"
#include "gps.h"
#include "gyro.h"
#include "packet.h"
#include "settings.h"

static const u1_t PROGMEM APPEUI[8] = { APPEUI_KEY }; //lsb format
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
static const u1_t PROGMEM DEVEUI[8]  = { DEVEUI_KEY }; // lsb format
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
static const u1_t PROGMEM APPKEY[16] = { APPKEY_KEY }; //msb format
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

static osjob_t sendjob;
uint8_t buffer[9];

// Pinmap TBEAM
const lmic_pinmap lmic_pins = {
    .nss = NSS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RST_PIN,
    .dio = {DIO0_PIN, DIO1_PIN, DIO2_PIN},
};

// Intervalo de envio
#ifndef USE_GYRO
const unsigned TX_INTERVAL = INTERVALO_ENVIO;
#endif

void do_send(osjob_t *j)
{
  if(LMIC.opmode & OP_TXRXPEND)
    Serial.println("OP_TXRXPEND, not sending");
  else
  {
    buildPacket(buffer);
    LMIC_setTxData2(1, buffer, sizeof(buffer), 0);
    Serial.println("Packet queued");
    Serial.print("Sending packet on frequency: ");
    Serial.println(LMIC.freq);
  }
}

void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        {
            u4_t netid = 0;
            devaddr_t devaddr = 0;
            u1_t nwkKey[16];
            u1_t artKey[16];
            LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey); // FUNÇÃO PARA OBTER AS KEYS DE SESSÃO.
            Serial.print("netid: ");
            Serial.println(netid, DEC);
            Serial.print("devaddr: ");
            Serial.println(devaddr, HEX);
            Serial.print("artKey: ");
            for (int i = 0; i < sizeof(artKey); ++i)
            {
                Serial.print(artKey[i], HEX);
            }
            Serial.println("");
            Serial.print("nwkKey: ");
            for (int i = 0; i < sizeof(nwkKey); ++i)
            {
                Serial.print(nwkKey[i], HEX);
            }
            Serial.println("");
        }
        Serial.println(F("Successful OTAA Join..."));

        LMIC_setLinkCheckMode(0); // Desativa a verificação de validade do link com a rede, só ativar no inicio de uma sessão
        break;
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
    case EV_TXCOMPLETE:
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen)
        {
            Serial.println(F("Received "));
            Serial.println(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }
        // Schedule next transmission
        #ifndef USE_GYRO
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
        #endif
        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        // data received in ping slot
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELED"));
        break;
    case EV_RXSTART:
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        break;
    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}

void setup() {
  Serial.begin(SERIAL_BAUND);

  configureAXP();
  configureGPS();
  #ifdef USE_GYRO
  configureGyro();
  #endif

  Serial.println("Starting");

  os_init(); // LMIC init
  LMIC_reset(); // Reset the MAC state. Session and pending data transfers will be discarded.
}

void loop() {
  loopGPS();

  #ifdef USE_GYRO
  loopGyro();
  if(getYawChange())
    do_send(&sendjob);
  else
    Serial.println("Not sended");
  #endif

  #ifndef USE_GYRO
  os_runloop_once();
  #endif
}