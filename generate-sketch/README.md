# Automatic generation of end devices sketches

This directory contains the necessary files for the automatic generation of Arduino sketches for WiFi LoRa 32 end devices. The configuration of an end device can be described in the form of a YAML file, that will be used to create a valid Arduino sketch for this device, thanks to this script.

The Python script is available in the file [generate-sketch.py](./generate-sketch.py). It takes one command line argument: the name of the YAML configuration file for the device. To use the script, run the following command in this folder:
```shell
python3 generate-sketch.py YAML_FILE
```
Python version 3+, as well as the packages `sys`, `os`, `re`, and `PyYAML`, are required. The first three are part of Python default library, and can normally be used natively, and `PyYAML` can be installed with the following command:
```shell
pip install pyyaml
```

The [templates](./templates) directory contains various templates that are used and filled by the script, based on the configuration file, to produce the Arduino sketch.
To accommodate new sensors, their templates must be provided into this folder, following the structure of the already present sensors. In this file, the placeholders must have the same name as their counterparts in the YAML configuration file.
The method to add the connection between the sensor and the device must then be added into the [sensors_methods.txt](sensors_methods.txt) file.

The [devices](./devices) directory contains sample YAML configuration files, that can be used as example for new devices.
