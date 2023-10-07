#include <Arduino.h>
#define Serial_BAUDRATE 74880

// WiFi connectivity and webserver
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
const char* my_ssid = "";
const char* my_pass = ""; 
const char * esp_name = "ESP LED";
int rssi = -1;
int apmode = 0;
int ssid_len = 0;
int ssid_ind =-1;
int pw_len = 0;
int pw_ind = -1;

// Setting that should survive power cycle
#include <EEPROM.h>
// Change this if the structure of "settings" is changing to avoid incorrect assignments
#define MAGIC_WORD 12569
struct settings {
  uint16_t magic=MAGIC_WORD;
  uint8_t mode = 0;  // Effect mode
  uint8_t speed = 10; // Speed setting for some modes
  uint8_t fade = 50;
  uint8_t hue = 20;
  uint8_t col_r = 255;
  uint8_t col_g = 0;
  uint8_t col_b = 0;
  uint8_t rgbmode = 0;
  uint8_t endless = 1;
  uint8_t reverse = 0;
  uint8_t brightness = 255;
  uint8_t ssid_len = 0;
  uint8_t pw_len = 0;
  char ssid[64]="ssid";
  char pw[64]="pw";
};

settings mysets;

// To test the page it can be renamed to .html
// uses the C++ raw string literals 
#include "webpage.h"

// Called when system is powered up

void setup() {
  Serial.begin(Serial_BAUDRATE);
  Serial.println("Starting ESP");

  // Try to read WiFi connection parameters from EEPROM
  EEPROM.begin(160);
  uint16_t magic;
  EEPROM.get(0,magic);
  //Check magic word to make sure settings are valid
  if (magic==MAGIC_WORD) {
    Serial.println("EEPROM valid, reading settings");
    EEPROM.get(0,mysets);
  }

  Serial.flush();
  delay(50);
  Serial.print("SSID: ");
  Serial.println(mysets.ssid);
  Serial.print("PASS: ");
  Serial.println(mysets.pw);

  // While developing prioritize hardcoded setting
  if (strlen(my_ssid)>0 and strlen(my_pass)>0) {
    WiFi.begin(my_ssid,my_pass);
  }
  WiFi.begin(mysets.ssid,mysets.pw);
  if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
      Serial.println("Error connecting Wifi!");
      // IP Adress used in AP Mode
      IPAddress local_IP(192,168,33,1);
      IPAddress subnet(255,255,255,0);
      WiFi.mode(WIFI_AP);
      WiFi.softAP(esp_name);
      delay(100);
      WiFi.softAPConfig(local_IP, local_IP, subnet);
      Serial.print("Soft-AP IP address = ");
      Serial.println(WiFi.softAPIP());
      apmode=1;
  } else {
      Serial.println("Successful WiFi connect with stored values");
      Serial.flush();    
      apmode=0;
  }
  
  WiFi.setAutoReconnect(true);
  WiFi.setHostname(esp_name);

  Serial.print("WiFi connected to ");
  Serial.print(WiFi.localIP());
  Serial.print(" with ");
  Serial.println(WiFi.RSSI());

  if (apmode) {
  server.on("/", []() {
    server.send(200, "text/html", login_page);
  });
  server.on("/login", []() {
    Serial.print("SSID:");
    Serial.println(server.arg("ssid"));
    ssid_len=urlDecode(server.arg("ssid").c_str(),mysets.ssid,64);
    Serial.println(mysets.ssid);
    Serial.print("Password:");
    Serial.println(server.arg("pw"));
    urlDecode(server.arg("pw").c_str(),mysets.pw,64);
    pw_len=Serial.println(mysets.pw);
    Serial.println(mysets.pw);
  });
  } else {
    // Access to root page - return predefined page
    server.on("/", []() {
      Serial.println("Page reload");
      server.send(200, "text/html", html_page);
    });
    // accept commands
    server.on("/command", []() { 
      Serial.println("command:"+server.arg("id"));
      server.send(200,"text/html","ok");
    });
    // query settings
    server.on("/settings", []() {
      Serial.println("settings");
      String reply="{";
      reply+= "\"temperature\":\""+String(20)+"\",\"humidity\":\""+String(80)+"\"}";
      server.send(200,"text/html",reply);
    });
    // accept changes for sliders
    server.on("/slider", []() { 
      Serial.println("slider:"+server.arg("slider"));
      Serial.println("check:"+server.arg("check"));
      server.send(200,"text/html","ok");
    });
    // next reboot will require all settings including WiFi to be set again
    server.on("/reset", []() {
      Serial.println("reset");
      mysets.magic=0;
      EEPROM.put(0,mysets);
      EEPROM.commit();      
    }); 
  }
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  Serial.println("Executing main");
  Serial.flush();
}

//Revert javascript function encodeURIComponent(pw) (%xx -> character)
int urlDecode(const char *src,char *dest,int len) {
  Serial.println(src);
  char ch[3];
  int d=0;
  int i, ii;
  for (i=0; i<strlen(src); i++) {
      if (src[i]=='%') {
          ch[0]=src[i+1];
          ch[1]=src[i+2];
          ch[2]=0;
          sscanf(ch, "%x", &ii);
          dest[d++]=static_cast<char>(ii);
          i=i+2;
      } else {
          dest[d++]=src[i];
      }
  }
  dest[d]=0;
  return d;
}

// Error page for unknown requests
void handleNotFound() {
  String message = "Page Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += "Headers:";
  message += server.headers();
  message += "\n";

  for (uint8_t i = 0; i < server.headers(); i++) {
    message += " " + server.headerName(i) + ":" + server.header(i) + "\n";
  }
  
  Serial.println(message);
  server.send(404, "text/plain", message);
}

// Scan serial for SSID/PW is stateful mode to allow AP Input in parallel
void getCredentials() {
  if (ssid_len == 0) {
    if (ssid_ind<0) {
      Serial.println();
      Serial.print("Enter SSID:");
      ssid_ind=0;
    } else {
      int len = Serial.readBytes(&mysets.ssid[ssid_ind], 64-ssid_ind);
      ssid_ind+=len;
      if (mysets.ssid[ssid_ind-1]==10) {
        mysets.ssid[ssid_ind-1]=0;
        ssid_len=ssid_ind-1;
        Serial.println(mysets.ssid);
        Serial.flush();
      }
    }
  }
  if (pw_len == 0 && ssid_len>0) {
    if (pw_ind<0) {
      Serial.println();
      Serial.print("Enter Password:");
      pw_ind=0;
    } else {
      int len = Serial.readBytes(&mysets.pw[pw_ind], 64-pw_ind);
      pw_ind+=len;
      if (mysets.pw[pw_ind-1]==10) {
        mysets.pw[pw_ind-1]=0;
        pw_len=pw_ind-1;
        Serial.println(mysets.pw);
        Serial.flush();
      }
    }
  }
  if (pw_len>0 && ssid_len>0) {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to "+String(mysets.ssid)+"/"+String(mysets.pw));
    Serial.flush();
    WiFi.begin(mysets.ssid,mysets.pw);
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
      Serial.println("Error connecting Wifi!");
    } else {
    Serial.println("Successfully registered new WiFi connection");
    EEPROM.put(0,mysets);
    EEPROM.commit();      
    }
    ESP.restart();
  }
}

/*
 *
 * Here starts the real stuff
 *
 */

// Executed repeatedly after setup has finished
void loop() {
  if (apmode) {
    getCredentials();
  }
  // Webserver processing
  server.handleClient();

  delay(2000);
  Serial.println("loop");
  Serial.flush();
}
