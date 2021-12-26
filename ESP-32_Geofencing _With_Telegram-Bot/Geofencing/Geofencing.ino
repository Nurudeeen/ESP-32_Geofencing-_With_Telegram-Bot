#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
//#define RXD2 16
//#define TXD2 17
#include <TinyGPS++.h>
TinyGPSPlus gps;
boolean validGPS = false;
// Replace with your network credentials
const char* ssid = "XXXXXXXX";
const char* password = "XXXXXXXX";
String latitude;
String longitude;
static const double BaseLat =7.519910169335457 , BaseLon = 4.520653569263144;
// Initialize Telegram BOT
#define BOTtoken "1910134024:AAE-xEzURplDbAc1eqXpitfia-KciaCzZTw"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "XXXXXXXX"
#define CHAT_ID1 "XXXXXXXX"
#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 19;
bool ledState = LOW;

// Handle what happens when you receive new messages

void setup() {
  Serial.begin(9600);
 //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //Serial.println("Serial Txd is on pin: "+String(TX));
  //Serial.println("Serial Rxd is on pin: "+String(RX));
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  while (Serial.available()) {
    //Serial.print(char(Serial.read()));
    validGPS = gps.encode(Serial.read());
  }
    if(validGPS  || gps.location.isUpdated())
{
// Query GPS data here..
  //Get the latest info from the gps object which it derived from the data sent by the GPS unit
    Serial.print("Satellite Count:");
    Serial.println(gps.satellites.value());
    Serial.print("Latitude:");
    String latitude = String(gps.location.lat(),6);
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude:");
    String longitude = String(gps.location.lng(),6); 
    Serial.println(gps.location.lng(), 6);
    String updt= ("Find your car at https://www.google.com/maps/?q="+ latitude +","+longitude);
    bot.sendMessage(CHAT_ID, updt, "");
    bot.sendMessage(CHAT_ID1, updt, "");
    delay (30000);
    }
    
    if (gps.location.isValid() || gps.speed.isValid() ){
      double vel = gps.speed.kmph();
       unsigned long dist =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      BaseLat, 
      BaseLon) / 1000;
      if (dist >= 1){
      char mystr[15]; 
      sprintf(mystr,"Millis: %ul",dist);
      String dynamics = ("Your car has breached the set perimeter and is "+String(dist)+"km away from the Car Park"+".\n"+"Moving at Speed: "+ String(vel)+"kmph"+".\n"+"click on /lock to stop your car now!");
      bot.sendMessage(CHAT_ID, dynamics, "");
      bot.sendMessage(CHAT_ID1, dynamics, "");
      
      delay(5000);
      }
      }
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
void handleNewMessages(int numNewMessages) {

  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
//    if (chat_id != CHAT_ID){
//      bot.sendMessage(chat_id, "Unauthorized user", "");
//      continue;
//    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/lock to lock fuel pump of car \n";
      welcome += "/start to fresh the bot \n";
      welcome += "/state to check for current car's location on Google Maps \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
    if (text == "/lock") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "Disengaging fuel pump and stopping car ", "");
      delay (3000);
      digitalWrite(ledPin, LOW);
    }
    
    if (text == "/state") {
       while (Serial.available()) {
    Serial.print(char(Serial.read()));
    validGPS = gps.encode(Serial.read());
  }
     
       if(validGPS  && gps.location.isUpdated())
{
// Query GPS data here..
  //Get the latest info from the gps object which it derived from the data sent by the GPS unit
    Serial.print("Satellite Count:");
    Serial.println(gps.satellites.value());
    Serial.print("Latitude:");
    String latitude = String(gps.location.lat(),6);
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude:");
    String longitude = String(gps.location.lng(),6); 
    Serial.println(gps.location.lng(), 6);
     String updt= ("Find your car at https://www.google.com/maps/?q="+ latitude +","+longitude);
    bot.sendMessage(chat_id, updt, "");
    }
  }
}}
