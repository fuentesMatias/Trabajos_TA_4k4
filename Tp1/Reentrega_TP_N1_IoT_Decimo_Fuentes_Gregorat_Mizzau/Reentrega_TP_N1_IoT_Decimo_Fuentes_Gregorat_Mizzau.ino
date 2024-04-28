/*
  Tecnologías para Automatización - Trabajo práctico N°1 - IoT - 4K4
  Integrantes del grupo:
  - Décimo, Sofía Mailén, 89401
  - Fuentes, Matias, 90463
  - Gregorat, Franco Lautaro, 89882
  - Mizzau Anadón, Federico Agustín, 89542
*/
// Importación de librerías
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <splash.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int pinLedAzul = LED_BUILTIN;   // Pin del LED Azul
const int pinLedRojo = 23;  // Pin del LED Rojo
const int pinBoton = 19; // Pin del pulsador
const int pinRelay = 18;    // Pin del Relay
const int potePin = 32;       // Pin del potenciometro

int contadorPulsador = 0;       // Contador de pulsaciones sobre el botón
int estadoLedAzul = LOW;        // Estado del LED
int ultimoEstado = HIGH;        // Estado del pulsador
unsigned long ultimoRebote = 0; // Último tiempo de rebote del pulsador
int delayRebote = 50;           // Tiempo de rebote del pulsador

volatile int estadoPote;      // Variable del nivel del potenciómetro
volatile int estadoPoteRelay; // Variable del nivel del potenciómetro mapeada al relay

// Crear un objeto display para usar el OLED
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup()
{
  Serial.begin(115200);
  pinMode(pinLedAzul, OUTPUT);     // Configurar el pin del LED Azul como salida
  pinMode(pinRelay, OUTPUT);  // Configurar el pin del Rele como salida
  pinMode(pinBoton, INPUT_PULLUP);    // Configurar el pin del pulsador como entrada
  pinMode(potePin, INPUT);     // Configurar el pin del potenciómetro como entrada
  pinMode(pinLedRojo, OUTPUT); // Configurar el pin del LED Rojo como salida
  displayInit(); // Ejecutar la función de configuración del display
  delay(2000);
}

void loop()
{
  int lectura = digitalRead(pinBoton);
  if (ultimoEstado != lectura && (millis() - ultimoRebote) > delayRebote) {
    ultimoRebote = millis();
    if (lectura == LOW) {
      estadoLedAzul = !estadoLedAzul;
      contadorPulsador++;
      digitalWrite(pinLedAzul, estadoLedAzul);
      Serial.printf("Pulsador presionado, Estado: %d \n", estadoLedAzul);
    }
  }
  ultimoEstado = lectura;

  //----------------Led Rojo con Pote-----------------
  estadoPote = map(analogRead(potePin), 0, 4095, 0, 255); // Leer el estado del pote y mapearlo a 8 bits
  analogWrite(pinLedRojo, estadoPote);

  //-----------------Led verde con relay + pote 50%----------
  estadoPoteRelay = analogRead(potePin) >= 2047 ? HIGH : LOW; // Leer el estado del pote y asignar el valor de HIGH o LOW según el valor leído
  digitalWrite(pinRelay, estadoPoteRelay);

  mostrarDatosPorDisplay();
  delay(20);
}

// Función de configuración del display OLED
void displayInit()
{
  display.begin(0x3C, true);
  display.clearDisplay(); // Vaciar la pantalla del display
  display.setTextSize(1); // Configurar tamaño de la letra
  display.setTextColor(SH110X_WHITE); // Configurar el color de la letra
  display.setCursor(0, 0); // Setear el cursor en la posición 0,0 (esq. vertical izquierda)
  display.println("   TP N1 - IoT - 4K4");
  display.println("Datos de Compilacion:");
  display.printf("Fecha %s\n", __DATE__);
  display.printf("Hora: %s\n", __TIME__);
  display.display();
}

void mostrarDatosPorDisplay() {
  // Limpiar el display y setear el cursor en la posición inicial
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("\n");
  // Escribir en el display el estado de cada dispositivo
  display.printf("- Puls.: %d acciones\n", contadorPulsador);
  display.printf("- Led A: %s\n", estadoLedAzul ? "Encendido" : "Apagado");
  display.printf("- Led R: %d%% potencia\n", map(estadoPote, 0, 255, 0, 100));
  display.printf("- Led V: %s\n", estadoPoteRelay ? "Encendido" : "Apagado");
  display.display();
}
