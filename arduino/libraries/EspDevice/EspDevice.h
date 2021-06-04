/**
 * Louvain-la-Neuve: a smart city
 * DE KEERSMAEKER François
 * VAN DE WALLE Nicolas
 *
 * Class to handle ESP32 devices.
 */

#ifndef EspDevice_h
#define EspDevice_h

// Includes
#include "Arduino.h"
// LoRaWAN
#include <ESP32_LoRaWAN.h>
// BME280/680
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <bsec.h>
// Gas sensor
#include "MQ135.h"
// Location / GPS
#include "HardwareSerial.h"
#include "TinyGPS++.h"
// Custom
#include "Packet.h"
#include "WifiSender.h"
#include "SoundSensor.h"

// Debug messages
#define DEBUG 1
// Default pressure value
#define SEALEVELPRESSURE_HPA (1013.25)
// Vext control GPIO
#define VEXT_GPIO 21
// Battery-related constants
#define BATTERY_PIN  36
#define MAX_ANALOG   2127
#define MIN_ANALOG   1606
// Delay after powering Vext
#define VEXT_DELAY_SEC 10
// Serial output constants
#define SERIAL_BAUD 115200
// BME680 virtual sensors list
#define BME680_SENSORS 10

class EspDevice {

	public:
			EspDevice(uint32_t license[4], uint8_t DevEui[8], uint8_t AppEui[8], uint8_t AppKey[16]);
			void initPacket(uint8_t wakeupPeriod, uint8_t nMeasurements = 1, uint8_t version = 1);
	    void initWifi(const char* ssid,
										const char* password,
										const char* googleKey,
										const char* urlInflux,
										const char* token);
	    void setup();

			// Adding sensors
			void addBme280(uint8_t sda, uint8_t scl,
                  	 int temperatureThreshold,
                     int pressureThreshold,
                     int humidityThreshold);
			bool startBme280();
	    void addGasSensor(uint8_t gpio, int co2Threshold);
	    void addGPS(uint32_t gpsSerialBaud, uint8_t gpsRx, uint8_t gpsTx);
			void addSoundSensor(uint8_t gpio, int noiseThreshold);
			void addBme680(uint8_t sda, uint8_t scl,
                  	 int temperatureThreshold,
                     int pressureThreshold,
                     int humidityThreshold,
									 	 int airQualityThreshold);
			bool startBme680();

	    void loop();

	private:
			// Device/app attributes
	    uint32_t _license[4];
	    uint8_t _DevEui[8];
	    uint8_t _AppEui[8];
	    uint8_t _AppKey[16];

			uint8_t _wakeupPeriod;
			uint8_t _nMeasurements;

			WifiSender wifi;

			// GPIO alim pin
			uint8_t _gpioAlimPin;
    	// BME280
    	Adafruit_BME280 bme280;
	    bool _bme280Connected;
			int _temperatureThreshold;
			int _pressureThreshold;
			int _humidityThreshold;
	    // Gas sensor
	    MQ135 gasSensor;
	    bool _gasSensorConnected;
			int _co2Threshold;
	    // Location / GPS
			const char* _ssid;
			const char* _password;
	    TinyGPSPlus gps;
			uint32_t _gpsSerialBaud;
			uint8_t _gpsRx;
			uint8_t _gpsTx;
	    bool _gpsConnected;
			bool _isAltitudeGps;
			// Sound sensor
			SoundSensor soundSensor;
			bool _soundSensorConnected;
			int _noiseThreshold;
			// BME680
			Bsec bme680;
			bool _bme680Connected;
			int _airQualityThreshold;

			// Emergency
			bool _emergency;

			// Misc
			void initStorage();
			void saveBattery();
			void printValue(char *name, float value);
			void displayValue(char *name, float value);
    	String devEUI_string();

			// Vext control
			void startVext();
			void stopVext();

			void getWifiLocation();
			void warmupSensors(uint32_t time);

			// Getting sensor values
			void getBattery();
			void getBme280Values();
			void getSoundValue();
			void getGasValue();
			void getGpsValues();
			void addLocationData();
			void getBme680Values();
			void computeAltitude(float temperature, float pressure);
			void getValues();

			// Encode and send
			void sendLora();
			void sendWifi();

};

#endif
