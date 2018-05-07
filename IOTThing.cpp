/*
 * IOTThing.cpp
 *
 *  Created on: 1 May 2018
 *      Author: John
 */

#include "Arduino.h"
#include "IOTThing.h"
#include "ArduinoJson.h"
#include <PubSubClient.h>
#include <time.h>


IOTThing::IOTThing(char* thingName) {
	name = thingName;
	status = "";
	level_percent=0;
	level_actual=0;
	AlarmActivated = false;
	Config_Full=100;
	Config_Zero=0;
	Config_LevelUnits="";
	Config_StatusLabel="";
	Config_WarningHigh=-1;
	Config_WarningLow=-1;

}

char* IOTThing::createShadowJSONstr(){

	char output[500];
	time_t now;
	struct tm * timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	char out[30];
	strftime(out, 30, "%F %R", timeinfo );// http://www.cplusplus.com/reference/ctime/strftime/

	StaticJsonBuffer<400> jsonBuffer;

	JsonObject& root = jsonBuffer.createObject();

	JsonObject& state =  root.createNestedObject("state");
	JsonObject& reported =  state.createNestedObject("reported");

	reported["name"] = name;
	reported["status"] = status;
	reported["level_percent"] = level_percent;
	reported["level_actual"] = level_actual;
	reported["AlarmActivated"] = AlarmActivated;
	reported["timestamp"] = now;
	reported["dateTimeStringUTC"] = out;

	JsonObject& config = reported.createNestedObject("config");
	config["LevelUnits"]  = Config_LevelUnits;
	config["StatusLable"]  = Config_StatusLabel;
	config["Zero"]  = Config_Zero;
	config["Full"]  = Config_Full;
	config["WarningHigh"]  = Config_WarningHigh;
	config["WarningLow"]  = Config_WarningLow;

	root.prettyPrintTo(output, 500);
	//root.printTo(output);

	//timeinfo = 0;

	return output;
}

void IOTThing::publishUpdate(PubSubClient client){

		char topic[50];
		sprintf(topic, "$aws/things/%s/shadow/update", name);


		if (client.connected()){
			Serial.print("Publish connected... ");
			client.publish(topic, createShadowJSONstr());
		}
		Serial.println("publish done");
}


