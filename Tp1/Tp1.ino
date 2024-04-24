/*
Tecnologías para Automatización - Trabajo práctico N°1 - IoT - 4K4
Integrantes del grupo:
  - Décimo, Sofía Mailén, 89401
  - Fuentes, Matias, 90463
  - Gregorat, Franco Lautaro, 89882
  - Mizzau Anadón, Federico Agustín, 89542
*/
// Importación de librerías
// Cambiar el import a esta librería para probar con la placa en clase
// #include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <splash.h>

#define LINEA_BLANCO F("                    ")
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int pinLedAzul = 32;   // Pin del LED Azul
const int pinLedRojo = 23;  // Pin del LED Rojo
const int pinBoton = 19; // Pin del pulsador
const int pinRelay = 18;    // Pin del Relay

int contadorPulsador = 0;       // Contador de pulsaciones sobre el botón
int estadoLedAzul = LOW;        // Estado del LED
int estadoBoton;                // Estado del pulsador
int ultimoEstado = HIGH;        // Estado anterior del pulsador
unsigned long ultimoRebote = 0; // Último tiempo de rebote del pulsador
int delayRebote = 50;           // Tiempo de rebote del pulsador

const int potePin = 25;       // Pin del potenciometro
volatile int estadoPote;      // Variable del nivel del potenciómetro
volatile int estadoPoteRelay; // Variable del nivel del potenciómetro mapeada al relay

// Crear un objeto display para usar el OLED
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Reemplazar el objeto display para probar con el OLED de la placa en clase
// Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
void setup()
{
  Serial.begin(115200);
  pinMode(pinLedAzul, OUTPUT);     // Configurar el pin del LED Azul como salida
  pinMode(pinRelay, OUTPUT);  // Configurar el pin del Rele como salida
  pinMode(pinBoton, INPUT_PULLUP);    // Configurar el pin del pulsador como entrada
  pinMode(potePin, INPUT);     // Configurar el pin del potenciómetro como entrada
  pinMode(pinLedRojo, OUTPUT); // Configurar el pin del LED Rojo como salida
  Serial.println("Configuración de pines realizada");
  delay(2000);
  Serial.println(F("Iniciando Display..."));
  displayInit(); // Ejecutar la función de configuración del display
  delay(2000);
  Serial.print(F("Display Iniciado"));
}

void loop()
{
  int lectura = digitalRead(pinBoton); // Leer el estado del pulsador
  if (lectura != ultimoEstado)
  { // Si se cambia el estado del pulsador, registrar el tiempo de rebote
    ultimoRebote = millis();
  }
  if ((millis() - ultimoRebote) > delayRebote)
  { // Si ha pasado suficiente tiempo desde el último rebote, actualizar el estado del pulsador
    if (lectura != estadoBoton)
    { // Si el estado del pulsador ha cambiado, actualizar el estado del LED
      estadoBoton = lectura;
      if (estadoBoton == HIGH) //  Si el pulsador ha sido presionado, cambiar el estado del LED y contar la pulsación
      {
        contadorPulsador++;
        Serial.println("Pulsador presionado.");
        estadoLedAzul = !estadoLedAzul;
        digitalWrite(pinLedAzul, estadoLedAzul);
      }
    }
  }
  ultimoEstado = lectura; // Guardar el último estado del pulsador

  estadoPote = map(analogRead(potePin), 0, 4095, 0, 255); // Leer el estado del pote y mapearlo a 8 bits
  analogWrite(pinLedRojo, estadoPote);

  estadoPoteRelay = analogRead(potePin) >= 2047 ? HIGH : LOW; // Leer el estado del pote y asignar el valor de HIGH o LOW según el valor leído
  digitalWrite(pinRelay, estadoPoteRelay);

  Serial.printf("\n• Pulsador: %d pulsaciones ", contadorPulsador);
  Serial.printf("\n• Led A: %s ", estadoLedAzul ? "Encendido" : "Apagado");
  Serial.printf("\n• Led R: %d de potencia", map(estadoPote, 0, 255, 0, 100));
  Serial.printf("\n• Led V: %s ", estadoPoteRelay ? "Encendido" : "Apagado");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("\n");
  display.printf("- Puls.: %d acciones\n", contadorPulsador);
  display.printf("- Led A: %s\n", estadoLedAzul ? "Encendido" : "Apagado");
  display.printf("- Led R: %d%% potencia\n", map(estadoPote, 0, 255, 0, 100));
  display.printf("- Led V: %s\n", estadoPoteRelay ? "Encendido" : "Apagado");
  display.display();
  delay(100);
}

// Función de configuración del display OLED
void displayInit()
{
  // Reemplazar las líneas comentadas por las correspondientes para probar con la placa en Clase
  // display.begin(0x3C, true);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); // Vaciar la pantalla del display
  display.setTextSize(1); // Configurar tamaño de la letra
  // display.setTextColor(SH110X_WHITE);
  display.setTextColor(SSD1306_WHITE); // Configurar el color de la letra
  display.setCursor(0, 0); // Setear el cursor en la posición 0,0 (esq. vertical izquierda)
  display.println("   TP N1 - IoT - 4K4");
  display.println("Datos de Compilacion:");
  display.printf("Fecha %s\n", __DATE__);
  display.printf("Hora: %s\n", __TIME__);
  display.display();
}
