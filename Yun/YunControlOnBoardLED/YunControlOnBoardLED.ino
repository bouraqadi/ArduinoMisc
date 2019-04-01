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
     return;
  }
  // Process request
  processRequest(client);

  // Close connection and free resources.
  client.stop();
}

void processRequest(BridgeClient client) {
  client.setTimeout(10);
  String command = client.readStringUntil('/');
  if(command[command.length() - 1] == '\n'){
    client.println("Command ends with \\n");
    command = command.substring(0, command.length()-2);
  }
  client.println(F("-----"));
  client.print(F("Received Command: "));
  client.print(command);
  client.println(F("-----"));
  
  if (command == "on"){
     digitalWrite(13, HIGH);
     client.println(F("turnOn(client)"));
     Bridge.put("D13", "HIGH");
     return;
  }
  if (command == "off"){
      digitalWrite(13, LOW);
       client.print(F("turnOff(client)"));
     Bridge.put("D13", "LOW");
      return;
  }
  if (command == "blink"){
      client.print(F("blink(client)"));
      return;
  }
  if (command == "fade"){
      client.print(F("fade(client)"));
      return;
  }
}
