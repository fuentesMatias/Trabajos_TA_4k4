#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <esp_wifi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaracion de pines
const int pinLedVerde = 23;
const int pinLedAzul = 2;
const int pinSensor = 33;
const int pinPote = 32;
const int pinRelay = 26;

// Declaracion de variables
int estadoLedAzul = LOW;
int valorLedVerde = 0;
String stringLedAzul = "off";
String stringEstadoRelay = "off";
String mensajeDisplayLeido = "";
int valorSlider = 0;
volatile int estadoPote;  // Variable del nivel del potenciómetro

float hum;
float temp;

String header;  // Variable para guardar el HTTP request

const char* ssid = "ACNET2";
const char* password = "";

DHT sensor(pinSensor, DHT22);
WiFiServer server(80);

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup() {
  Serial.begin(115200);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedAzul, OUTPUT);
  pinMode(pinSensor, INPUT);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, LOW);

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
  displayInit();
}

void loop() {
  leerPote();
  leerSensorDHT();
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

            // enciende y apaga el GPIO
            if (header.indexOf("GET /ledAzulOn") >= 0) {  
              digitalWrite(pinLedAzul, HIGH);
              stringLedAzul = "on";
              Serial.println("Led Azul ON");

            } else if (header.indexOf("GET /ledAzulOff") >= 0) {
              digitalWrite(pinLedAzul, LOW);
              stringLedAzul = "off";
              Serial.println("Led Azul OFF");
            } else if (header.indexOf("GET /estadoRelayOn") >= 0) {
              digitalWrite(pinRelay, HIGH);
              stringEstadoRelay = "on";
              Serial.println("Estado Relay ON");
            } else if (header.indexOf("GET /estadoRelayOff") >= 0) {
              digitalWrite(pinRelay, LOW);
              stringEstadoRelay = "off";
              Serial.println("Estado Relay OFF");
            }

            // Si se recibe la solicitud de cambiar la intensidad del LED verde mediante el formulario
            if (header.indexOf("POST /cambiarIntensidadLed") >= 0) {
              String body = client.readString();
              int index = body.indexOf("intensidadLed=");
              if (index != -1) {
                String intensityValueStr = body.substring(index + 14);  // 14 es la longitud de "intensidadLed="
                int intensityValue = intensityValueStr.toInt();
                valorSlider = intensityValue;
                Serial.println("Intensidad a setear en el led verde: " + String(intensityValue));
                analogWrite(pinLedVerde, map(intensityValue, 0, 100, 0, 255));

                Serial.println("Intensidad del LED verde cambiada a: " + String(intensityValue));
              }
            }

            if (header.indexOf("POST /mensajeDisplay") >= 0) {
              String body = client.readString();
              body.replace("+", " ");
              Serial.println("Body: " + body);
              int index = body.indexOf("mensajeDisplay=");
              if (index != -1) {
                String msg = body.substring(index + 15);
                Serial.println("Msg: " + msg);
                mensajeDisplayLeido = msg;
                Serial.println("Mensaje leido: " + mensajeDisplayLeido);
                //------------ Logica de escribir en el lcd----
                actualizarDisplay(mensajeDisplayLeido);
              }
            }
            // Muestra la página web
            mostrarPaginaWeb(client);
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
void mostrarPaginaWeb(WiFiClient client) {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><meta charset='utf-8' />");
  client.println("<title>Servidor Web ESP32 - Grupo 11</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; background-color: #222; color: #fff; }");
  client.println("h1 { margin-top: 20px; }");
  client.println(".button { border: none; color: white; padding: 10px 20px; text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer; border-radius: 5px; }");
  client.println(".button-submit { background-color: #4CAF50; }");     // Color de botón para enviar
  client.println(".button-off { background-color: #FF5733; }");        // Color de botón para apagar (rojo)
  client.println(".button-on { background-color: #57FF33; }");         // Color de botón para encender (verde)
  client.println(".range-container { width: 50%; margin: 0 auto; }");  // Ajuste del contenedor del slider
  client.println(".range-slider { -webkit-appearance: none; appearance: none; width: 100%; height: 25px; background: #555; outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s; border-radius: 5px; }");
  client.println(".range-slider:hover { opacity: 1; }");
  client.println(".range-slider::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 25px; height: 25px; background: #4CAF50; cursor: pointer; border-radius: 50%; }");
  client.println(".range-slider::-moz-range-thumb { width: 25px; height: 25px; background: #4CAF50; cursor: pointer; border-radius: 50%; }");
  client.println(".button-on span { color: #333; }");  // Color de texto oscuro para botones "Encender"
  client.println("</style></head>");

  client.println("<body>");
  client.println("<center>");
  client.println("<h1>Servidor Web ESP32 - Grupo 11</h1>");
  client.println("<h3>Led Azul</h3>");
  client.println("<p>Estado: ");
  if (stringLedAzul == "off") {
    client.println("<span style='color: red;'>Apagado</span>");
    client.println("<p><a href='/ledAzulOn'><button class='button button-on'><span>Encender</span></button></a></p>");
  } else if (stringLedAzul == "on") {
    client.println("<span style='color: green;'>Encendido</span>");
    client.println("<p><a href='/ledAzulOff'><button class='button button-off'><span>Apagar</span></button></a></p>");
  }
  client.println("</p>");
  client.println("<br />");
  client.println("<h3>Estado del Relé</h3>");
  client.println("<p>Estado: ");
  if (stringEstadoRelay == "off") {
    client.println("<span style='color: red;'>Apagado</span>");
    client.println("<p><a href='/estadoRelayOn'><button class='button button-on'><span>Encender</span></button></a></p>");
  } else if (stringEstadoRelay == "on") {
    client.println("<span style='color: green;'>Encendido</span>");
    client.println("<p><a href='/estadoRelayOff'><button class='button button-off'><span>Apagar</span></button></a></p>");
  }
  client.println("</p>");
  client.println("<br />");
  client.println("<h3>Intensidad del LED verde</h3>");
  client.println("<div class='range-container'>");
  client.println("<form method='post' action='/cambiarIntensidadLed'>");
  client.println("<input type='range' min='0' max='100' value='" + String(valorSlider) + "' name='intensidadLed' class='range-slider'>");
  client.println("<input type='submit' value='Enviar' class='button button-submit'>");
  client.println("</form>");
  client.println("</div>");
  client.println("<br />");
  client.println("<h3>Datos del sensor DHT</h3>");
  client.println("<p>Lectura de humedad: " + String(hum) + "</p>");
  client.println("<p>Lectura de temperatura: " + String(temp) + "</p>");
  client.println("<br />");
  client.println("<h3>Datos del potenciómetro</h3>");
  client.println("<p>Nivel del potenciómetro: " + String(estadoPote) + "%</p>");
  client.println("<br />");
  client.println("<h3>Mensaje Display</h3>");
  client.println("<form method='post' action='/mensajeDisplay'>");
  client.println("<input type='text' value='" + mensajeDisplayLeido + "' name='mensajeDisplay'>");
  client.println("<input type='submit' value='Enviar' class='button button-submit'>");
  client.println("</form>");
  client.println("<br />");
  client.println("</center>");
  client.println("</body></html>");
}

void displayInit() {
  // Reemplazar las líneas comentadas por las correspondientes para probar con la placa en Clase
  display.begin(0x3C, true);
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();  // Vaciar la pantalla del display
  display.setTextSize(1);  // Configurar tamaño de la letra
  display.setTextColor(SH110X_WHITE);
  //display.setTextColor(SSD1306_WHITE); // Configurar el color de la letra
  display.setCursor(0, 0);  // Setear el cursor en la posición 0,0 (esq. vertical izquierda)
  display.println("   TP N2 - IoT - 4K4");
  display.println("Datos de Compilacion:");
  display.printf("Fecha %s\n", __DATE__);
  display.printf("Hora: %s\n", __TIME__);
  display.display();
}


void leerPote() {
  estadoPote = map(map(analogRead(pinPote), 0, 4095, 0, 255), 0, 255, 0, 100);
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

void actualizarDisplay(String mensaje) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Mensaje:\n");
  display.println(mensaje);
  display.display();
}