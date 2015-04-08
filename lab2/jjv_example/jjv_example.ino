#include <SPI.h>
#include <Ethernet.h>
#include <UDP.h>
#include <Time.h>  

// bit set / clear
#ifndef cbi
#define cbi(PORT, BIT) (_SFR_BYTE(PORT) &= ~_BV(BIT))
#endif
#ifndef sbi
#define sbi(PORT, BIT) (_SFR_BYTE(PORT) |= _BV(BIT))
#endif

// Circuit
// pin3 - LED -------- GND

byte timeServer[] = { 133,243,238,164 }; // ntp.nict.jp NTP server

const unsigned int localPort = 8888;   // local UDP port
const int NTP_PACKET_SIZE= 48;
byte packetBuffer[NTP_PACKET_SIZE];
byte timecode[60];
unsigned long lastNTPTime = 0;

void setup()
{
  Serial.begin(9600);

  // Ethernet settings
  byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };
  byte ip[] = { 192,168,0,177 };
  byte gateway[] = { 192,168,0,1 };
  byte subnet[] = { 255,255,255,0 };

  Ethernet.begin(mac, ip, gateway, subnet);
  delay(1000);
  Udp.begin(localPort);
  NTPSetTime();
  setupTimeCode();
}

void loop()
{
  int wait_start = second();
  while (wait_start == second()); // wait until time is corrected
  unsigned long startTime = millis();

  // generate 40khz from 3 pin using PWM
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = F_CPU / 2 / 40000/*hz*/;
  OCR2B = OCR2A / 2; /* 50% duty */
  sbi(TCCR2A,COM2B1);

  // print out current time
  Serial.print(year());
  Serial.print('/');
  Serial.print(month());
  Serial.print('/');
  Serial.print(day());
  Serial.print(' ');
  Serial.print(hour());
  Serial.print(':');
  Serial.print(minute());
  Serial.print(':');
  Serial.print(second());
  Serial.print('(');
  Serial.print(weekday());
  Serial.print(')');
  Serial.println(dayOfYear());

  // calc signal duration (ms)
  int ms = calcTimeCodeDuration();

  // wait ms and stop PWM
  while (millis() - startTime < ms);
  cbi(TCCR2A,COM2B1);
  
  if (millis() - lastNTPTime > 10*60*1000L) {
    NTPSetTime();
    lastNTPTime = millis();
  }
}

//=========================== NTP ===========================
void NTPSetTime()
{
  sendNTPpacket(timeServer);
  Serial.println("Waiting NTP response ...");
  delay(100);  // wait 100 ms to ensure the packet is sent

  if (!Udp.available()) { } // wait the reply packet
  Udp.readPacket(packetBuffer,NTP_PACKET_SIZE);
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  unsigned int fraction_hi = word(packetBuffer[44], packetBuffer[45]);

  // Unix time starts on Jan 1 1970, v.s. NTP ans is since Jan 1 1900.
  const unsigned long seventyYears = 2208988800UL;     
  unsigned long epoch = secsSince1900 - seventyYears;  

  // wait until next sencod
  delay(900 - fraction_hi / (65536/1000));

  // Set current time in JST (GMT+0900)
  setTime(epoch + 1 + 9*60*60);

  Serial.print("localtime = ");
  Serial.println(epoch);
}

unsigned long sendNTPpacket(byte *address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //send NTP request packet (port 123)
  Udp.sendPacket( packetBuffer,NTP_PACKET_SIZE,  address, 123); 
}


//=========================== JJY ===========================

unsigned int calcTimeCodeDuration()
{
  int s = second();
  if (s == 0)
    setupTimeCode();
  return timecode[s] * 100;
}

void setupTimeCode()
{
  int i;
  memset(timecode, 8, sizeof(timecode));

  setupTimeCode100(minute(), 0);
  timecode[0] = 2;

  setupTimeCode100(hour(), 10);

  int d = dayOfYear();
  setupTimeCode100(d/10, 20);
  setupTimeCode100(d%10*10, 30);

  int parity1 = 0, parity2 = 0;
  for (i = 12; i < 20; i++) parity1 ^= timecode[i] == 5;
  for (i =  1; i < 10; i++) parity2 ^= timecode[i] == 5;
  timecode[36] = parity1 ? 5 : 8;
  timecode[37] = parity2 ? 5 : 8;

  setupTimeCode100(year()%100, 40);
  for (i = 44; i > 40; i--)
    timecode[i] = timecode[i-1];
  timecode[40] = 8;

  int w = weekday() - 1;
  timecode[50] = (w & 4) ? 5 : 8;
  timecode[51] = (w & 2) ? 5 : 8;
  timecode[52] = (w & 1) ? 5 : 8;
  timecode[59] = 2;
  
  /* dump */
  for (i = 0; i < 60; i++) {
    Serial.print(timecode[i], DEC);
    Serial.print(i % 10 == 9 ? "\r\n" : " ");
  }
}

void setupTimeCode100(int m, int i)
{
  timecode[i+0] = ((m/10) & 8) ? 5 : 8;
  timecode[i+1] = ((m/10) & 4) ? 5 : 8;
  timecode[i+2] = ((m/10) & 2) ? 5 : 8;
  timecode[i+3] = ((m/10) & 1) ? 5 : 8;
  timecode[i+4] = 8;
  timecode[i+5] = ((m%10) & 8) ? 5 : 8;
  timecode[i+6] = ((m%10) & 4) ? 5 : 8;
  timecode[i+7] = ((m%10) & 2) ? 5 : 8;
  timecode[i+8] = ((m%10) & 1) ? 5 : 8;
  timecode[i+9] = 2;
}

int dayOfYear()
{
  tmElements_t tm = {0, 0, 0, 0, 1, 1, CalendarYrToTm(year())};
  time_t t = makeTime(tm);
  return (now() - t) / SECS_PER_DAY + 1;
}
