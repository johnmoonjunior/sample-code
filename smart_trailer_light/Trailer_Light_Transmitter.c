// EEPROM - Version: Latest 
#include <EEPROM.h>

// Program for using the RFM69HCW Transciever to recieve
// commands from a paired transmitter and convert them into
// appropriate outputs for a Semi-Truck brakelight system
//
// Uses the RFM69 library by Felix Rusu, LowPowerLab.com
// Original library: https://www.github.com/lowpowerlab/rfm69
// SparkFun repository: https://github.com/sparkfun/RFM69HCW_Breakout
//
// Modified last by John Moon Jr. on 02/06/2020 
//
// Covered under the GNU GPLv3.0: https://www.gnu.org/licenses/gpl-3.0.en.html

// Include the RFM69 and SPI libraries:

#include <RFM69.h>
#include <SPI.h>

// Addresses for this node.

#define NETWORKID     0   // Must be the same for all nodes (0 to 255)
#define TRANSMITNODEID      182   // My node ID (0 to 255)
#define RECIEVENODEID      214   // Destination node ID (0 to 254, 255 = broadcast)

// RFM69 frequency, uncomment the frequency of your module:

//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):

#define ENCRYPT       true // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

// Use ACKnowledge when sending messages (or not):

#define USEACK        true // Request ACKs or not

//Output pin for status LED (pairing and warnings)

#define STATUSLED 3

//Switch for choosing transmitter or reciever

#define SLIDESWITCHPIN  4 

//Input pins (wired to buttons for test, using analog in pins as GPIO):

#define CLRSIDEIN           A1 // Clearance / Side Lights
#define LEFTLIGHTIN           A2 // Left Brake Light
#define RIGHTLIGHTIN           A3 // Right Brake Light
#define STOPLIGHTIN           A4 // Stop Light
#define TAILRUNIN           A5 // All Tail Lights (dimmed)

// Output pins (wired to LEDs for test):

#define CLRSIDEOUT           5 // Clearance / Side Lights
#define LEFTLIGHTOUT           6 // Left Brake Light
#define RIGHTLIGHTOUT           7 // Right Brake Light
#define STOPLIGHTOUT           8 // Stop Light
#define TAILRUNOUT           9 // All Tail Lights (dimmed)

#define SETUPREG 17     // Register in EEPROM that holds the flag showing if this is the device's first run
#define SETUPREGFLAG 1 // Value to check for in EEPROM to see if device has run before

#define MESSAGELENGTH 14 // Length of the messages to transmit + 1 char for the Zero termination

// Create a library object for our RFM69HCW module:

RFM69 radio;
char DevID[4] = {}; // 4 alphanumeric characters uniquely identifying this device

// Variables to keep track of the previous state of the input pins
int lastClrSideState = LOW;
int lastLeftLightState = LOW;
int lastRightLightState = LOW;
int lastStopLightState = LOW;
int lastTailRunState = LOW;

// Variables to read in current input pin state
int clrSideState = LOW;
int leftLightState = LOW;
int rightLightState = LOW;
int stopLightState = LOW;
int tailRunState = LOW;

bool firstLoop; //Bool to tell if device is in transmit or recieve mode

//Variable to track time spent waiting for an ACK
unsigned long startTime;

void setup()
{
  // Open a serial port so we can send keystrokes to the module:
  Serial.begin(9600);

  //See if device has been run before; if yes, read DevID if stored in EEPROM 
  if(EEPROM.read(SETUPREG) == SETUPREGFLAG){
    for(int i=0;i<4;i++){
    DevID[i] = EEPROM.read(i);
    }
  }
  
  //Otherwise initalize DevID
  else{
    EEPROM.update(SETUPREG, SETUPREGFLAG);
    randomSeed(analogRead(0));
    int randAlph;
    for(int i=0;i<4;i++){
      randAlph = random(0,62);
      if(randAlph > 35){
        randAlph = 'A' + (randAlph-36);
      }
      else if(randAlph > 9){
        randAlph = 'a' + (randAlph-10);
      }
      else{
        randAlph = '0' + randAlph;
      }
      DevID[i] = char(randAlph);
      EEPROM.update(i, DevID[i]);
    }
  }
  
  Serial.print("DevID: ");
  Serial.println(DevID);
  
  
  
  //Setup inputs
  
  //pinMode(SLIDESWITCHPIN, INPUT);
  pinMode(CLRSIDEIN, INPUT);
  pinMode(LEFTLIGHTIN, INPUT);
  pinMode(RIGHTLIGHTIN, INPUT);
  pinMode(STOPLIGHTIN, INPUT);
  pinMode(TAILRUNIN, INPUT);

  // Set up the output LEDs
  
  pinMode(CLRSIDEOUT,OUTPUT);
  digitalWrite(CLRSIDEOUT,LOW);  
  pinMode(LEFTLIGHTOUT,OUTPUT);
  digitalWrite(LEFTLIGHTOUT,LOW);
  pinMode(RIGHTLIGHTOUT,OUTPUT);
  digitalWrite(RIGHTLIGHTOUT,LOW);
  pinMode(STOPLIGHTOUT,OUTPUT);
  digitalWrite(STOPLIGHTOUT,LOW);
  pinMode(TAILRUNOUT,OUTPUT);
  digitalWrite(TAILRUNOUT,LOW);
  
  pinMode(STATUSLED,OUTPUT);
  digitalWrite(STATUSLED,LOW);
    
  // Initialize the RFM69HCW:
  //  radio.setCS(10);  //uncomment if using Pro Micro
  radio.initialize(FREQUENCY, TRANSMITNODEID, NETWORKID); // Initialize as transmitter node
  Serial.print("Node ");
  Serial.print(TRANSMITNODEID,DEC);
  Serial.println(" ready"); 

 
  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);
    
  firstLoop = true;
}

void loop()
{
  if(firstLoop){
  radio.initialize(FREQUENCY, TRANSMITNODEID, NETWORKID);
  Serial.print("NODE ID changed to ");
  Serial.println(TRANSMITNODEID,DEC);
  firstLoop = false;
  }
  transmit();
}

//Function handling input pin checking to see if a message needs to be sent
void transmit()
{
   // Set up a "buffer" for characters that we'll send; message format is [4 char DevID, 10 char message, end with Zero termination character]
  
  static char sendbuffer[MESSAGELENGTH];

  // SENDING

  // Read inputs, then check against previous state
  
  clrSideState = digitalRead(CLRSIDEIN);
  leftLightState = digitalRead(LEFTLIGHTIN);
  rightLightState = digitalRead(RIGHTLIGHTIN);
  stopLightState = digitalRead(STOPLIGHTIN);
  tailRunState = digitalRead(TAILRUNIN);
  
  if(clrSideState != lastClrSideState){
    
    if(clrSideState == HIGH){
      memcpy(&sendbuffer[0], &DevID[0], 4);
      memcpy(&sendbuffer[4], "CLRSIDEON*", 10);
      sendMessage(sendbuffer);
    }
    else{
    memcpy(&sendbuffer[0], &DevID[0], 4);
    memcpy(&sendbuffer[4], "CLRSIDEOFF", 10);
    sendMessage(sendbuffer);
    }
    
  }
  
  if(leftLightState != lastLeftLightState){
    
    if(leftLightState == HIGH){
      memcpy(&sendbuffer[0], &DevID[0], 4);
      memcpy(&sendbuffer[4], "LEFTON****", 10);
      sendMessage(sendbuffer);
    }
    else{
    memcpy(&sendbuffer[0], &DevID[0], 4);
    memcpy(&sendbuffer[4], "LEFTOFF***", 10);
    sendMessage(sendbuffer);
    }
    
  }

  if(rightLightState != lastRightLightState){
    
    if(rightLightState == HIGH){
      memcpy(&sendbuffer[0], &DevID[0], 4);
      memcpy(&sendbuffer[4], "RIGHTON***", 10);
      sendMessage(sendbuffer);
    }
    else{
    memcpy(&sendbuffer[0], &DevID[0], 4);
    memcpy(&sendbuffer[4], "RIGHTOFF**", 10);
    sendMessage(sendbuffer);
    }
    
  }

  if(stopLightState != lastStopLightState){
    
    if(stopLightState == HIGH){
      memcpy(&sendbuffer[0], &DevID[0], 4);
      memcpy(&sendbuffer[4], "STOPON****", 10);
      sendMessage(sendbuffer);
    }
    else{
    memcpy(&sendbuffer[0], &DevID[0], 4);
    memcpy(&sendbuffer[4], "STOPOFF***", 10);
    sendMessage(sendbuffer);
    }
    
  }

  if(tailRunState != lastTailRunState){
    
    if(tailRunState == HIGH){
      memcpy(&sendbuffer[0], &DevID[0], 4);
      memcpy(&sendbuffer[4], "TAILRUNON*", 10);
      sendMessage(sendbuffer);
    }
    else{
    memcpy(&sendbuffer[0], &DevID[0], 4);
    memcpy(&sendbuffer[4], "TAILRUNOFF", 10);
    sendMessage(sendbuffer);
    }
    
  }
  
  // Set previous state for each input
  lastClrSideState = clrSideState;
  lastLeftLightState = leftLightState;
  lastRightLightState = rightLightState;
  lastStopLightState = stopLightState;
  lastTailRunState = tailRunState;
}

//Function to transmit the message to a reciever
void sendMessage(char* message)
{
  Serial.print("sending to node ");
  Serial.print(RECIEVENODEID, DEC);
  Serial.print(": [");
  Serial.print(message);
  Serial.println("]");
  
  // There are two ways to send packets. If you want
  // acknowledgements, use sendWithRetry():
  
  if (USEACK)
  {
    startTime = millis();
    while (!(radio.sendWithRetry(RECIEVENODEID, message, MESSAGELENGTH))){
      Serial.println("Waiting for ACK");
      if(millis() - startTime > 3000){
        //TODO: Implement warning function if no ACK is recieved in a certain time 
        digitalWrite(STATUSLED,HIGH);
      }
    }
    digitalWrite(STATUSLED,LOW);
    Serial.println("ACK received!");
    Serial.print("time spent waiting: ");
    Serial.print(millis()-startTime);
    Serial.println("ms");
  }

  // If you don't need acknowledgements, just use send():
  
  else // don't use ACK
  {
    radio.send(RECIEVENODEID, message, MESSAGELENGTH);
  }
}
