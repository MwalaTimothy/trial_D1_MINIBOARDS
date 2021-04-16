// My inspiration for the MQTT-part   https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino 
// resulting temperature gets printed here: https://io.adafruit.com/mergenthaler/feeds/192-168-0-56

/*
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be filled wherever it is marked.

  Copyright (c) [2016] [Andreas Spiess]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  virginSoilFull V2.2.2
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 17                                        // Button pin on the esp for selecting modes. D3 for the Wemos!


 // GPIO17 on ESP32 refelects the D3 pin of Wemos

#include <IOTAppStory.h>                                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory


// my adders are here

#include <Wire.h>
// #include <PubSubClient.h> // added for MQTT 2020-12-21  and commented out due to the next two liines
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_BMP085.h>            // go to Library Manager an add the Adafruit BMP085 Library

 
Adafruit_BMP085 bmp;


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "timomwala"
#define AIO_KEY         "aio_txtY76P8vnnNpWjhmoxlUkF6Eq4K"

WiFiClient client; 


Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/
// Setup a feed called 'pressure,temperature' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish pressureandtemperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/covid");
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");





// my adders end here


// ================================================ EXAMPLE VARS =========================================
// used in this example to print variables every 10 seconds
unsigned long printEntry;
String deviceName = "virginsoil";
String chipId;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* lbl         = "Light Show";
char* exampleURL  = "http://someapi.com/getdata.php?userid=1234&key=7890abc";
char* nrOf        = "6";

char* doSomething = "1";
char* chosen      = "0";

char* updInt      = "60";
char* ledPin      = "2";
char* timeZone    = "0.0";



// ================================================ SETUP ================================================
void setup() {

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);

  
  // create a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
  
  /* TIP! delete lines below when not used */
  IAS.preSetDeviceName(deviceName);                         // preset deviceName this is also your MDNS responder: http://virginsoil-123.local
  //IAS.preSetAutoUpdate(false);                            // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                            // automaticConfig (true, false)
  //IAS.preSetWifi("ssid","password");                      // preset Wifi
  /* TIP! Delete Wifi cred. when you publish your App. */
  

  IAS.addField(lbl, "textLine", 16);                        // These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(exampleURL, "Textarea", 80, 'T');            // reference to org variable | field label value | max char return | Optional "special field" char
  IAS.addField(nrOf, "Number", 8, 'N');                     // Find out more about the optional "special fields" at https://iotappstory.com/wiki
  
  IAS.addField(doSomething, "Checkbox:Check me", 1, 'C');
  IAS.addField(chosen, "Selectbox:Red,Green,Blue", 1, 'S');

  IAS.addField(updInt, "Interval", 8, 'I');
  IAS.addField(ledPin, "ledPin", 2, 'P');
  IAS.addField(timeZone, "Timezone", 4, 'Z');
  


  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.

  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonVeryLongPress([]() {
    Serial.println(F(" If mode button is released, I won't do anything unless you program me to."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });

  IAS.onFirmwareUpdateProgress([](int written, int total){
      Serial.print(".");

      /*
      if(written%5==0){
        Serial.print(F("\n Written "));
        Serial.print(written);
        Serial.print(F(" of "));
        Serial.print(total);
      }
      */
  });
  
  /* 
  IAS.onModeButtonNoPress([]() {
    Serial.println(F(" Mode Button is not pressed."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {                              
    Serial.println(F(" Run or display something on the first time this app boots"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateCheck([]() {
    Serial.println(F(" Checking if there is a firmware update available."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    Serial.println(F(" Downloading and Installing firmware update."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateError([]() {
    Serial.println(F(" Update failed...Check your logs"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onConfigMode([]() {
    Serial.println(F(" Starting configuration mode. Search for my WiFi and connect to 192.168.4.1."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */ 

  /* TIP! delete the lines above when not used */
 
  IAS.begin();                                            // Run IOTAppStory
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------


{
  Serial.begin(115200);
  //Wire.begin (4, 5);
  if (!bmp.begin()) 
  {
    Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
    while (1) {}
  }
}
 


}  // end of void setup bracket

 
uint32_t x=0; 

// ================================================ LOOP =================================================

void loop() {


  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" Celsius");
 
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pascal");
  
  Serial.println();
  delay(5000);


 // Serial.print(F("Temperature = "));
 // Serial.print(F(bmp.readTemperature()));
 // Serial.println(F(" Celsius"));
 
 // Serial.print(F("Pressure = "));
 // Serial.print(F(bmp.readPressure()));
 // Serial.println(F(" Pascal"));

// here I dump the MQTT part   ***************************


  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\nSending photocell val "));
 // Serial.print(x);     I commented this out to replace it by the following
 //   Serial.print(bmp.readPressure());    or temperature
 Serial.print(bmp.readTemperature());


// in the following, i corrected (not yet calibrated) the measured value to be minus 6 as the microcontroller heates the sensor 
  
  Serial.print("...");
  if (! pressureandtemperature.publish(bmp.readPressure()-6)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");

  //  here the dumped MQTT part ends 


  //-------- Your Setup ends  here ---------------




  
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------


  if (millis() - printEntry > 10000) {          // Serial.print the example variables every 10 seconds



    
    
    Serial.println(F(" LABEL\t\t| VAR\t\t| VALUE"));

    Serial.print(F(" textLine\t| lbl\t\t| "));
    Serial.println(lbl);
    Serial.print(F(" Textarea\t| exampleURL\t| "));
    Serial.println(exampleURL);
    Serial.print(F(" Number\t\t| nrOf\t\t| "));
    Serial.println(nrOf);

    Serial.print(F(" Checkbox\t| doSomething\t| "));
    Serial.println(doSomething);
    Serial.print(F(" Selectbox\t| chosenColor\t| "));
    Serial.println(chosen);

    Serial.print(F(" Interval\t| updInt\t| "));
    Serial.println(updInt);
    Serial.print(F(" Led pin\t| ledPin\t| "));
    Serial.println(atoi(ledPin));  
    Serial.print(F(" Timezone\t| timeZone\t| "));
    Serial.println(atof(timeZone));

    Serial.println(F("*-------------------------------------------------------------------------*"));
    printEntry = millis();
  }
}
