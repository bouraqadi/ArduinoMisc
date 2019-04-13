/*
  Arduino Yún example from http://noury.tech

  This example for the YunShield/Yún shows how
  to use the Bridge library to turn ON and OFF 
  the on-board LED via REST calls.
  
  Commands can be sent on a web browser or the curl
  command line using the following URLs:

  "http://<myArduinoName>.local/arduino/on" turn ON the LED
  "http://<myArduinoName>.local/arduino/off" turn OFF the LED
  "http://<myArduinoName>.local/arduino/blink/2/s" toggles the LED every 2 seconds
  "http://<myArduinoName>.local/arduino/fade/50/300/ms" fades in or out the LED changing the intensity by 50 every 300 milliseconds. 
  Intensity step can be any value between 1 and 255.

*/

#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;
String command;
int ledState;
const int ledPin = 13;

unsigned long stepDuration;
unsigned long lastStepTime;
int ledFadingDelta;

void setup() {
  // Bridge startup
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Bridge.begin();
  digitalWrite(ledPin, LOW);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
 }

void loop() {
  // Get clients coming from server
  BridgeClient client = server.accept();

  // Skip if there is no client
  if (!client) {
     return doCommandStep();
  }
  // Process request
  processRequest(client);
 
  // Close connection and free resources.
  client.stop();
}

void doCommandStep(){
  if(command == "blink"){
    return blinkStep();
  }
  if(command == "fade"){
    return fadeStep();
  }
  delay(50); // Poll every 50ms
}

void processRequest(BridgeClient client) {
  client.setTimeout(0);
  client.available(); //Workaround bug. Ensures receving the full parameters
  command = readString(client);
  client.println(F("-----"));
  client.print(F("Received Command: "));
  client.println(command);
  client.println(F("-----"));
  
  if (command == "on"){
     return turnOn(client);
  }
  if (command == "off"){
      return turnOff(client);
  }
  if (command == "blink"){
       return startBlinking(client);
  }
  if (command == "fade"){
      return startFading(client);
  }
  client.println(F("Error: Unknown command!"));
  client.println(F("Valid commands: on, off, blink, fade"));
  client.println(F("Examples: "));
  client.println(F("http://<myArduinoName>.local/arduino/on"));
  client.println(F("---> turn ON the LED"));
  client.println();
  client.println(F("http://<myArduinoName>.local/arduino/off"));
  client.println(F("---> turn OFF the LED"));
  client.println();
  client.println(F("http://<myArduinoName>.local/arduino/blink/2/s"));
  client.println(F("---> toggles the LED ON/OFF every 2 seconds"));
  client.println();
  client.println(F("http://<myArduinoName>.local/arduino/fade/10/100/ms"));
  client.println(F("---> fades the LED in/out by 10 every 100 milliseconds."));
  client.println(F("Intensity step range is 1 to 255."));
}

void turnOn(BridgeClient client){
     digitalWrite(ledPin, HIGH);
     client.println(F("LED is now ON"));
}

void turnOff(BridgeClient client){
     digitalWrite(ledPin, LOW);
     client.println(F("LED is now OFF"));
}

String readString(BridgeClient client){
  String data = client.readStringUntil('/');
  int lastCharIndex = data.length() - 1;
  if(data[lastCharIndex] == '\n'){ // Drop end of line chararcter if any
    return data.substring(0, lastCharIndex - 1);
  }
  return data;
}

unsigned long readDuration(BridgeClient client){
   int stepValue = client.parseInt();
   if(stepValue == 0){
      return 100;
   }
   client.read(); // skip the / after the value
   String stepUnit = readString(client);
   if(stepUnit == "s"){
     return stepValue * 1000;
   }
   return stepValue;
}

void startBlinking(BridgeClient client){
  ledState = HIGH;
  digitalWrite(ledPin, ledState); 
  stepDuration = readDuration(client);
  client.println(F("Start blinking with LED initially ON")); 
  client.print(F("Toggling LED state every "));
  client.print(stepDuration);
  client.println(F(" ms"));
  lastStepTime = millis();
}

unsigned long deltaTimeSinceLastStep(){
  return millis() - lastStepTime;
}

void blinkStep(){
  if(deltaTimeSinceLastStep() < stepDuration){
    return;
  }
  if(ledState == LOW) {
    ledState = HIGH;
  } else{
    ledState = LOW;
  }
  digitalWrite(ledPin, ledState);
  lastStepTime = millis();
}

void startFading(BridgeClient client){
  ledState = 0;
  analogWrite(ledPin, ledState); 
  ledFadingDelta = client.parseInt();
  if(ledFadingDelta > 255){
    ledFadingDelta = 255;
  }
  if(ledFadingDelta < 1){
    ledFadingDelta = 1;
  }  
  stepDuration = readDuration(client);
  int totalSteps = 256 / ledFadingDelta;
  unsigned long totalDuration = stepDuration * totalSteps ;
  client.println(F("Start fading with LED initially OFF"));
  client.print(F("Each fading step will change LED intesity by "));
  client.println(ledFadingDelta);
  client.print(F("Each fading step will take "));
  client.print(stepDuration);
  client.println(F(" ms"));
  client.print(F("One full fading in/out cycle will take about "));
  client.print(totalDuration);
  client.println(F(" ms"));
  lastStepTime = millis();
}

void fadeStep(){
  if(deltaTimeSinceLastStep() < stepDuration){
    return;
  }
  ledState = ledState + ledFadingDelta;
  if(ledState < 0){
    ledState = 0;
  }
  if(ledState > 255){
    ledState = 255;
  }
  analogWrite(ledPin, ledState); 
  if(ledState >= 255 || ledState <= 0){
    ledFadingDelta = -1 * ledFadingDelta;
  }
  lastStepTime = millis();
}
