// Pin 13 has an LED connected on most Arduino boards.
// give it a name:

int pin = 6;

// the setup routine runs once when you press reset:
void setup() {                
  pinMode(pin,OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  send_bit1();
  send_bit0();
  send_marker_bit();
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
