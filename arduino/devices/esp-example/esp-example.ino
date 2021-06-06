/**
 * Using LoRaWAN and Wi-Fi for smart city monitoring in Louvain-la-Neuve
 * DE KEERSMAEKER Francois
 * VAN DE WALLE Nicolas
 *
 * Auto-generated Arduino sketch for the device esp-example
 */

#include <EspDevice.h>

// Wake-up period and number of measurements before sending
#define WAKEUP_PERIOD_MIN 10
#define N_MEASUREMENTS    4
#define VERSION           3

/* Heltec license to use LoRaWan with the device */
uint32_t license[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

/* OTAA parameters */
uint8_t DevEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AppEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AppKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

EspDevice esp(license, DevEui, AppEui, AppKey);

// WiFi parameters
const char *ssid = "SSID";
const char *password = "PASSWORD";
const char *googleKey = "GOOGLE_API_KEY";
const char *urlInflux = "https://lln-smart-city.azurewebsites.net/api/InfluxDBIntegration";
const char *token = "AUTHORIZATION_TOKEN";

// BME280 constants
#define BME280_SDA_PIN 4
#define BME280_SCL_PIN 15
#define TEMPERATURE_EMERGENCY_THRESHOLD 30
#define PRESSURE_EMERGENCY_THRESHOLD 102000
#define HUMIDITY_EMERGENCY_THRESHOLD 80

// MQ135 constants
#define MQ135_PIN   36
#define CO2_EMERGENCY_THRESHOLD 700

// Sound sensor constants
#define SOUND_PIN   39
#define NOISE_EMERGENCY_THRESHOLD 4090

// GPS constants
#define GPS_SERIAL_BAUD 9600
#define GPS_RX  16
#define GPS_TX  17

void setup()
{
    // Run setup method of the ESP32 device
    esp.initPacket(WAKEUP_PERIOD_MIN, N_MEASUREMENTS, VERSION);
    esp.initWifi(ssid, password, googleKey, urlInflux, token);
    esp.setup();
    esp.addBme280(BME280_SDA_PIN, BME280_SCL_PIN,
                  TEMPERATURE_EMERGENCY_THRESHOLD,
                  PRESSURE_EMERGENCY_THRESHOLD,
                  HUMIDITY_EMERGENCY_THRESHOLD);
    esp.addGasSensor(MQ135_PIN, CO2_EMERGENCY_THRESHOLD);
    esp.addSoundSensor(SOUND_PIN, NOISE_EMERGENCY_THRESHOLD);
    esp.addGPS(GPS_SERIAL_BAUD, GPS_RX, GPS_TX);
}

void loop()
{
    // Run loop method of the ESP32 device
    esp.loop();
}
