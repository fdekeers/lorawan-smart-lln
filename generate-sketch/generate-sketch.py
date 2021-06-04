import sys
import os
import re
import yaml

DEFAULT_VAL = 1
OPTIONAL_KEYS = [
    "nMeasurements",
    "version"
]
ARRAY_KEYS = [
    "license",
    "devEui",
    "appEui",
    "appKey"
]

def deep_get(dict_in, key):
    """
    Searches for a key in a nested dictionary.
    :param dict_in: dictionary to search in
    :param key: key to search for
    :return: the value corresponding to the key
    """
    if type(dict_in) is dict:
        value = dict_in.get(key, None)
        if value is None:
            for k in dict_in.keys():
                value = deep_get(dict_in[k], key)
                if value is not None:
                    return value
            return None
        else:
            return value
    else:
        return None


def is_optional(key):
    """
    Checks if the key is optional.
    :param key: key to check for
    :return: true if the key is optional,
             false otherwise
    """
    return key in OPTIONAL_KEYS


def is_array_key(key):
    """
    Checks if the key corresponds to an array parameter.
    :param key: key to check for
    :return: true if the key corresponds to an array parameter,
             false otherwise
    """
    return key in ARRAY_KEYS


def populate_template(filename, dictionary, array):
    """
    Populates a template file with parameters from a dictionary,
    and write the resulting lines in an array.
    :param filename: name of the template file
    :param dictionary: dictionary containing the parameters
    :param array: array to write the resulting lines into
    """
    with open(f"templates/{filename}", 'r') as template:
        for line in template:
            match = re.search("\{.*\}", line)
            if match is not None:
                key = match.group().strip('{}')
                value = deep_get(dictionary, key)
                if value is None and is_optional(key):
                    value = DEFAULT_VAL
                value = str(value)
                if is_array_key(key):
                    line = re.sub(match.group(), f"{{{value}}}", line)
                else:
                    line = re.sub(match.group(), value, line)
            array.append(line)


# Read command line argument
if len(sys.argv) != 2 or not sys.argv[1].lower().endswith(('.yaml', '.yml')):
    sys.stderr.write("Wrong arguments.\nPlease specify YAML configuration file name.\n")
    exit(-1)
filename = sys.argv[1]

# Read YAML configuration file into dictionary
parameters = {}
with open(filename, 'r') as yaml_file:
    parameters = yaml.safe_load(yaml_file)

# New sketch name
sketch_name = parameters.get("deviceName", "esp32-auto-generated")

# Create folder for new sketch
folder = f"../arduino/devices/{sketch_name}"
if not os.path.exists(folder):
    os.mkdir(folder)

# Write header
sketch_lines = []
populate_template("header.txt", parameters, sketch_lines)

# Write configuration
populate_template("config.txt", parameters, sketch_lines)

# Write sensors
sensors = parameters.get('sensors', [])
if len(sensors) > 0:
    sensors_to_add = []
    for sensor in sensors:
        sensors_to_add.append(sensor['type'])
        populate_template(f"{sensor['type']}.txt", sensor, sketch_lines)

# Write setup
with open("templates/setup.txt", 'r') as setup:
    for line in setup:
        sketch_lines.append(line)

# Write sensors methods
if len(sensors) > 0:
    sensors_methods = {}
    with open("templates/sensors_methods.yaml", 'r') as sensors_methods_file:
        sensors_methods = yaml.safe_load(sensors_methods_file)
    for sensor in sensors_to_add:
        sketch_lines.append(sensors_methods[sensor])

# Close setup
sketch_lines.append("}\n\n")

# Write loop
with open("templates/loop.txt") as loop:
    for line in loop:
        sketch_lines.append(line)

# Flush on new sketch
with open(f"../arduino/devices/{sketch_name}/{sketch_name}.ino", 'w') as sketch:
    sketch.writelines(sketch_lines)
