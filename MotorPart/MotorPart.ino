#include <LowPower.h>

#include <RFM69.h>
#include <SPI.h>

// Addresses for this node. CHANGE THESE FOR EACH NODE!

#define NETWORKID 0  // Must be the same for all nodes
#define MYNODEID 2   // My node ID
#define TONODEID 1   // Destination node ID

// RFM69 frequency, uncomment the frequency of your module:

#define FREQUENCY RF69_868MHZ

#define ENCRYPT true                   // Set to "true" to use encryption
#define ENCRYPTKEY "TOPSECRETPASSWRD"  // Use the same 16-byte key on all nodes

// Use ACKnowledge when sending messages (or not):

#define USEACK true  // Request ACKs or not

// Packet sent/received indicator LED (optional):

#define LED 9  // LED positive pin
#define GND 8  // LED ground pin

// Create a library object for our RFM69HCW module:

RFM69 radio;

int buttonStart = 7;
int buttonEnd = 8;
int led_pin = 13;
int motor_pin1 = 6;
byte speed = 50;
float sensorValue;
float sensorVolts;

int counter = 0;

void setup() {
  // Open a serial port so we can send keystrokes to the module:

  Serial.begin(9600);
  Serial.print("Node ");
  Serial.print(MYNODEID, DEC);
  Serial.println(" ready");

  pinMode(buttonStart, INPUT_PULLUP);
  pinMode(buttonEnd, INPUT_PULLUP);
  pinMode(motor_pin1, OUTPUT);
  pinMode(led_pin, OUTPUT);
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower();  // Always use this for RFM69HCW

  // Turn on encryption if desired:

  if (ENCRYPT) {
    radio.encrypt(ENCRYPTKEY);
  }
}

void loop() {
  if (radio.receiveDone()) {
    if (radio.ACKRequested()) {
      radio.sendACK();
      Serial.println("ACK sent");
    }
    unsigned long startTime = millis();

    while (millis() - startTime < 30000) {
      // Set up a "buffer" for characters that we'll send:
      static char sendbuffer[62];
      static int sendlength = 0;

      // SENDING

      // In this section, we'll gather serial characters and
      // send them to the other node if we (1) get a carriage return,
      // or (2) the buffer is full (61 characters).

      // If there is any serial input, add it to the buffer:

      if (Serial.available() > 0) {
        char input = Serial.read();

        if (input != '\r')  // not a carriage return
        {
          sendbuffer[sendlength] = input;
          sendlength++;
        }

        // If the input is a carriage return, or the buffer is full:

        if ((input == '\r') || (sendlength == 61))  // CR or buffer full
        {
          // Send the packet!


          Serial.print("sending to node ");
          Serial.print(TONODEID, DEC);
          Serial.print(", message [");
          for (byte i = 0; i < sendlength; i++)
            Serial.print(sendbuffer[i]);
          Serial.println("]");

          if (USEACK) {
            if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
              Serial.println("ACK received!");
            else
              Serial.println("no ACK received");
          } else  // don't use ACK
          {
            radio.send(TONODEID, sendbuffer, sendlength);
          }

          sendlength = 0;  // reset the packet
          Blink(LED, 10);
        }
      }

      // RECEIVING

      // In this section, we'll check with the RFM69HCW to see
      // if it has received any packets:
      if (radio.receiveDone())  // Got one!
      {
        // Print out the information:

        Serial.print("received from node ");
        Serial.print(radio.SENDERID, DEC);
        Serial.print(", message [");

        // The actual message is contained in the DATA array,
        // and is DATALEN bytes in size:

        for (byte i = 0; i < radio.DATALEN; i++)
          Serial.print((char)radio.DATA[i]);

        // RSSI is the "Receive Signal Strength Indicator",
        // smaller numbers mean higher power.

        Serial.print("], RSSI ");
        Serial.println(radio.RSSI);

        /* Try to read incoming string into double variable */
        double sensorValue = strtod(radio.DATA, NULL);
        Serial.println("Sensor value = ");
        Serial.print(sensorValue);
        Serial.print('\n');

        delay(100);
         if (sensorValue > 10) {
          Serial.println("TRUE");

          while (true) {

            if (digitalRead(buttonStart) == HIGH) {
              Serial.println("First");
              digitalWrite(motor_pin1, HIGH);
              
            } else if (digitalRead(buttonEnd) == HIGH) {
              Serial.println("Third");
              digitalWrite(motor_pin1, LOW);
              break;
            }
          }
        } else {

          Serial.println("FALSE");
        }

    // Send an ACK if requested.
    // (You don't need this code if you're not using ACKs.)
      } else {
        delay(50);
      }

      // Send an ACK if requested.

      if (radio.ACKRequested()) {
        radio.sendACK();
        Serial.println("ACK sent");
      }
      // Blink(LED, 10);
    }
    Serial.println("sleep");
    sleepForSeconds(24);
    Serial.println("work");
  }
}

void Blink(byte PIN, int DELAY_MS)
// Blink an LED for a given number of ms
{
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void sleepForSeconds(unsigned int seconds) {
  unsigned int sleepCycles = seconds / 8;
  for (unsigned int i = 0; i < sleepCycles; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
 }
}