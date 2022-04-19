#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>

#define HELLO_MESSAGE 0b10000000U
#define RESPONSE_MESSAGE 0b00000000U
#define REQUEST_MESSAGE 0b01000000U
#define REMOTE_PORT 8276
#define MESSAGE_COUNT 0b00111100U
#define RESPONSE_PACKET_SIZE 2
#define REQUEST_PACKET_SIZE 1
#define WAIT_TIME 3380
#define AWAIT_HELLO_STATE 0
#define AWAIT_RESPONSE_STATE 1
#define READY_FOR_REQUEST 2


void showResponse(byte *packetBuffer, size_t len);
void sendRequest(ZsutIPAddress ip, byte packetNum);

ZsutEthernetUDP Udp;
unsigned int localPort= REMOTE_PORT;
byte mac[] = {0xb8, 0x27, 0xeb, 0xfb, 0xeb, 0x80};
byte packetNum = 0;
ZsutIPAddress remoteIp = ZsutIPAddress(192,168,56,103);
unsigned long beginTime;

char requestPacketBuffer[1];
char incomingPacketBuffer[2];

uint16_t outputValue;
int packetSize;
int currentState = AWAIT_HELLO_STATE;

void setup() {
  Serial.begin(115200);

  ZsutEthernet.begin(mac);

  Serial.println(ZsutEthernet.localIP());
  Udp.begin(localPort);
  beginTime = ZsutMillis();
}

void loop() {

  int packetSize = Udp.parsePacket();
  int packetBuffer;

  switch (currentState){
    case AWAIT_HELLO_STATE:
      Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));
      packetSize = Udp.parsePacket()
      if(packetSize){
        if(incomingPacketBuffer[0] & HELLO_MESSAGE){
          packetNum = incomingPacketBuffer[0] >> 2;
          currentState = READY_FOR_REQUEST;
        }
      }

      break;
    case AWAIT_RESPONSE_STATE:
      Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));
      packetSize = Udp.parsePacket()
      if(packetSize){
        Serial.println("Client has received response. time:");
        Serial.print(ZsutMillis());
        packetNum = (incomingPacketBuffer[1] >> 2) & 0b00001111;
        outputValue = incomingPacketBuffer[0];
        outputValue = ((incomingPacketBuffer[1] & 0b00000011) << 8) | outputValue;
        Serial.println("Packet number:");
        Serial.print(packetNum);
        Serial.println("Value:");
        Serial.print(outputValue);
        currentState = READY_FOR_REQUEST;
      }
      break;
    case READY_FOR_REQUEST:
      if(ZsutMillis() - beginTime >= WAIT_TIME){
        
        requestPacketBuffer[0] = (char)REQUEST_MESSAGE;
        requestPacketBuffer[0] = requestPacketBuffer[0] & (packetNum << 2);
        Serial.println("Sending request = ");
        Serial.print(requestPacketBuffer[0], BIN);
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(requestPacketBuffer);
        Udp.endPacket();
        beginTime = ZsutMillis();
        
        currentState = AWAIT_RESPONSE_STATE;
      }
      break;
  }
}
