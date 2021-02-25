/*
  Anemometer demo

  The example demonstrates the use of anemometer.

  When the wind speed transmitter makes one round in one second, 
  the transmitter outputs 20 pulses, which means that the wind speed 
  is 1.75m/S.


  
*/

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1000;    // the debounce time; increase if the output flickers 

int pinInterrupt = 2; 

int Count=0;

void onChange()
{
   if ( digitalRead(pinInterrupt) == LOW )
      Count++;
}


void setup() {

  Serial.begin(115200); //Initialize serial port
  pinMode( pinInterrupt, INPUT_PULLUP);// set the interrupt pin


  //Enable
  attachInterrupt( digitalPinToInterrupt(pinInterrupt), onChange, FALLING);
   
}

void loop() {
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    Serial.print(Count*8.75);
    Count=0;
    Serial.println("cm/s");
  }
  delay(1); 

  
}
