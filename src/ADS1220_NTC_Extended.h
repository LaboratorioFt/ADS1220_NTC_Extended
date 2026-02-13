/*******************************************************************************
 * ADS1220_NTC Extended Library - Header File
 * 
 * Librería extendida para medición diferencial de NTC Y SENSORES DE FLUJO TÉRMICO
 * con ADS1220 e inyección de corriente IDAC configurable
 * 
 * Características:
 * - Medición diferencial en dos canales (AIN0-AIN1 y AIN2-AIN3)
 * - Soporte para NTC con ecuación Steinhart-Hart
 * - Soporte para sensores de flujo térmico tipo gSKIN (termoeléctricos)
 * - Inyección de corriente IDAC configurable (0 µA a 1500 µA)
 * - Filtro de promedio móvil configurable para ambos tipos de sensores
 * - Compatible con ESP32 y Arduino Mega
 * 
 * Autor: Sistema de control PID con Peltier
 * Fecha: Enero 2025 - Versión Extendida
 * Versión: 2.0.0
 * 
 * Nuevas características v2.0:
 * - Lectura de flujo térmico (W/m²) con sensores gSKIN
 * - Cálculo de potencia térmica (W)
 * - Calibración de sensibilidad dependiente de temperatura
 * - Modo de operación configurable (NTC o Heat Flux)
 * - Soporte para IDAC = 0 (sin corriente de excitación)
 ******************************************************************************/

#ifndef ADS1220_NTC_EXTENDED_H
#define ADS1220_NTC_EXTENDED_H

#include <Arduino.h>
#include <SPI.h>

// ===== COMANDOS SPI DEL ADS1220 =====
#define ADS1220_CMD_RESET       0x06
#define ADS1220_CMD_START_SYNC  0x08
#define ADS1220_CMD_POWERDOWN   0x02
#define ADS1220_CMD_RDATA       0x10
#define ADS1220_CMD_RREG        0x20
#define ADS1220_CMD_WREG        0x40

// ===== DIRECCIONES DE REGISTROS =====
#define ADS1220_REG0            0x00
#define ADS1220_REG1            0x01
#define ADS1220_REG2            0x02
#define ADS1220_REG3            0x03

// ===== CONSTANTES DE CONFIGURACIÓN =====
// Configuración de MUX (Registro 0, bits 7:4)
#define ADS1220_MUX_AIN0_AIN1   0x00
#define ADS1220_MUX_AIN0_AIN2   0x10
#define ADS1220_MUX_AIN0_AIN3   0x20
#define ADS1220_MUX_AIN1_AIN2   0x30
#define ADS1220_MUX_AIN1_AIN3   0x40
#define ADS1220_MUX_AIN2_AIN3   0x50
#define ADS1220_MUX_AIN1_AIN0   0x60
#define ADS1220_MUX_AIN3_AIN2   0x70

// Configuración de GANANCIA (Registro 0, bits 3:1)
#define ADS1220_GAIN_1          0x00
#define ADS1220_GAIN_2          0x02
#define ADS1220_GAIN_4          0x04
#define ADS1220_GAIN_8          0x06
#define ADS1220_GAIN_16         0x08
#define ADS1220_GAIN_32         0x0A
#define ADS1220_GAIN_64         0x0C
#define ADS1220_GAIN_128        0x0E

// PGA Bypass (Registro 0, bit 0)
#define ADS1220_PGA_ENABLED     0x00
#define ADS1220_PGA_BYPASSED    0x01

// Data Rate (Registro 1, bits 7:5)
#define ADS1220_DR_20SPS        0x00
#define ADS1220_DR_45SPS        0x20
#define ADS1220_DR_90SPS        0x40
#define ADS1220_DR_175SPS       0x60
#define ADS1220_DR_330SPS       0x80
#define ADS1220_DR_600SPS       0xA0
#define ADS1220_DR_1000SPS      0xC0

// Modo de operación (Registro 1, bits 4:3)
#define ADS1220_MODE_NORMAL     0x00
#define ADS1220_MODE_DUTY       0x08
#define ADS1220_MODE_TURBO      0x10

// Modo de conversión (Registro 1, bit 2)
#define ADS1220_CM_SINGLE       0x00
#define ADS1220_CM_CONTINUOUS   0x04

// Referencia de voltaje (Registro 2, bits 7:6)
#define ADS1220_VREF_INTERNAL   0x00
#define ADS1220_VREF_REFP0      0x40
#define ADS1220_VREF_REFP1      0x80
#define ADS1220_VREF_AVDD       0xC0

// Filtro 50/60Hz (Registro 2, bits 5:4)
#define ADS1220_REJECT_OFF      0x00
#define ADS1220_REJECT_BOTH     0x10
#define ADS1220_REJECT_50HZ     0x20
#define ADS1220_REJECT_60HZ     0x30

// Corriente IDAC (Registro 2, bits 2:0)
#define ADS1220_IDAC_OFF        0x00
#define ADS1220_IDAC_10UA       0x01
#define ADS1220_IDAC_50UA       0x02
#define ADS1220_IDAC_100UA      0x03
#define ADS1220_IDAC_250UA      0x04
#define ADS1220_IDAC_500UA      0x05
#define ADS1220_IDAC_1000UA     0x06
#define ADS1220_IDAC_1500UA     0x07

// Routing IDAC (Registro 3, bits 7:5 y 4:2)
#define ADS1220_IDAC_DISABLED   0x00
#define ADS1220_IDAC_AIN0       0x01
#define ADS1220_IDAC_AIN1       0x02
#define ADS1220_IDAC_AIN2       0x03
#define ADS1220_IDAC_AIN3       0x04
#define ADS1220_IDAC_REFP0      0x05
#define ADS1220_IDAC_REFN0      0x06

// ===== CONSTANTES FÍSICAS =====
#define ADS1220_VREF            2.048f  // Solo hemos usado la referencia interna de modulo
#define ADS1220_MAX_CODE        8388607
#define ADS1220_IDAC_CURRENT    10e-6f  // Corriente por defecto para NTC

// ===== PARÁMETROS NTC POR DEFECTO =====
#define NTC_A_COEFF             1.025084947e-03f
#define NTC_B_COEFF             2.398038697e-04f
#define NTC_C_COEFF             1.540123584e-07f
#define NTC_T0_KELVIN           298.15f // A 25°C

// ===== PARÁMETROS SENSORES DE FLUJO TÉRMICO POR DEFECTO =====
// Basados en gSKIN-XI 27 9C (calibración específica del usuario)
#define HFS_DEFAULT_SENSITIVITY     54.3f       // µV/(W/m²) @ To (valor calibrado)
#define HFS_DEFAULT_TEMP_COEFF      0.0697f     // (µV/(W/m²))/°C (valor calibrado)
#define HFS_DEFAULT_CALIB_TEMP      22.5f       // °C
#define HFS_DEFAULT_SENSOR_AREA     0.000324f   // m² (18mm x 18mm para XI 27 9C)

// ===== ENUMERACIONES =====
enum ADS1220_Channel {
    CHANNEL_0 = 0,  // Canal diferencial AIN0-AIN1
    CHANNEL_1 = 1   // Canal diferencial AIN2-AIN3
};

enum ADS1220_SensorMode {
    MODE_NTC = 0,           // Modo NTC con IDAC activo
    MODE_HEAT_FLUX = 1      // Modo sensor de flujo térmico (IDAC OFF, lectura directa de voltaje)
};

// ===== ESTRUCTURAS =====
struct HeatFluxCalibration {
    float So;           // Sensibilidad @ temperatura de calibración [µV/(W/m²)]
    float Sc;           // Coeficiente de corrección por temperatura [(µV/(W/m²))/°C]
    float To;           // Temperatura de calibración [°C]
    float sensorArea;   // Área del sensor [m²]
};

// ===== CLASE PRINCIPAL =====
class ADS1220_NTC_Extended {
public:
    // ===== CONSTRUCTORES =====
    /**
     * @brief Constructor básico
     * @param csPin Pin CS del SPI
     * @param drdyPin Pin Data Ready (opcional, 255 si no se usa)
     */
    ADS1220_NTC_Extended(uint8_t csPin = 5, uint8_t drdyPin = 255);
    
    /**
     * @brief Constructor con corriente IDAC personalizada
     * @param csPin Pin CS del SPI
     * @param drdyPin Pin Data Ready (opcional, 255 si no se usa)
     * @param idacCurrent_uA Corriente IDAC en µA (0 = OFF, útil para sensores de flujo)
     */
    ADS1220_NTC_Extended(uint8_t csPin, uint8_t drdyPin, float idacCurrent_uA);
    
    // ===== INICIALIZACIÓN =====
    /**
     * @brief Inicializa el ADS1220 y configura el modo de operación
     * @param spiInstance Puntero a la instancia SPI
     * @return true si la inicialización fue exitosa
     */
    bool begin(SPIClass *spiInstance = &SPI);
    
    // ===== CONFIGURACIÓN DE MODO DE OPERACIÓN =====
    /**
     * @brief Configura el modo de operación de un canal
     * @param channel Canal a configurar (CHANNEL_0 o CHANNEL_1)
     * @param mode Modo de operación (MODE_NTC o MODE_HEAT_FLUX)
     */
    void setChannelMode(ADS1220_Channel channel, ADS1220_SensorMode mode);
    
    /**
     * @brief Obtiene el modo de operación actual de un canal
     * @param channel Canal a consultar
     * @return Modo de operación actual
     */
    ADS1220_SensorMode getChannelMode(ADS1220_Channel channel);
    
    // ===== CONFIGURACIÓN NTC (modo existente) =====
    void setNTCCoefficients(ADS1220_Channel channel, float A, float B, float C);
    void setIDACCurrent(float idacCurrent_uA);
    void setMeasuredIDACCurrent(float measuredCurrent_uA);
    void setMovingAverageSize(uint8_t size);
    
    // ===== CONFIGURACIÓN SENSORES DE FLUJO TÉRMICO (NUEVO) =====
    
    /**
     * @brief Configura los parámetros de calibración del sensor de flujo térmico
     * @param channel Canal del sensor (CHANNEL_0 o CHANNEL_1)
     * @param So Sensibilidad @ temperatura de calibración [µV/(W/m²)]
     * @param Sc Coeficiente de corrección por temperatura [(µV/(W/m²))/°C]
     * @param To Temperatura de calibración [°C]
     * @param sensorArea Área del sensor [m²]
     * 
     * Ejemplo para gSKIN-XI 27 9C (según certificado de calibración):
     *   So = 54.3 µV/(W/m²)
     *   Sc = 0.0697 (µV/(W/m²))/°C
     *   To = 22.5 °C
     *   sensorArea = 0.000324 m² (18mm x 18mm)
     */
    void setHeatFluxCalibration(ADS1220_Channel channel, float So, float Sc, float To, float sensorArea);
    
    /**
     * @brief Configura solo la sensibilidad (asume valores por defecto para otros parámetros)
     * @param channel Canal del sensor
     * @param So Sensibilidad @ temperatura de calibración [µV/(W/m²)]
     */
    void setHeatFluxSensitivity(ADS1220_Channel channel, float So);
    
    /**
     * @brief Obtiene la calibración actual del sensor de flujo térmico
     * @param channel Canal del sensor
     * @return Estructura con parámetros de calibración
     */
    HeatFluxCalibration getHeatFluxCalibration(ADS1220_Channel channel);
    
    // ===== FUNCIONES DE LECTURA NTC (modo existente) =====
    int32_t readRaw(ADS1220_Channel channel);
    float readVoltage(ADS1220_Channel channel);
    float readResistance(ADS1220_Channel channel);
    float readTemperature(ADS1220_Channel channel);
    float readTemperatureFiltered(ADS1220_Channel channel);
    
    // ===== FUNCIONES DE LECTURA DE FLUJO TÉRMICO (NUEVO) =====
    
    /**
     * @brief Lee el voltaje del sensor de flujo térmico
     * @param channel Canal del sensor
     * @return Voltaje en µV
     * 
     * Nota: Esta función es similar a readVoltage() pero está optimizada
     * para sensores de flujo térmico (no requiere IDAC)
     */
    float readHeatFluxVoltage(ADS1220_Channel channel);
    
    /**
     * @brief Calcula la sensibilidad corregida por temperatura
     * @param channel Canal del sensor
     * @param sensorTemp Temperatura del sensor [°C]
     * @return Sensibilidad corregida S [µV/(W/m²)]
     * 
     * Fórmula: S = So + (Ts - To) * Sc
     * donde:
     *   So = sensibilidad @ temperatura de calibración
     *   Ts = temperatura del sensor
     *   To = temperatura de calibración
     *   Sc = coeficiente de corrección por temperatura
     */
    float getTemperatureCorrectedSensitivity(ADS1220_Channel channel, float sensorTemp);
    
    /**
     * @brief Lee el flujo térmico con corrección de temperatura
     * @param channel Canal del sensor
     * @param sensorTemp Temperatura del sensor [°C]
     * @return Flujo térmico φ [W/m²]
     * 
     * Fórmula: φ = U / S
     * donde:
     *   U = voltaje del sensor [µV]
     *   S = sensibilidad corregida por temperatura [µV/(W/m²)]
     */
    float readHeatFlux(ADS1220_Channel channel, float sensorTemp);
    
    /**
     * @brief Lee el flujo térmico sin corrección de temperatura
     * @param channel Canal del sensor
     * @return Flujo térmico φ [W/m²]
     * 
     * Usa la sensibilidad @ temperatura de calibración (So)
     * Precisión reducida fuera de temperatura de calibración
     */
    float readHeatFluxUncorrected(ADS1220_Channel channel);
    
    /**
     * @brief Lee el flujo térmico con corrección de temperatura y filtro de media móvil
     * @param channel Canal del sensor
     * @param sensorTemp Temperatura del sensor [°C]
     * @return Flujo térmico filtrado φ [W/m²]
     */
    float readHeatFluxFiltered(ADS1220_Channel channel, float sensorTemp);
    
    /**
     * @brief Lee la potencia térmica total a través del sensor
     * @param channel Canal del sensor
     * @param sensorTemp Temperatura del sensor [°C]
     * @return Potencia térmica Φ [W]
     * 
     * Fórmula: Φ = A * φ = A * U / S
     * donde:
     *   A = área del sensor [m²]
     *   φ = flujo térmico [W/m²]
     */
    float readThermalPower(ADS1220_Channel channel, float sensorTemp);
    
    /**
     * @brief Lee la potencia térmica con filtro de media móvil
     * @param channel Canal del sensor
     * @param sensorTemp Temperatura del sensor [°C]
     * @return Potencia térmica filtrada Φ [W]
     */
    float readThermalPowerFiltered(ADS1220_Channel channel, float sensorTemp);
    
    // ===== FUNCIONES DE CONFIGURACIÓN DEL ADC =====
    void setGain(uint8_t gain);
    void setPGABypass(bool bypass);
    void setDataRate(uint8_t dataRate);
    void setConversionMode(uint8_t mode);
    
    // ===== FUNCIONES DE CONTROL =====
    void startConversion();
    bool dataReady();
    void powerDown();
    void reset();
    
    // ===== FUNCIONES DE DEBUG =====
    void printRegisters(Stream* serial = &Serial);
    void printLastReading(Stream* serial = &Serial);
    void printHeatFluxInfo(ADS1220_Channel channel, Stream* serial = &Serial);
    bool testCommunication();

private:
    // ===== VARIABLES PRIVADAS =====
    SPIClass *_spi;
    uint8_t _csPin;
    uint8_t _drdyPin;
    
    // Registros de configuración (shadow copy)
    uint8_t _reg0, _reg1, _reg2, _reg3;
    
    // Modo de operación por canal
    ADS1220_SensorMode _channelMode[2];  // [0]=Canal 0, [1]=Canal 1
    
    // Parámetros NTC (Steinhart-Hart) por canal
    float _ntc_A[2], _ntc_B[2], _ntc_C[2];
    
    // Parámetros de calibración de flujo térmico por canal
    HeatFluxCalibration _heatFluxCal[2];
    
    // Corriente IDAC real (en amperes)
    float _idacCurrent;
    
    // Ganancia actual del PGA (valor numérico: 1, 2, 4, 8, 16, 32, 64, 128)
    uint8_t _currentGain;
    
    // Buffer para moving average (usado tanto para NTC como para Heat Flux)
    static const uint8_t MAX_AVG_SIZE = 20;
    float _tempBuffer[2][MAX_AVG_SIZE];  // Buffer genérico para ambos modos
    uint8_t _avgSize;
    uint8_t _bufferIndex[2];
    bool _bufferFull[2];
    
    // Variables de debug
    int32_t _lastRawReading;
    float _lastVoltage;
    float _lastResistance;       // Para modo NTC
    float _lastTemperature;      // Para modo NTC
    float _lastHeatFlux;         // Para modo Heat Flux
    float _lastThermalPower;     // Para modo Heat Flux
    ADS1220_Channel _lastChannel;
    
    // ===== FUNCIONES PRIVADAS SPI =====
    void sendCommand(uint8_t cmd);
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    int32_t readConversionData();
    
    // ===== FUNCIONES PRIVADAS DE CONFIGURACIÓN =====
    void configureChannel(ADS1220_Channel channel);
    void configureIDAC(ADS1220_Channel channel);
    uint8_t _getCurrentRegisterSetting(float desiredCurrent_uA);
    
    // ===== FUNCIONES PRIVADAS DE CÁLCULO NTC =====
    float rawToVoltage(int32_t rawValue);
    float voltageToResistance(float voltage);
    float resistanceToTemperature(float resistance);
    
    // ===== FUNCIONES PRIVADAS DE CÁLCULO HEAT FLUX =====
    float voltageToHeatFlux(float voltage_uV, float sensitivity);
    float heatFluxToThermalPower(float heatFlux, float sensorArea);
    
    // ===== FUNCIONES AUXILIARES =====
    float updateMovingAverage(ADS1220_Channel channel, float newValue);
    void csLow();
    void csHigh();
    bool waitForData(uint16_t timeout_ms = 1000);
    void _init_pwm_arduino();
    void _init_pwm_esp32();
};

#endif // ADS1220_NTC_EXTENDED_H
