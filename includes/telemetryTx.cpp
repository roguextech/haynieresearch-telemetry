/*
**********************************************************
* CATEGORY	HARDWARE
* GROUP		TELEMETRY SYSTEM
* AUTHOR	LANCE HAYNIE <LANCE@HAYNIEMAIL.COM>
* DATE		2020-06-05
* PURPOSE	TELEMETRY TRANSMITTER FUNCTIONS
* FILE		TELEMETRYTX.CPP
**********************************************************
* MODIFICATIONS
* 2020-06-05 - LHAYNIE - INITIAL VERSION
**********************************************************
*/

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "serialComm.h"
#include "telemetryTx.h"
#include "checkSum.h"
#include "rtty.h"
#include "TinyGPS++.h"
#include "charTrim.h"
#include <stdlib.h>

static const int gpsRxPin = 4, gpsTxPin = 3;
static const uint32_t gpsBaud = 9600;
SoftwareSerial ss(gpsRxPin, gpsTxPin);
TinyGPSPlus gps;

char 	txStationID[10];
int		txObsNumber = 0;
char	txCurrentTime[10];
char	txCurrentDate[10];
char	txLatitude[15];
char	txLongitude[15];
char	txAltitude[8];
char	txMaxAltitude[8];
char	txSpeed[8];
char	txMaxSpeed[8];
char	txAcceleration[8];
char	txMaxAcceleration[8];
char	txOutData[100];
float   maxAltTracking = 0;
float	maxSpeedTracking = 0;
char 	strBuffer[15];

void telemetryTx::gpsInit() {
	ss.begin(gpsBaud);
	const char *gpsStream =
	  "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
	  "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
	  "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
	  "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
	  "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
	  "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";

	  while (*gpsStream)
	    if (gps.encode(*gpsStream++));
}

int telemetryTx::update() {
	strcpy(txStationID, "HRDUAV");
	txObsNumber = txObsNumber + 1;

	strcpy (txCurrentTime, dtostrf(gps.time.hour(),2,0,strBuffer));
	strcat (txCurrentTime, ":");
	strcat (txCurrentTime, dtostrf(gps.time.minute(),2,0,strBuffer));
	charTrim.trim(txCurrentTime);

	strcpy (txCurrentDate, dtostrf(gps.date.year(),4,0,strBuffer));
	strcat (txCurrentDate, "/");
	strcat (txCurrentDate, dtostrf(gps.date.month(),2,0,strBuffer));
	strcat (txCurrentDate, "/");
	strcat (txCurrentDate, dtostrf(gps.date.day(),2,0,strBuffer));

	strcpy(txLatitude, dtostrf(gps.location.lat(),11,6,strBuffer));
	charTrim.trim(txLatitude);

	strcpy(txLongitude, dtostrf(gps.location.lng(),11,6,strBuffer));
	charTrim.trim(txLongitude);

	String buf = "";
	char gpsAltitude[20];
	buf += itoa(gps.altitude.meters(), gpsAltitude, 10);
	strcpy(txAltitude, gpsAltitude);

	if (gps.altitude.meters() > maxAltTracking) {
		maxAltTracking = gps.altitude.meters();
		strcpy(txMaxAltitude, gpsAltitude);
	}

	buf = "";
	char gpsSpeed[20];
	buf += itoa(gps.speed.kmph()/3.6, gpsSpeed, 10);
	strcpy(txSpeed, gpsSpeed);

	if (gps.speed.kmph()/3.6 > maxAltTracking) {
		maxSpeedTracking = gps.speed.kmph()/3.6;
		strcpy(txMaxSpeed, gpsSpeed);
	}

	strcpy(txAcceleration, "67");
	strcpy(txMaxAcceleration, "93");

	return 0;
}

char* telemetryTx::format() {
	String buf = "";
	char obs[20];
	buf += itoa(txObsNumber, obs, 10);

	char txData[100];

	strcpy (txData, txStationID);
	strcat (txData, ",");
	strcat (txData, obs);
	strcat (txData, ",");
	strcat (txData, txCurrentTime);
	strcat (txData, ",");
	strcat (txData, txLatitude);
	strcat (txData, ",");
	strcat (txData, txLongitude);
	strcat (txData, ",");
	strcat (txData, txAltitude);
	strcat (txData, ",");
	strcat (txData, txMaxAltitude);
	strcat (txData, ",");
	strcat (txData, txSpeed);
	strcat (txData, ",");
	strcat (txData, txMaxSpeed);
	strcat (txData, ",");
	strcat (txData, txAcceleration);
	strcat (txData, ",");
	strcat (txData, txMaxAcceleration);

	strcpy (txOutData, txData);

	Serial.print("<CTIME:");
	Serial.print(txCurrentTime);
	Serial.println(">");

	Serial.print("<OBS:");
	Serial.print(obs);
	Serial.println(">");

	Serial.print("<LAT:");
	Serial.print(txLatitude);
	Serial.println(">");

	Serial.print("<LONG:");
	Serial.print(txLongitude);
	Serial.println(">");

	Serial.print("<ALT:");
	Serial.print(txAltitude);
	Serial.println(">");

	Serial.print("<MAXALT:");
	Serial.print(txMaxAltitude);
	Serial.println(">");

	Serial.print("<SPEED:");
	Serial.print(txSpeed);
	Serial.println(">");

	Serial.print("<MAXSPEED:");
	Serial.print(txMaxSpeed);
	Serial.println(">");

	Serial.print("<ACCEL:");
	Serial.print(txAcceleration);
	Serial.println(">");

	Serial.print("<MAXACCEL:");
	Serial.print(txMaxAcceleration);
	Serial.println(">\n");

	return txOutData;
}

int telemetryTx::tx(char* msg) {
	Serial.print("<TXDATA:");
	Serial.print(msg);
	Serial.println(">");

	rtty.attach(12);
	rtty.tx(msg);

	return 0;
}
