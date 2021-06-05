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
  
  pinMode(SLIDESWITCHPIN, INPUT);
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
  firstLoop = true;
  radio.initialize(FREQUENCY, RECIEVENODEID, NETWORKID); // Initialize as reciever node
  Serial.print("Node ");
  Serial.print(RECIEVENODEID,DEC);
  Serial.println(" ready"); 

  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);
}

void loop()
{
  if(firstLoop){
    radio.initialize(FREQUENCY, RECIEVENODEID, NETWORKID);
    firstLoop = false;
    Serial.print("NODE ID changed to ");
    Serial.println(RECIEVENODEID,DEC);
  } 
  
  recieve();

}

//Function handling recieving data and translating it into output pins
void recieve()
{
  // RECEIVING

  // In this section, we'll check with the RFM69HCW to see
  // if it has received any packets:

  if (radio.receiveDone()) // Got one!
  {
    // Send an ACK if requested.
    // (You don't need this code if you're not using ACKs.)
    
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println("ACK sent");
    }
    
    char transmitterID[4]; // The ID of the transmitter that sent the message
    char message[11] = {'\0'}; // The 10 character message we are interpreting, plus a terminating null
    
    //Extract the transmitterID and message from the recieved packet
    memcpy(&message[0], &radio.DATA[4], 10);
    memcpy(&transmitterID[0], &radio.DATA[0], 4);
    
    
    // Print out the information:
    
    Serial.print("received from node ");
    Serial.print(radio.SENDERID, DEC);
    Serial.print(": [");

    // The actual message is contained in the DATA array,
    // and is DATALEN bytes in size:
    
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);

    // RSSI is the "Receive Signal Strength Indicator",
    // smaller numbers mean higher power.
    
    Serial.print("], RSSI ");
    Serial.println(radio.RSSI);
    
    Serial.print("Transmitter ID: ");
    Serial.print(transmitterID);
    Serial.println(";");
    
    Serial.print("Message content: ");
    Serial.print(message);
    Serial.println(";");

    
    //Output logic inverted to feed into relay
    if(strcmp(message,"LEFTON****") == 0)
      digitalWrite(LEFTLIGHTOUT, HIGH);
    else if(strcmp(message,"LEFTOFF***") == 0)
      digitalWrite(LEFTLIGHTOUT, LOW);
    else if(strcmp(message,"RIGHTON***") == 0)
      digitalWrite(RIGHTLIGHTOUT, HIGH);
    else if(strcmp(message,"RIGHTOFF**") == 0)
      digitalWrite(RIGHTLIGHTOUT, LOW);
    else if(strcmp(message,"TAILRUNON*") == 0)
      digitalWrite(TAILRUNOUT, HIGH);
    else if(strcmp(message,"TAILRUNOFF") == 0)
      digitalWrite(TAILRUNOUT, LOW);
    else if(strcmp(message,"STOPON****") == 0)
      digitalWrite(STOPLIGHTOUT, HIGH);
    else if(strcmp(message,"STOPOFF***") == 0)
      digitalWrite(STOPLIGHTOUT, LOW);
    else if(strcmp(message,"CLRSIDEON*") == 0)
      digitalWrite(CLRSIDEOUT, HIGH);
    else if(strcmp(message,"CLRSIDEOFF") == 0)
      digitalWrite(CLRSIDEOUT, LOW);

  }
}
