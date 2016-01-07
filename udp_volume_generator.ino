#include <SD.h>
#include <SPI.h>
#include <ctype.h>
//#include <EthernetV2_0.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/wdt.h>

/* Rich Moore, Jan 4, 2016
 *   *  
 *  receives messages via UDP that control the amplitude of a squarewave. Message format is 2 ascii coded hex bytes
 *  Note: actual resolution is only 12-bits
 *  
 *    0xxx where x is [0-F]
 *    
 *    examples: 
 *    00FF    // note this is literal ascii string "00FF"
 *    0123    // "0123"    
 *    
 *    
 *    if an SD is used, it should contain a file called 'config.txt' with the following format:
 *    
 *    MAC address, 6 comma seperated ascii hex bytes
 *    IP address
 *    PORT
 *    
 *    Example:
 *    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
 *    192.168.11.21
 *    8888
 *    
 *    
 */

// defines for the oscillator
#define PERIOD  10000  // 100Hz
#define DAC_MAX_VAL 4095  // DACs are only 6 bits
#define MAX_LINE_LEN  80

// Ed's IP address is '192.168.35.150'
// Enter a MAC address and IP address for your controller below.
//[TODO] read this in from a file on the SD card
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
byte   myIp[4] = {192,168,35,150};
IPAddress ip(myIp[0], myIp[1],myIp[2],myIp[3]);
unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "ack";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
int packet_bytes;
unsigned int amplitude=DAC_MAX_VAL/4; // lower initial startup volume
int state=LOW;
unsigned int value;
unsigned long t_now=0;
unsigned long t_last=0;
unsigned long t_period=PERIOD;
unsigned long t_half_period=PERIOD/2;
File fd;
char config_filename[] = "config.txt";
char c,buf[MAX_LINE_LEN];

void setup() {
  Serial.begin(9600);

 
  // start the Ethernet and UDP:
  IPAddress ip(myIp[0], myIp[1], myIp[2], myIp[3]);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.print("IP address set to ");
  Serial.print(Ethernet.localIP());
  Serial.print(" Listening on port ");
  Serial.println(localPort);

  Serial.print("Setting amplitude to 0x");
  Serial.println(amplitude,HEX);
  dac_write(amplitude);

  // set Data direction registers
  DDRB = DDRB | B00000001;  // LSB of  Port B is D08, which is MSB of one DAC
  DDRC = DDRC | B00111111;  // Port C is normal Analog inputs A0-A5
  DDRD = DDRD | B11101100;  // Port D has pins 0-4 of one DAC. bits 0,1,4 are reserved for other uses
  
}

void loop() {

      unsigned int msb,lsb;
      int packetSize = Udp.parsePacket();
      if (packetSize) {
        
        // read the packet into packetBufffer
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        msb = fromHex(packetBuffer[0],packetBuffer[1]);
        lsb = fromHex(packetBuffer[2],packetBuffer[3]);        

        amplitude= (msb<<8) + lsb;

        Serial.print("Setting amplitude to 0x");
        Serial.println(amplitude,HEX);
        
        dac_write(amplitude);
        
        // echo the command sent back to the IP address and port that sent it
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(ReplyBuffer);
        Udp.endPacket();
      }


      
      // toggle the outputs at the defined frequency
      t_now=micros();   // get current time in microseconds
      
      // check for rollover, once every 70 min
      if (t_now < t_last)
        t_last=0;

      // have we waited long enough to toggle a 50% square wave
      if ((t_now-t_last) >= t_half_period) {
        // time to toggle outputs
        if (state == LOW) {
          // set state to high, set R & L DAC outputs to their respective amplitude values
          state = HIGH;
          dac_write(amplitude);        
        } else { 
          // set state to low, set R & L DAC outputs to zeroes
          state = LOW;
          dac_write(0);
      }
      t_last = t_now; // update time of last toggle
        

      } 
}

void dac_write(unsigned int dac_val) {
  // DAC bit mapping is:  MSB[PB0, PD7, PD6, PD5, PD3, PD2,PC5,PC4,PC3,PC2,PC1,PC0]LSB

#ifdef DEBUG
  Serial.print("dac_val=0b");
  Serial.println(dac_val,BIN);
  Serial.println("Before");
  Serial.print("PORTB=0x");
  Serial.println(PORTB,HEX);
  Serial.print("PORTC=0x");
  Serial.println(PORTC,HEX);
  Serial.print("PORTD=0x");
  Serial.println(PORTD,HEX);
#endif //DEBUG
  
  // DAC bits 0-5 are port C
  PORTC = (unsigned char)(dac_val & 0x03F);

  // DAC bit 11, PB0
  if (dac_val & 0x0800)
      PORTB = PORTB | 0x01;
  else
      PORTB = PORTB & ~0x01;

  // DAC bit 10, PD7
  if (dac_val & 0x0400)
    PORTD = PORTD | 0x080;
  else
    PORTD = PORTD & ~0x080;

  // DAC bit 9, PD6
  if (dac_val & 0x0200)
    PORTD = PORTD | 0x040;
  else
    PORTD = PORTD & ~0x040;

  // DAC bit 8, PD5
  if (dac_val & 0x0100)
    PORTD = PORTD | 0x020;
  else
    PORTD = PORTD & ~0x020;

  // DAC bit 7, PD3
  if (dac_val & 0x080)
    PORTD = PORTD | 0x08;
  else
    PORTD = PORTD & ~0x08;
    
  // DAC bit 6
  if (dac_val & 0x040)
    PORTD = PORTD | 0x04;
  else
    PORTD = PORTD & ~0x04;

#ifdef DEBUG
  Serial.println("After");
  Serial.print("PORTB=0x");
  Serial.println(PORTB,HEX);
  Serial.print("PORTC=0x");
  Serial.println(PORTC,HEX);
  Serial.print("PORTD=0x");
  Serial.println(PORTD,HEX);
#endif //DEBUG
  return;
}


uint8_t fromHex(char hi, char lo)
{
 uint8_t b;
 hi = toupper(hi);
 if( isxdigit(hi) ) {
   if( hi > '9' ) hi -= 7;      // software offset for A-F
   hi -= 0x30;                  // subtract ASCII offset
   b = hi<<4;
   lo = toupper(lo);
   if( isxdigit(lo) ) {
     if( lo > '9' ) lo -= 7;  // software offset for A-F
     lo -= 0x30;              // subtract ASCII offset
     b = b + lo;
     return b;
   } // else error
 }  // else error
 return 0;
}



