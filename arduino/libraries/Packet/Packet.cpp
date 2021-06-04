/**
 * Louvain-La-Neuve: a smart city
 * DE KEERSMAEKER François
 * VAN DE WALLE Nicolas
 *
 * Class for formatting LoRaWAN data packets.
 */

#include "Packet.h"

/* No-arg constructor */
Packet::Packet() {}

/**
 * CONSTRUCTOR
 * Initializes a Packet object
 * @param interval: time interval between measurements (in minutes)
 * @param nMeasurements: number of measurements to store
 * @param version: packet format version
 */
Packet::Packet(uint8_t interval, uint8_t nMeasurements, uint8_t version) {
	init(interval, nMeasurements, version);
}

/**
 * Initializes the measurements array of the Packet object.
 * @param interval: time interval between measurements (in minutes)
 * @param nMeasurements: number of measurements to store in the array
 * @param version: packet format version
 */
void Packet::init(uint8_t interval, uint8_t nMeasurements, uint8_t version) {
	_index = 0;
	_interval = interval;
	// Set version
	if (nMeasurements == 1)
		_version = 1;
	else if (nMeasurements > 1 && version == 1)
		_version = 2;
	else
		_version = version;
	// Set number of measurements
	uint8_t maxMeasurements = MAX_MEASUREMENTS_VERSION[_version];
	_nMeasurements = (nMeasurements <= maxMeasurements) ? nMeasurements : maxMeasurements;
}

/**
 * Retrieves the version of the packet encoding.
 * @return: the version of the packet encoding
 */
uint8_t Packet::getVersion() {
	return _version;
}

/**
 * Retrieves the current index in the measurements array.
 * @return: the current index in the measurements array
 */
uint8_t Packet::getIndex() {
	return _index;
}

/**
 * Retrieves the current size of the LoRaWAN payload.
 * @return: the current size of the LoRaWAN payload
 */
uint8_t Packet::getPayloadSize() {
	return *_size_ptr;
}

/**
 * Retrieves a pointer to the measurements array.
 * @return: a pointer to the measurements array
 */
measurement_t* Packet::getMeasurementsArray() {
	return _measurementsArray;
}

/**
 * Sets the measurements array.
 * @param measurementsArray: pointer to the measurements array
 */
void Packet::setMeasurementsArray(measurement_t* measurementsArray) {
	_measurementsArray = measurementsArray;
}

/**
 * Prints a value on the serial port.
 * @name: name of the value to print, will be printed before the value
 * @value: value to be printed
 */
void Packet::printValue(char *name, float value)
{
	Serial.print(name);
	Serial.print(": ");
	Serial.println(value);
}

/*
 * Prints the content of the measurements array.
 */
void Packet::printArray() {
	Serial.println("PRINTING MEASUREMENTS ARRAY");
	for (uint8_t i = 0; i <= _index; i++) {
		printValue("TIMESTAMP", i);
		printValue("	Battery [%]", _measurementsArray[i].battery);
		printValue("	Temperature", _measurementsArray[i].temperature);
		printValue("	Pressure", _measurementsArray[i].pressure);
		printValue("	Humidity", _measurementsArray[i].humidity);
		printValue("	Light", _measurementsArray[i].light);
		printValue("	Latitude", _measurementsArray[i].latitude);
		printValue("	Longitude", _measurementsArray[i].longitude);
		printValue("	Altitude", _measurementsArray[i].altitude);
		printValue("	CO2", _measurementsArray[i].co2);
		printValue("	Noise", _measurementsArray[i].noise);
		printValue("	Air quality", _measurementsArray[i].airQuality);
	}
	Serial.println("---------------------------");
}

/**
 * Prints the bytes contained in the payload.
 */
void Packet::printPayload() {
  for (int i = 0; i < *_size_ptr; i++) {
    Serial.print("0x");
		uint8_t byte = *(_payload+i);
		if (byte < 0x10)
			Serial.print(0);
    Serial.print(byte, 16);
    Serial.print(" ");
  }
  Serial.println();
}

/**
 * Appends the battery percentage data to the measurements.
 * @param battery: battery percentage value
 */
void Packet::addBattery(uint8_t battery, uint8_t index) {
	_index = index;
	_measurementsArray[index].battery = battery;
}

/**
 * Appends the temperature data to the payload.
 * @param temperature: temperature value
 */
void Packet::addTemperature(float temperature, uint8_t index) {
	_index = index;
	_measurementsArray[index].temperature = temperature;
}

/**
 * Appends the pressure data to the payload.
 * @param pressure: pressure value
 */
void Packet::addPressure(float pressure, uint8_t index) {
	_index = index;
	_measurementsArray[index].pressure = pressure;
}

/**
 * Appends the humidity data to the payload.
 * @param humidity: humidity value
 */
void Packet::addHumidity(float humidity, uint8_t index) {
	_index = index;
	_measurementsArray[index].humidity = humidity;
}

/**
 * Appends the altitude data to the payload.
 * @param altitude: altitude value
 */
void Packet::addAltitude(float altitude, uint8_t index) {
	_index = index;
	_measurementsArray[index].altitude = altitude;
}

/**
 * Appends the light data to the payload.
 * @param light: light value
 */
void Packet::addLight(uint16_t light, uint8_t index) {
	_index = index;
	_measurementsArray[index].light = light;
}

/**
 * Appends the latitude data to the payload.
 * @param latitude: latitude value
 */
void Packet::addLatitude(float latitude, uint8_t index) {
	_index = index;
	_measurementsArray[index].latitude = latitude;
}

/**
 * Appends the longitude data to the payload.
 * @param longitude: longitude value
 */
 void Packet::addLongitude(float longitude, uint8_t index) {
	 _index = index;
 	_measurementsArray[index].longitude = longitude;
 }

/**
 * Appends the CO2 concentration data to the payload.
 * @param co2: CO2 concentration value
 */
void Packet::addCO2(float co2, uint8_t index) {
	_index = index;
	_measurementsArray[index].co2 = co2;
}

/**
 * Appends the noise data to the payload.
 * @param noise: noise value
 */
void Packet::addNoise(uint16_t noise, uint8_t index) {
	_index = index;
	_measurementsArray[index].noise = noise;
}

/**
 * Appends the air quality data to the payload.
 * @param noise: air quality value
 */
void Packet::addAirQuality(float airQuality, uint8_t index) {
	_index = index;
	_measurementsArray[index].airQuality = airQuality;
}

/**
 * Builds the LoRaWAN payload with the recorded measurements.
 * @param size: size (in bytes) of the LoRaWAN payload
 * @return: pointer to the start of the payload
 */
uint8_t* Packet::buildLoraPayload() {
	// Initialize payload
	uint8_t size = 2 + (_index + 2) * (sizeof(measurement_t));
	_payload = (uint8_t*) malloc(size);
	_size_ptr = (uint8_t*) malloc(sizeof(uint8_t));
	addPreamble();
	// Populate payload
	populateLoraPayload();
	clearArray();
	return _payload;
}

/**
 * Builds the LoRaWAN payload with the recorded measurements.
 * @param payload: pointer to the start payload
 * @param size_ptr: pointer to an integer containing the size of the payload
 * @return: pointer to the start of the payload
 */
uint8_t* Packet::buildLoraPayload(uint8_t* payload, uint8_t* size_ptr) {
	// Initialize payload
	_payload = payload;
	_size_ptr = size_ptr;
	addPreamble();
	// Populate payload
	populateLoraPayload();
	clearArray();
	return _payload;
}

/**
 * Adds the preamble at the beginning of the payload,
 * i.e. the version number and, if version is 2,
 * the time interval between measurements.
 */
void Packet::addPreamble() {
	*_payload = _version;
	*_size_ptr = 1;
	if (_version > 1) {
		*(_payload + 1) = _interval;
		*_size_ptr = 2;
	}
}

/**
 * Appends the specified timestamp to the LoRaWAN payload.
 * @param timestamp: timestamp to append to the payload
 */
void Packet::appendTimestamp(uint8_t timestamp) {
	*(_payload + *_size_ptr) = timestamp;
	*_size_ptr += sizeof(uint8_t);
}

/**
 * Appends the specified data to the LoRaWAN payload,
 * with the header specifying its type.
 * Warning: the byte order is inverted when copying !
 * @param header: header of the data, specifying its type
 * @param data_ptr: pointer to the data
 * @param size: size of the data in bytes
 */
void Packet::appendDataToPayload(uint8_t header, uint8_t* data_ptr, uint8_t size) {
	// Copy header
	*(_payload + *_size_ptr) = header;

	// Copy data, inverting the byte order
	for (int i = 1; i <= size; i++) {
		*(_payload + *_size_ptr + i) = *(data_ptr + size - i);
	}

	*_size_ptr += size + 1;
}

/**
 * Appends the specified int to the LoRaWAN payload,
 * with the header specifying its type.
 * @param header: header of the data, specifying its type
 * @param data: value of the data
 * @param data_ptr: pointer to the data
 * @param size: size of the data in bytes
 */
void Packet::appendIntToPayload(uint8_t header, uint32_t data, uint8_t* data_ptr, uint8_t size) {
	if (data != DEFAULT_INT)
		appendDataToPayload(header, data_ptr, size);
}

/**
 * Appends the specified float to the payload,
 * with the header specifying its type.
 * @param header: header corresponding to the data
 * @param data: data value
 * @param precision: number of decimal places
 */
void Packet::appendFloatToPayload(uint8_t header, float data, uint8_t precision) {
	if (data > DEFAULT_FLOAT) {
		double factor = pow(10.0, (double) precision);
		uint32_t data_int = (uint32_t) round(data*factor);
		uint8_t* data_ptr = (uint8_t*) &data_int;
		uint8_t size = sizeof(data_int);
		appendDataToPayload(header, data_ptr, size);
	}
}

/**
 * Close a measurement in the payload by appending a terminating null byte.
 */
void Packet::closeMeasurement() {
	*(_payload + *_size_ptr) = 0;
	*_size_ptr += 1;
}

/**
 * Populates the LoRaWAN payload with recorded measurements.
 */
void Packet::populateLoraPayload() {
	// Geolocation data
	if (_version == 3) {
		appendFloatToPayload(HEADER_LATITUDE, _measurementsArray[_index].latitude, 6);
		appendFloatToPayload(HEADER_LONGITUDE, _measurementsArray[_index].longitude, 6);
		appendFloatToPayload(HEADER_ALTITUDE, _measurementsArray[_index].altitude, 2);
	}

	for (uint8_t i = 0; i <= _index; i++) {
		// Timestamp
		if (_version > 1)
			appendTimestamp(i);
		// Battery [%]
		uint8_t battery = _measurementsArray[i].battery;
		appendIntToPayload(HEADER_BATTERY, battery, &battery, sizeof(uint8_t));
		// Temperature
		float temperature = _measurementsArray[i].temperature;
		appendFloatToPayload(HEADER_TEMPERATURE, temperature, 2);
		// Pressure
		float pressure = _measurementsArray[i].pressure;
		appendFloatToPayload(HEADER_PRESSURE, pressure, 2);
		// Humidity
		float humidity = _measurementsArray[i].humidity;
		appendFloatToPayload(HEADER_HUMIDITY, humidity, 2);
		// Light
		uint16_t light = _measurementsArray[i].light;
		appendIntToPayload(HEADER_LIGHT, light, (uint8_t*) (&light), sizeof(uint16_t));
		// Geolocation data
		if (_version <= 2) {
			appendFloatToPayload(HEADER_LATITUDE, _measurementsArray[i].latitude, 6);
			appendFloatToPayload(HEADER_LONGITUDE, _measurementsArray[i].longitude, 6);
			appendFloatToPayload(HEADER_ALTITUDE, _measurementsArray[i].altitude, 2);
		}
		// CO2
		float co2 = _measurementsArray[i].co2;
		appendFloatToPayload(HEADER_CO2, co2, 2);
		// Noise
		uint16_t noise = _measurementsArray[i].noise;
		appendIntToPayload(HEADER_NOISE, noise, (uint8_t*) (&noise), sizeof(uint16_t));
		// Air quality
		float airQuality = _measurementsArray[i].airQuality;
		appendFloatToPayload(HEADER_AIR_QUALITY, airQuality, 2);
		// Close measurement
		if (_version > 1)
			closeMeasurement();
	}
}

/**
 * Clears the measurements array,
 * by setting all its values to 0.
 */
void Packet::clearArray() {
	for (uint8_t i = 0; i < _nMeasurements; i++) {
		_measurementsArray[i].battery = DEFAULT_INT;
		_measurementsArray[i].temperature = DEFAULT_FLOAT;
		_measurementsArray[i].pressure = DEFAULT_FLOAT;
		_measurementsArray[i].humidity = DEFAULT_FLOAT;
		_measurementsArray[i].light = DEFAULT_INT;
		_measurementsArray[i].latitude = DEFAULT_FLOAT;
		_measurementsArray[i].longitude = DEFAULT_FLOAT;
		_measurementsArray[i].altitude = DEFAULT_FLOAT;
		_measurementsArray[i].co2 = DEFAULT_FLOAT;
		_measurementsArray[i].noise = DEFAULT_INT;
		_measurementsArray[i].airQuality = DEFAULT_FLOAT;
	}
}
