// Header values definitions
var HEADER_BATTERY     = 0x01;
var HEADER_TEMPERATURE = 0x02;
var HEADER_PRESSURE    = 0x03;
var HEADER_HUMIDITY    = 0x04;
var HEADER_ALTITUDE    = 0x05;
var HEADER_LIGHT       = 0x06;
var HEADER_LATITUDE    = 0x07;
var HEADER_LONGITUDE   = 0x08;
var HEADER_CO2         = 0x09;
var HEADER_NOISE       = 0x0A;
var HEADER_AIR_QUALITY = 0x0B;

// Entry function
function decodeUplink(input) {
  // Match on version number
  switch (input.bytes[0]) {
    case 1:
      return {
          data: decoder_v1(input.bytes),
          warnings: [],
          errors: []
        };
    case 2:
    case 3:
      return {
          data: decoder_v2(input.bytes),
          warnings: [],
          errors: []
        };
    default:
      // Unknown version, drop packet
      return {};
  }
}

/**
 * Payload decoder, V1
 * @param bytes: payload received
 * @returns: decoded payload
 */
function decoder_v1(bytes) {
  var decoded = {version: 1};
  var i = 1;
  while (i < bytes.length) {
    var obj = readValue(bytes, i);
    i += obj.size;
    mergeObjects(decoded, obj);
  }
  return decoded;
}

/**
 * Payload decoder, V2
 * @param bytes: payload received
 * @returns: decoded payload
 */
function decoder_v2(bytes) {
  var decoded = {
    version: bytes[0],
    interval: bytes[1],
    measurements: []
  };
  var payload_idx = 2;
  var decoded_idx = 0;
  while (isFixedHeader(bytes[payload_idx])) {
    var obj = readValue(bytes, payload_idx);
    payload_idx += obj.size;
    mergeObjects(decoded, obj);
  }
  while (payload_idx < bytes.length) {
    // Add timestamp
    decoded.measurements.push({timestamp: bytes[payload_idx]});
    payload_idx++;
    // Add measurements
    while (bytes[payload_idx] != 0) {
      var obj = readValue(bytes, payload_idx);
      payload_idx += obj.size;
      mergeObjects(decoded.measurements[decoded_idx], obj);
    }
    decoded_idx++;
    payload_idx++;
  }
  return decoded;
}

/**
 * Rebuilds an int of a specific size, at a specific index of the payload.
 * @param bytes: payload, represented by an array of bytes
 * @param index: index at which the int data is in the payload
 * @param size: size of the int in bytes
 * @return: the int number
 */
function rebuildInt(bytes, index, size) {
  var data = 0;
  for (var i = 0; i < size; i++) {
    var shift = bytes[index+i] << (8*(size-1-i));
    data += shift;
  }
  return data;
}

/**
 * Rebuilds a float of a specific size, at a specific index of the payload.
 * Float data is multiplied by a power of 10 before being put in the payload,
 * to convert it to an int without losing the decimal places.
 * @param bytes: payload, represented by an array of bytes
 * @param index: index at which the float data is in the payload
 * @param precision: number of decimal places
 * @return: the float number
 */
function rebuildFloat(bytes, index, precision) {
  var factor = Math.pow(10, precision);
  return rebuildInt(bytes, index, 4) / factor;
}

/**
 * Checks if the given header represents data sent only once per batch.
 * @param header: header of the value
 * @return: true if the header represents said data, false otherwise
 */
function isFixedHeader(header) {
  return (header == HEADER_LATITUDE ||
          header == HEADER_LONGITUDE ||
          header == HEADER_ALTITUDE);
}

/**
 * Reads the value at a specific index of the payload.
 * @param bytes: payload, represented by an array of bytes
 * @param index: index where the value to read is in the payload
 * @return: a JSON object representing the value
 */
function readValue(bytes, index) {
  var size;
  obj = {};
  var data_index = index + 1;
  switch (bytes[index]) {
    case HEADER_BATTERY: // Battery percentage, 8-bit int
      size = 1;
      obj.battery = rebuildInt(bytes, data_index, size);
      break;
    case HEADER_TEMPERATURE: // Temperature, 32-bit float
      size = 4;
      var precision = 2;
      obj.temperature = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_PRESSURE: // Pressure, 32-bit float
      size = 4;
      var precision = 2;
      obj.pressure = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_HUMIDITY: // Humidity, 32-bit float
      size = 4;
      var precision = 2;
      obj.humidity = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_ALTITUDE: // Altitude, 32-bit float
      size = 4;
      var precision = 2;
      obj.altitude = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_LIGHT: // Light, 16-bit int
      size = 2;
      obj.light = rebuildInt(bytes, data_index, size);
      break;
    case HEADER_LATITUDE: // Latitude, 32-bit float
      size = 4;
      var precision = 6;
      obj.latitude = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_LONGITUDE: // Longitude, 32-bit float
      size = 4;
      var precision = 6;
      obj.longitude = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_CO2: // CO2 concentration, 32-bit float
      size = 4;
      var precision = 2;
      obj.co2 = rebuildFloat(bytes, data_index, precision);
      break;
    case HEADER_NOISE: // Noise, 16-bit int
      size = 2;
      obj.noise = rebuildInt(bytes, data_index, size);
      break;
    case HEADER_AIR_QUALITY: // Air quality, 32-bit float
      size = 4;
      var precision = 2;
      obj.airQuality = rebuildFloat(bytes, data_index, precision);
      break;
    default:
      size = 0;
  }
  obj.size = size + 1;
  return obj;
}

/**
 * Adds the fields of obj_2 to obj_1.
 * Doesn't add the field "size".
 * @param obj_1: first object
 * @param obj_2: second object
 */
function mergeObjects(obj_1, obj_2) {
  for (var field in obj_2) {
    if (field != "size")
      obj_1[field] = obj_2[field];
  }
}
