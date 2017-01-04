// These variables store the flash pattern
// and the current state of the LED
 
int ledPin =  2;      // the number of the LED pin
int ledState = LOW;             // ledState used to set the LED
unsigned long previousMillis = 0;        // will store last time LED was updated
long milliseconds = 1000;
long OnTime = 5*60*milliseconds;           // milliseconds of on-time
long OffTime = 15*60*milliseconds;          // milliseconds of off-time

void setup() 
{
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);      
}
 
void loop()
{
  // check to see if it's time to change the state of the LED
  unsigned long currentMillis = millis();
 
  if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
  {
    ledState = LOW;  // Turn it off
    previousMillis = currentMillis;  // Remember the time
    digitalWrite(ledPin, ledState);  // Update the actual LED
  }
  else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
  {
    ledState = HIGH;  // turn it on
    previousMillis = currentMillis;   // Remember the time
    digitalWrite(ledPin, ledState);    // Update the actual LED
  }
}
