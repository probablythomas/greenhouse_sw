#include <Adafruit_DHT.h>

#define TIMEZONE    -4              // located in Toronto, time zone is EST (UTC-4)

#define DHTPIN      2               // temperature and humidity sensor pin
#define DHTTYPE     DHT22		    // model number - DHT22==AM2302

#define FANPIN      6               // pin selected to control the fan
#define LIGHTPIN    0               // pin selected to control the lights

#define BRIGHTPIN   A1              // pin selected for photocell reading
#define LIDPIN      5               // pin selected for lid shut sensor

#define FADESPEED   20              // how fast the lights dim on and off

bool printFlag = false;

int fanOverrideFlag = 0;            // fan override flag - 0 is override OFF, 1 is override ON, 2 is override OFF
int lightOverrideFlag = 0;          // light override flag - 0 is override OFF, 1 is override ON, 2 is override OFF

DHT dht(DHTPIN, DHTTYPE);           // temperature and humidity sensor

double temperature = 0;             // temperature measurement
double humidity = 0;                // humidity measurement
double brightness = 0;              // brightness measurement
bool lidShut = false;               // lid sensor
bool fanState = false;              // fan stte (ON/OFF)
bool lightState = false;            // light state (ON/OFF)
bool lightStateLast = false;        // previous light state

unsigned long lastMeasTime = 0;     // time when sensors were last polled
unsigned long now;                  // current loop time
int localHour = 0;                  // local hour
int onTime = 6;                     // lights default on at 6 AM
int offTime = 23;                   // lights default off at 11 PM (23 h)

// particle setup
void setup() {
    pinMode(FANPIN, OUTPUT);        // fan control
    pinMode(LIGHTPIN, OUTPUT);      // light control
    
    // pinMode(BRIGHTPIN, INPUT);      // photocell pin - an analog pin??
    // pinMode(LIDPIN, INPUT);         // lid shut pin - digital pin??

    
    // particle function definitions
    //Particle.function("fanORide", fanOverride);
    Particle.function("lightORide", lightOverride);
    Particle.function("setLightTime", setLightTime);
    
    // particle variable definitions
    Particle.variable("temp", temperature);
    Particle.variable("humid", humidity);
    
    //Particle.variable("brightness", brightness);
    //Particle.variable("lidShut", lidShut);
    //Particle.variable("fanState", fanState);
    Particle.variable("lightState", lightState);

    Particle.variable("localHour", localHour);
    Particle.variable("onTime", onTime);
    Particle.variable("offTime", offTime);
    
    if (!Time.isDST()) {
        Time.zone(TIMEZONE);
    } else {
        Time.zone(TIMEZONE-1);
    }

    Serial.begin(9600);
}

void loop() {
    // give the board some idle time
    delay(100);
    
    // get current time for next run through
    now = millis();
    localHour = Time.hour();
    
    // Get temperature and humidity
	// DHT22/AM2303 is slow, needs about 2 seconds between measurements
	if ((now - lastMeasTime) > 2000) {
    	humidity = dht.getHumidity();
    	temperature = dht.getTempCelcius();
    	lastMeasTime = now;
	}
	
	// Check if the lid is open
    // maybe write this section?
/*
	// fan control
    if ((fanOverrideFlag == 0 && (humidity > 70 || temperature > 30)) || fanOverrideFlag == 1) {
        digitalWrite(FANPIN, HIGH);
        fanState = true;
    } 
    // turn fan off 
    else if ((fanOverrideFlag == 0 && (humidity < 50 || temperature < 25)) || fanOverrideFlag == 2) {
        digitalWrite(FANPIN, LOW);
        fanState = false;
    }
    // else, do nothing!
    else {
        // yup, nothing.
    }
*/    
    // light control
    if (lightOverrideFlag == 2) {
        lightState = false;
    }
    else if (lightOverrideFlag == 1 || (lightOverrideFlag == 0 && (localHour > onTime && localHour < offTime))) {
        lightState = true;
    }
    // else turn off the lights
    else {
        lightState = false;
    }
    
    lightStateLast = setLights(lightStateLast, lightState);
    
    if (printFlag) {
        Serial.print(localHour); Serial.print("\t");
        Serial.print(temperature); Serial.print("\t");
        Serial.print(humidity); Serial.print("\t");
        Serial.print(lightOverrideFlag); Serial.print("\t");
        Serial.print(onTime); Serial.print("\t");
        Serial.print(offTime); Serial.print("\t");
        Serial.print(lightState); Serial.print("\t");
        Serial.print(lightStateLast); Serial.print("\t");
        Serial.println("");
    }
    
}

// internal function "setLights"
bool setLights(bool prevState, bool currentState)
{
    bool toggle = currentState;
    int w;
    
    if (currentState != prevState) {
        if (toggle) {
            for (w = 0; w < 255; w++) {
                analogWrite(LIGHTPIN, w);
                delay(FADESPEED);
            }
            analogWrite(LIGHTPIN, 255);
        }
        else {
            for (w = 255; w > 0; w--) {
                analogWrite(LIGHTPIN, w);
                delay(FADESPEED);
            }
            analogWrite(LIGHTPIN, 0);
        }
        prevState = currentState;
    }
    return prevState;
}

// particle function "fanOverride" to set the current state of the fan override
int fanOverride(String command)
{
    // set fanOverrideFlag according to website selection (default to 0)
    if (command == "0") {
        fanOverrideFlag = 0;
    } else if (command == "1") {
        fanOverrideFlag = 1;
    } else if (command == "2") {
        fanOverrideFlag = 2;
    } else {
        fanOverrideFlag = 0;
    }
    
    return 0;
}

// particle function "lightOverride" to set the current state of the light override 
int lightOverride(String command)
{
    // set lightOverrideFlag according to website selection (default to 0)
    if (command == "0") {
        lightOverrideFlag = 0;
    } else if (command == "1") {
        lightOverrideFlag = 1;
    } else if (command == "2") {
        lightOverrideFlag = 2;
    } else {
        lightOverrideFlag = 0;
    }
    
    return 0;
}

// particle function "setLightTime" to set On and Off times for lights
int setLightTime(String command)
{
    char cmd[6];
    command.toCharArray(cmd,sizeof(cmd));
    char* newTime = strtok(cmd, ",");
    onTime = atoi(newTime);
    newTime = strtok(0, ",");
    offTime = atoi(newTime);
}