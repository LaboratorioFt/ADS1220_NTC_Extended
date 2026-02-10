/*******************************************************************************
 * Ejemplo: Lectura Simple de Temperatura con NTC
 * 
 * Este ejemplo demuestra cómo:
 * 1. Configurar el ADS1220 para medición con termistor NTC
 * 2. Leer temperatura con y sin filtro de media móvil
 * 3. Usar coeficientes Steinhart-Hart predeterminados
 * 
 * Hardware:
 * - ESP32
 * - Módulo CJMCU-1220 (ADS1220)
 * - Termistor NTC 10kΩ @ 25°C
 * 
 * Conexiones ESP32 - ADS1220:
 * - GPIO 5  -> CS
 * - GPIO 18 -> SCLK
 * - GPIO 23 -> MOSI
 * - GPIO 19 -> MISO
 * - 3.3V    -> VDD
 * - GND     -> GND
 * 
 * Conexiones NTC - ADS1220:
 * - Terminal 1 del NTC -> AIN0
 * - Terminal 2 del NTC -> AIN1
 * - El IDAC inyecta corriente automáticamente
 * 
 * Autor: Pedro Emigdio García González - IFM-UMSNH
 * Fecha: Enero 2025
 ******************************************************************************/

#include <SPI.h>
#include "ADS1220_NTC_Extended.h"

// ===== CONFIGURACIÓN DE PINES =====
#define CS_PIN      5
#define DRDY_PIN    255  // No usado

// ===== CONFIGURACIÓN DEL IDAC =====
// Corriente de excitación para el NTC en µA
// Valores comunes: 10, 50, 100, 250, 500, 1000, 1500 µA
#define IDAC_CURRENT_UA  50.0f

// ===== INSTANCIA DEL SENSOR =====
ADS1220_NTC_Extended ntcSensor(CS_PIN, DRDY_PIN, IDAC_CURRENT_UA);

// ===== VARIABLES GLOBALES =====
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 1000;  // Imprimir cada 1 segundo

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println(F("\n========================================"));
    Serial.println(F("   Lectura Simple de Temperatura NTC"));
    Serial.println(F("========================================\n"));
    
    // ===== INICIALIZAR ADS1220 =====
    Serial.println(F("Inicializando ADS1220..."));
    if (!ntcSensor.begin(&SPI)) {
        Serial.println(F("ERROR: No se pudo inicializar el ADS1220"));
        Serial.println(F("Verificar:"));
        Serial.println(F("  - Conexiones SPI (CS, SCLK, MOSI, MISO)"));
        Serial.println(F("  - Alimentación del módulo (3.3V)"));
        Serial.println(F("  - Pin CS correcto (GPIO 5)"));
        while (1) {
            delay(1000);
        }
    }
    Serial.println(F("✓ ADS1220 inicializado correctamente\n"));
    
    // ===== CONFIGURAR MODO NTC =====
    Serial.println(F("Configurando modo NTC..."));
    ntcSensor.setChannelMode(CHANNEL_0, MODE_NTC);
    Serial.println(F("✓ Modo configurado: NTC (IDAC activo)\n"));
    
    // ===== CONFIGURAR COEFICIENTES STEINHART-HART =====
    // Usando valores predeterminados de la librería
    // Si tienes los coeficientes específicos de tu NTC, cámbialos aquí:
    // ntcSensor.setNTCCoefficients(CHANNEL_0, A, B, C);
    Serial.println(F("Usando coeficientes Steinhart-Hart predeterminados:"));
    Serial.println(F("  A = 1.025084947e-03"));
    Serial.println(F("  B = 2.398038697e-04"));
    Serial.println(F("  C = 1.540123584e-07\n"));
    
    // ===== CONFIGURAR CORRIENTE IDAC =====
    Serial.print(F("Corriente IDAC configurada: "));
    Serial.print(IDAC_CURRENT_UA, 0);
    Serial.println(F(" µA\n"));
    
    // ===== CONFIGURAR FILTRO DE MEDIA MÓVIL =====
    Serial.println(F("Configurando filtro de media móvil..."));
    ntcSensor.setMovingAverageSize(10);  // Promedio de 10 muestras
    Serial.println(F("✓ Filtro configurado: 10 muestras\n"));
    
    // ===== CONFIGURAR VELOCIDAD DE MUESTREO =====
    Serial.println(F("Configurando velocidad de muestreo..."));
    ntcSensor.setDataRate(ADS1220_DR_20SPS);  // 20 muestras por segundo
    Serial.println(F("✓ Velocidad: 20 SPS\n"));
    
    // ===== IMPRIMIR CONFIGURACIÓN DE REGISTROS =====
    ntcSensor.printRegisters(&Serial);
    
    Serial.println(F("========================================"));
    Serial.println(F("NOTAS:"));
    Serial.println(F("========================================"));
    Serial.println(F("- 'Temp (sin filtro)' = lectura instantánea"));
    Serial.println(F("- 'Temp (filtrada)' = promedio móvil de 10 muestras"));
    Serial.println(F("- El filtro reduce el ruido pero añade retardo"));
    Serial.println(F("========================================\n"));
    
    Serial.println(F("Iniciando lecturas...\n"));
    delay(2000);
}

void loop() {
    unsigned long currentTime = millis();
    
    // Imprimir cada PRINT_INTERVAL ms
    if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
        lastPrintTime = currentTime;
        
        // ===== LEER DATOS DEL NTC =====
        
        // 1. Lectura directa (sin filtro)
        float temperature = ntcSensor.readTemperature(CHANNEL_0);
        
        // 2. Lectura con filtro de media móvil
        float temperatureFiltered = ntcSensor.readTemperatureFiltered(CHANNEL_0);
        
        // 3. Lecturas adicionales (opcional, para debugging)
        float voltage = ntcSensor.readVoltage(CHANNEL_0);
        float resistance = ntcSensor.readResistance(CHANNEL_0);
        
        // ===== IMPRIMIR RESULTADOS =====
        Serial.println(F("========================================"));
        Serial.print(F("Tiempo: "));
        Serial.print(currentTime / 1000.0f, 1);
        Serial.println(F(" s"));
        Serial.println(F("========================================"));
        
        Serial.print(F("Voltaje:           "));
        Serial.print(voltage * 1000.0f, 3);  // Convertir a mV
        Serial.println(F(" mV"));
        
        Serial.print(F("Resistencia:       "));
        Serial.print(resistance / 1000.0f, 3);  // Convertir a kΩ
        Serial.println(F(" kΩ"));
        
        Serial.print(F("Temp (sin filtro): "));
        Serial.print(temperature, 2);
        Serial.println(F(" °C"));
        
        Serial.print(F("Temp (filtrada):   "));
        Serial.print(temperatureFiltered, 2);
        Serial.println(F(" °C"));
        
        // Mostrar diferencia entre lecturas
        float diff = abs(temperature - temperatureFiltered);
        Serial.print(F("Diferencia:        "));
        Serial.print(diff, 2);
        Serial.println(F(" °C"));
        
        Serial.println(F("========================================\n"));
    }
}

/*******************************************************************************
 * NOTAS SOBRE CONFIGURACIÓN DEL NTC
 ******************************************************************************/

/*
 * COEFICIENTES STEINHART-HART:
 * 
 * Si tienes los coeficientes específicos de tu NTC (generalmente proporcionados
 * por el fabricante o calculados a partir de la tabla R-T), puedes configurarlos:
 * 
 *   ntcSensor.setNTCCoefficients(CHANNEL_0, A, B, C);
 * 
 * Donde:
 *   A, B, C = coeficientes de la ecuación Steinhart-Hart
 * 
 * La temperatura se calcula como:
 *   1/T = A + B*ln(R) + C*(ln(R))^3
 * 
 * Donde:
 *   T = temperatura en Kelvin
 *   R = resistencia del NTC en Ohmios
 */

/*
 * CORRIENTE IDAC:
 * 
 * La corriente IDAC determina el voltaje desarrollado en el NTC:
 *   V = I * R
 * 
 * Consideraciones:
 * - Mayor corriente = mayor voltaje = mejor resolución
 * - Mayor corriente = más auto-calentamiento del NTC
 * - Para NTC 10kΩ con IDAC=250µA: V ≈ 2.5V @ 25°C
 * 
 * Valores recomendados:
 * - NTC 1kΩ:  50-100 µA
 * - NTC 10kΩ: 100-250 µA
 * - NTC 100kΩ: 10-50 µA
 */

/*
 * FILTRO DE MEDIA MÓVIL:
 * 
 * El filtro suaviza las lecturas ruidosas pero introduce retardo:
 * 
 * - Tamaño 1: Sin filtro (rápido, ruidoso)
 * - Tamaño 5: Filtro ligero (balance)
 * - Tamaño 10: Filtro medio (recomendado)
 * - Tamaño 20: Filtro fuerte (lento, muy suave)
 * 
 * Para cambiar el tamaño del filtro:
 *   ntcSensor.setMovingAverageSize(tamaño);
 */
