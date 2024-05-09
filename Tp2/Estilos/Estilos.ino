#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <esp_wifi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const int pinLedVerde = 23;
const int pinLedAzul = 2;
const int pinSensor = 33;
const int pinPote = 32;
const int pinRelay = 26;

int estadoLedAzul = LOW;
int valorLedVerde = 0;
String stringLedAzul = "off";
String stringEstadoRelay = "off";
String mensajeDisplayLeido = "";
int valorSlider = 0;
volatile int estadoPote;      // Variable del nivel del potenciómetro

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
    String currentLine = "";        //
    while (client.connected()) {    // loop mientras el cliente está conectado
      if (client.available()) {     // si hay bytes para leer desde el cliente. Devuelve el número de bytes disponibles para lectura en el búfer de entrada del cliente
        char c = client.read();     // lee un byte
        header += c;
        if (c == '\n') {  // si el byte es un caracter de salto de linea
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");        //la solicitud HTTP se ha procesado correctamente.
            client.println("Content-type:text/html"); //establece el tipo de contenido que se enviará al cliente en la respuesta. En este caso se trata de una página HTML.
            client.println("Connection: close");      //la conexión entre el servidor y el cliente se cerrará después de enviar la respuesta
            client.println();

            // enciende y apaga el GPIO
            if (header.indexOf("GET /ledAzulOn") >= 0) {   //busca la primera aparición de "GET /on" y devuelve la posición donde se encuentra. Si no se encuentra devuelve -1
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
                String intensityValueStr = body.substring(index + 14); // 14 es la longitud de "intensidadLed="
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

            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else {  // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
  }
}

void displayInit(){
  // Reemplazar las líneas comentadas por las correspondientes para probar con la placa en Clase
  display.begin(0x3C, true);
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); // Vaciar la pantalla del display
  display.setTextSize(1); // Configurar tamaño de la letra
  display.setTextColor(SH110X_WHITE);
  //display.setTextColor(SSD1306_WHITE); // Configurar el color de la letra
  display.setCursor(0, 0); // Setear el cursor en la posición 0,0 (esq. vertical izquierda)
  display.println("   TP N2 - IoT - 4K4");
  display.println("Datos de Compilacion:");
  display.printf("Fecha %s\n", __DATE__);
  display.printf("Hora: %s\n", __TIME__);
  display.display();
}

void mostrarPaginaWeb(WiFiClient client) {
  // Muestra la página web
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><meta charset='utf-8' />");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("<title>Servidor Web ESP32 - Grupo 11</title>");
  // CSS to style the on/off buttons
  client.println("<style>html { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4;}");
  client.println(".container { max-width: 600px; margin: 20px auto; padding: 20px; background-color: #fff; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);}");
  client.println("h1 { color: #333; text-align: center;}");
  client.println("h3 { color: #555; margin-top: 30px;}");
  client.println("p { color: #777; margin-bottom: 10px;}");
  client.println("button { background-color: #4CAF50; border: none; color: white; padding: 10px 20px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin-top: 10px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s ease;}");
  client.println("button:hover { background-color: #45a049;}");
  client.println("input[type='range'] { width: 100%; margin-top: 10px;}");
  client.println("input[type='submit'] { background-color: #008CBA; color: white; border: none; padding: 10px 20px; text-decoration: none; display: inline-block; font-size: 16px; margin-top: 10px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s ease;}");
  client.println("input[type='submit']:hover { background-color: #005f80;}");
  client.println("</style></head>");

  // Web Page Heading
  client.println("<body><div class=\"container\">");
  client.println("<h1>Servidor Web ESP32 - Grupo 11</h1>");
  client.println("<div>");
  client.println("<h3>Led Azul</h3>");
  client.println("<p>Estado " + stringLedAzul + "</p>");
  if (stringLedAzul == "off") {
    client.println("<p><a href='/ledAzulOn'><button>ON</button></a></p>");
  } else if (stringLedAzul == "on") {
    client.println("<p><a href='/ledAzulOff'><button>OFF</button></a></p>");
  }
  client.println("<br />");
  client.println("<h3>Datos del sensor DHT</h3>");
  client.println("<p>Lectura de humedad " + String(hum) + "</p>");
  client.println("<p>Lectura de temperatura " + String(temp) + "</p>");
  client.println("<br />");
  client.println("<h3>Datos del potenciometro</h3>");
  client.println("<p>Nivel del potenciometro " + String(estadoPote) + "%</p>");
  client.println("<br />");
  client.println("<h3>Estado del Relay</h3>");
  client.println("<p>Estado del Relay " + stringEstadoRelay + "</p>");
  if (stringEstadoRelay == "off") {
    client.println("<p><a href='/estadoRelayOn'><button>ON</button></a></p>");
  } else if (stringEstadoRelay == "on") {
    client.println("<p><a href='/estadoRelayOff'><button>OFF</button></a></p>");
  }
  client.println("<br />");
  client.println("<h3>Intensidad del LED verde</h3>");
  client.println("<form method='post' action='/cambiarIntensidadLed'>");
  client.println("<input type='range' min='0' max='100' value='" + String(valorSlider) + "' name='intensidadLed'>");
  client.println("<input type='submit' value='Submit'>");
  client.println("</form>");
  client.println("<br />");
  client.println("<h3>Mensaje Display</h3>");
  client.println("<form method='post' action='/mensajeDisplay'>");
  client.println("<input type='text' value='" + mensajeDisplayLeido + "' name='mensajeDisplay'>");
  client.println("<input type='submit' value='Submit'>");
  client.println("<br />");
  client.println("</div></div></body></html>");
}

void leerPote() {
  estadoPote = map(map(analogRead(pinPote), 0, 4095, 0, 255), 0, 255, 0, 100); // Leer el estado del pote y mapearlo a 8 bits
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