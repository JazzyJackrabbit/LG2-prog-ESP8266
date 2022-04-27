
// ESP8266 12e (larg card)
// preferences: http://arduino.esp8266.com/stable/package_esp8266com_index.json
// type:   .. NodeMCU 1.0  ( ESP8266 12e ) ..
//
// "Bbox-0776BE57"
//  "5EE8A1CC23"
//


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "TimedAction.h"

#include <SoftwareSerial.h> // << communication ardui

#ifndef STASSID

//#define STASSID "Bbox-0776BE57"     
//#define STAPSK  "5EE8A1CC23"            

#define STASSID "LaserGame"
#define STAPSK  "passwd00" 

#endif

void changeHostname(){
  // String routername = "LG11";
  // WiFi.hostname(routername); // used for id
  // not working: wifi_station_set_hostname(routername.c_str());
}

const char* ssid = STASSID;
const char* password = STAPSK;

String TagData = "";
String _hostname = "";

// << communication ardui
SoftwareSerial readerSerial( /*D7 GPIO13 rx */ 15   , /*D8 GPIO15 rx */ 13 );


ESP8266WebServer server(80);


String tempServerData = "";




// read from arduino pin
void blinkARDUINO(){

  if(Serial.available() > 0) {
    char r = Serial.read();
    //readerSerial.write(r);
    Serial.print("test: ");
    Serial.println(r);

    tempServerData += r;
  }

  if(readerSerial.available() == 0) return;

  while(readerSerial.available() != 0){   
    
    //byte
    char incomingData = readerSerial.read();
    byte test = incomingData;

    tempServerData += incomingData;
    
    /*Serial.print("rx arduino pin:  ");
    Serial.print(  incomingData  );
    Serial.print(  "   -   "  );
    Serial.println(  test  );  */

  }
  
  
  
  
  //tempServerData += ";";
  
}




void handleRoot() {
  if (server.method() == HTTP_POST) {
    String message = "";
    bool isServerPing = false;
    bool askingHostname = false;
    for (uint8_t i = 0; i < server.args(); i++) {
      String _argName = server.argName(i);
      String _arg = server.arg(i);
      message += " " + _argName + ": " + _arg + "\n";
      if(_argName == "player"){
        
        if(_arg == "I"){ // init mode erase buffer
          tempServerData = "";
        }
                
        // send data to arduino pin 
        readerSerial.print(_arg);


      } 
      if( _argName == "server"){
        isServerPing = true;
      }
      if( _argName == "hostname"){
        askingHostname = true;
      }
    }
    if(askingHostname){
       server.send(200, "text/plain", _hostname);
       return;
    }  
    if( isServerPing){
        server.send(200, "text/plain", tempServerData);
        //tempServerData = ""; // < todo
    }else
        server.send(200, "text/plain", "OK_HTTP_POST: \n"+ message);

  }else{
    server.send(200, "text/plain", "OK");
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "KO_404");
}

/*void sendToARDUINO(String _data){
  
}*/

void setup(void) {
  Serial.begin(9600);
  
  readerSerial.begin(9600);   // << communication arduin
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  
  changeHostname(); // << do nothing
  
  _hostname = String(WiFi.hostname());
  
   Serial.print("HOSTNAME: ");
   Serial.println(_hostname);

  server.on("/", handleRoot);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

TimedAction threadARDUINO = TimedAction( 10 /*ms*/, blinkARDUINO);
TimedAction threadWEBSRVR = TimedAction( 10 /*ms*/, blinkWEBSRVR);

void loop(void) {
  threadWEBSRVR.check();
  threadARDUINO.check();
}

void blinkWEBSRVR(){
  server.handleClient();
  MDNS.update();
}
