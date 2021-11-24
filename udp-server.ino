#include <DFRobot_OSD.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP udp;

const int cs = D3;
DFRobot_OSD osd(cs);

const char *ssid = "";
const char *password = "";

byte ledPin1 = 2; // D5
byte ledPin2 = 5; // D6
byte buzzerPin = 0; // D8

char packetBuffer[128];
char nameOfDevice1[64] = "No Name";
char nameOfDevice2[64] = "No Name";

// state of devices
char stateOfDevice1[8] = "OFF";
char stateOfDevice2[8] = "OFF";

unsigned int udpPort = 56095;

float temperature = 0.0;
float humidity = 0.0;
const float temperatureThreshold = 31.00;

// for screen/display
unsigned long timeElapsed = 0;
unsigned long lastScreenRefresh = 0;
const unsigned int screenTimeout = 1000;

void setup() {
  Serial.begin(115200);
  configurePins();
  initializeOSD();
  connectToWifi();
  udp.begin(udpPort);
}

void loop() {
  timeElapsed = millis();
  parseIncomingPackets();
  displayData();
}

void configurePins() {
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}
void initializeOSD() {
  osd.init();
  osd.clear();
}
void connectToWifi() {
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println(F("Successfully connected to wifi!"));
}

void parseIncomingPackets() {
  int packetSize = udp.parsePacket();

  if (packetSize > 0) {
    udp.read(packetBuffer, packetSize);
    packetBuffer[packetSize] = NULL;

    parseDevicePackets();
    parseAndroidPackets();
    parseDHT11Packets();

    udp.flush();
  }
}
void parseAndroidPackets() {
  char firstChar = packetBuffer[0];
  if (!(firstChar == '@' || firstChar == '$')) {
    return;
  }

  char *deviceName = strtok(packetBuffer, &firstChar);
  if (firstChar == '@') {
    strcpy(nameOfDevice1, deviceName);
  } else {
    strcpy(nameOfDevice2, deviceName);
  }
}
void parseDevicePackets() {
  if (strstr(packetBuffer, "A") == NULL) {
    return;
  }

  if (strcmp(packetBuffer, "A11") == 0) {
    strcpy(stateOfDevice1, "ON");
    digitalWrite(ledPin1, HIGH);
  }
  if (strcmp(packetBuffer, "A10") == 0) {
    strcpy(stateOfDevice1, "OFF");
    digitalWrite(ledPin1, LOW);
  }

  if (strcmp(packetBuffer, "A21") == 0) {
    strcpy(stateOfDevice2, "ON");
    digitalWrite(ledPin2, HIGH);
  }
  if (strcmp(packetBuffer, "A20") == 0) {
    strcpy(stateOfDevice2, "OFF");
    digitalWrite(ledPin2, LOW);
  }
}
void parseDHT11Packets() {
  if (strstr(packetBuffer, "A") != NULL) {
    return;
  }

  if (strstr(packetBuffer, "@") != NULL || strstr(packetBuffer, "$") != NULL) {
    return;
  }

  if (strstr(packetBuffer, ",") == NULL) {
    return;
  }

  char delimiter[] = ",";
  char *temperatureString;
  char *humidityString;

  temperatureString = strtok(packetBuffer, delimiter);
  temperature = atof(temperatureString);
  checkTemperature();

  humidityString = strtok(NULL, delimiter);
  humidity = atof(humidityString);
}
void checkTemperature() {
  if (temperature >= temperatureThreshold) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
}


void displayData() {
  if (timeElapsed - lastScreenRefresh >= screenTimeout) {
    lastScreenRefresh = timeElapsed;
    displayTemperature();
    displayHumidity();
    displayDevice1();
    displayDevice2();
    Serial.println();
  }
}
void displayDevice1() {
  // 1: <device name> :<state (initially off)>
  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "1: %s :%s", nameOfDevice1, stateOfDevice1);
  osd.displayString(14, 2, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}
void displayDevice2() {
  // 2: <device name> :<state (initially off)>
  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "2: %s :%s", nameOfDevice2, stateOfDevice2);
  osd.displayString(14, 16, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}
void displayTemperature() {
  // convert float to string
  char temperatureString[16];
  dtostrf(temperature, 4, 2, temperatureString);

  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "Temp: %s*C", temperatureString);
  osd.displayString(1, 7, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}
void displayHumidity() {
  // convert float to string
  char humidityString[16];
  dtostrf(humidity, 4, 2, humidityString);

  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "Hum: %s%%", humidityString);
  osd.displayString(1, 21, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}
