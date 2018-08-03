# Amazon Web Services IoT MQTT Subscribe/Publish Example

This is an adaptation of the [AWS IoT C SDK](https://github.com/aws/aws-iot-device-sdk-embedded-C) "subscribe_publish" example for ESP-IDF.

# Configuration

## Configuring Your Device

### Installing Private Key & Certificate

As part of creating a device certificate, you downloaded a Private Key (`xxx-private.pem.key`) and a Certificate file (`xxx-certificate.pem.crt`). These keys need to be loaded by the ESP32 to identify itself.

There are currently two options for how to load the key & cert.

* Embed the files into the app binary (default)
* Load the files from SD card

### Option 1: Embedded Key & Cert into App Binary

Copy the `.pem.key` and `.pem.crt` files to the `main/certs` subdirectory of the example. Rename them by removing the device-specific prefix - the new names are `private.pem.key` and `certificate.pem.crt`.

As these files are bound to your AWS IoT account, take care not to accidentally commit them to public source control. In a commercial IoT device these files would be flashed to the device via a provisioning step, but for these examples they are compiled in.

### Option 2: Loading Key & Cert from SD Card

The alternative to embedding the key and certificate is to load them from a FAT filesystem on an SD card.

Before loading data from SD, format your SD card as FAT and run the `examples/storage/sd_card` example on it to verify that it's working as expected in ESP-IDF. This helps cut down the possible causes of errors in the more complex AWS IoT examples!

Run `make menuconfig`, navigate to "Example Configuration" and change "AWS IoT Certificate Source" to "Load from SD card".

Three new prompts will appear for filenames for the device key, device certificate and root CA certificate path. These paths start with `/sdcard/` as this is where the example mounts the (FAT formatted) SD card.

Copy the certificate and key files to the SD card, and make sure the file names match the names given in the example configuration (either rename the files, or change the config). For the Root CA certificate file (which is not device-specific), you can find the file in the `main/certs` directory or download it from AWS.

*Note: By default, esp-idf's FATFS support only allows 8.3 character filenames. However, the AWS IoT examples pre-configure the sdkconfig to enable long filenames. If you're setting up your projects, you will probably want to enable these options as well (under Component Config -> FAT Filesystem Support). You can also consider configure the FAT filesystem for read-only support, if you don't need to write to the SD card.*

## Find & Set AWS Endpoint Hostname

Your AWS IoT account has a unique endpoint hostname to connect to. To find it, open the AWS IoT Console and click the "Settings" button on the bottom left side. The endpoint hostname is shown under the "Custom Endpoint" heading on this page.

Run `make menuconfig` and navigate to `Component Config` -> `Amazon Web Service IoT Config` -> `AWS IoT MQTT Hostname`. Enter the host name here.

*Note: It may seem odd that you have to configure parts of the AWS settings under Component Config and some under Example Configuration.* The IoT MQTT Hostname and Port are set as part of the component because when using the AWS IoT SDK's Thing Shadow API (in examples or in other projects) the `ShadowInitParametersDefault` structure means the Thing Shadow connection will default to that host & port. You're not forced to use these config values in your own projects, you can set the values in code via the AWS IoT SDK's init parameter structures - `ShadowInitParameters_t` for Thing Shadow API or `IoT_Client_Init_Params` for MQTT API.

# Monitoring MQTT Data from the device

After flashing the example to your ESP32, it should connect to Amazon and start subscribing/publishing MQTT data.

The example code publishes MQTT data to the topic `test_topic/esp32`. Amazon provides a web interface to subscribe to MQTT topics for testing:

* On the AWS IoT console, click "MQTT Client" near the top-right.
* Click "Generate Client ID" to generate a random client ID.
* Click "Connect"

One connection succeeds, you can subscribe to the data published by the ESP32:

* Click "Subscribe to Topic"
* Enter "Subscription Topic" `test_topic/esp32`
* Click "Subscribe"

... you should see MQTT data published from the running example.

To publish data back to the device:

* Click "Publish to Topic"
* Enter "Publish Topic" `test_topic/esp32`
* Enter a message in the payload field
* Click Publish
