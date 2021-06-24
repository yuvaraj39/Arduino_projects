
#include<SPI.h>
#define BLYNK_PRINT Serial 
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>
WidgetLCD pin(V0);
#include <Wire.h>
#include <DHT.h>      // including the library of DHT11 temperature and humidity sensor
#define DHTTYPE DHT11   // DHT 11
#define dht_dpin 2//d4
DHT dht(dht_dpin, DHTTYPE); 
BlynkTimer timer;
#include <BlynkSimpleEsp8266.h>
#include <SPI.h>
const char* ssid     = "SSID";                 // SSID of local network
const char* password = "wifi_password";                    // Password on network
String APIKEY = "openweather_API_KEY";                                 
String CityID = "CITY_ID";                                 
char auth[] = "BLYNK_AUTH_CODE";

WiFiClient client;
char servername[]="api.openweathermap.org";              // remote server we will connect to
String result;

int  counter = 60;                                      

String weatherDescription ="";
String weatherLocation = "";
const int VAL_PROBE = A0; //Analog pin 0
int Motor=15;//data pin D8
const int MOISTURE_LEVEL = 600; // the value after the LED goes on
LiquidCrystal_I2C lcd(0x27, 16, 2);    // Address of your i2c LCD back pack should be updated.
void setup() {
  pinMode(Motor, OUTPUT);
  Serial.begin(115200);
  int cursorPosition=0;
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.print("   Connecting");  
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  
             while (WiFi.status() != WL_CONNECTED) 
            {
            delay(500);
            lcd.setCursor(cursorPosition,2); 
            lcd.print(".");
            cursorPosition++;
            }
  lcd.clear();
  lcd.print("   Connected!");
  Serial.println("Connected");
  delay(1000);
  Blynk.begin(auth, ssid, password);
  dht.begin();
 timer.setInterval(5000L, sendSensor);
 timer.setInterval(5000L, sendweather);
 pin.clear();
 timer.setInterval(7000L, moisture_blynk);

}

void loop() {
    if(counter == 60)                                 //Get new data every 10 minutes
    {
      counter = 0;
      displayGettingData();
      delay(1000);
      getWeatherData();
    }else
    {

      counter++;
      displayWeather(weatherLocation,weatherDescription);
      delay(1500);

    lcd.clear();
  float humidity = dht.readHumidity();
  float temperature= dht.readTemperature();
  Serial.print("dht working ");
  lcd.setCursor(1,0);
  lcd.print("H="); 
  lcd.print(humidity); 
  lcd.print("%"); 
   lcd.print("_|_"); 
  Serial.println(humidity); 
  Serial.println(" H");
  lcd.setCursor(9,0);
  lcd.print("T=");
  lcd.print(temperature); 
  lcd.print("*C");
  Serial.println(temperature);
  Serial.print(" *C ");
  delay(1000);
  float moisture_percentage;
  moisture_percentage = ( 100.00 - ( (analogRead(VAL_PROBE)/1023.00) * 100.00 ) );
  lcd.clear();
  lcd.setCursor(1,1);
  lcd.print("moisture=");
  lcd.print(moisture_percentage);
  lcd.print("%");
  delay(1000);
 int moisture=analogRead(VAL_PROBE);
          if(weatherDescription=="rainy")
         {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Going to Rain");
          delay(1000);
         }
         else
           { 
            if(moisture < MOISTURE_LEVEL)
            {
              
             lcd.clear();
             lcd.setCursor(0,1);
             lcd.print("High Moisture");
             delay(1000);
             
            }
            else
            {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Moisture Low");
              lcd.setCursor(0,1);
              lcd.print("Turn ON Motor");
             
            }}}

  delay(1000);

  Blynk.run(); // Initiates Blynk
  timer.run();
}

void getWeatherData()                                //client function to send/receive GET request data.
{
  if (client.connect(servername, 80))   
          {                                         //starts client connection, checks for connection
          client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&APPID="+APIKEY+"&cnt=1");
          client.println("Host: api.openweathermap.org");
          client.println("User-Agent: ArduinoWiFi/1.1");
          client.println("Connection: close");
          client.println();
          } 
  else {
         Serial.println("connection failed");        //error message if no client connect
          Serial.println();
       }

  while(client.connected() && !client.available()) 
  delay(1);                                          //waits for data
  while (client.connected() || client.available())    
       {                                             //connected or data available
         char c = client.read();                     //gets byte from ethernet buffer
         result = result+c;
       }

client.stop();                                      //stop client
result.replace('[', ' ');
result.replace(']', ' ');
Serial.println(result);
char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';
 StaticJsonDocument<1024> doc;
auto error = deserializeJson(doc,jsonArray);
if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
}
String location = doc["name"];
String weather = doc["weather"]["main"];
String description = doc["weather"]["description"];
weatherDescription = description;
weatherLocation = location;
int moisture = analogRead(VAL_PROBE);

}
void displayWeather(String location,String description)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(location);
  Serial.println(location);
  lcd.setCursor(0,1);
  lcd.print(description);
  Serial.println(description);
  
}

void displayGettingData()
{
  lcd.clear();
  lcd.print("Getting data");
}
void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  
}
void sendweather()
{   
    pin.clear();
    pin.print(0,0,weatherLocation);
    pin.print(0,1,weatherDescription);
 
}
BLYNK_WRITE(V3)
{ 
   if(param.asInt()){
       
        if(weatherDescription ="rainy")
         {
          Blynk.notify("Going to rain in 3 hrs no need for irrigation");
          
         }
         else
           { 
            Blynk.notify("Going to rain in 3 hrs no need for irrigation");
           }
           }
}
void moisture_blynk()
{
  int moisture_percentage;
  moisture_percentage = ( 100.00 - ( (analogRead(VAL_PROBE)/1023.00) * 100.00 ) );
  pin.clear();
  pin.print(0,0,"moisture=");
  pin.print(9,0,moisture_percentage);
  pin.print(12,0,"%");
  int moisture=(analogRead(VAL_PROBE));
            if(moisture > MOISTURE_LEVEL)
            {
             pin.print(0,1,"moisture low");             
            }
}
BLYNK_WRITE(V5)       
{
  if(param.asInt())
   {
    digitalWrite(Motor,HIGH);
    Blynk.virtualWrite(V6,255);
    Blynk.notify("Motor Turned ON");
   }
  else
  {
   digitalWrite(Motor,LOW);
   Blynk.virtualWrite(V6,0);
   Blynk.notify("Motor Turned OFF");
  }
}
//lcd to V0
//T to V1
//H to V2
//Timer to V3
