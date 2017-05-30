/*
 This sketch was created for a tutorial called Home Automation with the Arduino and the Amazon Echo Part 4
 That was presented on the ForceTronics YouTube Channel. This code is public domain for anybody to 
 use or modify at your own risk
 5/28/17

 Note that this code was leveraged from the Arduino WiFi101 examples and from a Sparkfun example 
 on using their cloud service Phant
 */

#include <SPI.h>
#include <WiFi101.h>
#include <Average.h>
#include "PhantMKR1K.h" //library was leveraged from Sparkfun's Phant library and modified to work on Arduino MKR1000

char ssid[] = "YourWiFiNetwork"; //  your network SSID (name)
char pass[] = "YourWiFiPassword";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

const int LED_PIN = 6; //MKR1000 LED
int status = WL_IDLE_STATUS; //set status variable
int baseLine = 0; //used to store baseline measurement for when washer is off

//define areas for phant cloud address and security keys
const char PhantHost[] = "data.sparkfun.com";
const char PublicKey[] = "YourPublicKey";
const char PrivateKey[] = "YourPrivateKey";

void setup() {
  pinMode(LED_PIN, OUTPUT); //setup LED pin
  digitalWrite(LED_PIN, LOW);
  analogReadResolution(12);
  calculateBaseValue();
 
  Serial.begin(57600);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    while (true);
  }
  
  
  Serial.println("about to connect...");
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
   status = WiFi.begin(ssid, pass);
 
  while (status != WL_CONNECTED) {
    //loop until we connect
  }

   delay(8000); //delay a bit for WiFi connection....

  Serial.println("connected now let's post data...");
}

void loop() {
  
  if (!postToPhant()) { /*(if true the post failed */ 
    Serial.println("failed to post data ");
   }

  //could go to sleep here
  delay(60000);
     
}


bool postToPhant()
{
  // LED turns on when we enter, it'll go off when we successfully post.
  digitalWrite(LED_PIN, HIGH);
  
  // Declare an object from the Phant library - phant
  PhantMKR1K phant(PhantHost, PublicKey, PrivateKey);
  //These calls build the web communication strings with Phant
//  Serial.print("baseline is ");
//  Serial.println(baseLine);
//  Serial.print("current value is ");
//  Serial.println(getCurrentValues());
  phant.add("washerstate", checkWasherState()); //get current washer state
  
  WiFiClient client; //Create client object to communicate with the phant server
 
  if (!client.connect(PhantHost, 80)) { //Attempt to connect to phant server using port 80
    // If we fail to connect, return 0.
    return 0;
  }

 // Serial.println(phantPost()); //if you want to see the string that is sent to phant server
  //Send post to phant server
  client.print(phant.post()); 
  
  // if there are incoming bytes available
  // from the server, read them and print them: not using any return data from server in this example
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Do something with data
  }
  client.stop();
 // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);
  
  return 1; // Return success
}

//check to see if washer is on, if it returns true washer is on
bool checkWasherState() {
  if(getCurrentValues() > (baseLine+30)) return true; //if the measurement is more than 30 above baseline then the washer is on
  else return false;
}

//gets an average of the highest values from 3 sets of measurements
int getCurrentValues() {
  int aSize = 3;
  Average<int> ave(aSize);
  
  for(int j=0; j<aSize; j++) ave.push(largestADCValue()); 
  
  return ave.mean(); //return mean of three sets of measurements
}

//gets a base value current measurement to establish what off state is
void calculateBaseValue() {
  int aSize = 6;
  Average<int> ave(aSize);
  bool done;
  float stD; 

  for(int j=0; j<aSize; j++) ave.push(largestADCValue()); //grab the highest value of six data sets
    baseLine = ave.mean(); //calculate baseline based off of average of 6 data sets
}

//make 50 measurements and return the highest value
int largestADCValue() {
  int aSize = 50;
  Average<int> mVal(aSize);
  // put your main code here, to run repeatedly:
  for(int i=0;i<aSize;i++) {
    mVal.push(analogRead(A0));
  }

  return  mVal.maximum();
}

//this function prints out the WiFi connection information
//Not using it in this example just here for debugging purposes
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println();
}
