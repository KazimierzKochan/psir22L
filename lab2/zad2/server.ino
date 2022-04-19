#include <Ethernet.h>
#include <EthernetUdp.h>

#define HELLO_MESSAGE 0b10000000U
#define RESPONSE_MESSAGE 0b00000000U
#define REQUEST_MESSAGE 0b01000000U
#define REMOTE_PORT 8276
#define MESSAGE_COUNT 0b00111100U

// to the pins used:
const int analogInPin = A2;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

uint16_t sensorValue = 0;        // value read from the pot
uint16_t outputValue = 0;        // value output to the PWM (analog out)

EthernetUDP Udp;
byte mac[] = {0xb8, 0x27, 0xeb, 0xfb, 0xeb, 0x80};
unsigned int localPort = 8276;
byte packetNum = 1;
IPAddress remoteIP = IPAddress(10,17,1,155);

char incomingPacketBuffer[1];
char responsePacketBuffer[2];
char helloMessage[] = {(char)HELLO_MESSAGE, (char)0};


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);

  Ethernet.begin(mac);
  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);

  //send HELLO message
  Udp.beginPacket(remoteIP, REMOTE_PORT);
  
  int r = Udp.write(helloMessage, sizeof(helloMessage));
  Udp.endPacket();

  Serial.println("Sent: ");
  Serial.print(r);
  Serial.println("bytes");
  packetNum++;
  
}

void loop() {

  int packetSize = Udp.parsePacket();
  int packetBuffer;
  if(packetSize > 0){
    Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));
    
    if(incomingPacketBuffer[0] & REQUEST_MESSAGE){
      Serial.println("Server has received data request");
      int receivedPacketNum = (incomingPacketBuffer[0] & 0b00111100) >> 2;
      Serial.println("Packet num:");
      Serial.print(receivedPacketNum);
      //if(incomingPacketBuffer & 0b00111100 == packetNum)

      sensorValue = analogRead(analogInPin);
      outputValue = map(sensorValue, 0, 1023, 0, 255);
      //analogWrite(analogOutPin, outputValue);
  
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      
      //set output val
      responsePacketBuffer[0] = (char)outputValue;
      responsePacketBuffer[1] = (char)(outputValue >> 8);
      //set flag
      responsePacketBuffer[1] = (char)((responsePacketBuffer[1] & RESPONSE_MESSAGE) | responsePacketBuffer[1]);
      //set packet num
      receivedPacketNum++;
      responsePacketBuffer[1] = (char)(responsePacketBuffer[1] | (receivedPacketNum << 2));
      
      Serial.println("Read value: ");
      Serial.print(outputValue);
      Serial.print(F("Our dgram: "));
      Serial.print(responsePacketBuffer[0], BIN);
      Serial.print(responsePacketBuffer[1], BIN);
      Serial.print(F("\n"));
      
      Udp.write(responsePacketBuffer);
      Udp.endPacket();
      
    }
  }
}
