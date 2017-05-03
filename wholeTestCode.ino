/*
 A sketch that controls the output of pins using GET requests on an Arduino-hosted website
 Dervied from http://www.filearchivehaven.com/2014/05/25/arduino-controlling-leds-remotely-wi-fi-using-arduino-yun/
 Modified by Jason Lim and Ofir Zeevi
 27 April 2016
*/

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

//Pins Arduino is using
#define LEDPIN  5
#define MOTORPIN  3
#define HEATPIN 2
#define LIGHT_INPUT 12
#define TEMP_INPUT 11

//Constants for Arduino I/O
#define MIN_INPUT 0
#define MAX_VOLTS 5
#define MAX_A_INPUT 1023
#define MAX_DIG_OUTPUT 255

//Constants for Temperature: 
#define TR1        150000
#define TR2        10000
#define TCONST_A   0.007796168
#define TCONST_B   3740000
#define TCONST_H   
#define TABS_ZERO -273
#define TVOLT_IN   9
#define TEMP_BUFFER 2

#define MIN_FAN_VOLTS 31

#define MIN_INPUT 0
#define MAX_VOLTS 5
#define MAX_A_INPUT 1023



//Code to send sections of the Website
void serveWebHead(YunClient client);
void serveWebLight(YunClient client);
void serveWebTemp(YunClient client);
void serveWebFan (YunClient client);
void serveWebHeat(YunClient client);

//Code to adjust the house
//void adjustLight(int lightPreference);
void fixedLight (int lightPercent);
void adjustTemp(float tempPreference);
void adjustFan(int fanSpeed);
void activateHeat(void);
void deactivateHeat(void);
float getCurrTemp(void);

YunServer server;

void setup() {
  // Start our connection
  Serial.begin(9600);
  pinMode(LEDPIN,OUTPUT);
  pinMode(MOTORPIN,OUTPUT);
  pinMode(HEATPIN,OUTPUT);

  pinMode(LIGHT_INPUT,INPUT);
  pinMode(TEMP_INPUT,INPUT);
  
  digitalWrite(LEDPIN,HIGH);
  
  
  // turn on Led while connecting
  //digitalWrite(MOTORPIN,HIGH); // turn on Motor while connecting
  Bridge.begin();  

  // Show a fancy flash pattern once connected
  digitalWrite(LEDPIN,LOW); 
  delay(150);
  digitalWrite(LEDPIN,HIGH); 
  delay(150);
  digitalWrite(LEDPIN,LOW); 
  delay(150);
  digitalWrite(LEDPIN,HIGH); 
  delay(150);
  digitalWrite(LEDPIN,LOW); 
  delay(150);
  
  // Disable for some connections:
  // Start listening for connections  
  
  // server.listenOnLocalhost();
  server.begin();
 
}

void loop() {
  // Listen for clients
  YunClient client = server.accept();
  // Client exists?
  if (client) {
    // Lets process the request!
    process(client);
    client.stop();
  }
  delay(50);
}

void process(YunClient client) {
  // Collect user commands
  String comm = client.readStringUntil('~'); // load whole string
  int counter = 0;
  char command[200];
  while(comm[counter] != '\0') {
    command[counter] = comm[counter];
    counter++;
  }
  
  // Enable HTML
  client.println("Status: 200");
  client.println("Content-type: text/html");
  client.println();
  
  // Show UI
//  client.println("<B><Center>");
//  client.println("<a href='http://team17smarterhomesarduino.local/arduino/light/on~'>Turn ON LED</a><br>");
//  client.println("<a href='http://team17smarterhomesarduino.local/arduino/light/off~'>Turn OFF LED</a><br>");

  // Motor Control
//  client.println("<a href='http://team17smarterhomesarduino.local/arduino/motor/on~'>Turn ON MOTOR</a><br>");
//  client.println("<a href='http://team17smarterhomesarduino.local/arduino/motor/off~'>Turn OFF MOTOR</a><br>");

  
//  client.println(command);
//  client.println("</B></Center>");
  
  // Check what the user entered ...
  
  // Turn on
  char lightResponse[50] = "The current Brightness is: ";
  char tempResponse[50] = "The selected Temperature is: ";
  char fanResponse[50] = "The current Fan speed is: ";
  char heatResponse[50] = "The Heater is currently OFF";
  char cat[20];
  int numb = 0; 
  //Scans string, extracting a string until a slash, then extracts an int.
  sscanf(command,"%[^/]/%d",cat,&numb);
  if (numb > 100) {
    numb = 100;
  } else if (numb < 0) {
    numb = 0;
  }
  
//If category is lights (auto or manually adjusted)  
  if (cat == "auto_lights") {
    sprintf(&lightResponse[27],"%d",numb);    
    adjustLight(numb);    
    
  } else if (cat == "manual_lights") {
    sprintf(&lightResponse[27],"%d",numb);
    fixedLight(numb);
  }
//  if category is temperature
  if (cat == "temp") {
    
    float number = numb;
    if (numb%5 != 0) {
      number = numb-0.5;
    }
    sprintf(&tempResponse[29],"%.1f",number);
    adjustTemp(number);
  }  
  //If category is 'fan'
  if (cat == "fan") {
    sprintf(&fanResponse[26],"%d%%",numb);
    adjustFan(numb);
  }
//If category is Heat
  if (cat == "heat") {
    if (numb == 0) {
      deactivateHeat();
         
    } else if (numb == 100) {
      activateHeat();
     heatResponse[27] = 'N';
     heatResponse[28] = '\0';
    }
  }

 //Send the webpage
 // serveWebHead(client);
  
//  serveWebLight(client);
  client.println(lightResponse);
  
//  serveWebTemp(client);
  client.println(tempResponse);
  
//  serveWebFan(client);
  client.println(fanResponse);
  
 // serveWebHeat(client);
  client.println(heatResponse);
}
//Adjusts Light Automatically to maintain a certain light level (So far, not user input)
void adjustLight(int lightPreference){
   int brightness = 0;
   float inputVolts = 0;
   int outputVolts = 0;
   
   brightness = analogRead(LIGHT_INPUT);
   inputVolts = map(brightness, 0, 1023, 0, 5);// if input was 1023, it means it was 5V
   outputVolts = map(inputVolts, 0, 5, 255, 0);//if input was 5V, output will be 0V (inverse to brightness)
   if (inputVolts > 3.5) { //If the light detector gives more that 3.5V, then the LED brightness won't be effective
    outputVolts = 0;
   } else if (inputVolts < 1.5) { //If light detector gives less that 1.5V, the light needs to be put on full
    outputVolts = 255;
   }
   analogWrite(LEDPIN, outputVolts);
}
//Fixes light at set brightness
void fixedLight (int lightPercent){
     float brightness = map(lightPercent, 0, 100, 0, MAX_DIG_OUTPUT);
   
   analogWrite(LEDPIN,brightness);
}
//adjusts the temp automatically
void adjustTemp(float tempPreference){

  float temp = getCurrTemp(); 

  if(temp >= tempPreference + TEMP_BUFFER) {
    float tempDifference = tempPreference - temp;
    float tempPercent = map(tempDifference, 0,tempPreference, 0, 100);
    adjustFan(tempPercent);
  } else if (temp <= tempPreference - TEMP_BUFFER) {
    activateHeat();
  }

}
//Adjusts fan speed by taking a number between 0 and 100
void adjustFan(int speedPercent){
  float motorSpeed = map(speedPercent, 0, 100, MIN_FAN_VOLTS, MAX_DIG_OUTPUT);
  analogWrite(MOTORPIN,motorSpeed);
}

void activateHeat(void) {
  digitalWrite(HEATPIN,HIGH);
}
void deactivateHeat(void){
  digitalWrite(HEATPIN,LOW);
}

float getCurrTemp(void) {

   float temp = 25;
   double denominator = 0;
   float voltsOut = analogRead(TEMP_INPUT);
   voltsOut = map(voltsOut, 0, MAX_A_INPUT, MIN_INPUT, MAX_VOLTS);

//Calculations below follow a specific equation for the circuit we made
   denominator = log(TR1*TR2*(TVOLT_IN-voltsOut))/(voltsOut*(TR1+TR2)-TVOLT_IN*TR2) -log(TCONST_A);
   temp = TCONST_B/denominator - TABS_ZERO;
   return temp;
}






void serveWebHead(YunClient client) {

  client.println(
"  <head>"
"    <link href=\"http://s3.amazonaws.com/codecademy-content/courses/ltp/css/shift.css\" rel=\"stylesheet\">"
"    <link href=\"http://s3.amazonaws.com/codecademy-content/courses/ltp/css/bootstrap.css\" rel=\"stylesheet\">"


"   <link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\">");
/*
"  </head>"
"<style>"
".navbar {"
"  position: fixed;"
"  width: 100%;"
"}"

"</style>"

"<nav class=\"navbar navbar-inverse\">"
"  <div class=\"container-fluid\">"
"    <div class=\"navbar-header\">"
"      <button type=\"button\" class=\"navbar-toggle\" data-toggle=\"collapse\" data-target=\"#myNavbar\">"
"        <span class=\"icon-bar\"></span>"
"        <span class=\"icon-bar\"></span>"
"        <span class=\"icon-bar\"></span>"
"      </button>"
"      <a class=\"navbar-brand\" href=\"http://team17smarterhomesarduino.local/arduino/\">Connex UniHome</a>"
"    </div>"
"    <div class=\"collapse navbar-collapse\" id=\"myNavbar\">"
"      <ul class=\"nav navbar-nav\">"
"        <li class=\"active\"><a href=\"http://team17smarterhomesarduino.local/arduino/\">Home</a></li>"
"        <li><a href=\"http://team17smarterhomesarduino.local/arduino/~#Lights\">Lighting</a></li>"
"        <li><a href=\"http://team17smarterhomesarduino.local/arduino/~#Temp\">Temperature</a></li>"
"        <li><a href=\"http://team17smarterhomesarduino.local/arduino/~#Fan\">Fan</a></li>"
"        <li><a href=\"http://team17smarterhomesarduino.local/arduino/~#Heat\">Heating</a></li>"
"      </ul>"
"     </div>"
"  </div>"
"</nav>"

"<style>"

".bg-grey {"
"  background-color: #FAFAFA;"
"}"
".bg-yellow {"
"  background-color: #FFFF66;"
"}"
".bg-blue {"
"  background-color: #00F0FF;"
"}"
".bg-purple{"
"  background-color: #CC00CC;"
"}"
".bg-red {"
"  background-color: #EE3333;"
"}"
".border-big {"
"  border-top: solid;"
"  border-bottom: solid;"
"  border-width: 5px;"
"  padding-top: 100px;"
"  padding-bottom: 120px;"
"}"
".some-pad {"
"  padding-bottom: 35px;"
"   padding-top: 50px;"
"  border-bottom: solid;"
"  border-width: 5px;"
"}"
".btn:hover {"
"  box-shadow: 0 12px 16px 0 rgba(0,0,0,0.24), 0 17px 50px 0 rgba(0,0,0,0.19);"
"  background-color: #47D147;"
"}"
".light-btn {"
"  color: #FFFFFF;"
"  background-color: #CCFF99;"
"}"
".fan-btn {"
"  background-color: #80FFBF;"
"}"
".temp-btn {"
"  background-color: #FFCCFF;"
"}"
".heat-btn {"
"  color: #FFFFFF;"
"  background-color: #FF715A;"
"}"
".off:hover {"
"  background-color: #FF5555;"
"  color: #FFFFFF;"
"}"
".hot:hover {"
"  background-color: #FF0000;"
"  color: #FFFFFF;"
"}"
".warm:hover {"
"  background-color: #F0E000;"
"  color: #FFFFFF;"
"}"
".cool:hover {"
"  background-color: #00FFFF;"
"  color: #FFFFFF;"
"}"
".cold:hover {"
"  background-color: #0000FF;"
"  color: #FFFFFF;"
"}"
".med:hover:"
"  background-color: #00FFC0;"
"  color: #FFFFFF;"
"}"
".btn:target {"
"  background-color: #0000FF;"
"}"
"</style>"

"<script type=\"text/javascript\">"
"function displayLOFF() {"
"  document.getElementById('result').innerHTML = \"OFF\" ;"
"}"
"function displayL100() {"
"  document.getElementById(\"result\").innerHTML = \"100%\";"
"}"
"function displayL75() {"
"  document.getElementById(\"result\").innerHTML = \"75%\";"
"}"
"function displayL50() {"
"  document.getElementById(\"result\").innerHTML = \"50%\";"
"}"
"function displayL25() {"
"  document.getElementById(\"result\").innerHTML = \"25%\";"
"}"
"function displayHON() {"
"  document.getElementById(\"result3\").innerHTML = \"ON\";"
"}"
"function displayHOFF() {"
"  document.getElementById(\"result3\").innerHTML = \"OFF\";"
"}"

"function displayT15() {"
"  document.getElementById(\"result1\").innerHTML = \"15.0\";"
"}"
"function displayT18() {"
"  document.getElementById(\"result1\").innerHTML = \"17.5\";"
"}"
"function displayT20() {"
"  document.getElementById(\"result1\").innerHTML = \"20.0\";"
"}"
"function displayT23() {"
"  document.getElementById(\"result1\").innerHTML = \"22.5\";"
"}"
"function displayT25() {"
"  document.getElementById(\"result1\").innerHTML = \"25.0\";"
"}"
"function displayT28() {"
"  document.getElementById(\"result1\").innerHTML = \"27.5\";"
"}"
"function displayT30() {"
  "  document.getElementById(\"result1\").innerHTML = \"30.0\";"
"}"
"function displayT33() {"
"  document.getElementById(\"result1\").innerHTML = \"32.5\";"
"}"
"function displayT35() {"
"  document.getElementById(\"result1\").innerHTML = \"35.0\";"
"}"
"function displayFOFF() {"
"  document.getElementById(\"result2\").innerHTML = \"OFF\";"
"}"
"function displayF25() {"
"  document.getElementById(\"result2\").innerHTML = \"25%\";"
"}"
"function displayF50() {"
"  document.getElementById(\"result2\").innerHTML = \"50%\";"
"}"
"function displayF75() {"
"  document.getElementById(\"result2\").innerHTML = \"75%\";"
"}"
"function displayF100() {"
"  document.getElementById(\"result2\").innerHTML = \"100%\";"
"}"
"</script>"

"<body>"
"  <div class=\"jumbotron\">"
"   <div class=\"container-fluid bg-grey some-pad\">"
"    <div class=\"container\">"

" <center>   <h1>UniHome</h1>"

"<head>"
"        <title>Connex UniHome</title>"
"</head>"
"<center><body>"

"<h1>Founded by Connex</h1>"
"<h3>The UniHome Project-</h3>"
"<h4>\"A brighter future for <i>all</i>\"</h4>"
"<br>"
"<img src=\"https://cdn4.iconfinder.com/data/icons/home3/102/Untitled-5-512.png\" height=\"300\" width=\"300\">"
"</center>"
"<div class=\"container-fluid bg-grey\">"
"<a name=\"Lights\"> </a>"
"</div>"

"</div>"
"</div>"
);
  */
}
void serveWebLight(YunClient client) {
client.println(
"<div class=\"container-fluid text-center bg-yellow border-big\">"
" <img src=\"http://images.clipartpanda.com/lightbulb-icon-LightBulbOn.png\" alt=\"Smiley face\" height=\"300\" width=\"300\">"
"   <br><br><br>");
/*
"<div class=\"btn-group\" data-toggle=\"buttons\">"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/lights/000~#Lights"
"class=\"btn btn-primary off light-btn\" onclick=displayLOFF() type=\"button\">OFF</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/lights/025~#Lights"
"class=\"btn btn-primary med light-btn\" onclick=displayL25() type=\"button\">LOW</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/lights/050~#Lights"
"class=\"btn btn-primary cool light-btn\" onclick=displayL50() type=\"button\">MEDIUM</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/lights/075~#Lights"
"class=\"btn btn-primary warm light-btn\" onclick=displayL75() type=\"button\">HIGH</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/lights/100~#Lights"
"class=\"btn btn-primary hot light-btn\" onclick=displayL100() type=\"button\">FULL</a>"
"  </div>"
"<p> The current brightness is: <b id='result'> </b></p>"
"</div>"
"<p>");
}
void serveWebTemp(YunClient client) {
  client.print(
"</p>" 
"<div class=\"container-fluid bg-yellow\">"
"<a name=\"Temp\"> </a>"
"</div>"


"<div class=\"container-fluid text-center bg-purple border-big\">"
" <img src=\"https://cdn2.iconfinder.com/data/icons/game-center-mixed-icons/512/temperature.png\" alt=\"Smiley face\" height=\"300\" width=\"300\">"
"   <br><br><br>"

"  <div class=\"btn-group\" data-toggle=\"buttons\">"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/015~#Temp"
"class=\"btn btn-default btn-xs cold temp-btn\" onclick=displayT15() type=\"button\">15.0</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/018~#Temp"
"class=\"btn btn-default btn-xs cold temp-btn\" onclick=displayT18() type=\"button\">17.5</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/020~#Temp"
"class=\"btn btn-default btn-xs cool temp-btn\" onclick=displayT20() type=\"button\">20.0</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/023~#Temp"
"class=\"btn btn-default btn-xs med temp-btn\" onclick=displayT23() type=\"button\">22.5</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/025~#Temp"
"class=\"btn btn-default btn-xs med temp-btn\" onclick=displayT25() type=\"button\">25.0</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/028~#Temp"
"class=\"btn btn-default btn-xs warm temp-btn\" onclick=displayT28() type=\"button\">27.5</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/030~#Temp"
"class=\"btn btn-default btn-xs warm temp-btn\" onclick=displayT30() type=\"button\">30.0</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/033~#Temp"
"class=\"btn btn-default btn-xs hot temp-btn\" onclick=displayT33() type=\"button\">32.5</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/temp/050~#Temp"
"class=\"btn btn-default btn-xs hot temp-btn\" onclick=displayT35() type=\"button\">35.0</a>"
"  </div>"
"<p>The current temperature selected is:<b id='result1'></b></p>"
"</div>"
"<p>");*/
}

void serveWebFan(YunClient client) {
  client.print(
"</p>"
"<div class=\"container-fluid bg-orange\">"
"<a name=\"Fan\"> </a>"
"</div>"

"<div class=\"container-fluid text-center bg-blue border-big\">"
" <img src=\"https://cdn3.iconfinder.com/data/icons/computer-system-and-data/512/31-512.png\" height=\"300\" width=\"300\">"
"   <br><br><br>"
"  <div class=\"btn-group\" data-toggle=\"buttons\">"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/fan/000~#Fan"
"class=\"btn btn-default off fan-btn\" onclick=displayFOFF() type=\"button\">OFF</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/fan/025~#Fan"
"class=\"btn btn-default med fan-btn\" onclick=displayF25() type=\"button\">25%</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/fan/050~#Fan"
"class=\"btn btn-default cool fan-btn\" onclick=displayF50() type=\"button\">50%</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/fan/075~#Fan"
"class=\"btn btn-default cold fan-btn\" onclick=displayF75() type=\"button\">75%</a>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/fan/100~#Fan"
"class=\"btn btn-default cold fan-btn\" onclick=displayF100() type=\"button\">Full</a>"
"  </div>"
"<p> The fan level selected is: <b id='result2'></b></p>"
"</div>"
"<p>"
);
}

void serveWebHeat(YunClient client) {
  client.print( 

"</p>"
/*
"<div class=\"container-fluid bg-blue\">"
"<a name=\"Heat\"> </a>"
"</div>"


"<div class=\"container-fluid text-center bg-red border-big\">"
" <img src=\"http://cotosociodeporte.es/wp-content/uploads/2015/06/icono-sauna.png\" height=\"300\" width=\"300\">"
"   <br><br><br>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/heat/000~#Heat\""
"class=\"btn btn-default btn-block hot heat-btn\" onclick=displayHOFF() type=\"button\">OFF</a>"
"   <br>"
"   <a href=\"http://team17smarterhomesarduino.local/arduino/heat/100~#Heat"
"class=\"btn btn-default btn-block med heat-btn\" onclick=displayHON() type=\"button\">ON</a>"
"<p>The heater is currently:  <b id='result3'></b></p>"
"</div>"
"</div>"
"<p>"*/
);
}

