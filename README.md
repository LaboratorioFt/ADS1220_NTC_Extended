# ADS1220_NTC_Extended Library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/)

Librería extendida para el ADC de 24 bits ADS1220 que soporta:
- **Termistores NTC** con ecuación de Steinhart-Hart
- **Sensores de flujo térmico gSKIN** (termoeléctricos)

Compatible con ESP32 y Arduino Mega.

## ✨ Características Principales

### Funcionalidad Original (NTC)
- ✅ Medición diferencial de temperatura con termistores NTC
- ✅ Inyección de corriente constante mediante IDAC (10µA - 1500µA)
- ✅ Conversión mediante ecuación de Steinhart-Hart
- ✅ Filtro de promedio móvil configurable

### Nueva Funcionalidad v2.0 (Sensores de Flujo Térmico)
- ✅ Medición directa de voltaje termoeléctrico (sin IDAC)
- ✅ Cálculo de flujo térmico (W/m²)
- ✅ Cálculo de potencia térmica total (W)
- ✅ Corrección de sensibilidad por temperatura
- ✅ Soporte específico para sensores gSKIN (greenTEG)
- ✅ Filtro de promedio móvil para flujo térmico

## 📋 Tabla de Contenidos
- [Hardware Soportado](#-hardware-soportado)
- [Instalación](#-instalación)
- [Conexiones Hardware](#-conexiones-hardware)
- [Uso Básico](#-uso-básico)
- [Ejemplos](#-ejemplos)
- [API Completa](#-api-completa)
- [Precisión y Errores](#-precisión-y-errores)
- [Troubleshooting](#-troubleshooting)
- [Contribuir](#-contribuir)
- [Licencia](#-licencia)

## 🔧 Hardware Soportado

### Sensores de Flujo Térmico Probados
- **gSKIN-XI 27 9C** (18mm x 18mm)
  - Sensibilidad: 35.0 - 65.0 µV/(W/m²)
  - Área: 324 mm² (0.000324 m²)
  - Rango de operación: -50°C a 150°C
  - Rango de calibración: -30°C a 70°C

### Termistores NTC
- NTC 10kΩ @ 25°C (ej: Measurement Specialties)
- Cualquier NTC con coeficientes Steinhart-Hart conocidos

### ADC
- **ADS1220** (Texas Instruments)
  - 24 bits de resolución
  - 2 canales diferenciales
  - Referencia interna 2.048V
  - Interfaz SPI
  - Módulo probado: CJMCU-1220

### Microcontroladores
- ESP32
- Arduino Mega

## 📥 Instalación

### Método 1: Arduino IDE (Recomendado)
1. Descarga el repositorio como ZIP:
   - Click en `Code` → `Download ZIP`
2. En Arduino IDE:
   - `Sketch` → `Include Library` → `Add .ZIP Library...`
   - Selecciona el archivo descargado
3. Reinicia el Arduino IDE

### Método 2: Manual
1. Descarga o clona este repositorio
2. Copia la carpeta `ADS1220_NTC_Extended` a tu carpeta de librerías:
   - **Windows:** `Documentos/Arduino/libraries/`
   - **Mac:** `~/Documents/Arduino/libraries/`
   - **Linux:** `~/Arduino/libraries/`
3. Reinicia el Arduino IDE

### Método 3: Git
```bash
cd ~/Arduino/libraries/
git clone https://github.com/YOUR_USERNAME/ADS1220_NTC_Extended.git
```

## 🔌 Conexiones Hardware

### ESP32 con ADS1220
```
ESP32          ADS1220
-----          -------
GPIO 5    -->  CS
GPIO 18   -->  SCLK
GPIO 23   -->  MOSI
GPIO 19   -->  MISO
3.3V      -->  VDD
GND       -->  GND
```

### Termistor NTC al ADS1220
```
NTC            ADS1220
---            -------
Terminal 1 --> AIN0
Terminal 2 --> GND (común)

ESP32/ADS1220
-------------
GND       --> AIN1 (referencia)
```

**Nota:** AIN1 se conecta a GND para establecer la referencia de medición.

### Sensor gSKIN-XI al ADS1220
```
gSKIN-XI       ADS1220
--------       -------
Rojo (+)  -->  AIN0
Negro (-) -->  AIN1
```

**⚠️ IMPORTANTE:**
- Montar el sensor con pasta térmica
- El lado positivo debe apuntar hacia el flujo de calor esperado
- Evitar burbujas de aire entre el sensor y la superficie

## 🚀 Uso Básico

### Lectura de Temperatura con NTC

```cpp
#include <SPI.h>
#include "ADS1220_NTC_Extended.h"

// Constructor con corriente IDAC
ADS1220_NTC_Extended sensor(5, 255, 50.0f);  // CS=5, DRDY=no usado, IDAC=50µA

void setup() {
    Serial.begin(115200);
    
    // Inicializar
    if (!sensor.begin(&SPI)) {
        Serial.println("Error al inicializar ADS1220");
        while(1);
    }
    
    // Configurar modo NTC
    sensor.setChannelMode(CHANNEL_0, MODE_NTC);
    
    // Configurar filtro (opcional)
    sensor.setMovingAverageSize(10);
}

void loop() {
    // Leer temperatura sin filtro
    float temp = sensor.readTemperature(CHANNEL_0);
    
    // Leer temperatura filtrada
    float tempFiltered = sensor.readTemperatureFiltered(CHANNEL_0);
    
    Serial.print("Temperatura: "); Serial.print(temp, 2);
    Serial.println(" °C");
    
    delay(1000);
}
```

### Lectura de Flujo Térmico con gSKIN

```cpp
#include <SPI.h>
#include "ADS1220_NTC_Extended.h"

// Constructor con IDAC = 0 (sin corriente, para sensores termoeléctricos)
ADS1220_NTC_Extended sensor(5, 255, 0.0f);

void setup() {
    Serial.begin(115200);
    
    if (!sensor.begin(&SPI)) {
        Serial.println("Error al inicializar ADS1220");
        while(1);
    }
    
    // Configurar modo Heat Flux
    sensor.setChannelMode(CHANNEL_0, MODE_HEAT_FLUX);
    
    // Configurar calibración del sensor (valores del certificado)
    sensor.setHeatFluxCalibration(
        CHANNEL_0,
        54.3f,      // So: Sensibilidad @ To [µV/(W/m²)]
        0.0697f,    // Sc: Coeficiente de temperatura [(µV/(W/m²))/°C]
        22.5f,      // To: Temperatura de calibración [°C]
        0.000324f   // Área del sensor [m²]
    );
    
    sensor.setMovingAverageSize(10);
}

void loop() {
    float sensorTemp = 25.0f;  // Temperatura del sensor (medir con NTC)
    
    // Leer flujo térmico con corrección de temperatura
    float heatFlux = sensor.readHeatFlux(CHANNEL_0, sensorTemp);
    Serial.print("Flujo térmico: "); Serial.print(heatFlux, 3);
    Serial.println(" W/m²");
    
    // Leer potencia térmica total
    float thermalPower = sensor.readThermalPower(CHANNEL_0, sensorTemp);
    Serial.print("Potencia térmica: "); Serial.print(thermalPower * 1000.0f, 3);
    Serial.println(" mW");
    
    delay(1000);
}
```

## 📚 Ejemplos

La librería incluye dos ejemplos completos:

### 1. Simple_NTC_Reading
Lectura básica de temperatura con termistor NTC.
- Lectura directa y filtrada
- Muestra voltaje y resistencia
- Usa coeficientes Steinhart-Hart predeterminados

[Ver ejemplo completo →](examples/Simple_NTC_Reading/)

### 2. gSKIN_HeatFlux_Sensor  
Medición de flujo térmico con sensor gSKIN-XI.
- Lectura de voltaje termoeléctrico
- Cálculo de flujo térmico y potencia
- Corrección de temperatura
- Interpretación de resultados

[Ver ejemplo completo →](examples/gSKIN_HeatFlux_Sensor/)

## 📖 API Completa

### Constructores

```cpp
// Constructor básico
ADS1220_NTC_Extended(uint8_t csPin = 5, uint8_t drdyPin = 255);

// Constructor con corriente IDAC personalizada
// Use idacCurrent_uA = 0 para sensores de flujo térmico
ADS1220_NTC_Extended(uint8_t csPin, uint8_t drdyPin, float idacCurrent_uA);
```

### Inicialización

```cpp
bool begin(SPIClass *spiInstance = &SPI);
```

### Configuración de Modo

```cpp
void setChannelMode(ADS1220_Channel channel, ADS1220_SensorMode mode);
// mode: MODE_NTC o MODE_HEAT_FLUX

ADS1220_SensorMode getChannelMode(ADS1220_Channel channel);
```

### Funciones de Lectura NTC

```cpp
int32_t readRaw(ADS1220_Channel channel);
float readVoltage(ADS1220_Channel channel);
float readResistance(ADS1220_Channel channel);
float readTemperature(ADS1220_Channel channel);
float readTemperatureFiltered(ADS1220_Channel channel);
```

### Funciones de Lectura Heat Flux

```cpp
float readHeatFluxVoltage(ADS1220_Channel channel);
float getTemperatureCorrectedSensitivity(ADS1220_Channel channel, float sensorTemp);
float readHeatFlux(ADS1220_Channel channel, float sensorTemp);
float readHeatFluxUncorrected(ADS1220_Channel channel);
float readHeatFluxFiltered(ADS1220_Channel channel, float sensorTemp);
float readThermalPower(ADS1220_Channel channel, float sensorTemp);
float readThermalPowerFiltered(ADS1220_Channel channel, float sensorTemp);
```

### Configuración del ADC

```cpp
void setGain(uint8_t gain);
// gain: ADS1220_GAIN_1, _2, _4, _8, _16, _32, _64, _128

void setDataRate(uint8_t dataRate);
// dataRate: ADS1220_DR_20SPS, _45SPS, _90SPS, _175SPS, _330SPS, _600SPS, _1000SPS

void setConversionMode(uint8_t mode);
// mode: ADS1220_CM_SINGLE o ADS1220_CM_CONTINUOUS

void setMovingAverageSize(uint8_t size);
// size: 1-20 muestras
```

### Funciones de Debug

```cpp
void printRegisters(Stream* serial = &Serial);
void printLastReading(Stream* serial = &Serial);
void printHeatFluxInfo(ADS1220_Channel channel, Stream* serial = &Serial);
bool testCommunication();
```

## 📊 Precisión y Errores

### Sensores de Flujo Térmico

#### Con Corrección de Temperatura
- **Precisión:** ±3% en rango de calibración (-30°C a 70°C)
- **Precisión fuera de rango:** ±5% a ±10%

#### Sin Corrección de Temperatura
- **@ 22.5°C (To):** ±3%
- **@ 40°C:** Error adicional ~2.2%
- **@ 60°C:** Error adicional ~5%

### Recomendaciones
1. Siempre usar corrección de temperatura si es posible
2. Medir temperatura del sensor con NTC en segundo ADS1220
3. Usar filtro de media móvil para reducir ruido
4. Evitar operación fuera de rango de calibración

## 🔍 Troubleshooting

### Error: No se pudo inicializar el ADS1220

**Causas posibles:**
- Conexiones SPI incorrectas
- Pin CS incorrecto
- Alimentación insuficiente
- Módulo ADS1220 defectuoso

**Solución:**
1. Verificar conexiones con multímetro
2. Verificar que el módulo tenga 3.3V
3. Probar con diferentes pines CS
4. Ejecutar `testCommunication()`

### Lecturas Ruidosas

**Causas posibles:**
- Montaje incorrecto del sensor
- Mala conexión térmica
- Interferencia electromagnética
- Cable del sensor muy largo

**Solución:**
1. Usar pasta térmica de calidad
2. Evitar burbujas de aire
3. Usar cable apantallado
4. Aumentar tamaño del filtro: `setMovingAverageSize(15)`

### Lecturas Incorrectas o Inestables

**Causas posibles:**
- Calibración incorrecta
- Temperatura del sensor no medida
- Sensor montado al revés

**Solución:**
1. Verificar valores de calibración del certificado
2. Implementar medición de temperatura con NTC
3. Verificar polaridad del sensor (rojo = +, negro = -)

## 🤝 Contribuir

Las contribuciones son bienvenidas. Por favor:

1. Fork el proyecto
2. Crea una rama para tu característica (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## 📝 Licencia

Este proyecto está licenciado bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para más detalles.

## ✍️ Autor

**Pedro Emigdio García González**  
Instituto de Física y Matemáticas - UMSNH  
Sistema de control PID con Peltier

## 📚 Referencias

- [ADS1220 Datasheet](https://www.ti.com/lit/ds/symlink/ads1220.pdf)
- [gSKIN Heat Flux Sensors](https://www.greenteg.com)
- [ISO 8301 Standard](https://www.iso.org/standard/15421.html)
- [Steinhart-Hart Equation](https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation)

## 📞 Soporte

Para reportar bugs o solicitar características:
- Abrir un [issue](https://github.com/YOUR_USERNAME/ADS1220_NTC_Extended/issues)
- Incluir:
  - Versión de la librería
  - Modelo de sensor usado
  - Código de ejemplo mínimo
  - Salida del Serial Monitor
  - Output de `printRegisters()` y `printLastReading()`

## 📅 Changelog

### v2.0.0 (Enero 2025)
- ✨ Añadido soporte para sensores de flujo térmico
- ✨ Nuevo modo MODE_HEAT_FLUX
- ✨ Funciones de corrección de temperatura para sensores gSKIN
- ✨ Cálculo de flujo térmico y potencia térmica
- ✨ Constructor con IDAC configurable (incluyendo 0µA)
- ✅ Compatibilidad completa con versión original (NTC)

### v1.0.0 (Original)
- ✅ Soporte para termistores NTC
- ✅ Ecuación de Steinhart-Hart
- ✅ IDAC configurable (10µA - 1500µA)
- ✅ Filtro de promedio móvil
- ✅ Compatible ESP32 y Arduino Mega

---

⭐ Si este proyecto te fue útil, considera darle una estrella en GitHub!
