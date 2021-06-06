/**
 * Louvain-la-Neuve: a smart city
 * DE KEERSMAEKER François
 * VAN DE WALLE Nicolas
 *
 * Class to handle ESP32 devices.
 */

#include "EspDevice.h"

// LoRaWAN parameters
/* ABP para (unused)*/
uint8_t NwkSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AppSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t DevAddr = (uint32_t)0x00000000;
/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_A;
/* Use OTAA over ABP */
bool overTheAirActivation = true;
/*ADR enable*/
bool loraWanAdr = true;
/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = false;
/* Application port */
uint8_t appPort = 2;
/* Number of send trials */
uint8_t confirmedNbTrials = 1;
/* LoRaWAN debug level, set in Arduino settings */
uint8_t debugLevel = LoRaWAN_DEBUG_LEVEL;
/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

// GPS serial
HardwareSerial gpsSerial(1);

// Variables stored in memory
RTC_DATA_ATTR bool _startup = true;
RTC_DATA_ATTR uint8_t _count;
RTC_DATA_ATTR float _latitude = DEFAULT_FLOAT;
RTC_DATA_ATTR float _longitude = DEFAULT_FLOAT;
RTC_DATA_ATTR float _altitude = DEFAULT_FLOAT;
RTC_DATA_ATTR measurement_t _measurementsArray[MAX_MEASUREMENTS];
RTC_DATA_ATTR Packet packet;
RTC_DATA_ATTR uint8_t bme680state[BSEC_MAX_STATE_BLOB_SIZE] = {0};

// BME680 configuration for the least power consumption
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_300s_28d/bsec_iaq.txt"
};
// BME680 virtual sensors list
bsec_virtual_sensor_t bme680SensorList[BME680_SENSORS] = {
	BSEC_OUTPUT_RAW_TEMPERATURE,
	BSEC_OUTPUT_RAW_PRESSURE,
	BSEC_OUTPUT_RAW_HUMIDITY,
	BSEC_OUTPUT_RAW_GAS,
	BSEC_OUTPUT_IAQ,
	BSEC_OUTPUT_STATIC_IAQ,
	BSEC_OUTPUT_CO2_EQUIVALENT,
	BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
	BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
	BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
};

/*
 * Constructor
 * Instantiates an ESP32 device.
 */
EspDevice::EspDevice(uint32_t license[4], uint8_t DevEui[8], uint8_t AppEui[8], uint8_t AppKey[16])
{
	// Initialize device specific values
	for (int i = 0; i < 16; i++)
	{
		if (i < 4)
		{
			_license[i] = license[i];
		}
		if (i < 8)
		{
			_DevEui[i] = DevEui[i];
			_AppEui[i] = AppEui[i];
		}
		_AppKey[i] = AppKey[i];
	}

	// Initialize connection to sensors
	_bme280Connected = false;
	_bme680Connected = false;
	_gasSensorConnected = false;
	_gpsConnected = false;
	_soundSensorConnected = false;
	_isAltitudeGps = false;

	_emergency = false;

	appDataSize = 1;
}

/**
 * Initializes the packet object associated to this device.
 * @param wakeupPeriod: time interval between successive wakeups
 * @param nMeasurements: number of measurements before sending packet
 * @param version: packet format version
 */
void EspDevice::initPacket(uint8_t wakeupPeriod, uint8_t nMeasurements, uint8_t version)
{
	_wakeupPeriod = wakeupPeriod;
	_nMeasurements = nMeasurements;
	packet.init(wakeupPeriod, nMeasurements, version);
	packet.setMeasurementsArray(_measurementsArray);
}

/**
 * Initializes the WifiSender object, with the given parameters.
 * @param ssid: ssid of the Wi-Fi AP to connect to
 * @param password: password of the Wi-Fi AP to connect to
 * @param googleKey: API key for the Google Geolocation API
 * @param urlInflux: URL for the InfluxDB cloud function
 * @param token: authentication token for the cloud function
 */
void EspDevice::initWifi(const char *ssid, const char *password, const char *googleKey, const char *urlInflux, const char *token)
{
	wifi.init(ssid, password, googleKey, urlInflux, token, devEUI_string());
}

/**
 * Applies all the battery-saving features.
 */
void EspDevice::saveBattery()
{
	// Sets CPU frequency to minimum
	setCpuFrequencyMhz(80);
	// Disable Wi-FI interface
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	// Disable Bluetooth interface
	btStop();
}

/**
 * Setup function, must be called when the device starts or wakes up.
 */
void EspDevice::setup()
{
	// Battery saving
	saveBattery();

#if DEBUG
	// Start serial ports
	Serial.begin(SERIAL_BAUD);
#endif

	// Initialize battery pin
	pinMode(BATTERY_PIN, INPUT);
	// Initialize Vext
	pinMode(VEXT_GPIO, OUTPUT);
	digitalWrite(VEXT_GPIO, HIGH);

	/*
	if(mcuStarted==0)
	{
	LoRaWAN.displayMcuInit();
	}
	*/

	SPI.begin(SCK, MISO, MOSI, SS);
	Mcu.init(SS, RST_LoRa, DIO0, DIO1, _license);
	deviceState = DEVICE_STATE_INIT;
}

/**
 * Turns the Vext pin on.
 */
void EspDevice::startVext()
{
	digitalWrite(VEXT_GPIO, LOW);
	delay(10);
}

/**
 * Turns the Vext pin off.
 */
void EspDevice::stopVext()
{
	digitalWrite(VEXT_GPIO, HIGH);
}

/**
 * Adds the BME280 sensor (temperature/pressure/humidity),
 * with the specified SDA and SCL GPIO pins.
 */
void EspDevice::addBme280(uint8_t sda, uint8_t scl,
					   int temperatureThreshold,
					   int pressureThreshold,
					   int humidityThreshold)
{
	Wire.begin(sda, scl);
	_bme280Connected = true;
	_temperatureThreshold = temperatureThreshold;
	_pressureThreshold = pressureThreshold;
	_humidityThreshold = humidityThreshold;
}

/**
 * Starts the BME280 sensor (temperature/pressure/humidity).
 * @returns: true if the sensor was successfully started, false otherwise
 */
bool EspDevice::startBme280()
{
	return bme280.begin(0x76);
}

/**
 * Adds the gas sensor (MQ-135) on the specified analog pin.
 * @param gpio: analog pin where the sensor is wired
 */
void EspDevice::addGasSensor(uint8_t gpio, int co2Threshold)
{
	gasSensor.setPin(gpio);
	_gasSensorConnected = true;
	_co2Threshold = co2Threshold;
}

/**
 * Adds the GPS module.
 * @param gpsSerialBaud: serial baud rate for communication with the GPS
 * @param gpsRx: GPIO pin used as serial RX for communication with the GPS
 * @param gpsRx: GPIO pin used as serial TX for communication with the GPS
 */
void EspDevice::addGPS(uint32_t gpsSerialBaud, uint8_t gpsRx, uint8_t gpsTx)
{
	_gpsSerialBaud = gpsSerialBaud;
	_gpsRx = gpsRx;
	_gpsTx = gpsTx;
	_gpsConnected = true;
}

/**
 * Adds the sound sensor on the specified ADC GPIO pin.
 * @param gpio: ADC GPIO pin where the sound sensor is wired
 */
void EspDevice::addSoundSensor(uint8_t gpio, int noiseThreshold)
{
	soundSensor.setAnalogPin(gpio);
	_soundSensorConnected = true;
	_noiseThreshold = noiseThreshold;
}

/**
 * Adds the BME680 sensor (temperature/pressure/humidity/air quality),
 * with the specified SDA and SCL GPIO pins,
 * and the specified threshold values.
 */
void EspDevice::addBme680(uint8_t sda, uint8_t scl,
                          int temperatureThreshold,
												  int pressureThreshold,
												  int humidityThreshold,
												  int airQualityThreshold)
{
	Wire.begin(sda, scl);
	_bme680Connected = true;
	_temperatureThreshold = temperatureThreshold;
	_pressureThreshold = pressureThreshold;
	_humidityThreshold = humidityThreshold;
	_airQualityThreshold = airQualityThreshold;
}

/**
 * Starts the BME680 sensor (temperature/pressure/humidity/air quality).
 * @returns: true if the sensor was successfully started and had new data,
 *           false otherwise
 */
bool EspDevice::startBme680()
{
	bme680.begin(BME680_I2C_ADDR_SECONDARY, Wire);
	bme680.setConfig(bsec_config_iaq);
	if (!_startup)
		bme680.setState(bme680state);
	bme680.updateSubscription(bme680SensorList, BME680_SENSORS, BSEC_SAMPLE_RATE_ULP);
	return bme680.status == BSEC_OK && bme680.bme680Status == BME680_OK && bme680.run();
}

/**
 * Retrieves device location with Google Geolocation API.
 */
void EspDevice::getWifiLocation()
{
	location_t location = wifi.getLocation();
	if (location.accuracy < MAX_LOC_ACCURACY) {
		_latitude = location.lat;
		_longitude = location.lon;
#if DEBUG
		printValue("Latitude (Wi-Fi)", _latitude);
		printValue("Longitude (Wi-Fi)", _longitude);
#endif
	}
}

/**
 * Warm-up delay with Vext on before reading sensor values.
 * If the GPS is connected, warm up the serial connection with it.
 * If not, simply wait with Vext on.
 * @param time: duration of the warm-up (in ms)
 */
void EspDevice::warmupSensors(uint32_t time)
{
	if (_gpsConnected)
	{
		gpsSerial.begin(_gpsSerialBaud, SERIAL_8N1, _gpsRx, _gpsTx);
		for (int i = 0; i < 10; i++)
		{
			while (gpsSerial.available())
			{
				gps.encode(gpsSerial.read());
			}
			delay(time / 10);
		}
	}
	else
	{
		delay(time);
	}
}

/**
 * Retrieves the percentage of remaining battery.
 */
void EspDevice::getBattery()
{
	// First measurement is always buggy
	float analog = analogRead(BATTERY_PIN);
	analog = analogRead(BATTERY_PIN);
	float m = 100.0 / (((float) MAX_ANALOG) - ((float) MIN_ANALOG));
	float p = -m * ((float) MIN_ANALOG);
	float percentage = m * analog + p;
	uint8_t battery;
	if (percentage >= 100.0)
		battery = 100;
	else if (percentage <= 1.0)
		battery = 1;
	else
		battery = (uint8_t) percentage;
	packet.addBattery(battery, _count);

	#if DEBUG
	printValue("Battery [%]", battery);
	#endif
}

/**
 * Retrieves the values from the BME280 sensor
 * (temperature, pressure, humidity).
 */
void EspDevice::getBme280Values()
{
	// Read values
	float temperature = bme280.readTemperature();
	packet.addTemperature(temperature, _count);
	float pressure = bme280.readPressure();
	packet.addPressure(pressure, _count);
	float humidity = bme280.readHumidity();
	packet.addHumidity(humidity, _count);
#if DEBUG
	printValue("Temperature", temperature);
	printValue("Pressure", pressure);
	printValue("Humidity", humidity);
#endif
	if (!_isAltitudeGps)
	{
		_altitude = bme280.readAltitude(SEALEVELPRESSURE_HPA);
#if DEBUG
		printValue("Altitude (BME280)", _altitude);
#endif
	}
	// Update emergency
	if (temperature >= _temperatureThreshold ||
		pressure >= _pressureThreshold ||
		humidity >= _humidityThreshold)
		_emergency = true;
}

/**
 * Retrieves the noise value from the sound sensor.
 */
void EspDevice::getSoundValue()
{
	uint16_t noise = soundSensor.readAnalogNoise();
	if (noise == 0)
		_soundSensorConnected = false;
	else
	{
		packet.addNoise(noise, _count);
		if (noise >= _noiseThreshold)
			_emergency = true;
	}

#if DEBUG
	printValue("Noise", noise);
#endif
}

/**
 * Retrieves the gas concentration value from the MQ135 gas sensor.
 */
void EspDevice::getGasValue()
{
	if (gasSensor.getRawValue() == 0)
	{
		_gasSensorConnected = false;
#if DEBUG
		Serial.println("Cannot read analog value from gas sensor.");
#endif
	}
	else
	{
		float co2 = gasSensor.getPPM();
		packet.addCO2(co2, _count);
		if (co2 >= _co2Threshold)
			_emergency = true;

#if DEBUG
		printValue("CO2", co2);
#endif
	}
}

/**
 * Retrieves the location values from the GPS.
 */
void EspDevice::getGpsValues()
{
	while (gpsSerial.available())
	{
		gps.encode(gpsSerial.read());
	}
	float latitude = gps.location.lat();
	float longitude = gps.location.lng();
	float altitude = gps.altitude.meters();
	if (latitude != 0.0 || longitude != 0.0)
	{
		_latitude = latitude;
		_longitude = longitude;
#if DEBUG
		printValue("Latitude (GPS)", _latitude);
		printValue("Longitude (GPS)", _longitude);
#endif
	}
	if (altitude > 0)
	{
		_isAltitudeGps = true;
		_altitude = altitude;
#if DEBUG
		printValue("Altitude (GPS)", _altitude);
#endif
	}
	gpsSerial.end();
}

/**
 * Adds the location data (latitude, longitude) to the Packet object.
 */
void EspDevice::addLocationData()
{
	packet.addLatitude(_latitude, _count);
	packet.addLongitude(_longitude, _count);
	packet.addAltitude(_altitude, _count);
}

/**
 * Retrieves the values from the BME280 sensor
 * (temperature, pressure, humidity).
 */
void EspDevice::getBme680Values() {
	// Read values
	float temperature = bme680.temperature;
	packet.addTemperature(temperature, _count);
	float pressure = bme680.pressure;
	packet.addPressure(pressure, _count);
	float humidity = bme680.humidity;
	packet.addHumidity(humidity, _count);
	float airQuality = bme680.iaq;
	packet.addAirQuality(airQuality, _count);
#if DEBUG
	printValue("Temperature", temperature);
	printValue("Pressure", pressure);
	printValue("Humidity", humidity);
	printValue("Air quality", airQuality);
#endif
	if (!_isAltitudeGps)
	{
		computeAltitude(temperature, pressure);
	#if DEBUG
		printValue("Altitude (BME680)", _altitude);
	#endif
	}
	// Update emergency
	if (temperature >= _temperatureThreshold ||
		pressure >= _pressureThreshold ||
		humidity >= _humidityThreshold ||
	  airQuality >= _airQualityThreshold)
		_emergency = true;
	bme680.getState(bme680state);
}

/**
 * Computes the approximate altitude based on the measured pressure and temperature.
 */
void EspDevice::computeAltitude(float temperature, float pressure) {
	float seaLevelPressure = SEALEVELPRESSURE_HPA * 100.0;
	float altitude = pow((seaLevelPressure/pressure), (1/5.257)) - 1;
	altitude = (altitude * (temperature + 273.15)) / 0.0065;
	_altitude = altitude;
}

/*
 * Retrieves values from the connected sensors.
 */
void EspDevice::getValues()
{
	// Battery level
	getBattery();

	if (_bme280Connected || _soundSensorConnected || _gpsConnected || _gasSensorConnected)
		startVext();
	if (_gpsConnected || _gasSensorConnected)
		warmupSensors(VEXT_DELAY_SEC * 1000);

	// Sound sensor
	if (_soundSensorConnected)
		getSoundValue();

	// GPS
	if (_gpsConnected) {
		getGpsValues();
	}

	// BME280
	if (_bme280Connected && startBme280())
		getBme280Values();

	// BME680
	if (_bme680Connected && startBme680())
		getBme680Values();

	addLocationData();

	// MQ135 (gas concentration)
	if (_gasSensorConnected)
		getGasValue();

	stopVext();
}

/**
 * Encodes and sends the LoRa packet, based on the measured values.
 */
void EspDevice::sendLora()
{
	packet.buildLoraPayload(appData, &appDataSize);

#if DEBUG
	printValue("appDataSize", appDataSize);
	packet.printPayload();
#endif

	//LoRaWAN.displaySending();
	LoRaWAN.send(loraWanClass);
}

/*
 * Encodes the JSON object based on the measured values,
 * and sends it over Wi-Fi to InfluxDB.
 */
void EspDevice::sendWifi()
{
	uint8_t version = packet.getVersion();
	measurement_t *measurementsArray = packet.getMeasurementsArray();
	uint8_t size = packet.getIndex();
	wifi.sendData(version, _wakeupPeriod, measurementsArray, size);
	packet.clearArray();
}

/**
 * Loop function, called indefinitely when the device is running.
 */
void EspDevice::loop()
{
	switch (deviceState)
	{
	case DEVICE_STATE_INIT:
	{
		LoRaWAN.init(loraWanClass, loraWanRegion);
		break;
	}
	case DEVICE_STATE_JOIN:
	{
		// Code to be run only when the device starts for the first time
		packet.clearArray();
		getWifiLocation();
		//LoRaWAN.displayJoining();
		LoRaWAN.join();
		break;
	}
	case DEVICE_STATE_SEND:
	{
		if (_startup) {
			_startup = false;
		}
		else
		{
			getValues();
	#if DEBUG
			packet.printArray();
	#endif
			if (_emergency) {
				sendWifi();
				_count = 0;
			}
			else if (_count >= _nMeasurements - 1)
			{
				sendLora();
				_count = 0;
			}
			else
			{
				_count++;
			}
		}

		deviceState = DEVICE_STATE_CYCLE;
		break;
	}
	case DEVICE_STATE_CYCLE:
	{
		// Schedule next packet transmission
		txDutyCycleTime = _wakeupPeriod * 60000 + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
		LoRaWAN.cycle(txDutyCycleTime);
		deviceState = DEVICE_STATE_SLEEP;
		break;
	}
	case DEVICE_STATE_SLEEP:
	{
		//LoRaWAN.displayAck();
		LoRaWAN.sleep(loraWanClass, debugLevel);
		break;
	}
	default:
	{
		deviceState = DEVICE_STATE_INIT;
		break;
	}
	}
}

/**
 * Prints a value on the serial port.
 * @name: name of the value to print, will be printed before the value
 * @value: value to be printed
 */
void EspDevice::printValue(char *name, float value)
{
	Serial.print(name);
	Serial.print(": ");
	Serial.println(value);
}

/**
 * Converts the DevEUI array into a string.
 * @returns: the DevEUI in string format
 */
String EspDevice::devEUI_string()
{
	String string = "";
	for (int i = 0; i < 8; i++)
	{
		uint8_t num = _DevEui[i];
		if (num <= 0xF)
		{
			string += String(0);
		}
		string += String(num, HEX);
	}
	return string;
}

/**
 * Displays a value on the OLED display.
 * @name: name of the value to print, will be printed before the value
 * @value: value to be printed
 */
void EspDevice::displayValue(char *name, float value)
{
	startVext();
	delay(20);
	Display.init();
	delay(20);
	Display.wakeup();
	/*
	Display.flipScreenVertically();
	Display.setFont(ArialMT_Plain_16);
	Display.setTextAlignment(TEXT_ALIGN_CENTER);
	*/
	Display.clear();
	Display.drawString(10, 10, name);
	Display.drawString(20, 20, String(value));
	Display.display();
	delay(5000);
	Display.sleep();
	stopVext();
}
