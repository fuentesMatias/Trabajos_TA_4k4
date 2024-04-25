
#include <Adafruit_SH110X.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <splash.h>

#define LINEA_BLANCO F("                    ")
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels



const int pinLedAzul = 2;  // Pin del LED
const int pinLedRojo = 23;  // Pin del LED Rojo
const int pinBoton = 19;    // Pin del pulsador

const int pinRelay = 18;    // Pin del Relay
volatile int estadoPoteRelay; // Variable del nivel del potenciómetro mapeada al relay

const int potePin = 32;     // Pin del potenciometro
volatile int estadoPote;      // Variable del nivel del potenciómetro



//-------- Uso Pulsador y led-------------------
int contadorPulsador = 0;       // Contador de pulsaciones sobre el botón
int estadoLedAzul = LOW; // Estado del LED   
int estadoBoton = LOW; // Estado del pulsador
int ultimoEstado = LOW; // Estado anterior del pulsador   
unsigned long ultimoRebote = 0; // Último tiempo de rebote del pulsador
int delayRebote = 50; // Tiempo de rebote del pulsador
//----------------------------------------------

//----------------Display-----------------------

//Placa fisica
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

//Wokwi
// Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);




void setup() {
  pinMode(pinLedAzul, OUTPUT); // Configurar el pin del LED como salida
  pinMode(pinBoton, INPUT_PULLUP); // Configurar el pin del pulsador como entrada
  pinMode(pinRelay, OUTPUT);  // Configurar el pin del Rele como salida
  pinMode(potePin, INPUT);     // Configurar el pin del potenciómetro como entrada
  pinMode(pinLedRojo, OUTPUT); // Configurar el pin del LED Rojo como salida
  Serial.begin(115200);
  displayInit();
}

void loop() {
  // Leer el estado del pulsador
  int lectura = digitalRead(pinBoton);

  // Si el estado del pulsador ha cambiado, registrar el tiempo de rebote
  if (lectura != ultimoEstado) {
    ultimoRebote = millis();
  }

  // Si ha pasado suficiente tiempo desde el último rebote, actualizar el estado del pulsador
  if ((millis() - ultimoRebote) > delayRebote) {
    // Si el estado del pulsador ha cambiado, actualizar el estado del LED
    if (lectura != estadoBoton) {
      estadoBoton = lectura;

      // Si el pulsador ha sido presionado, cambiar el estado del LED
      if (estadoBoton == LOW) {
        estadoLedAzul = !estadoLedAzul;
        digitalWrite(pinLedAzul, estadoLedAzul);
        contadorPulsador++;
      }
    }
  }
  // Guardar el último estado del pulsador
  ultimoEstado = lectura;

  //----------------Led Rojo con Pote-----------------
  estadoPote = map(analogRead(potePin), 0, 4095, 0, 255); // Leer el estado del pote y mapearlo a 8 bits
  analogWrite(pinLedRojo, estadoPote);


  //-----------------Led verde con relay + pote 50%----------
  estadoPoteRelay = analogRead(potePin) >= 2047 ? HIGH : LOW; // Leer el estado del pote y asignar el valor de HIGH o LOW según el valor leído
  digitalWrite(pinRelay, estadoPoteRelay);


  //---------------Escribir display--------------------
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("\n");
  display.printf("- Puls.: %d acciones\n", contadorPulsador);
  display.printf("- Led A: %s\n", estadoLedAzul ? "Encendido" : "Apagado");
  display.printf("- Led R: %d%% potencia\n", map(estadoPote, 0, 255, 0, 100));
  display.printf("- Led V: %s\n", estadoPoteRelay ? "Encendido" : "Apagado");
  display.display();


}

void displayInit()
{
  // Reemplazar las líneas comentadas por las correspondientes para probar con la placa en Clase
  display.begin(0x3C, true);
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); // Vaciar la pantalla del display
  display.setTextSize(1); // Configurar tamaño de la letra
  display.setTextColor(SH110X_WHITE);
  //display.setTextColor(SSD1306_WHITE); // Configurar el color de la letra
  display.setCursor(0, 0); // Setear el cursor en la posición 0,0 (esq. vertical izquierda)
  display.println("   TP N1 - IoT - 4K4");
  display.println("Datos de Compilacion:");
  display.printf("Fecha %s\n", __DATE__);
  display.printf("Hora: %s\n", __TIME__);
  display.display();
}