Servo myservo;  // create servo object to control a servo
Servo myservo2;

const int buttonPin = 6;     // the number of the pushbutton pin
const int rfPin = 13;

int potpin = 0;  // analog pin used to connect the potentiometer
int potpin2 = 1;
int val;    // variable to read the value from the analog pin
int val2;
int moving = 0;
int message[10];
int pointerPos = 0;
int buttonState = 0;         // variable for reading the pushbutton status
int messageSend = 0;
int buttonReset = 0;
void setup() {
  // attaches the servo on pin 9 an 8 to the servo object
  myservo.attach(9);
  myservo2.attach(8);

  //set sampling rate to 9600
  Serial.begin(9600);

  //set rf Pin
  pinMode(rfPin, OUTPUT);
  //set button Pin
  pinMode(buttonPin, INPUT);

  // set message Value
  buildMessage();

  //Clear timer config.
  TCCR3A = 0;
  TCCR3B = 0;
  //Set to CTC (mode 4)
  TCCR3B |= (1<<WGM32);

  //Set prescaler to 256
  TCCR3B |= (1<<CS30);

  //Set TOP value (0.05 seconds)
  OCR3A = 8000;

  //Enable interupt A for timer 3.
  TIMSK3 |= (1<<OCIE3A);

  //Set timer to 0 (optional here).
  TCNT3 = 0;

  //=======================

  //PWM
  //Clear timer config
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 &= ~(1<<OCIE1C);
  //Set to Fast PWM (mode 15)
  TCCR1A |= (1<<WGM10) | (1<<WGM11);
  TCCR1B |= (1<<WGM12) | (1<<WGM13);

  //Enable output C.
  TCCR1A |= (1<<COM1C1);
  //No prescaler
  TCCR1B |= (1<<CS10);

  OCR1A = 421;  //38mhz frequency
  OCR1C = 0;  //Target

}

ISR(TIMER3_COMPA_vect)
{
  //Slowly incrase the duty cycle of the LED.
  // Could change OCR1A to increase/decrease the frequency also.
  if (messageSend == 1){
    if(message[pointerPos] == 1) {
      OCR1C = 140;
    }
    else{
      OCR1C = 0;
    }

    pointerPos +=1 ;
    if (pointerPos > 9){
      pointerPos = 0;
      messageSend = 0;
    }
  }
}

void buildMessage(){

  //set start message bits followed by number (least significant first)
  message[0] = 1;
  message[1] = 0;
  message[2] = 0;
  message[3] = 0;
  message[4] = 0;
  message[5] = 0;
  message[6] = 1;
  message[7] = 1;
  message[8] = 1;
  message[9] = 1;
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && messageSend != 1 && buttonReset == 0){
    Serial.print("1 \n");
    buttonReset = 1;
    messageSend = 1;
  }
  if (buttonState == HIGH && buttonReset == 1){
    buttonReset = 0;
  }
  if (moving == 0){
    moving = 15;
    val = analogRead(potpin);  // reads the value of the potentiometer (value between 0 and 1023)
    val2 = analogRead(potpin2);
    val = map(val, 0, 1023, 0, 180);
    val2 = map(val2, 0, 1023, 0, 180);  // scale it to use it with the servo (value between 0 and 180)
    myservo.write(val);
    myservo2.write(val2);  // sets the servo position according to the scaled value
  }
  moving-= 1;

}
