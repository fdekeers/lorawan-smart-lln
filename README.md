# Using LoRaWAN and Wi-Fi for smart city monitoring in Louvain-la-Neuve
UCLouvain master's thesis\
François DE KEERSMAEKER & Nicolas VAN DE WALLE
Supervised by Pr. Yves Deville & Pr. Ramin Sadre

This repository contains the software for our master's thesis:
**Using LoRaWAN and Wi-Fi for smart city monitoring in Louvain-la-Neuve**.

This repository contains the following folders:
- [arduino](./arduino/): Arduino sketches for our Heltec WiFi Lora 32 end devices,
as well as the needed Arduino libraries
- [functions](./functions/): code for our backend cloud platform, i.e. cloud function and InfluxDB database
- [generate-sketch](./generate-sketch/): python script for the automatic generation of devices Arduino sketches
- [measurements-parser](./measurements-parser/): data from the energy consumption measurements at Welcome,
with python script to parse and generate graphs
- [the-things-network](./the-things-network/): payload formatter used on our The Things Network application
- [web-app](./web-app/): web application to generate authorization tokens for new devices
