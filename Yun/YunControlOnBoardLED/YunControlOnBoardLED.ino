/*
  Arduino Yún example from http://noury.tech

  This example for the YunShield/Yún shows how
  to use the Bridge library to turn ON and OFF 
  the on-board LED via REST calls.
  
  Commands can be sent on a web browser or the curl
  command line using the following URLs:

  "http://<myArduinoName>.local/arduino/on" turn ON the LED
  "http://<myArduinoName>.local/arduino/off" turn OFF the LED
  "http://<myArduinoName>.local/arduino/blink/300/ms" toggles the LED every 300 milliseconds
  "http://<myArduinoName>.local/arduino/fade/2/s" fades in or out the LED during 2 seconds

*/

#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;
String command;

void setup() {
  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Bridge.begin();
  digitalWrite(13, LOW);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
 }

void loop() {
  delay(50); // Poll every 50ms

  // Get clients coming from server
  BridgeClient client = server.accept();

  // Skip if there is no client
  if (!client) {
     return doOneStep();
  }
  // Process request
  processRequest(client);

  // Close connection and free resources.
  client.stop();
}

void doOneStep(){
  if(command == "blink"){
    return blinkStep();
  }
  if(command == "fade"){
    return fadeStep();
  }
}

String readString(BridgeClient client){
  String data = client.readStringUntil('/');
  int lastCharIndex = data.length() - 1;
  if(data[lastCharIndex] == '\n'){
    return data.substring(0, lastCharIndex-1);
  }
  return data;
}

void processRequest(BridgeClient client) {
  client.setTimeout(10);
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
}

void turnOn(BridgeClient client){
     digitalWrite(13, HIGH);
     client.println(F("LED is now ON"));
}

void turnOff(BridgeClient client){
     digitalWrite(13, LOW);
     client.println(F("LED is now OFF"));
}

int ledState;
unsigned long stepDuration;
unsigned long lastStepTime;

unsigned long readDuration(BridgeClient client){
   client.read(); // skip the /
   unsigned long stepValue = client.parseInt();
   client.read(); // skip the /
   String stepUnit = readString(client);
   if(stepUnit == "s"){
     return stepValue * 1000;
   }
   return stepValue;
}

void startBlinking(BridgeClient client){
  ledState = digitalRead(13);
  stepDuration = readDuration(client);
  client.print(F("Start blinking with LED initially ")); 
  if(ledState == HIGH){
    client.println(F("ON"));
  } else{
    client.println(F("OFF"));
  }
  client.print(F("Toggling LED state every "));
  client.print(stepDuration);
  client.println(F(" ms"));
  lastStepTime = millis();
}


void blinkStep(){
  unsigned long timeDelta = millis() - lastStepTime;
  if(timeDelta < stepDuration){
    return;
  }
  if(ledState == LOW) {
    ledState = HIGH;
  } else{
    ledState = LOW;
  }
  digitalWrite(13, ledState);
  lastStepTime = millis();
}

void startFading(BridgeClient client){
  
}

void fadeStep(){
  
}

