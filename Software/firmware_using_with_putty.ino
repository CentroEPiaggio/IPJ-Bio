#include <CmdMessenger.h>
#include <Base64.h>
#include <Streaming.h>
#include <math.h>

// Mustnt conflict / collide with our message payload data. Fine if we use base64 library ^^ above
char field_separator = ',';
char command_separator = ';';
boolean conn=false;
///byte incomed=0;


int piezo=4;     // Pin Digitale a cui è collegato il l'anodo dell'OC

unsigned int times[3]= {16,50,1000}; // in questo vettore ho:  Periodo (ms), Dutycycle (%) & durata dell'onda quadra (ms)
unsigned int high;                
unsigned int low;
unsigned long m;  //dove salvo il tempo in ms

int systemstate;

// Attach a new CmdMessenger object to the default Serial port
CmdMessenger cmdMessenger = CmdMessenger(Serial, field_separator, command_separator);


enum
{
  kCOMM_ERROR    = 000, // Lets Arduino report serial port comm error back to the PC (only works for some comm errors)
  kACK           = 001, // Arduino acknowledges cmd was received
  kARDUINO_READY = 002, // After opening the comm port, send this cmd 02 from PC to check arduino is ready
  kERR           = 003, // Arduino reports badly formatted cmd, or cmd not recognised
  kSTATE         = 004,
 

  // Now we can define many more 'send' commands, coming from the arduino -> the PC, eg
  // kICE_CREAM_READY,
  // kICE_CREAM_PRICE,
  // For the above commands, we just call cmdMessenger.sendCmd() anywhere we want in our Arduino program.

  kSEND_CMDS_END, // Mustnt delete this line
};

// Commands we send from the PC and want to recieve on the Arduino.
// We must define a callback function in our Arduino program for each entry in the list below vv.
// They start at the address kSEND_CMDS_END defined ^^ above as 004
messengerCallbackFunction messengerCallbacks[] = 
{
 
  off, //005
  on, //006
  Times, //007
  NULL
};
// Its also possible (above ^^) to implement some symetric commands, when both the Arduino and
// PC / host are using each other's same command numbers. However we recommend only to do this if you
// really have the exact same messages going in both directions. Then specify the integers (with '=')


// ------------------ C A L L B A C K  M E T H O D S -------------------------
 
 

void off()
{
  // Message data is any ASCII bytes (0-255 value). But can't contain the field
  // separator, command separator chars you decide (eg ',' and ';')
  cmdMessenger.sendCmd(kACK,"off message recieved");
  systemstate = 0;
   
 }
 
 void on()
{
  // Message data is any ASCII bytes (0-255 value). But can't contain the field
  // separator, command separator chars you decide (eg ',' and ';')
  cmdMessenger.sendCmd(kACK,"on message recieved");
  systemstate = 1;
  
  Serial.print("Periodo ");
  Serial.println(times[0]);
  
  Serial.print("DutyCycle ");
  Serial.println(times[1]);
  
  high = times[0]*times[1]/100;
  low =  times[0]*(100-times[1])/100;
  
  Serial.print("milliseconds Up ");
  Serial.println(high);
  
  Serial.print("milliseconds down ");
  Serial.println(low);
  
  Serial.print("Durata onda quadra (ms) ");
  Serial.println(times[2]);

 }
 
 void Times ()
{
  // Message data is any ASCII bytes (0-255 value). But can't contain the field
  // separator, command separator chars you decide (eg ',' and ';')
  cmdMessenger.sendCmd(kACK,"Times message recieved");
  byte cmdindex=0;
  while ( cmdMessenger.available() )
   {
    char buf[350] = { '\0' };
    cmdMessenger.copyString(buf, 350);
    if(buf[0]) {    
    times[cmdindex]= atoi(buf);
    Serial.print("index ( 0 = Periodo, 1 = Dutycycle, 2 = Durata onda quadra)");
    Serial.println(cmdindex);
    Serial.print("times[index]");
    Serial.println(times[cmdindex]);
  
    cmdindex=cmdindex+1;
    
     }
   
   }
   
 }
 

// ------------------ D E F A U L T  C A L L B A C K S -----------------------

void arduino_ready()
{
  // In response to ping. We just send a throw-away Acknowledgement to say "im alive"
  cmdMessenger.sendCmd(kACK,"Arduino ready"); //change it!!
}

void unknownCmd()
{
  // Default response for unknown commands and corrupt messages
  cmdMessenger.sendCmd(kERR,"Unknown command");
}

// ------------------ E N D  C A L L B A C K  M E T H O D S ------------------



// ------------------ S E T U P ----------------------------------------------

void attach_callbacks(messengerCallbackFunction* callbacks)
{
  int i = 0;
  int offset = kSEND_CMDS_END;
  while(callbacks[i])
  {
    cmdMessenger.attach(offset+i, callbacks[i]);
    i++;
  }
}



void setup() {

 Serial.begin(9600);

 pinMode(piezo, OUTPUT);

  // cmdMessenger.discard_LF_CR(); // Useful if your terminal appends CR/LF, and you wish to remove them
  cmdMessenger.print_LF_CR();   // Make output more readable whilst debugging in Arduino Serial Monitor
  
  // Attach default / generic callback methods
  cmdMessenger.attach(kARDUINO_READY, arduino_ready);
  cmdMessenger.attach(unknownCmd);

  // Attach my application's user-defined callback methods
  attach_callbacks(messengerCallbacks);

  arduino_ready();

  // blink

 
}

// ------------------ M A I N ( ) --------------------------------------------

// Timeout handling

void loop() 
{
     cmdMessenger.feedinSerialData();    

switch (systemstate) {
     
     case 0: 
     
 {   // Serial.println ("OFF");
     
     digitalWrite( piezo, LOW );
     
 }
 
  break;
 
    case 1: 
     
 {   //Serial.println ("ON");
   
   m=millis();
   
   while(millis() <= m+times[2])  {
  
   digitalWrite( piezo,HIGH );
   delay(high);
   digitalWrite(piezo, LOW);
   delay(low);
   
   }
   
  }
 
  systemstate =0;
  break;
  
    
  }

}


