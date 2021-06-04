/*
 * Louvain-la-Neuve: a smart city
 * DE KEERSMAEKER Francois
 * VAN DE WALLE Nicolas
 *
 * Arduino code for an ESP32 device.
 */

#include <EspDevice.h>

#define SEND_PERIOD_MIN 30 // TODO: define LoRaWAN sending period

// GPIO pins for sensors
#define BME_SDA_PIN 4   // TODO: define BME280 SDA pin
#define BME_SCL_PIN 15  // TODO: define BME280 SCL pin
#define MQ135_PIN   36  // TODO: define MQ135 data pin
#define GPS_RX      16  // TODO: define GPS RX pin
#define GPS_TX      17  // TODO: define GPS TX pin
#define SOUND_PIN   39  // TODO: define sound sensor data pin

// Serial baud rates
#define GPS_SERIAL_BAUD 9600  // TODO: define GPS serial baud rate

// Emergency threshold constants
// TODO: define the threshold values
#define TEMPERATURE_EMERGENCY_THRESHOLD 50
#define PRESSURE_EMERGENCY_THRESHOLD 150000
#define HUMIDITY_EMERGENCY_THRESHOLD 90
#define CO2_EMERGENCY_THRESHOLD 1000
#define NOISE_EMERGENCY_THRESHOLD 4090

/*
    TODO:
    * Get Chip ID with the get chip id sketch
    * Navigate to http://resource.heltec.cn/search
    * Copy chip id in the search field
    * Copy license here below
*/
uint32_t license[4] = {/* LICENSE GOES HERE */};

/* OTAA parameters */
uint8_t DevEui[] = {/* TODO: insert a device eui (e.g. 0x00 0x00 0xchipid) */};
uint8_t AppEui[] = {/* TODO: insert application EUI from TTN here*/};
uint8_t AppKey[] = {/* TODO: insert the application key from TTN here */};

EspDevice esp(license, DevEui, AppEui, AppKey, SEND_PERIOD_MIN);
// WiFi parameters
const char *ssid = "Insert wifi network SSID here";         // TODO
const char *password = "Insert wifi network password here"; // TODO
const char *googleKey = "Insert Google Geolocation API key here"; // TODO
const char *urlInflux = "https://lln-smart-city.azurewebsites.net/api/InfluxDBIntegration";

/*
    TODO:
    * Access the following url: https://lln-smart-city.web.app/
    * Log in (if allowed to)
    * Insert a device id
    * Click generate
    * Paste the returned token below
*/
const char *token = "PASTE THE TOKEN HERE"; // TODO

void setup()
{
    // Run setup method of the ESP32 device
    esp.initWifi(ssid, password, googleKey, urlInflux, token);
    esp.setup();
    esp.addBme(BME_SDA_PIN, BME_SCL_PIN,
               TEMPERATURE_EMERGENCY_THRESHOLD,
               PRESSURE_EMERGENCY_THRESHOLD,
               HUMIDITY_EMERGENCY_THRESHOLD);
    //esp.addGasSensor(MQ135_PIN, CO2_EMERGENCY_THRESHOLD);
    //esp.addGPS(GPS_SERIAL_BAUD, GPS_RX, GPS_TX);
    //esp.addSoundSensor(SOUND_PIN, NOISE_EMERGENCY_THRESHOLD);
}

void loop()
{
    // Run loop method of the ESP32 device
    esp.loop();
}
