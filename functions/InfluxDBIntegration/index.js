const jwt = require('jsonwebtoken');
var firebase = require('firebase/app');
require('firebase/firestore');

const {
    InfluxDB,
    Point,
    HttpError,
    AbortError,
    RequestTimedOutError,
} = require('@influxdata/influxdb-client');

const tokenValidationSecret = 'Insert secret here';

const influxDBURL = 'http://lln-smart-city.westeurope.cloudapp.azure.com:8086/';
const influxDBToken =
    'Insert token here';
const influxDBOrg = 'lln-smart-city';
const influxDBBucket = 'lln-smart-city';

const allowedDataTypes = {
    temperature: 'float',
    pressure: 'float',
    humidity: 'float',
    co2: 'float',
    co: 'float',
    light: 'float',
    battery: 'float',
    event: 'string',
    altitude: 'float',
    no2: 'float',
    noise: 'float',
    airQuality: 'float',
};

var firebaseConfig = {
    apiKey: 'Insert Firebase API key here',
    authDomain: 'lln-smart-city.firebaseapp.com',
    projectId: 'lln-smart-city',
    storageBucket: 'lln-smart-city.appspot.com',
    messagingSenderId: '699639001376',
    appId: '1:699639001376:web:bdbd687ce54d9b0dc1a745',
};

firebase.initializeApp(firebaseConfig);

const validateToken = async (context, req) => {
    context.log('Verifying authorization token');
    const authorizationToken = req.headers['authorization'];
    const token = jwt.verify(authorizationToken, tokenValidationSecret);
    context.log('Token signature: CHECK');

    const tokenDocument = await firebase
        .firestore()
        .collection('tokens')
        .doc(token.id)
        .get();
    if (
        tokenDocument.exists &&
        tokenDocument.data().token === authorizationToken
    ) {
        context.log('Token validity: CHECK');
        context.log('Token validated');
    } else {
        context.log('Token validity: ERROR');
        throw Error('The token is not valid (anymore)');
    }
};

const getRequestData = (context, req) => {
    let deviceId;
    let payload;
    let metadata;
    if ('hardware_serial' in req.body) {
        // TTNv2
        context.log('Payload version: TTNv2');
        context.log(
            'This payload format should not be used anymore. Consider updating to v2.',
        );
        deviceId = req.body.hardware_serial.toLowerCase();
        payload = req.body.payload_fields || {};
        metadata = req.body.metadata || {};
    } else if (
        'end_device_ids' in req.body &&
        'dev_eui' in req.body.end_device_ids
    ) {
        // TTNv3
        context.log('Payload version: TTNv3');
        deviceId = req.body.end_device_ids.dev_eui.toLowerCase();
        payload = req.body.uplink_message.decoded_payload;
        if (
            'locations' in req.body.uplink_message &&
            'user' in req.body.uplink_message.locations
        ) {
            metadata = req.body.uplink_message.locations.user;
        } else {
            metadata = {};
        }
    } else {
        throw Error('Malformed request body');
    }
    return {
        deviceId,
        payload,
        metadata,
        version: payload.version || 1,
    };
};

const getFirebaseDeviceInfo = async (context, deviceId) => {
    context.log('Retrieving Firebase device information');
    const deviceDocument = await firebase
        .firestore()
        .collection('devices')
        .doc(deviceId)
        .get();
    let deviceDisplayName = deviceId;
    let firebaseLocation = {
        longitude: 0,
        latitude: 0,
    };

    if (deviceDocument.exists) {
        deviceDisplayName =
            deviceDocument.data().displayName || deviceDisplayName;
        firebaseLocation = deviceDocument.data().location || firebaseLocation;
        context.log('Device display name:', deviceDisplayName);
    }
    return {
        deviceDisplayName,
        firebaseLocation,
    };
};

const getLocation = (
    metadata,
    payload,
    firebaseLocation,
    batchLocation = {},
) => {
    const hasMetadataLocation =
        'longitude' in metadata && 'latitude' in metadata;
    const hasBatchLocation =
        'longitude' in batchLocation && 'latitude' in batchLocation;
    const hasPayloadLocation =
        'longitude' in payload &&
        'latitude' in payload &&
        (payload.longitude !== 0 || payload.latitude !== 0);
    const hasFirebaseLocation =
        'longitude' in firebaseLocation &&
        'latitude' in firebaseLocation &&
        (firebaseLocation.longitude !== 0 || firebaseLocation.latitude !== 0);

    let location = {
        longitude: 0,
        latitude: 0,
    };

    let locationSource = 'DEFAULT';

    if (hasPayloadLocation) {
        locationSource = 'PAYLOAD';
        location = {
            longitude: payload['longitude'],
            latitude: payload['latitude'],
        };
    } else if (hasBatchLocation) {
        locationSource = 'PAYLOAD_BATCH';
        location = {
            longitude: batchLocation['longitude'],
            latitude: batchLocation['latitude'],
        };
    } else if (hasFirebaseLocation) {
        locationSource = 'FIREBASE';
        location = {
            longitude: firebaseLocation['longitude'],
            latitude: firebaseLocation['latitude'],
        };
    } else if (hasMetadataLocation) {
        locationSource = 'TTN_METADATA';
        location = {
            longitude: metadata['longitude'],
            latitude: metadata['latitude'],
        };
    }

    return { ...location, locationSource };
};

const writeData = (
    context,
    writeApi,
    payload,
    longitude,
    latitude,
    timestamp,
) => {
    timestamp = timestamp || new Date();
    Object.keys(payload).forEach((dataType) => {
        if (dataType in allowedDataTypes) {
            context.log(
                'Adding data [' +
                    dataType +
                    '] with value: ' +
                    payload[dataType],
            );

            const point = new Point(dataType);

            if (allowedDataTypes[dataType] === 'float') {
                point.floatField('value', payload[dataType]);
            } else if (allowedDataTypes[dataType] === 'string') {
                point.stringField('value', payload[dataType]);
            } else {
                return;
            }

            point
                .floatField('longitude', longitude)
                .floatField('latitude', latitude);

            point.timestamp(timestamp);
            writeApi.writePoint(point);
        }
    });
};

const processBatchMeasurements = (
    context,
    writeApi,
    metadata,
    payload,
    firebaseLocation,
) => {
    const maxTimestamp = Math.max.apply(
        Math,
        payload.measurements.map((o) => o.timestamp),
    );

    context.log('Inserting data from batch into influxDB at :', influxDBURL);

    const batchLocation = {};
    if ('longitude' in payload && 'latitude' in payload) {
        batchLocation['latitude'] = payload.latitude;
        batchLocation['longitude'] = payload.longitude;
    }
    context.log('Batch Location', batchLocation);

    payload.measurements.forEach((measurement) => {
        const timestamp = new Date();
        timestamp.setMinutes(
            timestamp.getMinutes() -
                payload.interval * (maxTimestamp - measurement.timestamp),
        );

        context.log('Timestamp:', timestamp);

        const { longitude, latitude, locationSource } = getLocation(
            metadata,
            measurement,
            firebaseLocation,
            batchLocation,
        );

        context.log(
            `Location from ${locationSource}: ${latitude},${longitude}`,
        );

        writeData(
            context,
            writeApi,
            measurement,
            longitude,
            latitude,
            timestamp,
        );
        context.log('===');
    });
};

const processSingleMeasurement = (
    context,
    writeApi,
    metadata,
    payload,
    firebaseLocation,
) => {
    const { longitude, latitude, locationSource } = getLocation(
        metadata,
        payload,
        firebaseLocation,
    );

    context.log('Inserting data into influxDB at :', influxDBURL);
    context.log(`Location from ${locationSource}: ${latitude},${longitude}`);

    writeData(context, writeApi, payload, longitude, latitude);
};

module.exports = async function (context, req) {
    context.log('Invoked InfluxDBIntegration function');
    context.log('Body', req.body);

    try {
        await validateToken(context, req);

        const { deviceId, payload, metadata, version } = getRequestData(
            context,
            req,
        );

        context.log('Device ID:', deviceId);
        context.log('Payload version:', version);

        const {
            deviceDisplayName,
            firebaseLocation,
        } = await getFirebaseDeviceInfo(context, deviceId);

        const writeApi = new InfluxDB({
            url: influxDBURL,
            token: influxDBToken,
        }).getWriteApi(influxDBOrg, influxDBBucket);
        writeApi.useDefaultTags({ deviceId, deviceDisplayName });

        if (version === 1) {
            context.log('Payload version', version, '(single measurement)');
            processSingleMeasurement(
                context,
                writeApi,
                metadata,
                payload,
                firebaseLocation,
            );
        } else if (version === 2) {
            context.log('Payload version', version, '(batch measurements)');
            processBatchMeasurements(
                context,
                writeApi,
                metadata,
                payload,
                firebaseLocation,
            );
        } else if (version === 3) {
            context.log(
                'Payload version',
                version,
                '(optimized batch measurements)',
            );
            processBatchMeasurements(
                context,
                writeApi,
                metadata,
                payload,
                firebaseLocation,
            );
        }

        await writeApi.close();

        context.res = {
            body: 'Done',
        };
        context.log('End of execution');
    } catch (e) {
        let status = 500;
        let error = 'An error occured';

        if (e instanceof jwt.JsonWebTokenError) {
            context.log('Invalid token');
            status = 401;
            error = 'Invalid authorization token. Operation denied';
        } else if (e instanceof HttpError) {
            error =
                'An HTTP error occured while contacting the InfluxDB database';
        } else if (e instanceof AbortError) {
            error = 'The connection with InfluxDB was aborted';
        } else if (e instanceof RequestTimedOutError) {
            error = 'The connection with the InfluxDB database timed out';
        }

        context.log('An error occured');
        context.log(e);

        context.res = {
            status: status /* Defaults to 200 */,
            body: error,
        };
    }
};
