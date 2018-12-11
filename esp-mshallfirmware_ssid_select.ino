#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <StreamString.h>

#include <WebSocketsClient.h>

#include <Hash.h>

#include <SimpleTimer.h>

//
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
//
SimpleTimer timer;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

// Parameters to change:

//#define R_ssid2use "ROUTER_ALIVE"
//#define R_password2use ""
//char * ssid = "ROUTER_ALIVE1";
//char * password = "";
#define R_host2use "alivehome.xyz"         // In double quotes eg: "10.3.100.207"
#define R_port2use 81
#define requestRead 16
#define bleState 14
String hardware_id = "DALIVESP";
String btmacadd = "C4:BE:84:09:FD:64";

String sessionprev = "";
String sessionnow = "";
int sessionVerification = 0;
int val = 0;
int count = 0;
int Connect=0;
int Disconnect=0;

int serverconnect = 0;

#define USE_SERIAL Serial
#define UNUSED_SERIAL Serial1

char * XORENC(char* in, char* key){
  int insize = strlen(in);
  int keysize = strlen(key);
  for(int x=0; x<insize; x++){
      in[x]=(in[x]^key[x]);
  }
  return in;
}

void sendPing() {
  if(serverconnect==1)webSocket.sendTXT("ping-"+hardware_id);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {


  switch (type) {
    case WStype_DISCONNECTED:
      {serverconnect = 0;}
      break;
    case WStype_CONNECTED:
      {
        while(btmacadd.length()<17){
        USE_SERIAL.println('n');
        delay(100);
        char c;
        btmacadd = "";
        String temp = "";
        while(USE_SERIAL.available()>0) {
        c = USE_SERIAL.read();
         if(c!='\n')
          temp += c;
          else if(c == '\n')break;
          delay(10);
        }
        btmacadd = temp.substring(2);
        }
        webSocket.sendTXT("HSIGN-"+hardware_id+"-"+btmacadd);
        serverconnect = 1;
      }
      break;
    case WStype_TEXT:
      {
        USE_SERIAL.println((char *)payload);
        String data = XORENC((char *)payload,"aswesdrftgrfdeyretgfrytghtyuij");
        USE_SERIAL.println(data);
        if(data.substring(0,7)=="SESSION"){sessionnow=data.substring(8,15);}
        else if(data.substring(0,11)=="PrevSESSION"){sessionprev=data.substring(12,19);
                                                      if(sessionprev==sessionnow){sessionVerification=1;}
                                                      else{ sessionVerification = 0;}                                                      
        }

        else if(sessionVerification==1){
        digitalWrite(requestRead, HIGH);
        delay(4);
        if(data == "OFF"){USE_SERIAL.println('a');}
        else if(data == "ONn"){USE_SERIAL.println('b');}
        else if(data == "STE"){USE_SERIAL.println('c');}
        digitalWrite(requestRead, LOW);
        sessionVerification = 0;
        }
      }
      break;
    case WStype_BIN:
      webSocket.sendTXT("Unknown Command");
      break;
  }

}

void setup() {
  UNUSED_SERIAL.begin(115200);
  USE_SERIAL.begin(115200);

  delay(5000);
  USE_SERIAL.println('p');

  WiFiManager wifiManager;
  
  pinMode(requestRead, OUTPUT);
  pinMode(bleState, INPUT);
  digitalWrite(requestRead, LOW);
  timer.setInterval(1000, sendPing);


wifiManager.setTimeout(20);

while(!wifiManager.autoConnect("DALIVESP", "DALIVES_P")) {
    //delay(3000);
    //ESP.reset();
    //delay(5000);
  } 

  webSocket.begin(R_host2use, R_port2use);
  webSocket.onEvent(webSocketEvent);
}

void sendStatus(char data)
{
  if(data == 'n'){ webSocket.sendTXT("DEVSTATE-"+hardware_id+"-turndown");}
  else if(data == 'o'){ webSocket.sendTXT("DEVSTATE-"+hardware_id+"turnon");}
  else if(data == 'p'){ webSocket.sendTXT("DEVSTATE-"+hardware_id+"-turndown-turnuptwo");}
 
}

void arduino_receive(void)
{
  char status_data;
   if (USE_SERIAL.available() > 0) {
   status_data = USE_SERIAL.read();
   if(serverconnect==1)sendStatus(status_data);
  }
}
void ble_connection_state(void)
{
  if(Connect==1){webSocket.sendTXT("PROXIMITY-"+hardware_id+"-Connected");digitalWrite(requestRead, HIGH);delay(4);USE_SERIAL.println('y');digitalWrite(requestRead, LOW);}
  if(Disconnect==1){webSocket.sendTXT("PROXIMITY-"+hardware_id+"-Disconnected");digitalWrite(requestRead, HIGH);delay(4);USE_SERIAL.println('x');digitalWrite(requestRead, LOW);}
}

void loop() {
  webSocket.loop();
  arduino_receive();
  timer.run();
  
//  val = digitalRead(bleState);
//  if(val==1)count++;
//  else if(val==0)count=1;
//  if(count>=1500){UNUSED_SERIAL.println("Connected");count=1500;Connect++;Disconnect=0;}
//  else if(count<1500){UNUSED_SERIAL.println("Disconnected");Connect=0;Disconnect++;}
//
//  ble_connection_state();
}
