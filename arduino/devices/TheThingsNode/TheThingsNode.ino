#include <TheThingsNode.h>
#include "Packet.h"

// Set your AppEUI and AppKey
const char *appEui = "appEui";
const char *appKey = "appKey";

#define loraSerial Serial1
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

#define INTERVAL 1
#define N_MEASUREMENTS 3

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
TheThingsNode *node;

uint32_t timestamp;
uint8_t count;
measurement_t measurementsArray[N_MEASUREMENTS];
Packet packet(INTERVAL, N_MEASUREMENTS);

#define PORT_SETUP 1
#define PORT_INTERVAL 2
#define PORT_MOTION 3
#define PORT_BUTTON 4

/*
Decoder payload function
------------------------

function Decoder(bytes, port) {
  var decoded = {};
  var events = {
    1: 'setup',
    2: 'interval',
    3: 'motion',
    4: 'button'
  };
  decoded.event = events[port];
  decoded.battery = (bytes[0] << 8) + bytes[1];
  decoded.light = (bytes[2] << 8) + bytes[3];
  if (bytes[4] & 0x80)
    decoded.temperature = ((0xffff << 16) + (bytes[4] << 8) + bytes[5]) / 100;
  else
    decoded.temperature = ((bytes[4] << 8) + bytes[5]) / 100;
  return decoded;
}
*/

void setup()
{
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  packet.setMeasurementsArray(measurementsArray);

  // Config Node
  node = TheThingsNode::setup();
  node->configLight(true);
  node->configInterval(true, 120000);
  node->configTemperature(true);
  node->onWake(wake);
  node->onInterval(interval);
  node->onSleep(sleep);
  node->onMotionStart(onMotionStart);
  node->onButtonRelease(onButtonRelease);

  // Test sensors and set LED to GREEN if it works
  node->showStatus();
  node->setColor(TTN_GREEN);

  debugSerial.println("-- TTN: STATUS");
  ttn.showStatus();

  debugSerial.println("-- TTN: JOIN");
  ttn.join(appEui, appKey);

  debugSerial.println("-- SEND: SETUP");
  sendData(PORT_SETUP);

}

void loop()
{
  node->loop();
}

void interval()
{
  node->setColor(TTN_BLUE);

  debugSerial.println("-- SEND: INTERVAL");
  sendData(PORT_INTERVAL);
}

void wake()
{
  node->setColor(TTN_GREEN);
}

void sleep()
{
  node->setColor(TTN_BLACK);
}

void onMotionStart()
{
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: MOTION");
  sendData(PORT_MOTION);
}

void onButtonRelease(unsigned long duration)
{
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: BUTTON");
  debugSerial.println(duration);

  sendData(PORT_BUTTON);
}

void sendData(uint8_t port)
{
  // Wake RN2483
  ttn.wake();

  ttn.showStatus();
  node->showStatus();

  // Get the data
  packet.addBattery(node->getBattery(), timestamp, count);
  packet.addLight(node->getLight(), timestamp, count);
  packet.addTemperature(node->getTemperatureAsFloat(), timestamp, count);
  packet.printArray();

  // Send data if needed
  if (count >= N_MEASUREMENTS - 1) {
    uint8_t* payload = packet.buildLoraPayload();
    uint8_t size = packet.getPayloadSize();
    packet.printPayload();
    ttn.sendBytes(payload, size, port);
    count = 0;
  } else {
    count++;
  }
  timestamp++;

  // Set RN2483 to sleep mode
  ttn.sleep(60000);

  // This one is not optional, remove it
  // and say bye bye to RN2983 sleep mode
  delay(50);
}
