#include <WiFi.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <Preferences.h>
#include <stdio.h>
#include <string.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const int Btn = 18;
const int ledPinGreen = 16;
const int ledPinBlue = 19;
const int ledPinRed = 17;
bool a = false;
bool c = true;
String ssid = "";      //dana do logowania, zapisu i odczytu z pamięci mikrokontrolera
String password = "";  //dana do logowania, zapisu i odczytu z pamięci mikrokontrolera
String SSID = "";      //zmienna potrzebna do odbioru z przeglądarki
String PASS = "";      //zmienna potrzebna do odbioru z przeglądarki :)
//bool logged = false;
Preferences preferences;
static uint32_t pressTime = 0;
static uint32_t lastMillisRed = 0;
static uint32_t lastMillisGreen = 0;
static uint32_t lastMillisBlue = 0;
static uint32_t NoConnectingTime = 0;
//bool wifiTime = true;
bool r = false;
bool g = false;
bool b = false;
int APon = 0;                        //wifi loguje się - 0 ,WiFi zalogowane - 1,Wifi rozłączone - 2,  AP działa -3, AP ma clienta połączonego - 4,
IPAddress local_ip(192, 168, 1, 1);  //AP
IPAddress gateway(192, 168, 1, 1);   //AP
IPAddress subnet(255, 255, 255, 0);  //AP
WiFiServer server(80);               //AP
String header;
WiFiClient client;
bool dataComplete = false;

String serverAdr = "https://87.205.10.16:443/greeting";  // Server URL
String serverAuth = "https://87.205.10.16:443/authenticate";
String serverMessages = "https://87.205.10.16:443/comend";
String serverDeleteComand = "https://87.205.10.16:443/deleteReceivedMessage/";
String DeviceName = "Jacek";
String Password = "Mateusz2001ab";
String token;
String modes = "1/1/1/0/1/0/1/0/1/0/1/0/1/0/1/1/";
String actualData = "0/1/1/1/1/1/1/1/1/1/1/1/1/1/1/0/";
HTTPClient http;

void serialDeviceComm(){

}

void deleteComand(String id) {
  String serv = serverDeleteComand + id;
  http.begin(serv.c_str());
  http.addHeader("Authorization", token);
  String requestBody = "";
  int httpResponseCode = http.POST(requestBody);
  Serial.print("Delete Comand code: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    switch (httpResponseCode) {
      case 401:
        Authorization();
        break;
      case 400:
        Authorization();
        break;
      case 200:
        Serial.print("Deleted comand id: ");
        Serial.println(id);
        Serial.println();
        break;
    }
  } else {
    Serial.printf("http response code: %d\n", httpResponseCode);
  }
  http.end();
}

void Greeting() {
  HTTPClient http;
  http.begin(serverAdr.c_str());
  http.addHeader("Authorization", token);
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    switch (httpResponseCode) {
      case 401:
        Authorization();
        break;
    }
  } else {
    Serial.printf("http response code: %d\n", httpResponseCode);
  }
  http.end();
}

void GetComend() {
  const char* messageId;
  const char* messageTitle;
  http.begin(serverMessages.c_str());
  http.addHeader("Authorization", token);
  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<200> doc;
  // Add values in the document
  doc["modes"] = modes;
  doc["actualData"] = actualData;
  String requestBody;
  serializeJson(doc, requestBody);
  int httpResponseCode = http.POST(requestBody);
  Serial.print("Get Comand code: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    switch (httpResponseCode) {
      case 401:
        Authorization();
        break;
      case 400:
        Authorization();
        break;
      case 200:
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload.c_str());
        messageId = doc["id"];
        messageTitle = doc["messageTitle"];
        //Serial.println();
        Serial.print("Comend ID: ");
        Serial.println(messageId);
        Serial.print("Comend TEXT: ");
        Serial.println(messageTitle);
        Serial.println();
        String i = String(messageId);
        int l = i.length();
        if (l >= 1) {
          deleteComand(i);
        }
        break;
    }
  } else {
    Serial.printf("http response code: %d\n", httpResponseCode);
  }
  http.end();
}

void Authorization() {
  //JSONVar data;
  http.begin(serverAuth.c_str());
  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<200> doc;
  // Add values in the document
  doc["username"] = DeviceName;
  doc["password"] = Password;
  String requestBody;
  serializeJson(doc, requestBody);
  int httpResponseCode = http.POST(requestBody);
  // int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload.c_str());
    String t = "Bearer ";
    const char* a = doc["token"];
    String to = String(a);
    token = t + to;
    Serial.print(token);
    Serial.println();
  } else {
    Serial.printf("http response code: %d\n", httpResponseCode);
  }
  http.end();
}
//--------------------------------------------------------------------
void nvsWrite(void) {
  APon = 0;
  preferences.clear();  //OK
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.putString("ssid", SSID);
  preferences.putString("password", PASS);
  preferences.putUInt("APon", APon);
  preferences.end();
  Serial.println("Restarting after saving data in 5 seconds...");
  delay(5000);
  ESP.restart();
}
//--------------------------------------------------------------------
void nvsWriteFlag(void) {  //ok
  preferences.clear();     //OK
  preferences.begin("wifi", false);
  // preferences.remove("ssid");
  // preferences.remove("pass");
  preferences.putUInt("APon", 3);
  // preferences.putString("ssid", "toolAP");
  // preferences.putString("password", "admin");
  preferences.end();
  Serial.println("Restarting in 5 seconds...");
  delay(5000);
  ESP.restart();
}
//-----------------------------------------------------------------
void nvsRead(void) {  //ok
  preferences.begin("wifi", false);
  ssid = preferences.getString("ssid", "none");
  password = preferences.getString("password", "none");
  APon = preferences.getUInt("APon", 0);
  preferences.end();
  Serial.print("ssid:   ");
  Serial.println(ssid);
  Serial.print("password:   ");
  Serial.println(password);
  Serial.print("APon:  ");
  Serial.println(APon);
}
//--------------------------------------------------------------------
void Website() {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // CSS to style the on/off buttons
  // Feel free to change the background-color and font-size attributes to fit your preferences
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".login { width: 282px; overflow: hidden;  margin: auto; margin: 20 0 0 0px; padding: 60px; background: #24623f; border-radius: 15px ;}");
  client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #77878A;}</style></head>");
  // Tytuł
  client.println("<body><h1>Configurator WiFiModule</h1>");
  // Pola textowe
  client.println("<form method='GET'><label><b>SSID</b></label>");
  client.println("<div>");
  client.println("<INPUT TYPE=TEXT NAME='SSID' VALUE='' SIZE='25' MAXLENGTH='50'>");
  client.println("<div>");
  client.println("<br>");
  client.println("<label><b>PASSWORD</b></label>");
  client.println("<div>");
  client.println("<INPUT TYPE=PASSWORD NAME='PASSWORD' VALUE='' SIZE='25' MAXLENGTH='50'>");
  client.println("<br><br>");
  //przycisk
  client.println("<button>Log in WiFi</button></form>");
  //zakończenie
  client.println("</body></html>");
  // The HTTP response ends with another blank line
  client.println();
  // Break out of the while loop
}
//--------------------------------------------------------------------
void Website1() {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // CSS to style the on/off buttons
  // Feel free to change the background-color and font-size attributes to fit your preferences
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".login { width: 282px; overflow: hidden;  margin: auto; margin: 20 0 0 0px; padding: 60px; background: #24623f; border-radius: 15px ;}");
  client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #77878A;}</style></head>");
  // Tytuł
  client.println("<body><h1>Now module connecting WiFi...</h1>");
  client.println("<div>");
  client.println("<label><b>jak dobrze wprowadziles dane to skocz po browara bedzie sukces</b></label>");
  client.println("</body></html>");
  // The HTTP response ends with another blank line
  client.println();
  // Break out of the while loop
}
//--------------------------------------------------------------------
void findData(String header) {  //"GET /?SSID=Jacek&PASSWORD=haslo HTTP/1.1"
  int q = header.indexOf('=');
  int a = header.indexOf('&');
  SSID = header.substring(q + 1, a);
  String q1 = header.substring(a + 10);
  int b = q1.indexOf('/');
  PASS = q1.substring(0, b - 5);
  Serial.print("_________________");
  Serial.print(PASS + "__");
  Serial.print("_________________________");
  if (PASS.length() >= 1) {
    if (PASS.equals("P/1.1")) {
      Serial.println("______________________odswiezono strone______________________");
    } else {
      if (SSID.length() >= 1) {
        dataComplete = true;  //ustawia flagę otrzymania poprawnych danych do logowania w sieci WiFi ??????????????????????????????????????????????????????????????????????????????????????????
        nvsWrite();           //zapis zmiennych do pamięci mikrokontrolera nvs i reset ESP
      } else {
        dataComplete = false;  //ustaw alert w postaci strony www informujący o braku danych do logowania - flaga potrzebna!!!!!!!!!!!!!
      }
    }
  }
}
//------------------------------------------------------------------
void blinkWhite() {
  if (!r) {
    lastMillisRed = millis();
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinBlue, LOW);
    digitalWrite(ledPinGreen, LOW);
    r = true;
  }
  if (millis() - lastMillisRed >= 500) {
    digitalWrite(ledPinRed, !digitalRead(ledPinRed));
    digitalWrite(ledPinGreen, !digitalRead(ledPinGreen));
    digitalWrite(ledPinBlue, !digitalRead(ledPinBlue));
  }
  if (millis() - lastMillisRed >= 1000) {
    digitalWrite(ledPinRed, !digitalRead(ledPinRed));
    digitalWrite(ledPinGreen, !digitalRead(ledPinGreen));
    digitalWrite(ledPinBlue, !digitalRead(ledPinBlue));
    r = false;
    lastMillisRed = 0;
  }
}
//----------------------------------------------------------------------
void blink(int timeLight, int pin) {
  switch (pin) {
    case ledPinRed:
      if (!r) {
        lastMillisRed = millis();
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinBlue, LOW);
        digitalWrite(ledPinGreen, LOW);
        r = true;
      }
      if (millis() - lastMillisRed >= timeLight) {
        digitalWrite(pin, !digitalRead(pin));
      }
      if (millis() - lastMillisRed >= timeLight * 2) {
        digitalWrite(pin, !digitalRead(pin));
        r = false;
        lastMillisRed = 0;
      }
      break;
    case ledPinGreen:
      if (!g) {
        lastMillisGreen = millis();
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinBlue, LOW);
        digitalWrite(ledPinGreen, LOW);
        g = true;
      }
      if (millis() - lastMillisGreen >= timeLight) {
        digitalWrite(pin, !digitalRead(pin));
      }
      if (millis() - lastMillisGreen >= timeLight * 2) {
        digitalWrite(pin, !digitalRead(pin));
        g = false;
        lastMillisGreen = 0;
      }
      break;
    case ledPinBlue:
      if (!b) {
        lastMillisBlue = millis();
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinBlue, LOW);
        digitalWrite(ledPinGreen, LOW);
        b = true;
      }
      if (millis() - lastMillisBlue >= timeLight) {
        digitalWrite(pin, !digitalRead(pin));
      }
      if (millis() - lastMillisBlue >= timeLight * 2) {
        digitalWrite(pin, !digitalRead(pin));
        b = false;
        lastMillisBlue = 0;
      }
      break;
  }
}
//----------------------------------------------------------------------
void pressBtnInWiFiLog() {
  if (digitalRead(Btn) == LOW) {
    if (a == false) {
      pressTime = millis();
      Serial.println("LOW");

      c = false;
      a = true;
    }
    if (millis() - pressTime >= 5000 && c == false) {
      Serial.println(" Minęło 5 sekund....");
      c = true;
      nvsWriteFlag();
      ESP.restart();
    }
  } else {
    a = false;
  }
}
//----------------------------------------------------------------------
void pressBtn() {
  if (digitalRead(Btn) == LOW) {
    if (a == false) {
      pressTime = millis();
      Serial.println("LOW");
      // if (APon >= 4) {
      //   APon = -1;
      // }
      // APon += 1;
      digitalWrite(ledPinRed, LOW);
      digitalWrite(ledPinBlue, LOW);
      digitalWrite(ledPinGreen, LOW);
      c = false;
      a = true;
    }
    if (millis() - pressTime >= 5000 && c == false) {
      Serial.println(" Minęło 5 sekund....");
      nvsWriteFlag();
      ESP.restart();
      c = true;
    }
  } else {
    a = false;
  }
}
//-----------------------------------------------------------------
void APcommunication() {
  //Serial.println("APcommunication.......");
  client = server.available();  // Oczekiwanie na połączenia przychodzące
  if (client) {                 // If a new client connects,
    APon = 4;
    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected()) {  // loop while the client's connected

      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // Display the HTML web page
            if (dataComplete) {
              Website1();  //wyświetla informacyjną stronę po udanym pozyskaniu danych
            } else {
              Website();  //wyświetla stronę z polami do wpisania danych do logowania
            }
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    String w = header.substring(6);
    int t = w.indexOf("/");
    String w1 = header.substring(6, t);
    int l = w1.length();
    if (l >= 16) {
      Serial.print("==========");
      Serial.println();
      findData(header);  //wyodrębnienie danych do logowania WiFi oraz zapis do pamięci nvs, ustawienie flagi i reset urządzenia do startu pracy w internecie
    } else {
      Serial.print("====+++++++++======");
    }
    header = "";    // Clear the header variable
    client.stop();  // Close the connection
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
//-----------------------------------------------------------------
void APstart() {
  Serial.println("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open.
  WiFi.mode(WIFI_AP);
  // SSID(zdefiniowane wcześniej): maksymalnie 63 znaki;
  // hasło(zdefiniowane wcześniej): minimum 8 znaków; ustaw na NULL, jeśli chcesz, aby punkt dostępu był otwarty
  // kanał: numer kanału Wi-Fi (1-13)
  // ssid_hidden: (0 = rozgłaszanie SSID, 1 = ukrywanie SSID)
  // max_connection: maksymalna liczba jednocześnie podłączonych klientów (1-4)
  String e = ssid;
  String f = password;
  WiFi.softAP("Wifi001");
  //WiFi.softAP("abcdef");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("Server started");
}
//------------------------------------------------------------------
void wifiComunication() {
  while (WiFi.status() != WL_CONNECTED) {
    pressBtnInWiFiLog();
    APon = 0;
    if (millis() - NoConnectingTime >= 12000) {
      Serial.print("....");
      Serial.println(WiFi.status());
      NoConnectingTime = millis();
    }
    if (millis() - NoConnectingTime <= 8000) {
      blinkWhite();
    } else {
      blink(250, ledPinRed);
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    APon = 1;
    GetComend();
    delay(300);
    //Serial.print("connected to WiFi:   ");
    // Serial.print(ssid);
    // Serial.print("  IP address:  ");
    // Serial.println(WiFi.localIP());
  }
}
//------------------------------------------------------------------
void wifiStart(void) {
  APon = 0;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
}
//------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  nvsRead();
  pinMode(Btn, INPUT);
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinBlue, LOW);
  digitalWrite(ledPinGreen, LOW);
  if (APon == 3) {
    APstart();
  } else {
    wifiStart();
  }
}
//------------------------------------------------------------------
void loop() {
  //wifi loguje się           - 0 /miga na biało z krótkimi przerwami na czerwono/
  //WiFi zalogowane           - 1 /świeci na biało/
  //Wifi rozłączone           - 2 /miga czerwone/
  //AP działa                 - 3 /świeci na niebiesko
  //AP ma clienta połączonego - 4/miga na niebiesko/
  switch (APon) {
    case 0:
      blinkWhite();
      Serial.println(APon);
      break;
    case 1:
      digitalWrite(ledPinRed, HIGH);
      digitalWrite(ledPinBlue, HIGH);
      digitalWrite(ledPinGreen, HIGH);
      Serial.println(APon);
      break;
    case 2:
      blink(400, ledPinRed);
      Serial.println(APon);
      break;
    case 3:
      digitalWrite(ledPinRed, LOW);
      digitalWrite(ledPinGreen, LOW);
      digitalWrite(ledPinBlue, HIGH);
      Serial.println(APon);
      break;
    case 4:
      blink(400, ledPinBlue);
      Serial.println(APon);
      break;
  }
  if (APon >= 3) {
    APcommunication();
  } else {
    wifiComunication();
  }
  pressBtn();
}