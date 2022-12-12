#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "gps.h"
#include "axp.h"
#include "configs.h"

//#define USE_COMPASS
#ifdef USE_COMPASS
#include "compass.h"
#endif

uint8_t txBuffer[9];
int last_direction = 0;
int direction;

static const u1_t PROGMEM APPEUI[8] = { APPEUI_KEY }; //lsb format
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
static const u1_t PROGMEM DEVEUI[8]  = { DEVEUI_KEY }; // lsb format
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
static const u1_t PROGMEM APPKEY[16] = { APPKEY_KEY }; //msb format
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

static osjob_t sendjob;

// Intervalo de envio 
const unsigned TX_INTERVAL = INTERVALO_ENVIO;

// Pinmap TBEAM
const lmic_pinmap lmic_pins = {
    .nss = NSS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RST_PIN,
    .dio = {DIO0_PIN, DIO1_PIN, DIO2_PIN},
};

void do_send(osjob_t *j)
{
    // Verifica se não está ocorrendo uma transmissão no momento TX/RX
    if (LMIC.opmode & OP_TXRXPEND)
    {
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else
    {
        if (gps.location.isUpdated()) 
        {
            buildTxBuffer(txBuffer);
        
            //LMIC_setTxData2(1, txBuffer, sizeof(txBuffer), 0);
        
            printGPS(txBuffer);

            Serial.println(F("Packet queued"));
            Serial.print(F("Sending packet on frequency: "));
            Serial.println(LMIC.freq);
        }
        else {
            LMIC_setTxData2(1, txBuffer, sizeof(txBuffer), 0);
        }
        #ifdef USE_COMPASS
            last_direction = direction;
        #endif
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
        #ifndef USE_COMPASS
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

void setup()
{
    Serial.begin(SERIAL_BAUND);

    configureAXP();
    configureGPS();
    #ifdef USE_COMPASS
    configureCompass();
    #endif

    Serial.println(F("Starting"));

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job
    do_send(&sendjob);
}

void loop()
{
    while (GPS.available()) // Faz a leitura do GPS. Não lembro se isso deve estar aqui ou na rotina de envio
        gps.encode(GPS.read());
    os_runloop_once();
    #ifdef USE_COMPASS
    loopCompass();
    //printCompass();
    direction = getCompass();
    /*Serial.print("last: ");
    Serial.println(last_direction);
    Serial.print("direction: ");
    Serial.println(direction);*/
    if(last_direction != direction)
    {
        do_send(&sendjob);
    }
    /*else
    {
        Serial.print("Not sended");
    }*/
    #endif
}
