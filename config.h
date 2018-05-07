/*
 * config.h
 *
 *  Created on: 1 May 2018
 *      Author: John
 */

#ifndef CONFIG_H_
#define CONFIG_H_


/*CONGIGURATION */

//Wifi settings
#define ssid "The Internet V3"
#define password "*"
#define wifiTimeout 20000

//Certificates
const char* clientCertificateFile = "/cert.der";
const char* privateKeyFile = "/private.der";
const char* CAFile = "/CA.der";


//AWS settings
#define AWS_endpoint "*.iot.eu-west-1.amazonaws.com"
#define mqttPortNumber 8883

//Laser distance settings
#define SDA_pin 5
#define SLC_pin 4
#define TRIGGER_pin 16

// Display stuff
const uint8_t displayAddress = 0x3c;
const int tankFull = 100;
const int tankEmpty = 1000;

//Time
const char* ntpServerName1 = "0.uk.pool.ntp.org";
const char* ntpServerName2 = "uk.pool.ntp.org";
const char* ntpServerName3 = "pool.ntp.org";
const long timeZoneOffset = 0; // GMT (in seconds)
const int DSToffset = 3600; // Daylight saving offset (in seconds)


#endif /* CONFIG_H_ */
