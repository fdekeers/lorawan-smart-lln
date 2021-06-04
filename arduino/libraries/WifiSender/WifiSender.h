/*
 * Louvain-La-Neuve: a smart city
 * DE KEERSMAEKER François
 * VAN DE WALLE Nicolas
 *
 * Class for sending data to our online platforms using WiFi.
 */

#ifndef WifiSender_h
#define WifiSender_h

#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WifiLocation.h>
#include <Packet.h>

#define WIFI_TIMEOUT_SEC 20
#define MAX_LOC_ACCURACY 500

// Microsoft certificate to authentify our cloud function URL
static const char certificate[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFWjCCBEKgAwIBAgIQDxSWXyAgaZlP1ceseIlB4jANBgkqhkiG9w0BAQsFADBa
MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl
clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw
MDcyMTIzMDAwMFoXDTI0MTAwODA3MDAwMFowTzELMAkGA1UEBhMCVVMxHjAcBgNV
BAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEgMB4GA1UEAxMXTWljcm9zb2Z0IFJT
QSBUTFMgQ0EgMDEwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCqYnfP
mmOyBoTzkDb0mfMUUavqlQo7Rgb9EUEf/lsGWMk4bgj8T0RIzTqk970eouKVuL5R
IMW/snBjXXgMQ8ApzWRJCZbar879BV8rKpHoAW4uGJssnNABf2n17j9TiFy6BWy+
IhVnFILyLNK+W2M3zK9gheiWa2uACKhuvgCca5Vw/OQYErEdG7LBEzFnMzTmJcli
W1iCdXby/vI/OxbfqkKD4zJtm45DJvC9Dh+hpzqvLMiK5uo/+aXSJY+SqhoIEpz+
rErHw+uAlKuHFtEjSeeku8eR3+Z5ND9BSqc6JtLqb0bjOHPm5dSRrgt4nnil75bj
c9j3lWXpBb9PXP9Sp/nPCK+nTQmZwHGjUnqlO9ebAVQD47ZisFonnDAmjrZNVqEX
F3p7laEHrFMxttYuD81BdOzxAbL9Rb/8MeFGQjE2Qx65qgVfhH+RsYuuD9dUw/3w
ZAhq05yO6nk07AM9c+AbNtRoEcdZcLCHfMDcbkXKNs5DJncCqXAN6LhXVERCw/us
G2MmCMLSIx9/kwt8bwhUmitOXc6fpT7SmFvRAtvxg84wUkg4Y/Gx++0j0z6StSeN
0EJz150jaHG6WV4HUqaWTb98Tm90IgXAU4AW2GBOlzFPiU5IY9jt+eXC2Q6yC/Zp
TL1LAcnL3Qa/OgLrHN0wiw1KFGD51WRPQ0Sh7QIDAQABo4IBJTCCASEwHQYDVR0O
BBYEFLV2DDARzseSQk1Mx1wsyKkM6AtkMB8GA1UdIwQYMBaAFOWdWTCCR1jMrPoI
VDaGezq1BE3wMA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYI
KwYBBQUHAwIwEgYDVR0TAQH/BAgwBgEB/wIBADA0BggrBgEFBQcBAQQoMCYwJAYI
KwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTA6BgNVHR8EMzAxMC+g
LaArhilodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vT21uaXJvb3QyMDI1LmNybDAq
BgNVHSAEIzAhMAgGBmeBDAECATAIBgZngQwBAgIwCwYJKwYBBAGCNyoBMA0GCSqG
SIb3DQEBCwUAA4IBAQCfK76SZ1vae4qt6P+dTQUO7bYNFUHR5hXcA2D59CJWnEj5
na7aKzyowKvQupW4yMH9fGNxtsh6iJswRqOOfZYC4/giBO/gNsBvwr8uDW7t1nYo
DYGHPpvnpxCM2mYfQFHq576/TmeYu1RZY29C4w8xYBlkAA8mDJfRhMCmehk7cN5F
JtyWRj2cZj/hOoI45TYDBChXpOlLZKIYiG1giY16vhCRi6zmPzEwv+tk156N6cGS
Vm44jTQ/rs1sa0JSYjzUaYngoFdZC4OfxnIkQvUIA4TOFmPzNPEFdjcZsgbeEz4T
cGHTBPK4R28F44qIMCtHRV55VMX53ev6P3hRddJb
-----END CERTIFICATE-----
)EOF";

class WifiSender {

	public:
		WifiSender();
		WifiSender(const char* ssid,
							 const char* password,
							 const char* googleKey,
							 const char* urlInflux,
							 const char* token,
							 String devEUI);
		void init(const char* ssid,
			        const char* password,
							const char* googleKey,
							const char* urlInflux,
							const char* token,
							String devEUI);
		bool connect();
		void disconnect();
		location_t getLocation();
		bool sendData(uint8_t version, uint8_t interval, measurement_t* measurementsArray, uint8_t size);
		void addData(String type, float data);
		void newMeasurement(uint8_t timestamp);
		void appendData(String type, float data);
		void appendInt(String type, float data);
		void appendFloat(String type, float data);
		void appendBattery(uint8_t battery);
		void appendTemperature(float temperature);
		void appendPressure(float pressure);
		void appendHumidity(float humidity);
		void appendAltitude(float altitude);
		void appendLight(uint16_t light);
		void appendLatitude(float latitude);
		void appendLongitude(float longitude);
		void appendCO2(float co2);
		void appendNoise(uint16_t noise);
		void appendAirQuality(float airQuality);
		void closeJson();
		void printJson();

	private:
		// Wi-Fi parameters
		const char* _ssid;
		const char* _password;
		// InfluxDB parameters
		const char* _urlInflux;
		const char* _token;

		String _devEUI;
		WifiLocation _wifiLocation;
		HTTPClient _http;
		String _json;
		bool _jsonStarted;
		uint8_t _version;
		int8_t _timestamp;

		void populateJson(uint8_t version, uint8_t interval, measurement_t* measurementsArray, uint8_t size);
		void startMeasurements();
		void resetJson();
};

#endif
