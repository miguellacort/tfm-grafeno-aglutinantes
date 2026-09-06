#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- PINES MOTOR Y ENCODER ---
const int pinSTEP = 12;
const int pinDIR  = 13;
const int pinCLK = 8;
const int pinDT  = 9;
const int pinSW  = 10;

// --- PINES BOTONES ---
const int pinPanic = 2; // Rojo (INTERRUPCIÓN)
const int pinStart = 6; // Azul Der
const int pinHome  = 7; // Azul Izq

// Configuración de AccelStepper (1 = Driver interface)
AccelStepper stepper(1, pinSTEP, pinDIR);

// Factor de conversión: Husillo T8 (1/32). 1 cm = 10 mm = 8000 pasos.
// Factor de conversión: Husillo T8 (1/4). 1 cm = 10 mm = 1000 pasos.
const int pasosPorMm = 100; 
volatile bool paradaEmergencia = false;

// --- VARIABLES DE INTERFAZ Y VALORES POR DEFECTO ---
int velocidad_mms = 10; // 1.0 cm/s
int distancia_cm = 20;  // 20 cm por defecto
int ultimoEstadoCLK;
unsigned long ultimoBoton = 0; // Temporizador para el filtro antirrebote

// --- MÁQUINA DE ESTADOS ---
enum EstadoMenu {
  BLOQUEO,
  EDIT_VEL,
  EDIT_DIS,
  LISTO
};
EstadoMenu estadoActual = BLOQUEO;

void setup() {
  lcd.init();
  lcd.backlight();
  
  pinMode(pinCLK, INPUT);
  pinMode(pinDT,  INPUT);
  pinMode(pinSW,  INPUT_PULLUP);
  pinMode(pinStart, INPUT_PULLUP);
  pinMode(pinHome,  INPUT_PULLUP);
  pinMode(pinPanic, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(pinPanic), emergencyStop, FALLING);

  // Configuración inicial del motor
  stepper.setMaxSpeed(velocidad_mms * pasosPorMm); 
  stepper.setAcceleration((velocidad_mms * pasosPorMm) * 0.5); 

  ultimoEstadoCLK = digitalRead(pinCLK);
  
  // Al encender, forzamos la parada para entrar en BLOQUEO directamente
  paradaEmergencia = true; 
  actualizarPantallaTop();
  mostrarEstado("BLOQUEO");
}

void loop() {
  // --- 1. GESTIÓN DE EMERGENCIA / BLOQUEO ---
  if (paradaEmergencia) {
    if (estadoActual != BLOQUEO) {
      estadoActual = BLOQUEO;
      mostrarEstado("BLOQUEO");
    }
    
    // Si pulsamos el botón del encoder, reseteamos el bloqueo y pasamos a editar velocidad
    if (digitalRead(pinSW) == LOW && millis() - ultimoBoton > 300) { 
      ultimoBoton = millis();
      paradaEmergencia = false;
      stepper.setCurrentPosition(0); 
      estadoActual = EDIT_VEL;
      mostrarEstado("EDIT VEL");
    }
    return; // En modo bloqueo, ignoramos el resto del bucle
  }

  // --- 2. NAVEGACIÓN DEL MENÚ (Clic en el Encoder) ---
  if (digitalRead(pinSW) == LOW && millis() - ultimoBoton > 300) {
    ultimoBoton = millis();
    
    if (estadoActual == EDIT_VEL) {
      estadoActual = EDIT_DIS;
      mostrarEstado("EDIT DIS");
    } 
    else if (estadoActual == EDIT_DIS) {
      estadoActual = LISTO;
      mostrarEstado("LISTO");
    }
    else if (estadoActual == LISTO) {
      estadoActual = EDIT_VEL;
      mostrarEstado("EDIT VEL");
    }
  }

  // --- 3. EDICIÓN DE VALORES (Giro del Encoder) ---
  int estadoActualCLK = digitalRead(pinCLK);
  if (estadoActualCLK != ultimoEstadoCLK && estadoActualCLK == 1) {
    int direccion = (digitalRead(pinDT) != estadoActualCLK) ? 1 : -1;
    
    if (estadoActual == EDIT_VEL) {
      velocidad_mms += direccion;
      if (velocidad_mms < 1) velocidad_mms = 1; 
      stepper.setMaxSpeed(velocidad_mms * pasosPorMm);
      stepper.setAcceleration((velocidad_mms * pasosPorMm) * 2);
      actualizarPantallaTop();
    } 
    else if (estadoActual == EDIT_DIS) {
      distancia_cm += direccion;
      if (distancia_cm < 1) distancia_cm = 1; // Límite inferior de 1 cm
      actualizarPantallaTop();
    }
  }
  ultimoEstadoCLK = estadoActualCLK;

  // --- 4. EJECUCIÓN DE MOVIMIENTO (Solo permitido si está en LISTO) ---
  if (estadoActual == LISTO) {
    // Calculamos los pasos: (cm * 10) = mm -> mm * pasosPorMm
    // Usamos '10L' para forzar matemática de tipo Long y evitar desbordamientos
    long pasosDestino = distancia_cm * 10L * pasosPorMm; 
    
    if (digitalRead(pinStart) == LOW) {
      ejecutarMovimiento(pasosDestino, "AVANZA"); 
    } 
    else if (digitalRead(pinHome) == LOW) {
      ejecutarMovimiento(-pasosDestino, "RETORNO"); 
    }
  }
}

// --- FUNCIONES AUXILIARES ---

void ejecutarMovimiento(long destino, String msj) {
  mostrarEstado(msj);
  stepper.move(destino); 
  
  while (stepper.distanceToGo() != 0) {
    if (paradaEmergencia) break; 
    stepper.run(); 
  }
  
  if (!paradaEmergencia) {
    estadoActual = LISTO; // Nos aseguramos de volver al estado correcto
    mostrarEstado("LISTO");
  }
}

void emergencyStop() {
  paradaEmergencia = true;
}

void actualizarPantallaTop() {
  lcd.setCursor(0, 0);
  float velocidad_cms = velocidad_mms / 10.0;
  
  lcd.print("V:");
  lcd.print(velocidad_mms);
  lcd.print(" mm/s D:");
  lcd.print(distancia_cm);
  lcd.print("cm "); // Los espacios finales limpian la pantalla de "restos" de caracteres
}

void mostrarEstado(String msg) {
  lcd.setCursor(0, 1);
  lcd.print(msg);
  // Un bucle limpio para borrar el resto de la segunda línea
  for (unsigned int i = msg.length(); i < 16; i++) {
    lcd.print(" ");
  }
}