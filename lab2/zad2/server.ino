//#include <Ethernet.h>
//#include <EthernetUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

const char* ssid="";
const char* password = "";

#define HELLO_MESSAGE 0b10000000U
#define RESPONSE_MESSAGE 0b11000000U
#define REQUEST_MESSAGE 0b01000000U
#define REMOTE_PORT 8276
#define MESSAGE_COUNT 0b00111100U

//Potentiometer's pin
const int analogInPin = A0;
//value read from the pot
uint16_t sensorValue = 0;
uint16_t sendValue = 0;

WiFiUDP Udp;
unsigned int localPort = 8276;
IPAddress clientIP = IPAddress(192,168,0,51);

//buffers for messages
char incomingPacketBuffer[1];
char responsePacketBuffer[2];
char helloMessage[2];

byte packetNum = 1;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

    while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");        
  }
  
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP() );
  Udp.begin(localPort);

  //prepare hello message
  //set flag
  helloMessage[1] = (char)HELLO_MESSAGE;
  //set packet num
  helloMessage[1] = (char)(helloMessage[1] | (packetNum << 2));
  helloMessage[0] = (char)0;

  //send HELLO message
  Udp.beginPacket(clientIP, REMOTE_PORT);
  
  int r = Udp.write(helloMessage, 2);
  Udp.endPacket();

  Serial.print("+++Sent hello message to ");
  Serial.print(clientIP);
  Serial.println(" +++");
  packetNum++;
}

void loop() {
  int packetSize = Udp.parsePacket();
  int packetBuffer;
  if(packetSize > 0){
    Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));

    //if request
    if(incomingPacketBuffer[0] & REQUEST_MESSAGE){
      Serial.println("===Server has received data request");
      byte receivedPacketNum = (incomingPacketBuffer[0] & 0b00111100) >> 2;
      
      if(receivedPacketNum == packetNum){
        Serial.print("Packet num: ");
        Serial.print(packetNum);
        Serial.print("\n");
        if(packetNum == 15){
          Serial.println("Max packet num (15), starting from 1");
          packetNum = 1;
        }else{
          packetNum++;
        }
      }else{
        Serial.println("Received and local packet numbers don't match");
      }
      Serial.println("===");
      sensorValue = analogRead(analogInPin);
      sendValue = map(sensorValue, 0, 1023, 0, 1023);
      sendValue = constrain(sendValue, 0, 1023);
  
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      
      //set output val
      responsePacketBuffer[0] = (char)sendValue;
      responsePacketBuffer[1] = (char)(sendValue >> 8);
      //set flag
      responsePacketBuffer[1] = (char)(responsePacketBuffer[1] | (responsePacketBuffer[1] & RESPONSE_MESSAGE));
      //set packet num
      responsePacketBuffer[1] = (char)(responsePacketBuffer[1] | (packetNum << 2));
      Serial.print("---Sending response\n");
      Serial.print("Read value: ");
      Serial.print(sendValue);
      Serial.print("\n");
      Serial.print("Packet number: ");
      Serial.print(packetNum);
      if(packetNum == 15){
        Serial.println("Max packet num (15), starting from 1");
        packetNum = 1;
      }else{
        packetNum++;
      }
      
      Udp.write(responsePacketBuffer, 2);
      Udp.endPacket();
      
      Serial.print("\n---\n");
    }
  }
}
