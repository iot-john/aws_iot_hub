/*
 * IOTThing.h
 *
 *  Created on: 1 May 2018
 *      Author: John
 */

#ifndef IOTTHING_H_
#define IOTTHING_H_

#include "Arduino.h"
#include "ArduinoJson.h"
#include <PubSubClient.h>

class IOTThing {
public:
	IOTThing(char* thingName);
	char* name;
	char* status;
	int level_percent;
	int level_actual;
	bool AlarmActivated;

	char* Config_StatusLabel;
	char* Config_LevelUnits;
	int Config_Zero;
	int Config_Full;
	int Config_WarningHigh;
	int Config_WarningLow;

	char* createShadowJSONstr();
	void publishUpdate(PubSubClient client);


};

#endif /* IOTTHING_H_ */
