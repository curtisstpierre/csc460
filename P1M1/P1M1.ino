// Project 1 Module 1 CSC 460
// Written By: Curtis St. Pierre and Mark Roller

// Pin assignments
const int buttonPin = 6;
const int rfPin = 13;
const int servoPin = 9;
const int potpin = 0;

// Knob to Servo values
int val = 0;
int waitTime = 0;

// Message creation variables
int message[10];
int pointerPos = 0;
char c = "A";
int i;

// Button trigger variables
int buttonState = 0; 
int messageSend = 0;
int buttonReset = 0;

void setup() {
  // Used for print
  //Serial.begin(9600);

  // Set rf Pin
  pinMode(rfPin, OUTPUT);
  // Set button Pin
  pinMode(buttonPin, INPUT);
  // Set servo Pin
  pinMode(servoPin, OUTPUT);

  // Set message Value
  buildMessage();

  // Clear timer config.
  TCCR3A = 0;
  TCCR3B = 0;
  
  // Set to CTC (mode 4)
  TCCR3B |= (1<<WGM32);

  // Set prescaler to 1
  TCCR3B |= (1<<CS30);

  // Set TOP value (0.0005 seconds)
  OCR3A = 8000;

  // Enable interupt A for timer 3.
  TIMSK3 |= (1<<OCIE3A);

  // Set timer to 0 (optional here).
  TCNT3 = 0;
  
  // PWM
  // Clear timer config
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 &= ~(1<<OCIE1C);
  
  // Set to Fast PWM (mode 15)
  TCCR1A |= (1<<WGM10) | (1<<WGM11);
  TCCR1B |= (1<<WGM12) | (1<<WGM13);

  // Enable output C.
  TCCR1A |= (1<<COM1C1);
  
  // No prescaler
  TCCR1B |= (1<<CS10);

  OCR1A = 421;  // 38khz frequency
  OCR1C = 0;    // Target
}

ISR(TIMER3_COMPA_vect){
  if(messageSend == 1){
    //Serial.print(message[pointerPos]);
    // Send a HIGH value
    if(message[pointerPos] == 1) {
      OCR1C = 140;
    }
    // Send a LOW value
    else{
      OCR1C = 0;
    }

    // Update position and check if finished message
    pointerPos+=1;
    if (pointerPos > 9){
      pointerPos = 0;
      messageSend = 0;
    }
  }
}

void buildMessage(){
  // Set start message bits followed by number (least significant first)
  message[0] = 1;
  message[1] = 0;
  
  for (i = 0; i < 10; i++) {
    message[i+2] = (c >> i) & 1;
  }
  
  //message[2] = 0;
  //message[3] = 0;
  //message[4] = 0;
  //message[5] = 0;
  //message[6] = 1;
  //message[7] = 1;
  //message[8] = 1;
  //message[9] = 1;
}

void loop() {
  buttonState = digitalRead(buttonPin);
  
  // Logic for handling button press and release
  if(buttonState == LOW && messageSend != 1 && buttonReset == 0){
    //Serial.print("1 \n");
    buttonReset = 1;
    messageSend = 1;
  }
  
  if(buttonState == HIGH && buttonReset == 1){
    buttonReset = 0;
  }
  
  // Moving the servo
  val = analogRead(potpin);
  val = map(val, 0, 1023, 10, 170);
  
  // This site showed me how to do this 
  // http://forum.arduino.cc/index.php?topic=5983.0
  waitTime = (val*11) + 500;
  digitalWrite(servoPin, HIGH);
  delayMicroseconds(waitTime);
  digitalWrite(servoPin, LOW);
  delay(50); 
}
