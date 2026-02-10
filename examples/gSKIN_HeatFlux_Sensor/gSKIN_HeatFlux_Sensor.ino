/*******************************************************************************
 * Ejemplo de uso: ADS1220_NTC_Extended con sensor gSKIN-XI 27 9C
 * 
 * Este ejemplo muestra cómo:
 * 1. Configurar el ADS1220 para sensores de flujo térmico
 * 2. Leer voltaje, flujo térmico y potencia térmica
 * 3. Aplicar corrección de temperatura
 * 4. Usar filtros de media móvil
 * 
 * Hardware:
 * - ESP32 o Arduino Mega
 * - Módulo ADS1220 #1 (para sensor gSKIN)
 * - Sensor gSKIN-XI 27 9C conectado a AIN0-AIN1
 * - Módulo ADS1220 #2 (para NTC de temperatura del sensor)
 * - NTC conectado al segundo ADS1220
 * 
 * Conexiones ADS1220 #1 (gSKIN):
 * - CS: GPIO 5
 * - SCLK: GPIO 18
 * - MOSI: GPIO 23
 * - MISO: GPIO 19
 * - DRDY: No conectado (255)
 * 
 * Conexiones gSKIN-XI 27 9C:
 * - Cable rojo (+) -> AIN0
 * - Cable negro (-) -> AIN1
 * - Montar sensor con pasta térmica
 * - Dirección positiva: hacia el flujo de calor esperado
 ******************************************************************************/

#include <SPI.h>
#include "ADS1220_NTC_Extended.h"

// ===== CONFIGURACIÓN DE PINES =====
#define CS_PIN_GSKIN    5
#define DRDY_PIN_GSKIN  255  // No conectado

// ===== CALIBRACIÓN DEL SENSOR gSKIN-XI 27 9C =====
// Valores del certificado de calibración
#define GSKIN_SO        54.3f        // µV/(W/m²) @ To
#define GSKIN_SC        0.0697f      // (µV/(W/m²))/°C
#define GSKIN_TO        22.5f        // °C
#define GSKIN_AREA      0.000324f    // m² (18mm x 18mm)

// ===== INSTANCIAS =====
// Constructor con IDAC = 0 (sin corriente de excitación para sensores termoeléctricos)
ADS1220_NTC_Extended heatFluxSensor(CS_PIN_GSKIN, DRDY_PIN_GSKIN, 0.0f);

// ===== VARIABLES GLOBALES =====
float sensorTemperature = 25.0f;  // Temperatura del sensor (leer desde NTC en segundo ADS1220)
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 1000;  // Imprimir cada 1 segundo

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println(F("\n========================================"));
    Serial.println(F("  gSKIN-XI 27 9C Heat Flux Sensor Test"));
    Serial.println(F("========================================\n"));
    
    // ===== INICIALIZAR ADS1220 =====
    Serial.println(F("Inicializando ADS1220..."));
    if (!heatFluxSensor.begin(&SPI)) {
        Serial.println(F("ERROR: No se pudo inicializar el ADS1220"));
        Serial.println(F("Verificar:"));
        Serial.println(F("  - Conexiones SPI"));
        Serial.println(F("  - Alimentación del módulo"));
        Serial.println(F("  - Pin CS correcto"));
        while (1) {
            delay(1000);
        }
    }
    Serial.println(F("✓ ADS1220 inicializado correctamente\n"));
    
    // ===== CONFIGURAR MODO HEAT FLUX =====
    Serial.println(F("Configurando modo Heat Flux..."));
    heatFluxSensor.setChannelMode(CHANNEL_0, MODE_HEAT_FLUX);
    Serial.println(F("✓ Modo configurado: HEAT_FLUX (IDAC OFF)\n"));
    
    // ===== CONFIGURAR CALIBRACIÓN DEL SENSOR =====
    Serial.println(F("Configurando calibración del gSKIN-XI 27 9C..."));
    heatFluxSensor.setHeatFluxCalibration(
        CHANNEL_0,
        GSKIN_SO,      // Sensibilidad @ To
        GSKIN_SC,      // Coeficiente de temperatura
        GSKIN_TO,      // Temperatura de calibración
        GSKIN_AREA     // Área del sensor
    );
    Serial.println(F("✓ Calibración configurada:"));
    Serial.print(F("  So = ")); Serial.print(GSKIN_SO, 4);
    Serial.println(F(" µV/(W/m²)"));
    Serial.print(F("  Sc = ")); Serial.print(GSKIN_SC, 6);
    Serial.println(F(" (µV/(W/m²))/°C"));
    Serial.print(F("  To = ")); Serial.print(GSKIN_TO, 2);
    Serial.println(F(" °C"));
    Serial.print(F("  Área = ")); Serial.print(GSKIN_AREA * 1e6f, 2);
    Serial.println(F(" mm²\n"));
    
    // ===== CONFIGURAR FILTRO DE MEDIA MÓVIL =====
    Serial.println(F("Configurando filtro de media móvil..."));
    heatFluxSensor.setMovingAverageSize(10);  // Promedio de 10 muestras
    Serial.println(F("✓ Filtro configurado: 10 muestras\n"));
    
    // ===== CONFIGURAR VELOCIDAD DE MUESTREO =====
    Serial.println(F("Configurando velocidad de muestreo..."));
    heatFluxSensor.setDataRate(ADS1220_DR_90SPS);  // 90 muestras por segundo
    Serial.println(F("✓ Velocidad: 90 SPS\n"));
    
    // ===== IMPRIMIR CONFIGURACIÓN DE REGISTROS =====
    heatFluxSensor.printRegisters(&Serial);
    
    // ===== INFORMACIÓN ADICIONAL =====
    Serial.println(F("========================================"));
    Serial.println(F("NOTAS IMPORTANTES:"));
    Serial.println(F("========================================"));
    Serial.println(F("1. Asegúrese de que el sensor esté correctamente montado"));
    Serial.println(F("2. Use pasta térmica para buen contacto térmico"));
    Serial.println(F("3. La dirección positiva del sensor debe coincidir con el flujo"));
    Serial.println(F("4. Para mejor precisión, mida la temperatura del sensor"));
    Serial.println(F("5. Rango de calibración: -30°C a 70°C"));
    Serial.println(F("6. Flujo positivo = calor fluyendo hacia arriba"));
    Serial.println(F("========================================\n"));
    
    Serial.println(F("Iniciando lecturas...\n"));
    delay(2000);
}

void loop() {
    unsigned long currentTime = millis();
    
    // Imprimir cada PRINT_INTERVAL ms
    if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
        lastPrintTime = currentTime;
        
        // ===== LEER TEMPERATURA DEL SENSOR =====
        // TODO: Implementar lectura desde el segundo ADS1220 con NTC
        // Por ahora, usar valor fijo o estimar
        // sensorTemperature = leerTemperaturaNTC();
        
        // Para prueba, estimar temperatura ambiente
        sensorTemperature = 25.0f;  // °C (valor estimado)
        
        // ===== LEER DATOS DEL SENSOR =====
        
        // 1. Voltaje RAW (sin procesar)
        float voltage_uV = heatFluxSensor.readHeatFluxVoltage(CHANNEL_0);
        
        // 2. Sensibilidad corregida por temperatura
        float sensitivity = heatFluxSensor.getTemperatureCorrectedSensitivity(CHANNEL_0, sensorTemperature);
        
        // 3. Flujo térmico con corrección de temperatura
        float heatFlux = heatFluxSensor.readHeatFlux(CHANNEL_0, sensorTemperature);
        
        // 4. Flujo térmico filtrado (con media móvil)
        float heatFlux_filtered = heatFluxSensor.readHeatFluxFiltered(CHANNEL_0, sensorTemperature);
        
        // 5. Potencia térmica total
        float thermalPower = heatFluxSensor.readThermalPower(CHANNEL_0, sensorTemperature);
        
        // ===== IMPRIMIR RESULTADOS =====
        Serial.println(F("========================================"));
        Serial.print(F("Timestamp: ")); Serial.print(currentTime / 1000.0f, 3);
        Serial.println(F(" s"));
        Serial.println(F("========================================"));
        
        Serial.print(F("Temperatura sensor: ")); Serial.print(sensorTemperature, 2);
        Serial.println(F(" °C"));
        
        Serial.print(F("Voltaje: ")); Serial.print(voltage_uV, 3);
        Serial.println(F(" µV"));
        
        Serial.print(F("Sensibilidad corregida: ")); Serial.print(sensitivity, 4);
        Serial.println(F(" µV/(W/m²)"));
        
        Serial.print(F("Flujo térmico: ")); Serial.print(heatFlux, 3);
        Serial.println(F(" W/m²"));
        
        Serial.print(F("Flujo filtrado: ")); Serial.print(heatFlux_filtered, 3);
        Serial.println(F(" W/m²"));
        
        Serial.print(F("Potencia térmica: ")); Serial.print(thermalPower * 1000.0f, 3);
        Serial.println(F(" mW"));
        
        Serial.println();
        
        // ===== INTERPRETACIÓN =====
        if (abs(heatFlux) < 1.0f) {
            Serial.println(F("Estado: Sin flujo térmico significativo"));
        } else if (heatFlux > 0) {
            Serial.println(F("Estado: Flujo térmico POSITIVO (calor hacia arriba)"));
        } else {
            Serial.println(F("Estado: Flujo térmico NEGATIVO (calor hacia abajo)"));
        }
        
        Serial.println(F("========================================\n"));
    }
}

/*******************************************************************************
 * FUNCIONES ADICIONALES (ejemplos)
 ******************************************************************************/

/**
 * @brief Ejemplo de cómo leer solo el flujo térmico sin corrección
 */
void ejemplo_lectura_simple() {
    float heatFlux = heatFluxSensor.readHeatFluxUncorrected(CHANNEL_0);
    
    Serial.print(F("Flujo térmico (sin corrección): "));
    Serial.print(heatFlux, 3);
    Serial.println(F(" W/m²"));
}

/**
 * @brief Ejemplo de cómo cambiar la calibración en tiempo real
 */
void ejemplo_cambiar_calibracion() {
    // Si se tiene otro sensor con diferente calibración
    float nuevo_So = 60.0f;  // Ejemplo
    
    heatFluxSensor.setHeatFluxSensitivity(CHANNEL_0, nuevo_So);
    
    Serial.println(F("Calibración actualizada"));
}

/**
 * @brief Ejemplo de cómo obtener información de calibración
 */
void ejemplo_obtener_calibracion() {
    HeatFluxCalibration cal = heatFluxSensor.getHeatFluxCalibration(CHANNEL_0);
    
    Serial.println(F("=== Calibración actual ==="));
    Serial.print(F("So: ")); Serial.print(cal.So, 4);
    Serial.println(F(" µV/(W/m²)"));
    Serial.print(F("Sc: ")); Serial.print(cal.Sc, 6);
    Serial.println(F(" (µV/(W/m²))/°C"));
    Serial.print(F("To: ")); Serial.print(cal.To, 2);
    Serial.println(F(" °C"));
    Serial.print(F("Área: ")); Serial.print(cal.sensorArea * 1e6f, 2);
    Serial.println(F(" mm²"));
}

/**
 * @brief Ejemplo de cómo usar diferentes tamaños de filtro
 */
void ejemplo_cambiar_filtro() {
    // Sin filtro (respuesta rápida pero ruidosa)
    heatFluxSensor.setMovingAverageSize(1);
    
    // Filtro medio (balance entre velocidad y suavidad)
    heatFluxSensor.setMovingAverageSize(5);
    
    // Filtro fuerte (respuesta lenta pero muy suave)
    heatFluxSensor.setMovingAverageSize(20);
}

/*******************************************************************************
 * NOTAS SOBRE PRECISIÓN Y CALIBRACIÓN
 ******************************************************************************/

/*
 * CORRECCIÓN DE TEMPERATURA:
 * 
 * La sensibilidad del sensor varía con la temperatura según:
 *   S = So + (Ts - To) * Sc
 * 
 * Donde:
 *   S  = sensibilidad corregida
 *   So = sensibilidad @ temperatura de calibración (54.3 µV/(W/m²))
 *   Ts = temperatura actual del sensor
 *   To = temperatura de calibración (22.5 °C)
 *   Sc = coeficiente de temperatura (0.0697 (µV/(W/m²))/°C)
 * 
 * Ejemplo:
 *   Si Ts = 40°C:
 *   S = 54.3 + (40 - 22.5) * 0.0697
 *   S = 54.3 + 1.22
 *   S = 55.52 µV/(W/m²)
 * 
 * Error sin corrección @ 40°C: ~2.2%
 */

/*
 * FLUJO TÉRMICO:
 * 
 * El flujo térmico se calcula como:
 *   φ = U / S
 * 
 * Donde:
 *   φ = flujo térmico [W/m²]
 *   U = voltaje medido [µV]
 *   S = sensibilidad [µV/(W/m²)]
 * 
 * Ejemplo:
 *   Si U = 1000 µV y S = 54.3 µV/(W/m²):
 *   φ = 1000 / 54.3
 *   φ = 18.42 W/m²
 */

/*
 * POTENCIA TÉRMICA:
 * 
 * La potencia térmica total se calcula como:
 *   Φ = A * φ
 * 
 * Donde:
 *   Φ = potencia térmica [W]
 *   A = área del sensor [m²]
 *   φ = flujo térmico [W/m²]
 * 
 * Ejemplo para gSKIN-XI 27 9C (18mm x 18mm = 324 mm² = 0.000324 m²):
 *   Si φ = 18.42 W/m²:
 *   Φ = 0.000324 * 18.42
 *   Φ = 0.00597 W = 5.97 mW
 */
