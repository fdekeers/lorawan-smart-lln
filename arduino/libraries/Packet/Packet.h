/**
 * Louvain-La-Neuve: a smart city
 * DE KEERSMAEKER François
 * VAN DE WALLE Nicolas
 *
 * Class for formatting LoRa data packets.
 */

#ifndef Packet_h
#define Packet_h

#include <Arduino.h>

// Default values
#define DEFAULT_INT   0
#define DEFAULT_FLOAT -200
// Maximum number of measurements for each version
const uint8_t MAX_MEASUREMENTS_VERSION[3] = {1, 5, 8};
#define MAX_MEASUREMENTS 8

// Header values definitions
const uint8_t HEADER_BATTERY     = 0x01;
const uint8_t HEADER_TEMPERATURE = 0x02;
const uint8_t HEADER_PRESSURE    = 0x03;
const uint8_t HEADER_HUMIDITY    = 0x04;
const uint8_t HEADER_ALTITUDE    = 0x05;
const uint8_t HEADER_LIGHT       = 0x06;
const uint8_t HEADER_LATITUDE    = 0x07;
const uint8_t HEADER_LONGITUDE   = 0x08;
const uint8_t HEADER_CO2         = 0x09;
const uint8_t HEADER_NOISE       = 0x0A;
const uint8_t HEADER_AIR_QUALITY = 0x0B;

// Structure for one measurement
typedef struct measurement {
	uint8_t battery;
	float temperature;
	float pressure;
	float humidity;
	uint16_t light;
	float latitude;
	float longitude;
	float altitude;
	float co2;
	uint16_t noise;
	float airQuality;
} measurement_t;

/* Packet class */
class Packet {

	public:
		Packet();
		Packet(uint8_t interval, uint8_t nMeasurements = 1, uint8_t version = 1);
		void init(uint8_t interval, uint8_t nMeasurements = 1, uint8_t version = 1);

		uint8_t getVersion();
		uint8_t getIndex();
		uint8_t getPayloadSize();
		measurement_t* getMeasurementsArray();

		void setMeasurementsArray(measurement_t* measurementsArray);

		void printValue(char* name, float value);
		void printArray();
		void printPayload();

		void addBattery(uint8_t battery, uint8_t index);
		void addTemperature(float temperature, uint8_t index);
		void addPressure(float pressure, uint8_t index);
		void addHumidity(float humidity, uint8_t index);
		void addAltitude(float altitude, uint8_t index);
		void addLight(uint16_t light, uint8_t index);
		void addLatitude(float latitude, uint8_t index);
		void addLongitude(float longitude, uint8_t index);
		void addCO2(float co2, uint8_t index);
		void addNoise(uint16_t noise, uint8_t index);
		void addAirQuality(float airQuality, uint8_t index);

		uint8_t* buildLoraPayload();
		uint8_t* buildLoraPayload(uint8_t* payload, uint8_t* size_ptr);
		void clearArray();

	private:
		uint8_t _version;
		uint8_t _interval;
		uint8_t _nMeasurements;
		measurement_t* _measurementsArray;
		uint8_t _index;
		uint8_t* _payload;
		uint8_t* _size_ptr;

		void addPreamble();
		void appendTimestamp(uint8_t timestamp);
		void appendDataToPayload(uint8_t header, uint8_t* data_ptr, uint8_t size);
		void appendIntToPayload(uint8_t header, uint32_t data, uint8_t* data_ptr, uint8_t size);
		void appendFloatToPayload(uint8_t header, float data, uint8_t precision);
		void closeMeasurement();
		void populateLoraPayload();

};

#endif
