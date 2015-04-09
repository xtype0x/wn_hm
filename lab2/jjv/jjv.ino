
int pin = 6; // led pin for debug

// 16000 / 8 / 2 / 25 = 40kHz
void setup() {                
  pinMode(6,OUTPUT);        
  TCCR3A = _BV(COM3B0);
  TCCR3B = _BV(WGM32) | _BV(CS31);
  OCR3A = 25;
}

// the loop routine runs over and over again forever:
void loop() {
//  analogWrite(pin,255);
//  delay(800);
//  analogWrite(pin,0);
//  delay(200);
//  send_bit1();
//  send_bit0();
//  send_marker_bit();
}

void send_bit0(){
  analogWrite(pin,HIGH);
  delay(800);
  analogWrite(pin,LOW);
  delay(200);
}

void send_bit1(){
  analogWrite(pin,HIGH);
  delay(500);
  analogWrite(pin,LOW);
  delay(500);
}

void send_marker_bit(){
  analogWrite(pin,HIGH);
  delay(200);
  analogWrite(pin,LOW);
  delay(800);
}
