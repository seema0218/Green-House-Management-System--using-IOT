//Webserver AP + DHT11 + AJAX + Analogue + Driver Enabled + Shade Enabled

#include <ESP8266WiFi.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "Timer.h"

Timer RelayT;
Servo myservo;
LiquidCrystal_I2C lcd(0x27,20,4);

#define DHTTYPE DHT11
#define DHTPin D4
/* Put your SSID & Password */
const char* ssid = "Green House";  // Enter SSID here
const char* password = "ecogreen";  //Enter Password here
String inputString = "";    
String outputString = "";  
/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);  

float Temperature;
float Humidity;
int SoilMoisture;
int Light;
int Shade_Servo_Pos=1;

byte dCelcious[8] = 
{
  0b01000,
  0b10100,
  0b01000,
  0b00011,
  0b00100,
  0b00100,
  0b00100,
  0b00011
};

#define Fanpin D7
bool Fanstatus = LOW;

#define Shadeppin D6
bool Shadestatus = LOW;

#define Wpumppin D5
bool WPumpstatus = LOW;

#define ARelaypin D0

bool Auto_status = true;

void setup() 
{
  myservo.attach(Shadeppin);
  RelayT.every(30000, read_SoilMoisture,0);
  LCD_Startup();
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cycle Shade Servo ");
  ini_shade_servo();
  lcd.setCursor(0,0);
  lcd.print("Initializing......");
  read_SoilMoisture(0);
  Serial.begin(115200);
  inputString.reserve(20);
  outputString.reserve(20);
  
  pinMode(DHTPin, INPUT);
  dht.begin();             
 

  
  pinMode(Fanpin, OUTPUT);
  pinMode(Shadeppin, OUTPUT);
  pinMode(Wpumppin, OUTPUT);
  pinMode(ARelaypin, OUTPUT);
  digitalWrite(ARelaypin,LOW);
  lcd.setCursor(0,0);
  lcd.print("Done."); 
  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/FanOn", handle_FanOn);
  server.on("/FanOff", handle_FanOff);
  server.on("/ShadeOn", handle_ShadeOn);
  server.on("/ShadeOff", handle_ShadeOff);
  server.on("/WPumpOn", handle_WPumpOn);
  server.on("/WPumpOff", handle_WPumpOff);
  server.on("/AutoOn", handle_AutoOn);
  server.on("/AutoOff", handle_AutoOff);
  server.onNotFound(handle_NotFound);
  
 
  server.begin();
  //Serial.println("HTTP server started");
  lcd.setCursor(0,0);
  lcd.print("HTTP server started");
  delay(3000);
}
void loop() 
{
  RelayT.update();
  print_data_LCD();
  delay(500);
  server.handleClient();
  if(Fanstatus)
  {
    digitalWrite(Fanpin, HIGH);
  }
  else
  {
    digitalWrite(Fanpin, LOW);
  }
  
  if(Shadestatus)
  {
    //digitalWrite(Shadeppin, HIGH);
    //ini_shade_servo();
     if(Auto_status==true)
     {
        test_shade();
     }
     else
     {
        Left_shade_servo();
     }
  }
  else
  {
    Right_shade_servo();
    //digitalWrite(Shadeppin, LOW);
  }
  if(WPumpstatus)
  {
    digitalWrite(Wpumppin, HIGH);
  }
  else
  {
    digitalWrite(Wpumppin, LOW);
  }
  if(Auto_status)
    Run_Auto_Task();
}

void handle_OnConnect() 
{
  //Fanstatus = LOW;
  //Shadestatus = LOW;
  //Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  server.send(200, "text/html", SendHTML(Fanstatus,Shadestatus,Temperature,Humidity,WPumpstatus)); 
}

void handle_FanOn() 
{
  Fanstatus = HIGH;
  //Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true,Shadestatus,Temperature,Humidity,WPumpstatus)); 
}

void handle_FanOff() 
{
  Fanstatus = LOW;
  //Serial.println("GPIO7 Status: OFF");
  server.send(200, "text/html", SendHTML(false,Shadestatus,Temperature,Humidity,WPumpstatus)); 
}

void handle_ShadeOn() 
{
  Shadestatus = HIGH;
  //Serial.println("GPIO6 Status: ON");
  server.send(200, "text/html", SendHTML(Fanstatus,true,Temperature,Humidity,WPumpstatus)); 
}

void handle_ShadeOff() 
{
  Shadestatus = LOW;
  //Serial.println("GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(Fanstatus,false,Temperature,Humidity,WPumpstatus)); 
}

void handle_WPumpOn() 
{
  WPumpstatus = HIGH;
  //Serial.println("GPIO5 Status: ON");
  server.send(200, "text/html", SendHTML(Fanstatus,Shadestatus,Temperature,Humidity,true)); 
}

void handle_WPumpOff() 
{
  WPumpstatus = LOW;
  //Serial.println("GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(Fanstatus,Shadestatus,Temperature,Humidity,false)); 
}

void handle_AutoOn() 
{
  Auto_status = true;
  //Serial.println("GPIO5 Status: ON");
  server.send(200, "text/html", SendHTML(Fanstatus,Shadestatus,Temperature,Humidity,WPumpstatus)); 
}

void handle_AutoOff() 
{
  Auto_status = false;
  //Serial.println("GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(Fanstatus,Shadestatus,Temperature,Humidity,WPumpstatus)); 
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t Fan,uint8_t Shade,float Temperature,float Humidity,uint8_t WPump)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Green House - The Tech Station</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #000000;margin: 50px auto 30px;} h3 {color: #000000;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 200px;background-color: grey;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: red;}\n";
  ptr +=".button-on:active {background-color: grey;}\n";
  ptr +=".button-off {background-color: green;}\n";
  ptr +=".button-off:active {background-color: grey;}\n";
  ptr +="p {font-size: 19px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  
  
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Green House Monitoring System</h1>\n";
  ptr +="<h1>by Rosary College, Margao</h1>\n";
  ptr +="<h3>Access Point Mode</h3>\n";

  ptr +="<p>Temperature: ";
  ptr +=(int)Temperature;
  ptr +="&deg;C</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)Humidity;
  ptr +="%</p>";
  ptr +="<p>Soil Moisture: ";
  //read_SoilMoisture();
  ptr +=SoilMoisture;
  ptr +="%</p>";
  ptr +="<p>Light: ";
  read_Light();
  ptr +=Light;
  ptr +="%</p>";
  
  if(Fanstat)
  {
    ptr +="<p>Fan Status: ON";
    if (!Auto_status)
      ptr +="<a class=\"button button-off\" href=\"/FanOff\">Switch it OFF</a>";
    ptr +="</p>\n";
  }
  else
  {
    ptr +="<p>Fan Status: OFF";
    if (!Auto_status)
      ptr +="<a class=\"button button-on\" href=\"/FanOn\">Switch it ON</a>";
    ptr +="</p>\n";
  }

  if(Shadestat)
  {
    ptr +="<p>Shade Status: ON";
    if (Auto_status==false)
      ptr +="<a class=\"button button-off\" href=\"/ShadeOff\">Switch it OFF</a>";
    ptr +="</p>\n";
  }
  else
  {
    ptr +="<p>Shade Status: OFF";
    if (Auto_status==false)
      ptr +="<a class=\"button button-on\" href=\"/ShadeOn\">Switch it ON</a>";
    ptr +="</p>\n";
  }
  
  if(WPumpstat)
  {
    ptr +="<p>Water Pump Status: ON";
    if (Auto_status==false)
      ptr +="<a class=\"button button-off\" href=\"/WPumpOff\">Switch it OFF</a>";
    ptr +="</p>\n";
  }
  else
  {
    ptr +="<p>Water Pump Status: OFF";
    if (Auto_status==false)
      ptr +="<a class=\"button button-on\" href=\"/WPumpOn\">Switch it ON</a>";
    ptr +="</p>\n";
  }

  if(Auto_status==true)
  {
    ptr +="<p>Auto Mode: ON<a class=\"button button-off\" href=\"/AutoOff\">Switch it Manual</a></p>\n";
  }
  else
  {
    ptr +="<p>Auto Mode: OFF<a class=\"button button-on\" href=\"/AutoOn\">Switch it to Auto</a></p>\n";
  }

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void read_SoilMoisture(void* context)
{
  digitalWrite(ARelaypin,HIGH); 
  delay(1000);
  SoilMoisture=analogRead(A0);
  digitalWrite(ARelaypin,LOW);
  delay(1000);
  send_slave_data();
}

void read_Light()
{
  Light=analogRead(A0);
}

void LCD_Startup()
{
  lcd.init();
  lcd.backlight();
  lcd.begin(20, 4);
  lcd.createChar(0, dCelcious);
  lcd.setCursor(0, 0);
  lcd.print("    Green House     ");
  lcd.setCursor(0, 1);
  lcd.print(" Monitering System  ");
  lcd.setCursor(0, 2);
  lcd.print(" by Rosary College  ");
  lcd.setCursor(0, 3);
  lcd.print("    Margao - Goa    ");
}

void print_data_LCD()
{
  read_Light();
  //read_SoilMoisture();
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(Temperature);
  lcd.write(byte(0));

  lcd.print("  Lt:");
  lcd.print(Light);
  lcd.print("    ");
  
  lcd.setCursor(0,1);
  lcd.print("Hmdt:");
  lcd.print(Humidity);
  lcd.print("%   ");
 
  lcd.print("SM:");
  lcd.print(SoilMoisture);
  lcd.print("       ");

  lcd.setCursor(0,2);
  
  lcd.print("Fan:");
  if(Fanstatus==LOW)
    lcd.print("OFF");
    else
    lcd.print("ON ");

  lcd.print("    Shade:");
  if(Shadestatus==LOW)
    lcd.print("OFF");
    else
    lcd.print("ON ");

  lcd.setCursor(0,3);
  lcd.print("Pump:");
  if(WPumpstatus==LOW)
    lcd.print("OFF");
    else
    lcd.print("ON ");

  if(Auto_status==false)
    lcd.print("  Manual    ");
    else
    lcd.print("  Automatic ");
}

void Run_Auto_Task()
{
  if(Temperature>29 ||Humidity>65)
    Fanstatus=HIGH;
    else
    Fanstatus=LOW;

  if(SoilMoisture<409)
    WPumpstatus=HIGH;
    else
    WPumpstatus=LOW;
    
  if(Light<337)
    Shadestatus=HIGH;
    else
    Shadestatus=LOW;
}

void send_slave_data()
{
  outputString="";
  outputString+=Temperature;
  outputString+=",";
  outputString+=Humidity;
  outputString+=",";
  outputString+=SoilMoisture;
  outputString+=",";
  outputString+=Light;
  Serial.println(outputString);
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
      Auto_status=((inputString.substring(0,commaIndex)).toInt());
      if(!Auto_status)
      {
        Fanstatus=((inputString.substring(commaIndex+1, secondCommaIndex)).toInt());
        WPumpstatus=((inputString.substring(secondCommaIndex+1,thirdCommaIndex)).toInt());
        Shadestatus=((inputString.substring(thirdCommaIndex+1)).toInt());
      }
      inputString ="";
      //print_values();
    }
    inputString += inChar;
  }
}

void ini_shade_servo()
{
  int pos;
  for (pos = 45; pos <= 135; pos += 1) 
  { 
    myservo.write(pos);              
    delay(5);                       
  }
  for (pos = 135; pos >= 45; pos -= 1) 
  {                                  
    myservo.write(pos);              
    delay(5);                       
  }
}

void test_shade()
{
  read_Light();
  if(Shadestatus==HIGH)
  {
    Left_shade_servo();
  }
  else
  {
    Right_shade_servo();
  }
}
void Left_shade_servo() //1
{
  if(Shade_Servo_Pos==2)
  {
    for (int pos = 135; pos >= 45; pos -= 1) 
    {                                  
      myservo.write(pos);              
      delay(5);                       
    }
    Shade_Servo_Pos=1;
  }
}

void Right_shade_servo() //2
{
  if(Shade_Servo_Pos==1)
  {
    for (int pos = 45; pos <= 135; pos += 1) 
    {                                  
      myservo.write(pos);              
      delay(5);                       
    }
    Shade_Servo_Pos=2;
  }
}
