#include <SoftwareSerial.h>
#include "tempo.h"

bool first_loop_exec;

int rpm=1000;
int vel=500;
int temp = 50;
byte high_rpm,high_vel,low_rpm,low_vel;
byte buf_rpm[2],buf_vel[2],buf_temp;

unsigned long time_1=0;
unsigned long time_2=0;

#define GPS_INFO_BUFFER_SIZE 128
char GPS_info_char;
char GPS_info_buffer[GPS_INFO_BUFFER_SIZE];
unsigned int received_char;

int i; // counter
bool message_started;

SoftwareSerial mySerial_GPS(7, 8); // 7=RX, 8=TX (needed to communicate with GPS)

// REAL TIME SCHEDULER PARAMETERS AND VARIABLES
#define SCHEDULER_TIME 10000 // scheduler interrupt runs every 20000us = 20ms
#define DIVIDER_STD 200 // logging message sent every 100 scheduler times (20ms) 1s
#define DIVIDER_DELAY 500 // delay after forwarding meggages is 3s
unsigned int divider=0;
unsigned int divider_max=DIVIDER_DELAY;


// SENDS THE POLLING MESSAGE TO GPS
void scheduled_interrupt() 
{
  divider++;
  if (divider==divider_max) {
    divider=0;
    divider_max=DIVIDER_STD;
    time_1 = millis();
    mySerial_GPS.println("$PUBX,00*33"); // data polling to the GPS
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Connected");
  mySerial_GPS.begin(9600);
  mySerial_GPS.println("Connected");

  first_loop_exec=true;
  i=0;
  message_started=false;

  Timer1.initialize(); // initialize 10ms scheduler timer
  Timer1.setPeriod(SCHEDULER_TIME); // sets the main scheduler time in microseconds (10ms in this case)
  Timer1.attachInterrupt(scheduled_interrupt); // attaches the interrupt
  Timer1.start(); // starts the timer
}

void loop() { // run over and over

  if (first_loop_exec == true){
   /*delay(2000);
   mySerial_GPS.println(F("$PUBX,40,RMC,0,0,0,0*47")); //RMC OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,VTG,0,0,0,0*5E")); //VTG OFF
    delay(100);
    */mySerial_GPS.println(F("$PUBX,40,GGA,0,0,0,0*5A")); //CGA OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GSA,0,0,0,0*4E")); //GSA OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GSV,0,0,0,0*59")); //GSV OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GLL,0,0,0,0*5C")); //GLL OFF
    delay(1000);
    first_loop_exec = false;
  }

  // MANAGES THE CHARACTERS RECEIVED BY GPS
  while (mySerial_GPS.available()) {
    GPS_info_char=mySerial_GPS.read();
    if (GPS_info_char == '$'){ // start of message
      message_started=true;
      received_char=0;
    }else if (GPS_info_char == '*'){ // end of message
      /*time_2 = millis();
      Serial.print("Time,");
      Serial.print(time_1);
      Serial.print(",");
      Serial.print(time_2);
      Serial.println(",");
      */for (i=0; i<received_char; i++){
        Serial.write(GPS_info_buffer[i]); // writes the message to the PC once it has been completely received
      }
      Serial.println();
      message_started=false; // ready for the new message
    }else if (message_started==true){ // the message is already started and I got a new character
      if (received_char<=GPS_INFO_BUFFER_SIZE){ // to avoid buffer overflow
        GPS_info_buffer[received_char]=GPS_info_char;
        received_char++;
      }else{ // resets everything (overflow happened)
        message_started=false;
        received_char=0;
      }
    }
  }

  while (Serial.available()) {
    mySerial_GPS.write(Serial.read());
  }
  
}
