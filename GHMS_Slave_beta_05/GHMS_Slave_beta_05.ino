
#include "config.h"
String inputString = "";    
String outputString = "";  

int Humidity_value=0;
int Temperature_value=0;
int Light_value=0;
int Soil_Moisture_value=0;
// holds the boolean (true/false) state of the light
bool Fan_is_on = false;
bool Shade_is_on = false;
bool Water_Pump_is_on = false;
bool Auto_is_on = true;
bool recieve_flag = true;
// track time of last published messages and limit feed->save events to once
// every IO_LOOP_DELAY milliseconds
#define IO_LOOP_DELAY 15000
unsigned long lastUpdate;

// set up the feed
AdafruitIO_Feed *Humidity = io.feed("Humidity");
AdafruitIO_Feed *Temperature = io.feed("Temperature");
AdafruitIO_Feed *Light = io.feed("Light");
AdafruitIO_Feed *Soil_Moisture = io.feed("Soil_Moisture");

AdafruitIO_Feed *Fan = io.feed("Fan");
AdafruitIO_Feed *Water_Pump = io.feed("Water_Pump");
AdafruitIO_Feed *Shade = io.feed("Shade");
AdafruitIO_Feed *Auto = io.feed("Auto");

void setup() 
{
  inputString.reserve(20);
  outputString.reserve(20);
  Serial.begin(115200);
  while(! Serial);
    //Serial.print("Connecting to MQTT Server");
  io.connect();

  Auto->onMessage(handleAuto);
  Fan->onMessage(handleFan);
  Shade->onMessage(handleShade);
  Water_Pump->onMessage(handleWater_Pump);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) 
  {
   //Serial.print(".");
    delay(500);
  }

  // we are connected
  //Serial.println();
  //Serial.println(io.statusText());

  // make sure all feeds get their current values right away
  //Fan->get();
  //Auto->get();
  //Shade->get();
  //Water_Pump->get();
  Auto->save(Auto_is_on);
  Fan->save(Fan_is_on);
  Shade->save(Shade_is_on);
  Water_Pump->save(Water_Pump_is_on);
}

void loop() 
{
  // process messages and keep connection alive
  io.run();
  serialEvent();
  if (millis() > (lastUpdate + IO_LOOP_DELAY)) 
  {
    //Serial.println();
    
    // save current count to io.adafruit
    //Serial.print("sending -> Humidity ");
    //Serial.println(Humidity_value);
    Humidity->save(Humidity_value);

    //Serial.print("sending -> Temperature ");
    //Serial.println(Temperature_value);
    Temperature->save(Temperature_value);

    //Serial.print("sending -> Light ");
    //Serial.println(Light_value);
    Light->save(Light_value);

    //Serial.print("sending -> Soil_Moisture ");
    //Serial.println(Soil_Moisture_value);
    Soil_Moisture->save(Soil_Moisture_value);

    //print_status();

  /*
    Serial.print("sending -> Fan Button ");
    Serial.println(Fan_is_on);
    Fan->save(Fan_is_on); 

    Serial.print("sending -> Shade Button ");
    Serial.println(Shade_is_on);
    Shade->save(Shade_is_on);

    Serial.print("sending -> Water Pump Button ");
    Serial.println(Water_Pump_is_on);
    Water_Pump->save(Water_Pump_is_on);

    Serial.print("sending -> Auto Button ");
    Serial.println(Auto_is_on);
    Auto->save(Auto_is_on);
*/
    lastUpdate = millis();
  }

}

void handleAuto(AdafruitIO_Data *data) 
{
  //Serial.print("received Auto <- ");
  //Serial.print(data->value());
  if(data->isTrue())
    {
      //Serial.println(" is on.");
      Auto_is_on=true;
    }
    else
    {
      //Serial.println(" is off.");
      Auto_is_on=false;
    }
   send_button_status();
}

void handleFan(AdafruitIO_Data *data) 
{
  //Serial.print("received Fan <- ");
  //Serial.print(data->value());
  if(Auto_is_on==false)
  {
    if(data->isTrue())
    {
      //Serial.println(" is on.");
      Fan_is_on=true;
    }
    else
    {
      //Serial.println(" is off.");
      Fan_is_on=false;
    }
  }
  else
  {
    //Fan->save(Fan_is_on);
    //Serial.println(" Auto is on. Please disable Auto and try again");
  }
  send_button_status();
}

void handleShade(AdafruitIO_Data *data) 
{
  //Serial.print("received Shade <- ");
  //Serial.print(data->value());
  if(Auto_is_on==false)
  {
    if(data->isTrue())
    {
      //Serial.println(" is on.");
      Shade_is_on=true;
    }
    else
    {
      //Serial.println(" is off.");
      Shade_is_on=false;
    }
  }
  else
  {
    //Shade->save(Shade_is_on);
    //Serial.println(" Auto is on. Please disable Auto and try again");
  }
  send_button_status();
}

void handleWater_Pump(AdafruitIO_Data *data) 
{
  //Serial.print("received Water Pump <- ");
  //Serial.print(data->value());
  if(Auto_is_on==false)
  {
    if(data->isTrue())
    {
      //Serial.println(" is on.");
      Water_Pump_is_on=true;
    }
    else
    {
      //Serial.println(" is off.");
      Water_Pump_is_on=false;
    }
  }
  else
  {
    //Water_Pump->save(Water_Pump_is_on);
    //Serial.println(" Auto is on. Please disable Auto and try again");
    //delay(5000);
  }
  send_button_status();
}

void serialEvent() 
{
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') 
    {
      int commaIndex=inputString.indexOf(',');
      int secondCommaIndex=inputString.indexOf(',',commaIndex+1);
      int thirdCommaIndex=inputString.indexOf(',',secondCommaIndex+1);
      Temperature_value=((inputString.substring(0,commaIndex)).toInt());
      Humidity_value=((inputString.substring(commaIndex+1, secondCommaIndex)).toInt());
      Soil_Moisture_value=((inputString.substring(secondCommaIndex+1,thirdCommaIndex)).toInt());
      Light_value=((inputString.substring(thirdCommaIndex+1)).toInt());
      inputString ="";
      //print_values();
    }
    inputString += inChar;
  }
}
/*
void print_status()
{
  Serial.print("Auto: ");
  Serial.println(Auto_is_on);
  Serial.print("Fan: ");
  Serial.println(Fan_is_on);
  Serial.print("Water Pump: ");
  Serial.println(Water_Pump_is_on);
  Serial.print("Shade: ");
  Serial.println(Shade_is_on);
}

void print_values()
{
  Serial.print("Temp: ");
  Serial.println(Temperature_value);
  Serial.print("Humid: ");
  Serial.println(Humidity_value);
  Serial.print("SM: ");
  Serial.println(Soil_Moisture_value);
  Serial.print("Ligh: ");
  Serial.println(Light_value);
}
*/
void send_button_status()
{
  outputString="";
  outputString+=Auto_is_on;
  outputString+=",";
  outputString+=Fan_is_on;
  outputString+=",";
  outputString+=Water_Pump_is_on;
  outputString+=",";
  outputString+=Shade_is_on;
  Serial.println(outputString);  
}
