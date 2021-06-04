/*
 * Louvain-La-Neuve: a smart city
 * DE KEERSMAEKER François
 * VAN DE WALLE Nicolas
 *
 * Class for sending data to our online platforms using WiFi.
 */

#include "Arduino.h"
#include "WifiSender.h"

// No-arg constructor
WifiSender::WifiSender() {}

// Constructor
WifiSender::WifiSender(const char* ssid,
											 const char* password,
											 const char* googleKey,
											 const char* urlInflux,
											 const char* token,
											 String devEUI) {
	init(ssid, password, googleKey, urlInflux, token, devEUI);
}

/**
 * Sets the instance variables.
 */
void WifiSender::init(const char* ssid,
											const char* password,
											const char* googleKey,
											const char* urlInflux,
											const char* token,
											String devEUI) {
	_ssid = ssid;
	_password = password;
	_wifiLocation.setKey(googleKey);
	_urlInflux = urlInflux;
	_token = token;
	_devEUI = devEUI;
	_timestamp = -1;
	resetJson();
}

/*
 * Connects to a WiFi access point, with the ssid and password attributes.
 * @returns: true if successfully connected, false otherwise
 */
bool WifiSender::connect() {
	WiFi.mode(WIFI_STA);
	WiFi.begin(_ssid, _password);

	// Try to connect to WiFi with a timeout
	uint32_t start = millis();
	while(WiFi.status() != WL_CONNECTED &&
				millis() - start < WIFI_TIMEOUT_SEC * 1000)
	{
		delay(10);
	}

	return WiFi.status() == WL_CONNECTED;
}

/*
 * Disconnects from and deactivates WiFi.
 */
void WifiSender::disconnect() {
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
}

/**
 * Gets device location via Wi-Fi with Google Geolocation API.
 * @returns: location struct containing location data
 */
location_t WifiSender::getLocation() {
	location_t location;
	if (connect()) {
		location = _wifiLocation.getGeoFromWiFi();
	} else {
		location.lat = 0;
		location.lon = 0;
		location.accuracy = MAX_LOC_ACCURACY;
	}
	disconnect();
	return location;
}

/*
 * Sends the JSON object containing the data to InfluxDB,
 * then resets JSON object.
 * @param version: version of the packet encoding
 * @param interval: time interval between two measurements
 * @param measurementsArray: array containing the recorded measurements
 * @param size: number of measurements to consider
 * @returns: true if packet was successfully sent over Wi-Fi, false otherwise
 */
bool WifiSender::sendData(uint8_t version, uint8_t interval, measurement_t* measurementsArray, uint8_t size) {
	_version = version;
	// Populates the JSON object with measurements
	populateJson(version, interval, measurementsArray, size);
	// Close JSON object to send
	closeJson();
	printJson();

	// Connect to Wi-Fi
	bool connected = connect();
	if (connected) {
		// Send JSON object to InfluxDB
		_http.begin(_urlInflux, certificate);
		_http.addHeader("Content-Type", "application/json");
		_http.addHeader("authorization", _token);
		int httpResponseCode = _http.POST(_json);
		Serial.print("HTTP response code: ");
		Serial.println(httpResponseCode);
		_http.end();
	} else {
		Serial.print("Could not connect to Wi-Fi.");
	}

	// Disconnect from Wi-Fi
	disconnect();

	// Reset JSON object
	resetJson();

	return connected;
}

/**
 * Populates the JSON object with the recorded measurements.
 * @param version: version of the packet encoding
 * @param interval: time interval between two measurements
 * @param measurementsArray: array containing the recorded measurements
 * @param size: number of measurements to consider
 */
void WifiSender::populateJson(uint8_t version, uint8_t interval, measurement_t* measurementsArray, uint8_t size) {
	// Premable: version and interval
	addData("version", version);
	if (version != 1)
		addData("interval", interval);
	// Geolocation data
	if (version == 3) {
		addData("latitude", measurementsArray[size].latitude);
		addData("longitude", measurementsArray[size].longitude);
		addData("altitude", measurementsArray[size].altitude);
	}
	// Measurements
	for (uint8_t i = 0; i <= size; i++) {
		// Timestamp
		if (version != 1)
			newMeasurement(i);
		// Battery [%]
		appendBattery(measurementsArray[i].battery);
		// Temperature
		appendTemperature(measurementsArray[i].temperature);
		// Pressure
		appendPressure(measurementsArray[i].pressure);
		// Humidity
		appendHumidity(measurementsArray[i].humidity);
		// Light
		appendLight(measurementsArray[i].light);
		// Geolocation data
		if (version <= 2) {
			appendLatitude(measurementsArray[i].latitude);
			appendLongitude(measurementsArray[i].longitude);
			appendAltitude(measurementsArray[i].altitude);
		}
		// CO2
		appendCO2(measurementsArray[i].co2);
		// Noise
		appendNoise(measurementsArray[i].noise);
		// Air quality
		appendAirQuality(measurementsArray[i].airQuality);
	}
}

/*
 * Adds data to the JSON object.
 * @param type: type of the data to add
 * @param data: data to add
 */
void WifiSender::addData(String type, float data) {
	if (_jsonStarted)
		_json += ",\n";

	_json += "\"";
	_json += type;
	_json += "\": ";
	_json += String(data);
	_jsonStarted = true;
}

/**
 * Initializes the measurements array.
 */
void WifiSender::startMeasurements() {
	if (_jsonStarted)
		_json += ",\n";

	_json += "\"measurements\": [\n";
}

/**
 * Initializes a new measurement, with the specified timestamp.
 * @param data: value of the timestamp
 */
void WifiSender::newMeasurement(uint8_t timestamp) {
	if (timestamp > _timestamp) {
		if (_timestamp == -1) {
			startMeasurements();
		} else {
			_json += "},\n";
		}
		_json += 	"{";
		_json += "\"timestamp\": ";
		_json += String(timestamp);
		_timestamp = timestamp;
	}
}

/**
 * Appends data to the measurements array in the JSON object.
 * @param type: type of the data
 * @param data: value of the data
 */
void WifiSender::appendData(String type, float data) {
	if (_version == 2) {
		_json += ",\n\"";
		_json += type;
		_json += "\": ";
		_json += String(data);
	} else {
		addData(type, data);
	}
}

/**
 * Appends an int value to the measurements array in the JSON object.
 * @param type: type of the data
 * @param data: value of the data
 */
void WifiSender::appendInt(String type, float data) {
	if (data != DEFAULT_INT)
		appendData(type, data);
}

/**
 * Appends a float value to the measurements array in the JSON object.
 * @param type: type of the data
 * @param data: value of the data
 */
void WifiSender::appendFloat(String type, float data) {
	if (data != DEFAULT_FLOAT)
		appendData(type, data);
}

/**
 * Appends battery percentage data to the measurements array.
 * @param battery: battery percentage data
 */
void WifiSender::appendBattery(uint8_t battery) {
	appendInt("battery", (float) battery);
}

/**
 * Appends temperature data to the measurements array.
 * @param temperature: temperature data
 */
void WifiSender::appendTemperature(float temperature) {
	appendFloat("temperature", temperature);
}

/**
 * Appends pressure data to the measurements array.
 * @param pressure: pressure data
 */
void WifiSender::appendPressure(float pressure) {
	appendFloat("pressure", pressure);
}

/**
 * Appends humidity data to the measurements array.
 * @param humidity: humidity data
 */
void WifiSender::appendHumidity(float humidity) {
	appendFloat("humidity", humidity);
}

/**
 * Appends altitude data to the measurements array.
 * @param altitude: altitude data
 */
void WifiSender::appendAltitude(float altitude) {
	appendFloat("altitude", altitude);
}

/**
 * Appends light data to the measurements array.
 * @param light: light data
 */
void WifiSender::appendLight(uint16_t light) {
	appendInt("light", (float) light);
}

/**
 * Appends latitude data to the measurements array.
 * @param latitude: latitude data
 */
void WifiSender::appendLatitude(float latitude) {
	appendFloat("latitude", latitude);
}

/**
 * Appends longitude data to the measurements array.
 * @param longitude: longitude data
 */
void WifiSender::appendLongitude(float longitude) {
	appendFloat("longitude", longitude);
}

/**
 * Appends CO2 concentration data to the measurements array.
 * @param co2: CO2 concentration data
 */
void WifiSender::appendCO2(float co2) {
	appendFloat("co2", co2);
}

/**
 * Appends noise data to the measurements array.
 * @param noise: noise data
 */
void WifiSender::appendNoise(uint16_t noise) {
	appendInt("noise", (float) noise);
}

/**
 * Appends air quality data to the measurements array.
 * @param noise: air quality data
 */
void WifiSender::appendAirQuality(float airQuality) {
	appendFloat("airQuality", airQuality);
}

/**
 * Prints the JSON object on the serial output.
 */
void WifiSender::printJson() {
	Serial.println(_json);
}

/*
 * Closes the JSON object.
 */
void WifiSender::closeJson() {
	if (_version == 2)
		_json += "\n}\n]";
	_json += "\n}\n}\n}";
}

/*
 * Resets the JSON object.
 */
void WifiSender::resetJson() {
	_json = "{\n\"end_device_ids\": {\n\"dev_eui\": \"";
	_json += _devEUI;
	_json += "\"\n},\n";
	_json += "\"uplink_message\": {\n\"decoded_payload\": {\n";
	_jsonStarted = false;
}
