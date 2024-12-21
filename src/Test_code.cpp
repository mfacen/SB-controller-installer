#include <Arduino.h>
//#include <SoftwareSerial.h> //including software serial library
void sendSMS(String,String); // function prototype 
void setup()
{
  Serial2.begin(9600);   // Setting the baud rate of GSM Module  
  Serial.begin(115200);    // Setting the baud rate of Serial Monitor (Arduino)

  //sendSMS(); // send SMS
Serial2.write("AT\r\n");
}
long lastMillis = 0;
void loop()
{
  if (millis() - lastMillis > 20000) {
    lastMillis = millis();
    //sendSMS("+529541632028", "Hello World"); // Send SMS to the given number with the given message
    //Serial.println("Hello World");
  }
   if (Serial2.available()>0)
   Serial.write(Serial2.read()); // prints the data collected from software serial port to serial monitor of arduino
    if (Serial.available()>0)
    Serial2.write(Serial.read()); // prints the data collected from serial monitor to software serial port of arduino
}
// void sendSMS() {
//   // AT command to set SIM900 to SMS mode
//   Serial2.print("AT+CMGF=1\r"); 
//   delay(200);

//   // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
//   // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
//   Serial2.println("AT+CMGS=\"+529541632028\""); 
//   delay(200);
  
//   // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
//   Serial2.println("Message example from Arduino Uno."); 
//   delay(200);

//   // End AT command with a ^Z, ASCII code 26
//   Serial2.println((char)26); 
//   delay(200);
//   Serial2.println();
//   // Give module time to send SMS
//   delay(5000); 
// }

// Function to send AT commands and wait for response
// cmd: AT command to send
// res: Expected response
// timeout: Timeout duration
void sendATCommand(String cmd, char *res, unsigned int timeout) {
  Serial2.println(cmd);
  bool foundResponse = false;
  unsigned long startTime = millis();

  while (millis() - startTime < timeout) {
    if (Serial2.available()) {
      String response = Serial2.readStringUntil('\n');
      Serial.println(response);
      if (response.indexOf(res) != -1) {
        foundResponse = true;
        break;
      }
    }
  }

  if (!foundResponse) {
    Serial.println("Timeout waiting for response");
  }
}

// Function to send SMS
void sendSMS(String phoneNumber, String message) {
  // Command to start SMS to the given number
  String cmd = "AT+CMGS=\"" + phoneNumber + "\"";
  sendATCommand(cmd, ">", 1000); // Wait for the prompt

  // Send message content
  Serial2.print(message);
  Serial2.write(26); // Ctrl+Z to send the SMS

  // Wait for response
  sendATCommand("", "OK", 10000); // Long timeout because SMS sending can take time
  Serial.println("SMS Sent!");
}