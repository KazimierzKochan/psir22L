//#include <Ethernet.h>
//#include <EthernetUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

const char* ssid="";
const char* password = "";

#define HELLO_MESSAGE 0b10000000U
#define RESPONSE_MESSAGE 0b00000000U
#define REQUEST_MESSAGE 0b01000000U
#define REMOTE_PORT 8276
#define MESSAGE_COUNT 0b00111100U

// to the pins used:
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

uint16_t sensorValue = 0;        // value read from the pot
uint16_t outputValue = 0;        // value output to the PWM (analog out)

WiFiUDP Udp;

unsigned int localPort = 8276;
byte packetNum = 0;

IPAddress clientIP = IPAddress(192,168,0,51);

char incomingPacketBuffer[1];
char responsePacketBuffer[2];
char helloMessage[2] = {(char)HELLO_MESSAGE, (char)0};


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
  
  Serial.println("Wifi Connected!");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP() );
  Udp.begin(localPort);

  //send HELLO message
  Udp.beginPacket(clientIP, REMOTE_PORT);
  
  int r = Udp.write(helloMessage, 2);
  Udp.endPacket();

  Serial.print("Sent hello message: ");
  Serial.print(r);
  Serial.println(" bytes");
  packetNum++;
  
}

void loop() {

  int packetSize = Udp.parsePacket();
  int packetBuffer;
  if(packetSize > 0){
    Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));
    
    if(incomingPacketBuffer[0] & REQUEST_MESSAGE){
      Serial.println("Server has received data request");
      byte receivedPacketNum = (incomingPacketBuffer[1] & 0b00111100) >> 2;
      Serial.print("Packet num: ");
      Serial.print(receivedPacketNum);
      Serial.print("\n");

      sensorValue = analogRead(analogInPin);
      outputValue = map(sensorValue, 0, 1023, 0, 255);
  
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      
      //set output val
      responsePacketBuffer[0] = (char)outputValue;
      responsePacketBuffer[1] = (char)(outputValue >> 8);
      //set flag
      responsePacketBuffer[1] = (char)(responsePacketBuffer[1] | (responsePacketBuffer[1] & RESPONSE_MESSAGE));
      //set packet num
      receivedPacketNum++;
      responsePacketBuffer[1] = (char)(responsePacketBuffer[1] | (receivedPacketNum << 2));
      
      Serial.print("Read value: ");
      Serial.print(outputValue);
      Serial.print(F("\nOur dgram: 0b"));
      Serial.print(responsePacketBuffer[0], BIN);
      Serial.print(" 0b");
      Serial.print(responsePacketBuffer[1], BIN);
      Serial.print(F("\n"));
      
      Udp.write(responsePacketBuffer, 2);
      Udp.endPacket();
      
    }
  }
}
