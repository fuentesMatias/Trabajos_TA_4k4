#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SH110X.h>
#include <esp_wifi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const int pinLedVerde = 23;
const int pinLedAzul = 2;
const int pinSensor = 33;
const int pinPote = 32;
const int pinRelay = 26;
int estadoLedAzul = LOW;

volatile int estadoPote;  // Variable del nivel del potenciómetro

float hum;
float temp;

String header;  // Variable para guardar el HTTP request

//const char* ssid = "IPLAN-609033";
//const char* password = "QXPK9D39Z4ED";

const char* ssid = "ACNET2";
const char* password = "";



DHT sensor(pinSensor, DHT22);
WiFiServer server(80);
//Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Variables para páginas HTML
String pagina;
String paginaOn;
String paginaOff;

void setup() {
  Serial.begin(115200);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedAzul, OUTPUT);
  pinMode(pinSensor, INPUT);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, LOW);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("SSID:  ");
  Serial.println(WiFi.SSID());
  Serial.print("ID:   ");
  Serial.println(WiFi.localIP());
  server.begin();
  sensor.begin();
  Serial.println("HTTP server started");
  //displayInit();
}

void loop() {
  handleClientRequests();  // Manejar solicitudes de clientes web

  leerPote();       // Leer potenciómetro
  leerSensorDHT();  // Leer sensor DHT

  updateWebPage();  // Actualizar página web
}

void handleClientRequests() {
  WiFiClient client = server.available();

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /ledAzulOn") >= 0) {
              digitalWrite(pinLedAzul, HIGH);
              estadoLedAzul = HIGH;
              client.println(paginaOn);
            } else if (header.indexOf("GET /ledAzulOff") >= 0) {
              digitalWrite(pinLedAzul, LOW);
              estadoLedAzul = LOW;
              client.println(paginaOff);
            } else if (header.indexOf("GET /actualizarDatos") >= 0) {
              client.println(constructWebPage());
            } else {
              client.println(pagina);
            }

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
  }
}

void updateWebPage() {
  // Enviar datos más recientes a la página web
  WiFiClient client = server.available();
  if (client) {
    client.println(constructWebPage());
    delay(1);
    client.stop();
  }
}

String constructWebPage() {
  String strLecturaHum = String(hum);
  String strLecturaTemp = String(temp);

  String pagina = "<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<meta charset='utf-8' />"
                  "<title>Servidor Web ESP32 - Grupo 11</title>"
                  "</head>"
                  "<body>"
                  "<center>"
                  "<h1>Servidor Web ESP32 - Grupo 11</h1>"
                  "<h3>Led Azul</h3>"
                  "<p><a href='/ledAzulOn'><button style='height:50px;width:150px;color:green'";
  if (estadoLedAzul == HIGH)
    pagina += " disabled";
  pagina += ">Activado</button></a></p>"
            "<p><a href='/ledAzulOff'><button style='height:50px;width:150px;color:red'";
  if (estadoLedAzul == LOW)
    pagina += " disabled";
  pagina += ">Desactivado</button></a></p>"
            "<h3>Datos del sensor DHT</h3>"
            "<p>Lectura de humedad: "
            + strLecturaHum + "</p>"
                              "<p>Lectura de temperatura: "
            + strLecturaTemp + "</p>"
                               "<h3>Datos del potenciómetro</h3>"
                               "<p>Nivel del potenciómetro: "
            + String(estadoPote) + "%</p>"
                                   "</center>"
                                   "</body>"
                                   "</html>";

  return pagina;
}

void leerPote() {
  estadoPote = map(analogRead(pinPote), 0, 4095, 0, 100);  // Leer el estado del potenciómetro y mapearlo a un rango de 0 a 100%
}

void leerSensorDHT() {
  float lecturaHumedad = sensor.readHumidity();
  float lecturaTemperatura = sensor.readTemperature();

  while (isnan(lecturaHumedad) || isnan(lecturaTemperatura)) {
    delay(2000);
    lecturaHumedad = sensor.readHumidity();
    lecturaTemperatura = sensor.readTemperature();
  }
  hum = lecturaHumedad;
  temp = lecturaTemperatura;
}
/*
void displayInit() {
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("   TP N1 - IoT - 4K4");
  display.println("Datos de Compilacion:");
  display.printf("Fecha %s\n", __DATE__);
  display.printf("Hora: %s\n", __TIME__);
  display.display();
}*/
