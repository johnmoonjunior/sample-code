/*
  Lightbar Test Sequencer
  
  This routine will run through a defined sequence of the controls to
  the lightbar to make sure that the outputs are correctly controlling the
  lightbar. The sequence it runs through is:
  1. Turn on the tailights and side markers using the Clearance Side Markers line. Pin 5
  2. Turn them off.
  3. Turn  on the tailights and side markers using the Tail Running Lights line. Pin 6
  4. Turn Stop lights on. Pin 7
  5. Turn Stop lights off.
  6. Turn Right Turn on. Pin 8
  7. Turn Right Turn off.
  8. Turn Left Turn on. Pin 9
  9. Turn Left Turn off.
  ""
*/
const int sidePin = 5;
const int tailrunPin = 6;
const int stopPin = 7;
const int rightPin = 8;
const int leftPin = 9;
// the setup function runs once when you press reset or power the board
void setup() {
  // Setup and initialize the output pins
  
  
  // initialize all of the outupt pins to output
  pinMode(sidePin, OUTPUT);
  pinMode(tailrunPin, OUTPUT);
  pinMode(stopPin, OUTPUT);
  pinMode(rightPin, OUTPUT);
  pinMode(leftPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(sidePin, HIGH);     // Turn on the tailights and side markers using the Clearance Side Markers line.
  delay(3000);                       // wait for 3  secondS
  digitalWrite(sidePin, LOW);      // Turn them ofF
  delay(3000);                       // wait for 3 seconds.
  digitalWrite(tailrunPin, HIGH);      // Turn  on the tailights and side markers using the Tail Running Lights line. Pin 6
   delay(3000);                       // wait for 3  secondS
  digitalWrite(stopPin, HIGH);         // Turn Stop lights on
  delay(3000);                      // wait for 3  secondS
  digitalWrite(stopPin, LOW);          // Turn Stop lights off
  delay(3000);                       // wait for 3  secondS
  digitalWrite(rightPin, HIGH);         // Turn Right Turn on
  delay(3000);                      // wait for 3  secondS
  digitalWrite(rightPin, LOW);          // Turn Right Turn off
  delay(3000);                       // wait for 3  secondS
  digitalWrite(leftPin, HIGH);         // Turn Left Turn on
  delay(3000);                      // wait for 3  secondS
  digitalWrite(leftPin, LOW);          // Turn Left Turn off
  delay (3000);                     // wait for 3 seconds.
  digitalWrite(tailrunPin, LOW);     // Turn off the tail lights.
  delay (3000);
}
