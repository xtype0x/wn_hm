#include <Time.h>
int pin = 6; // led pin for debug
int m,h,d,y,w,parity_M,parity_H,mon;

int M []={40,20,10,0,8,4,2,1};
int H []={20,10,0,8,4,2,1};
int D []={200,100,0,80,40,20,10,8,4,2,1};
int Y []={80,40,20,10,8,4,2,1};
int W []={4,2,1};
time_t t=now();
// 16000 / 8 / 2 / 25 = 40kHz

int eomYear[14][2] = {
  {0,0},      // Begin
  {31,31},    // Jan
  {59,60},    // Feb
  {90,91},    // Mar
  {120,121},  // Apr
  {151,152},  // May
  {181,182},  // Jun
  {212,213},  // Jul
  {243,244},  // Aug
  {273,274},  // Sep
  {304,305},  // Oct
  {334,335},  // Nov
  {365,366},  // Dec
  {366,367}   // overflow
};

void setup() {                
  pinMode(6,OUTPUT);        
  TCCR3A = _BV(COM3B0);
  TCCR3B = _BV(WGM32) | _BV(CS31);
  OCR3A = 17;
  Serial.begin(9600);
  setTime(12,30,54,13,4,2012);
}

// the loop routine runs over and over again forever:
void loop() {
 
   t=now();
   WWVB_simu(t);
   //JJY_simu(t);
   //delay(1000);
//analogWrite(pin,255);
//delay(800);
//  analogWrite(pin,0);
//  delay(200);
  //send_bit1();
  //send_bit0();
  //send_marker_bit();
}

void send_bit0(){
 //  Serial.print("0\n");
  //analogWrite(pin,HIGH);
  TCCR3A = _BV(COM3B0);
  delay(800);
  //analogWrite(pin,LOW);
  TCCR3A = _BV(COM3B1);
  delay(200);
}

void send_bit1(){
  // Serial.print("1\n");
  TCCR3A = _BV(COM3B0);
//  analogWrite(pin,HIGH);
  delay(500);
//  analogWrite(pin,LOW);
  TCCR3A = _BV(COM3B1);
  delay(500);
}

void send_marker_bit(){
 //  Serial.print("M\n");
  TCCR3A = _BV(COM3B0);
  //analogWrite(pin,HIGH);
  delay(200);
  //analogWrite(pin,LOW);
  TCCR3A = _BV(COM3B1);
  delay(800);
  
}

void JJY_simu(time_t t)
{
  m=minute(t);
  h=hour(t);
  d=day(t);
  y=year(t);
  w=weekday(t);
  mon=month(t);
  
  parity_M=false;
  parity_H=false;
  
  y = y % 100;
  w = w-1;
  mon = mon -1;
  
  if(y % 4 == 0 && y != 0){
    //Leap year
    d = d + eomYear[mon][1];
  }
  else{
    d = d + eomYear[mon][0];
  }   
  
  
  showtime(t);
  
  send_marker_bit();  //:00
  for(int i=0;i<8;i++){
    if(i == 3){
      send_bit0();
      continue;
    }  
    else if (m>=M[i])
          {send_bit1(); //:01 -08
           m=m-M[i];
           parity_M=!parity_M;}
    else  {send_bit0();} //:01 -08
  }
  send_marker_bit();  //:09
  send_bit0();   //:10
  send_bit0();   //:11
  ///////////////////  hours 
  for(int i=0;i<7;i++){
    if(i == 2){
      send_bit0();
      continue;
    }  
    else if (h>=H[i])
          {send_bit1(); //:12-18
           h=h-H[i];
           parity_H=!parity_H;}
    else  {send_bit0();} //:12-18
  } 
  send_marker_bit();  //:19 
  send_bit0();   //:20
  send_bit0();   //:21
  /////////////////////////day
  for(int i=0;i<7;i++){
    if(i == 2){
       send_bit0();
       continue; 
    }  
    if (d>=D[i])
          {send_bit1(); //:22-28
           d=d-D[i];}
    else  {send_bit0();} //:22-28
  } 
  send_marker_bit();  //:29
  for(int i=7;i<11;i++){
    if (d>=D[i])
          {send_bit1(); //:30-33
           d=d-D[i];}
    else  {send_bit0();} //:30-33
  }
  send_bit0();   //:34
  send_bit0();   //:35

  if(parity_H)
      send_bit1(); //:36
  else
      send_bit0(); //:36
      
  if(parity_M)
      send_bit1(); //:37
  else
      send_bit0(); //:37
  send_bit0(); //:38
  send_marker_bit();  //:39
  send_bit0(); //:40
  for(int i=0;i<8;i++){
    if (y>=Y[i])
          {send_bit1(); //:41-48
           y=y-Y[i];}
    else  {send_bit0();} //:41-48
  }  
  send_marker_bit();  //:49
  for(int i=0;i<3;i++){
    if (w>=W[i])
          {send_bit1(); //:50-52
           w=w-W[i];}
    else  {send_bit0();} //:50-52
  }
  send_bit0(); //:53
  send_bit0(); //:54
  send_bit0(); //:55
  send_bit0(); //:56
  send_bit0(); //:57
  send_bit0(); //:58
  send_marker_bit();  //:59
}

void WWVB_simu(time_t t)
{
  m=minute(t);
  h=hour(t);
  d=day(t);
  y=year(t);
  w=weekday(t);
  mon=month(t);
  
  y = y % 100;
  w = w-1;
  mon = mon -1;
  
  if(y % 4 == 0 && y != 0){
    //Leap year
    d = d + eomYear[mon][1];
  }
  else{
    d = d + eomYear[mon][0];
  }  
  
  showtime(t);
  
  send_marker_bit();  //:00
  for(int i=0;i<8;i++){
    if(i == 3){
      send_bit0();
       continue; 
    }  
    else if (m>=M[i])
          {send_bit1(); //:01 -08
           m=m-M[i];
           parity_M=!parity_M;}
    else  {send_bit0();} //:01 -08
  }
  send_marker_bit();  //:09
  send_bit0();   //:10
  send_bit0();   //:11
  ///////////////////  hours 
  for(int i=0;i<7;i++){
    if(i == 2 ){
        continue; 
    }  
    else if (h>=H[i])
          {send_bit1(); //:12-18
           h=h-H[i];}
    else  {send_bit0();} //:12-18
  } 
  send_marker_bit();  //:19 
  send_bit0();   //:20
  send_bit0();   //:21
  /////////////////////////day
  for(int i=0;i<7;i++){
    if(i == 2){
       send_bit0();
       continue; 
    }  
    else if (d>=D[i])
          {send_bit1(); //:22-28
           d=d-D[i];}
    else  {send_bit0();} //:22-28
  } 
  send_marker_bit();  //:29
  for(int i=7;i<11;i++){
    if (d>=D[i])
          {send_bit1(); //:30-33
           d=d-D[i];}
    else  {send_bit0();} //:30-33
  }
  send_bit0();   //:34
  send_bit0();   //:35

  //DUT1 sign set "-"
  send_bit0();   //:36
  send_bit1();   //:37
  send_bit0();   //:38
  send_marker_bit();  //:39
  send_bit0();   //:40
  send_bit1();   //:41
  send_bit1();   //:42
  send_bit0();   //:43
  send_bit0();   //:44

  for(int i=0;i<4;i++){
    if (y>=Y[i])
          {send_bit1(); //:45-48
           y=y-Y[i];}
    else  {send_bit0();} //:45-48
  }  

  send_marker_bit();  //:49

  for(int i=4;i<8;i++){
    if (y>=Y[i])
          {send_bit1(); //:50-53
           y=y-Y[i];}
    else  {send_bit0();} //:50-53
  }

  send_bit0(); //:54
  
  if(y % 4 == 0 && y != 0){
    send_bit1(); //:55	
  }
  else{
	send_bit0(); //:55
  }
  
  send_bit0(); //:56  assume leap second not set
  send_bit0(); //:57  dst not in effect  
  send_bit0(); //:58  dst not in effect  
  send_marker_bit();  //:59
}

void showtime(time_t t)
{
   Serial.print("hour:");
   Serial.print(hour(t));
   Serial.print("\n");
   
   Serial.print("minute:");
   Serial.print(minute(t));
   Serial.print("\n");
 
   Serial.print("second:");
   Serial.print(second(t));
   Serial.print("\n");
   
   Serial.print("day:");
   Serial.print(day(t));
   Serial.print("\n");

   Serial.print("weekday:");
   Serial.print(weekday(t));
   Serial.print("\n");   
   
   Serial.print("month:");
   Serial.print(month(t));
   Serial.print("\n");   
   
   Serial.print("year:");
   Serial.print(year(t));
   Serial.print("\n");  
   Serial.print(" //////////\n");  
}
