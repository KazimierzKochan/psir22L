/*  
* PSIR 2022L Laboratorium 2 zadanie 1
* Monika Lewandowska, Kazimierz Kochan
 */

//niezbedne biblioteki
#include <ZsutDhcp.h>           
#include <ZsutEthernet.h>       
#include <ZsutEthernetUdp.h>    
#include <ZsutFeatures.h>       

#define UDP_SERVER_PORT         12375 //numer portu z tresci zadania - TODO zmienic na ten z instrukcji Kaz

byte MAC[]={0xb8, 0x27, 0xeb, 0xfb, 0xeb, 0x80}; //MAC adres karty sieciowej

//dlugosc pakietu z danymi dla/z UDP
#define PACKET_BUFFER_LENGTH        120
unsigned char packetBuffer[PACKET_BUFFER_LENGTH];

//numer portu na jakim nasluchujemy 
unsigned int localPort=UDP_SERVER_PORT;    

uint16_t t;

ZsutEthernetUDP Udp;       

unsigned int val = 0;

void setup() {
    //Zwyczajowe przywitanie z userem (niech wie ze system sie uruchomil poprawnie)
    Serial.begin(115200);
    Serial.print(F("Zsut eth udp server init... ["));Serial.print(F(__FILE__));
    Serial.print(F(", "));Serial.print(F(__DATE__));Serial.print(F(", "));Serial.print(F(__TIME__));Serial.println(F("]")); 

    //inicjaja karty sieciowej
    ZsutEthernet.begin(MAC);

    //potwierdzenie na jakim IP dzialamy
    Serial.print(F("My IP address: "));
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
        Serial.print(ZsutEthernet.localIP()[thisByte], DEC);Serial.print(F("."));
    }
    Serial.println();

    ZsutPinMode(ZSUT_PIN_Z2, INPUT); // TODO Zmienic zasob/pin na ten z instrukcji Kaz

    //Uruchomienie nasluchiwania na datagaramy UDP
    Udp.begin(localPort);
}

void loop() {
    val = ZsutDigitalRead(ZSUT_PIN_Z2); // wartosc od 0 do 1023
    packetBuffer[0] = val;
    Serial.print(F("Value od Z2: "));
    Serial.println(val);
    len = strlen(packetBuffer);

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(packetBuffer, len);
    Udp.endPacket();

    millis(2920); //TODO zmienic na czas z instrukcji KAZ
    
   
}

/*  Testy, nc to narzedzie netcat,
 *  dla windows pod https://eternallybored.org/misc/netcat/netcat-win32-1.12.zip)
 *  wywolaj z cmd (jedna linia):
 *  
 *  echo "test" | nc -w 1 -u 192.168.89.3 1234
 *  
 *  Info o nc:
 *  https://en.wikipedia.org/wiki/Netcat
 *  https://nc110.sourceforge.io/
 */
