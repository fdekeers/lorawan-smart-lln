# Using LoRaWAN and Wi-Fi for smart city monitoring in Louvain-la-Neuve
UCLouvain master's thesis\
François DE KEERSMAEKER & Nicolas VAN DE WALLE

This repository contains the software for our master's thesis:
**Using LoRaWAN and Wi-Fi for smart city monitoring in Louvain-la-Neuve**.

This repository contains the following folders:
- **arduino**: Arduino sketches for our Heltec WiFi Lora 32 end devices,
as well as the needed Arduino libraries
- **functions**: code for our backend cloud platform, i.e. cloud function and InfluxDB database
- **generate-sketch**: python script for the automatic generation of devices Arduino sketches
- **measurements-parser**: data from the energy consumption measurements at Welcome,
with python script to parse and generate graphs
- **the-things-network**: payload formatter used on our The Things Network application
- **web-app**: web application to generate authorization tokens for new devices
