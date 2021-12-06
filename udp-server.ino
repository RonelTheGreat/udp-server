// Library imports
#include <DFRobot_OSD.h>  // This is a third party library
#include <ESP8266WiFi.h>  // This is a package from esp core library
#include <WiFiUdp.h>      // This is a package from esp core library


// This line will just copy all WiFiUDP's functions/variables and settings to udp variable.
// Let's just say WiFiUDP is a blueprint and udp is the carbon copy of the blueprint.
// So whatever is inside WiFiUDP is now available on udp variable.
WiFiUDP udp;


// This will select CS pin for osd.
// cs is just the chip-select pin for SPI communications.
const int cs = D3;
DFRobot_OSD osd(cs);


// These two variables below will hold
// the password and ssid of the access point to connect to.
const char *ssid = "";
const char *password = "";


// These will hold the pin number of led1, led2 and buzzer.
byte ledPin1 = 2; // D5
byte ledPin2 = 5; // D6
byte buzzerPin = 0; // D8


// This will hold the actual packet/message received.
char packetBuffer[128];
// These variables will hold the name of device 1 and 2.
// Initial value is set to "No Name".
char nameOfDevice1[64] = "No Name";
char nameOfDevice2[64] = "No Name";


// These variables will hold the state of device 1 and 2.
// either ON or OFF
char stateOfDevice1[8] = "OFF";
char stateOfDevice2[8] = "OFF";


// This will hold the UDP port
unsigned int udpPort = 1032;


// These variables will hold the temperature, humidity and
// the temperature threshold.
float temperature = 0.0;
float humidity = 0.0;
const float temperatureThreshold = 31.00;


// This will hold the timer or the time passed since esp was turned on (i.e. in milliseconds).
unsigned long timeElapsed = 0;
// This will hold the last time screen was refreshed.
unsigned long lastScreenRefresh = 0;
// This will hold the screen timeout or the time interval of which the display/screen is refreshed.
const unsigned int screenTimeout = 1000;


void setup() {
  Serial.begin(115200);
  // This will call/execute the function in POINT 1
  configurePins();

  // This will call/execute the function in POINT 2
  initializeOSD();

  // This will call/execute the function in POINT 3
  connectToWifi();

  // This will initialize UDP transmission
  udp.begin(udpPort);
}


void loop() {
  // This will record/save the time passed since esp was turned on (i.e. in milliseconds).
  // In this case, timeElapsed variable will hold the time.
  // millis() is a built-in function from arduino.
  timeElapsed = millis();

  // This will call/execute the function  in POINT 4
  parseIncomingPackets();

  // This will call/execute the function in POINT 9
  displayData();
}


//------ POINT 1 ---------//
// This function will just configure pins for
// led1, led2 and buzzer
void configurePins() {
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}


//------ POINT 2 ---------//
// This function will initialize osd and will clear the 
// screen (if there are any displayed).
void initializeOSD() {
  osd.init();
  osd.clear();
}


//------ POINT 3 ---------//
// This function will connect the board to wifi.
// Given the correct ssid and password of the access point.
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


//------ POINT 4 ---------//
// This function will parse/read incoming udp packets from
// sensors and android app.
void parseIncomingPackets() {
  // udp.parsePacket() will parse udp packet.
  // "Parse" meaning it will process the incoming packet/message.
  // If a UDP packet is present, then this function's result will be the size of the packet/message.
  // That's why the result is stored to packetSize variable that is an integer.
  int packetSize = udp.parsePacket();

  // Now, if the packet size above is greater than 0 (zero), then
  // the code inside the curly braces will be executed.
  if (packetSize > 0) {
    // udp.read() is used in conjunction with udp.parsePacket() function.
    // udp.read() can only be successfully called/executed after udp.parsePacket().
    // udp.read() accepts 2 arguments (i.e. <buffer>, <maximum size of the buffer>).
    // <buffer> or the variable/container of the received packet.
    // <maximum size of the buffer>, the maximum packet/message size allowable.
    // This line will save the received packet to packetBuffer variable.
    udp.read(packetBuffer, packetSize);
    // This line will mark the end of packet received.
    // NULL is a special character (ascii code 0) that signify the end of string.
    // For more info https://www.arduino.cc/reference/en/language/variables/data-types/string/
    packetBuffer[packetSize] = NULL;

    // This will call/execute the function in POINT 5
    parseDevicePackets();

    // This will call/execute the function in POINT 6
    parseAndroidPackets();

    // This will call/execute the function in POINT 7
    parseDHT11Packets();

    udp.flush();
  }
}


//------ POINT 5 ---------//
// This function will check if the incoming packets
// are from the devices (i.e. magnetic switch and dht11).
void parseDevicePackets() {
  // This will check if the received buffer has NO letter "A" in it.
  // If it does NOT contain letter "A" then we immediately exit out of the function.
  // strstr() accepts 2 arguments (i.e. <the string where to search>, <the string/character to search>).
  // strstr() will result to NULL if letter "A" is not in the packet/message buffer.
  if (strstr(packetBuffer, "A") == NULL) {
    return;
  }

  // These checks below will just run if the packet received contains letter "A".

  // This will check if the packet contains or is "A11".
  // If it is, then the code inside the curly braces will be executed.
  // strcmp() accepts 2 arguments (i.e. the strings to compare; can be interchanged).
  // strcmp() will result to 0 (zero) if two strings matches/the same.
  if (strcmp(packetBuffer, "A11") == 0) {
    // strcpy() accepts 2 arguments (i.e. <destination>, <string to copy>).
    // <destination> is from the word itself the destination of the string or
    // where to store the copied string.
    // <string to copy> is self explanatory.
    // So in this case, the string "ON" will be copied/stored to stateOfDevice1 variable.
    strcpy(stateOfDevice1, "ON");
    // This will turn on led 1.
    digitalWrite(ledPin1, HIGH);
  }
  // These lines of code below are similar from above.
  // Only that this is the "OFF" state of device 1.
  if (strcmp(packetBuffer, "A10") == 0) {
    strcpy(stateOfDevice1, "OFF");
    digitalWrite(ledPin1, LOW);
  }


  // These lines of code below are similar from above.
  // Only that this is for device 2's states (i.e. ON/OFF).
  if (strcmp(packetBuffer, "A21") == 0) {
    strcpy(stateOfDevice2, "ON");
    digitalWrite(ledPin2, HIGH);
  }
  if (strcmp(packetBuffer, "A20") == 0) {
    strcpy(stateOfDevice2, "OFF");
    digitalWrite(ledPin2, LOW);
  }
}


//------ POINT 6 ---------//
// This function will check if the incoming packets
// are from the android app.
void parseAndroidPackets() {
  // This line will get the first character of the received packet.
  // And will store it to firstChar variable.
  char firstChar = packetBuffer[0];
  // Then we will check if the first character is NOT '@' or '$'.
  // If it is not, then we don't have any business with the packet, so we will
  // exit out of the function and move on with our lives.
  if (!(firstChar == '@' || firstChar == '$')) {
    return;
  }

  // These lines of code below will just run if the packet received starts with 
  // the character '@' or '$'.

  // strtok() is a built-in function in arduino.
  // strtok() will split the string given a delimiter.
  // For more info https://www.cplusplus.com/reference/cstring/strtok/
  // So in this case, we will split the received packet by using the first
  // character as the delimiter.
  // So if the packet is "@Window", the splitted value or result would be "Window" without
  // the first character. And we will save/store it to deviceName variable.
  char *deviceName = strtok(packetBuffer, &firstChar);

  // This will check if the first character is '@'.
  // If it is, then the code inside the curly braces will be executed.
  if (firstChar == '@') {
    // This will copy the device name that we obtained above to nameOfDevice1 variable.
    strcpy(nameOfDevice1, deviceName);

  // this will run if the first character is NOT '@'.
  } else {
    // Same as above, this will copy the device name obtained above to nameOfDevice2 variable.
    strcpy(nameOfDevice2, deviceName);
  }
}


//------ POINT 7 ---------//
// This function will check if the incoming packets
// are from the dht11 sensor
void parseDHT11Packets() {
  // This will check if the packet contains letter "A".
  // If it is, then exit.
  if (strstr(packetBuffer, "A") != NULL) {
    return;
  }

  // This will check if the packet contains the character "@" or "$".
  // If it is, then exit.
  if (strstr(packetBuffer, "@") != NULL || strstr(packetBuffer, "$") != NULL) {
    return;
  }

  // This will check if the packet DOES NOT contain comma (,).
  // If it doesn't then exit.
  if (strstr(packetBuffer, ",") == NULL) {
    return;
  }


  // These lines of code below will just run if the  
  // packet received contains a comma.

  // This holds the delimiter.
  // In this case the delimiter is a comma.
  char delimiter[] = ",";

  // These variables will hold the temperature and humidity.
  char *temperatureString;
  char *humidityString;

  // This will split the packets using comma as the delimiter.
  // The first time we call strtok(), the value will be stored to temperatureString variable.
  temperatureString = strtok(packetBuffer, delimiter);
  // Then we will convert the string to a floating point number by using atof() function.
  // Then we will store it to temperature variable (the actual temperature that we need that is converted).
  temperature = atof(temperatureString);
  // And we will call checkTemperature() function (see POINT 8). 
  checkTemperature();

  // The second time we call strtok(), the value will be stored to humidityString variable.
  // We passed NULL as the first argument so that it will continue on the last operation that was performed above.
  // So the remaining string now is no other than the humidity.
  humidityString = strtok(NULL, delimiter);
  // Then we convert the string to a floating point number and store it to humidity variable.
  humidity = atof(humidityString);
}


//------ POINT 8 ---------//
// This function will check if the temperature
// is greater than or equal to threshold.
// If it is, then the buzzer will be turned ON and OFF otherwise.
void checkTemperature() {
  if (temperature >= temperatureThreshold) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
}

//------ POINT 9 ---------//
// This function will display all the data on the screen every second.
void displayData() {
  // This will check if the time passed since the board was turned ON
  // minus the previously recorded time is greater than or equal to the screen timeout (i.e. 1 second).
  // for example:
  //    lastScreenRefresh = 0;
  //    timeElapsed = 1002
  //    screentTimeout = 1000; <- the value of this variable is constant
  //    therefore, 1002 - 0 = 1002
  // so the code inside the curly braces will be executed
  if (timeElapsed - lastScreenRefresh >= screenTimeout) {
    // Then will store the current time to be used on the next timing
    lastScreenRefresh = timeElapsed;

    // This will call/execute the function in POINT 10
    displayTemperature();

    // These 3 remaining function calls have the same explanation in POINT 10
    displayHumidity();
    displayDevice1();
    displayDevice2();
    Serial.println();
  }
}


//------ POINT 10 ---------//
void displayTemperature() {
  // Prepare buffer/container in which the converted (i.e. converted from float to string) temperature is stored.
  char temperatureString[16];

  // dtostrf() will convert a floating point number to a string.
  // For example: from 30.00 to "30.00"
  // dtostrf() accepts 3 arguments
  // (i.e. <value to be converted>, <how many digits are there>, <how many digits after the decimal place>, <where to store the converted value>)
  dtostrf(temperature, 4, 2, temperatureString);

  // Prepare buffer/container for the final formatted string that will be shown on the screen.
  char finalStringToDisplay[32];

  // sprintf() will format the string using placeholders.
  // In this case, the final output we want is, for example: Temp: 30*C 
  // sprintf() accepts 3 main arguments (i.e. <where to store the formatted string>, <the format of the string>, <the value of placeholder/s>).
  // <where to store the formatted string>, self-explanatory. In this case finalStringToDisplay buffer/container is used.
  // <the format of the string>, what we want to output by using placeholders for dynamic values.
  // Example of placeholder: %s     which means we want to place a string.
  // <the value of placeholder/s>, this will substitute the placeholders in order.
  sprintf(finalStringToDisplay, "Temp: %s*C", temperatureString);
  
  // Finally, we will display it on the screen.
  osd.displayString(1, 7, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}

// Same as POINT 10,
// the only difference is that this is for humidity.
void displayHumidity() {
  char humidityString[16];
  dtostrf(humidity, 4, 2, humidityString);

  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "Hum: %s%%", humidityString);
  osd.displayString(1, 21, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}

// Same as POINT 10,
// the only difference is that this is for device 1.
void displayDevice1() {
  // 1: <device name> :<state (initially off)>
  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "1: %s :%s", nameOfDevice1, stateOfDevice1);
  osd.displayString(14, 2, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}

// Same as POINT 10,
// the only difference is that this is for device2.
void displayDevice2() {
  // 2: <device name> :<state (initially off)>
  char finalStringToDisplay[32];
  sprintf(finalStringToDisplay, "2: %s :%s", nameOfDevice2, stateOfDevice2);
  osd.displayString(14, 16, finalStringToDisplay);
  Serial.println(finalStringToDisplay);
}

