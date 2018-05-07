#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FS.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <time.h>
#include <math.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#include "IOTThing.h"
#include "config.h"

time_t lastTrigger ;
time_t lastSubscription;

void callback(char* topic, byte* payload, unsigned int length) {
	//TODO: process messages from subscribed topics

	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.println("] ");

	lastSubscription = time(nullptr);

	for (int i = 0; i < length; i++)
		Serial.print((char)payload[i]);

	Serial.println();
}

WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, mqttPortNumber, callback, espClient);
char* clientName;

IOTThing pooTank("PooTank");
bool triggerStateOn = false;

SSD1306Wire  display(displayAddress, SDA_pin, SLC_pin);

char* startupScreenText[5];

void displayStartupScreen(int attempts){

	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	display.clear();

	for(int x = 0; x < 5; x++){
		Serial.println(x);
		display.drawString(0, x * 11, String(startupScreenText[x]));
	}

	if (attempts > 0 ){
		display.drawRect(0,58,125,63);

		for(int y = 0; y < 1 + (attempts % 8); y++){
			display.fillRect((y * 16) + 2, 60, 12, 2);
		}
	}
	display.display();

}

void connectToWifi(){
	//TODO: store wifi credentials in eeprom
	// https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/libraries/DNSServer/examples/CaptivePortalAdvanced/credentials.ino
	Serial.println("a");

	startupScreenText[0] = "Starting Wifi...";
	int counter = 0;
	Serial.println("b");
	displayStartupScreen(counter);
	Serial.println("c");

	WiFi.begin(ssid, password);
	WiFi.mode(WIFI_STA); //turn off AP
	long startTime = millis();

	while (WiFi.status() != WL_CONNECTED and startTime+wifiTimeout >= millis()) {
		delay(100);
		counter++;
		displayStartupScreen(counter);
	}

	if (WiFi.status() != WL_CONNECTED) {
		//errorMsg = "Wifi Timout";
		Serial.println("time out");
		//return false;
	}
	else{
		startupScreenText[0] = "Starting Wifi... WiFi connected OK.";
		displayStartupScreen(0);

		Serial.println("WiFi connected ok.");
	}

	clientName = "esp8266-AWS_IOT_hub";

}

void getInternetTime(){

	startupScreenText[1] = "Getting Internet time...";
	int counter = 0;
	displayStartupScreen(counter);

	configTime(timeZoneOffset, DSToffset, ntpServerName1, ntpServerName2, ntpServerName3);
	while (time(nullptr)<30000) {
		counter++;
		displayStartupScreen(counter);
		delay(50);
	}

}

void loadcerts(){

	char* errorMsg = "";

	startupScreenText[2] = "Loading certs. ";
	int counter = 0;
	displayStartupScreen(counter++);

	if (!SPIFFS.begin()) {
		errorMsg = "Failed to mount file system";
		Serial.println(errorMsg); return;
	}
	displayStartupScreen(counter++);

	// Load client certificate file from SPIFFS
	File cert = SPIFFS.open(clientCertificateFile, "r");
	//File cert = SPIFFS.open("/cert.der", "r");
	if (!cert) {
		errorMsg = "Failed to open client certificate " ;
		Serial.println(errorMsg); return;
	}
	displayStartupScreen(counter++);

	// Set client certificate
	if (!espClient.loadCertificate(cert)){
		errorMsg = "Client certificate not loaded";
		Serial.println(errorMsg); return;
	}

	startupScreenText[2] = "Loading certs. CL.ok";
	displayStartupScreen(counter++);

	//delay(500);

	// Load client private key file from SPIFFS
	File private_key = SPIFFS.open(privateKeyFile, "r");
	//	File private_key = SPIFFS.open("/private.der", "r");
	if (!private_key) {
		errorMsg = "Failed to open private key file" ;
		Serial.println(errorMsg); return;
	}
	displayStartupScreen(counter++);

	// Set client private key
	if (!espClient.loadPrivateKey(private_key)){
		errorMsg ="Private key not loaded";
		Serial.println(errorMsg); return;
	}
	displayStartupScreen(counter++);

	startupScreenText[2] = "Loading certs. CL.ok PK.ok ";
	displayStartupScreen(counter++);

	//delay(1000);

	// Load CA file from SPIFFS
	File ca = SPIFFS.open(CAFile, "r");
	//	File ca = SPIFFS.open("/CA.der", "r");
	if (!ca) {
		errorMsg = "Failed to open CA file";
		Serial.println(errorMsg); return;
	}
	displayStartupScreen(counter++);

	// Set server CA file
	if (!espClient.loadCACert(ca)){
		errorMsg ="CA not loaded";
		Serial.println(errorMsg); return;
	}

	startupScreenText[2] = "Certs. CL.ok PK.ok CA.ok";
	displayStartupScreen(counter++);

	Serial.println("Certificates loaded ok...");
}


int pooTank_getLevel(){
	//TODO: maybe put Thing specific stuff in another class?
	const int numReadings = 10;
	long int total = 0;
	int average = 0;
	int goodReadings=0;
	int maxReading=0;
	int minReading=10000;

	VL53L0X sensor;
	int level;
	Serial.println("xxx");

	//todo: check laser sensor i2c address

	Wire.begin(SDA_pin, SLC_pin);
	sensor.init();

	sensor.setTimeout(500);
	Serial.println("???");
	sensor.setMeasurementTimingBudget(200000);
	Serial.println("xxx");

	for (int thisReading = 0; thisReading < numReadings; thisReading++) {

		level = sensor.readRangeSingleMillimeters();
		if (sensor.timeoutOccurred()) {
			level = 0;
		}
		goodReadings++;
		total += level;

		if(level > maxReading) maxReading = level;
		if(level < minReading) minReading = level;

	}
	Serial.println("xxx");

	average = (total - minReading - maxReading) / (goodReadings - 2);

	return average;

}

void pooTankSetup(){

	startupScreenText[3] = "Setting up Thing shadows...";
	int counter = 0;
	displayStartupScreen(counter);
	Serial.println("aaa");

	//TODO: get thing shadow on setup?

	pooTank.Config_StatusLabel="Tank Level";
	pooTank.Config_LevelUnits="mm";
	pooTank.Config_Full = tankFull;
	pooTank.Config_Zero = tankEmpty;

	pooTank.level_actual = pooTank_getLevel();


}

void checkThingTriggers (){

	if(triggerStateOn and digitalRead(TRIGGER_pin) == LOW){ //only fire when trigger is released
		int level = pooTank_getLevel(); //TODO: take multiple readings and average them
		if(level >= 0){

			time(&lastTrigger);

			pooTank.level_actual = level;
			pooTank.publishUpdate(client);
			triggerStateOn = false;
		}
	}

	if (digitalRead(TRIGGER_pin) == HIGH) triggerStateOn = true;


}

void subscribeToTopics(){
	//TODO: create array of subscriptions / maybe config file

	//TODO: create a Thing update trigger topic

	if (client.connected()){
		client.subscribe("$aws/things/+/shadow/update/delta");
		client.loop();
	}
}


void reconnect(){

	startupScreenText[4] = "Starting MQTT client.";
	int counter = 0;
	displayStartupScreen(counter++);

	// Loop until we're reconnected
	while (!client.connected()) {
		// Attempt to connect
		if (client.connect(clientName)) {

			client.loop();

			startupScreenText[4] = "Starting MQTT client. Subscriptions.";
			displayStartupScreen(0);
			subscribeToTopics();

			Serial.println("MQTT client setup and listening..."); Serial.println();
			startupScreenText[4] = "MQTT client setup and listening...";
			displayStartupScreen(0);

		} else {

			Serial.print("failed, rc=");
			Serial.println(client.state());
			// Wait 5 seconds before retrying
			delay(1000);
		}
		displayStartupScreen(counter++);

	}
}


void WriteScreenInfo(){

	display.clear();

	time_t now;
	struct tm * timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	char out[20];
	strftime(out, 20, "%H:%M:%S", timeinfo );// http://www.cplusplus.com/reference/ctime/strftime/

//If triggered, display
	int triggerCountdown = now - lastTrigger;
	if(triggerCountdown < 10){
		display.fillCircle(12,24,10 - triggerCountdown);
	}
//If mqtt message recieved, display
	int subscriptionCountdown = now - lastSubscription;
	if(subscriptionCountdown < 10){
		display.fillCircle(115,24,10 - subscriptionCountdown);
	}

//Pulse
//	if(timeinfo->tm_sec % 5 == 0) display.drawCircle(122,25,2);

//Day of week
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	strftime(out, 20, "%a", timeinfo );// http://www.cplusplus.com/reference/ctime/strftime/
	display.drawString(0, 0, String(out));

//Date
	display.setTextAlignment(TEXT_ALIGN_RIGHT);

	strftime(out, 20, "%e %b %Y", timeinfo );// http://www.cplusplus.com/reference/ctime/strftime/
	display.drawString(128, 0, String(out));

//Time
	if(timeinfo->tm_sec % 5 == 0) // flashing semi-colon pulse
		strftime(out, 20, "%H %M", timeinfo );// http://www.cplusplus.com/reference/ctime/strftime/
	else
		strftime(out, 20, "%H:%M", timeinfo );
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.setFont(ArialMT_Plain_24);
	display.drawString(64, 12, String(out));

//Tank progress bar
	int progress = map(pooTank.level_actual , pooTank.Config_Zero, pooTank.Config_Full, 0, 100);

	display.drawProgressBar(0, 40, 120, 6, progress);

// Actual Level
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 48, String(pooTank.level_actual) + String(pooTank.Config_LevelUnits));

//Percentage full
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(128, 48, String(progress) + "%");

	display.display();

}


void setup()
{
	Serial.begin(115200);
	Serial.println();
	Serial.println("**************************************************");
	Serial.println();
	Serial.println();

	//iotjsontest();

	display.init();
	display.flipScreenVertically();
	//	Serial.println("1");

	connectToWifi();
	//	Serial.println("2");

	getInternetTime();
	//	Serial.println("3");

	loadcerts();
	//	Serial.println("4");

	pooTankSetup();
	//	Serial.println("5");

	pinMode(TRIGGER_pin, INPUT_PULLDOWN_16);
	//	Serial.println("6");

}



void loop()
{

	if (!client.connected()) {
		reconnect();
	}

	checkThingTriggers();

	WriteScreenInfo();

	client.loop();
	delay(500);

}
