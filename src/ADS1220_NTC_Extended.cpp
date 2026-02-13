/*******************************************************************************
 * Estimado Juan, has favor de leer la librería completa, conociendo su funcionamiento
 * podras entender la mayoría de los problemas en tu arreglo experimental.
 * 
 * ADS1220_NTC Extended Library - Implementation File
 * 
 * Implementación completa de funciones para medición de NTC y flujo térmico
 ******************************************************************************/

#include "ADS1220_NTC_Extended.h"

// =============================================================================
// CONSTRUCTORES
// =============================================================================

ADS1220_NTC_Extended::ADS1220_NTC_Extended(uint8_t csPin, uint8_t drdyPin) {
    _csPin = csPin;
    _drdyPin = drdyPin;
    _spi = nullptr;
    
    // Inicializar registros
    _reg0 = 0x00;
    _reg1 = 0x00;
    _reg2 = 0x00;
    _reg3 = 0x00;
    
    // Inicializar coeficientes NTC con valores por defecto
    for (int ch = 0; ch < 2; ch++) {
        _ntc_A[ch] = NTC_A_COEFF;
        _ntc_B[ch] = NTC_B_COEFF;
        _ntc_C[ch] = NTC_C_COEFF;
    }
    
    // Inicializar calibración de flujo térmico con valores por defecto
    for (int ch = 0; ch < 2; ch++) {
        _heatFluxCal[ch].So = HFS_DEFAULT_SENSITIVITY;
        _heatFluxCal[ch].Sc = HFS_DEFAULT_TEMP_COEFF;
        _heatFluxCal[ch].To = HFS_DEFAULT_CALIB_TEMP;
        _heatFluxCal[ch].sensorArea = HFS_DEFAULT_SENSOR_AREA;
    }
    
    // Modo por defecto: NTC
    _channelMode[0] = MODE_NTC;
    _channelMode[1] = MODE_NTC;
    
    // Corriente IDAC por defecto
    _idacCurrent = ADS1220_IDAC_CURRENT;
    
    // Ganancia por defecto
    _currentGain = 1;
    
    // Inicializar moving average
    _avgSize = 5;
    for (int ch = 0; ch < 2; ch++) {
        _bufferIndex[ch] = 0;
        _bufferFull[ch] = false;
        for (int i = 0; i < MAX_AVG_SIZE; i++) {
            _tempBuffer[ch][i] = 0.0f;
        }
    }
    
    // Variables de debug
    _lastRawReading = 0;
    _lastVoltage = 0.0f;
    _lastResistance = 0.0f;
    _lastTemperature = 0.0f;
    _lastHeatFlux = 0.0f;
    _lastThermalPower = 0.0f;
    _lastChannel = CHANNEL_0;
}

ADS1220_NTC_Extended::ADS1220_NTC_Extended(uint8_t csPin, uint8_t drdyPin, float idacCurrent_uA) 
    : ADS1220_NTC_Extended(csPin, drdyPin) {
    _idacCurrent = idacCurrent_uA * 1e-6f;  // µA → A
    
    // Si IDAC = 0, configurar Channel 0 en modo HEAT_FLUX por defecto
    if (idacCurrent_uA == 0.0f) {
        _channelMode[0] = MODE_HEAT_FLUX;
    }
}

// =============================================================================
// INICIALIZACIÓN
// =============================================================================

bool ADS1220_NTC_Extended::begin(SPIClass *spiInstance) {
    _spi = spiInstance;
    
    pinMode(_csPin, OUTPUT);
    csHigh();
    
    if (_drdyPin != 255) {
        pinMode(_drdyPin, INPUT);
    }
    
    _spi->begin();
    delay(100);
    
    reset();
    delay(50);
    
    if (!testCommunication()) {
        return false;
    }
    
    // Configuración de registros
    _reg0 = ADS1220_MUX_AIN0_AIN1 | ADS1220_GAIN_1 | ADS1220_PGA_ENABLED;
    writeRegister(ADS1220_REG0, _reg0);
    
    // Extraer y guardar ganancia configurada
    uint8_t gainBits = (_reg0 >> 1) & 0x07;
    switch(gainBits) {
        case 0: _currentGain = 1; break;
        case 1: _currentGain = 2; break;
        case 2: _currentGain = 4; break;
        case 3: _currentGain = 8; break;
        case 4: _currentGain = 16; break;
        case 5: _currentGain = 32; break;
        case 6: _currentGain = 64; break;
        case 7: _currentGain = 128; break;
        default: _currentGain = 1; break;
    }
    
    _reg1 = ADS1220_DR_90SPS | ADS1220_MODE_NORMAL | ADS1220_CM_CONTINUOUS;
    writeRegister(ADS1220_REG1, _reg1);
    
    uint8_t idac_setting = _getCurrentRegisterSetting(_idacCurrent * 1e6f);
    _reg2 = ADS1220_VREF_INTERNAL | ADS1220_REJECT_OFF | idac_setting;
    writeRegister(ADS1220_REG2, _reg2);
    
    if (_idacCurrent > 0.0f) {
        _reg3 = (ADS1220_IDAC_AIN0 << 5) | (ADS1220_IDAC_AIN2 << 2) | 0x00;
    } else {
        _reg3 = (ADS1220_IDAC_DISABLED << 5) | (ADS1220_IDAC_DISABLED << 2) | 0x00;
    }
    writeRegister(ADS1220_REG3, _reg3);
    
    delayMicroseconds(300);
    
    uint8_t verify_reg0 = readRegister(ADS1220_REG0);
    if (verify_reg0 != _reg0) {
        return false;
    }
    
    startConversion();
    return true;
}

// =============================================================================
// CONFIGURACIÓN DE MODO
// =============================================================================

void ADS1220_NTC_Extended::setChannelMode(ADS1220_Channel channel, ADS1220_SensorMode mode) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    _channelMode[ch] = mode;
    
    if (mode == MODE_HEAT_FLUX) {
        // Desactivar IDAC para sensores termoeléctricos
        _reg2 = (_reg2 & 0xF8) | ADS1220_IDAC_OFF;
        writeRegister(ADS1220_REG2, _reg2);
        
        if (channel == CHANNEL_0) {
            _reg3 = (_reg3 & 0x1F) | (ADS1220_IDAC_DISABLED << 5);
        } else {
            _reg3 = (_reg3 & 0xE3) | (ADS1220_IDAC_DISABLED << 2);
        }
        writeRegister(ADS1220_REG3, _reg3);
    } else {
        // Activar IDAC para NTC
        uint8_t idac_setting = _getCurrentRegisterSetting(_idacCurrent * 1e6f);
        _reg2 = (_reg2 & 0xF8) | idac_setting;
        writeRegister(ADS1220_REG2, _reg2);
        
        if (channel == CHANNEL_0) {
            _reg3 = (_reg3 & 0x1F) | (ADS1220_IDAC_AIN0 << 5);
        } else {
            _reg3 = (_reg3 & 0xE3) | (ADS1220_IDAC_AIN2 << 2);
        }
        writeRegister(ADS1220_REG3, _reg3);
    }
}

ADS1220_SensorMode ADS1220_NTC_Extended::getChannelMode(ADS1220_Channel channel) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    return _channelMode[ch];
}

// =============================================================================
// CONFIGURACIÓN NTC
// =============================================================================

void ADS1220_NTC_Extended::setNTCCoefficients(ADS1220_Channel channel, float A, float B, float C) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    _ntc_A[ch] = A;
    _ntc_B[ch] = B;
    _ntc_C[ch] = C;
}

void ADS1220_NTC_Extended::setIDACCurrent(float idacCurrent_uA) {
    _idacCurrent = idacCurrent_uA * 1e-6f;
    
    if (_spi != nullptr) {
        uint8_t idac_setting = _getCurrentRegisterSetting(idacCurrent_uA);
        _reg2 = (_reg2 & 0xF8) | idac_setting;
        writeRegister(ADS1220_REG2, _reg2);
    }
}

void ADS1220_NTC_Extended::setMeasuredIDACCurrent(float measuredCurrent_uA) {
    _idacCurrent = measuredCurrent_uA * 1e-6f;
}

void ADS1220_NTC_Extended::setMovingAverageSize(uint8_t size) {
    if (size > MAX_AVG_SIZE) size = MAX_AVG_SIZE;
    if (size < 1) size = 1;
    _avgSize = size;
    
    for (int ch = 0; ch < 2; ch++) {
        _bufferIndex[ch] = 0;
        _bufferFull[ch] = false;
    }
}

// =============================================================================
// CONFIGURACIÓN HEAT FLUX
// =============================================================================

void ADS1220_NTC_Extended::setHeatFluxCalibration(ADS1220_Channel channel, float So, float Sc, float To, float sensorArea) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    _heatFluxCal[ch].So = So;
    _heatFluxCal[ch].Sc = Sc;
    _heatFluxCal[ch].To = To;
    _heatFluxCal[ch].sensorArea = sensorArea;
}

void ADS1220_NTC_Extended::setHeatFluxSensitivity(ADS1220_Channel channel, float So) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    _heatFluxCal[ch].So = So;
}

HeatFluxCalibration ADS1220_NTC_Extended::getHeatFluxCalibration(ADS1220_Channel channel) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    return _heatFluxCal[ch];
}

// =============================================================================
// LECTURA NTC
// =============================================================================

int32_t ADS1220_NTC_Extended::readRaw(ADS1220_Channel channel) {
    configureChannel(channel);
    configureIDAC(channel);
    
    if (!waitForData(2000)) {
        return 0;
    }
    
    int32_t rawValue = readConversionData();
    
    _lastRawReading = rawValue;
    _lastChannel = channel;
    
    return rawValue;
}

float ADS1220_NTC_Extended::readVoltage(ADS1220_Channel channel) {
    int32_t raw = readRaw(channel);
    float voltage = rawToVoltage(raw);
    _lastVoltage = voltage;
    return voltage;
}

float ADS1220_NTC_Extended::readResistance(ADS1220_Channel channel) {
    float voltage = readVoltage(channel);
    float resistance = voltageToResistance(voltage);
    _lastResistance = resistance;
    return resistance;
}

float ADS1220_NTC_Extended::readTemperature(ADS1220_Channel channel) {
    float resistance = readResistance(channel);
    float temperature = resistanceToTemperature(resistance);
    _lastTemperature = temperature;
    return temperature;
}

float ADS1220_NTC_Extended::readTemperatureFiltered(ADS1220_Channel channel) {
    float temp = readTemperature(channel);
    return updateMovingAverage(channel, temp);
}

// =============================================================================
// LECTURA HEAT FLUX (NUEVO)
// =============================================================================

float ADS1220_NTC_Extended::readHeatFluxVoltage(ADS1220_Channel channel) {
    configureChannel(channel);
    
    // Asegurar que IDAC esté OFF
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    if (_channelMode[ch] == MODE_HEAT_FLUX) {
        _reg2 = (_reg2 & 0xF8) | ADS1220_IDAC_OFF;
        writeRegister(ADS1220_REG2, _reg2);
    }
    
    if (!waitForData(2000)) {
        return 0.0f;
    }
    
    int32_t rawValue = readConversionData();
    float voltage = rawToVoltage(rawValue);
    float voltage_uV = voltage * 1e6f;  // V → µV
    
    _lastRawReading = rawValue;
    _lastVoltage = voltage;
    _lastChannel = channel;
    
    return voltage_uV;
}

float ADS1220_NTC_Extended::getTemperatureCorrectedSensitivity(ADS1220_Channel channel, float sensorTemp) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    // S = So + (Ts - To) * Sc
    float deltaT = sensorTemp - _heatFluxCal[ch].To;
    float S = _heatFluxCal[ch].So + (deltaT * _heatFluxCal[ch].Sc);
    
    return S;
}

float ADS1220_NTC_Extended::readHeatFlux(ADS1220_Channel channel, float sensorTemp) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    float voltage_uV = readHeatFluxVoltage(channel);
    float S_corrected = getTemperatureCorrectedSensitivity(channel, sensorTemp);
    
    // φ = U / S  [W/m²]
    float heatFlux = voltage_uV / S_corrected;
    
    _lastHeatFlux = heatFlux;
    return heatFlux;
}

float ADS1220_NTC_Extended::readHeatFluxUncorrected(ADS1220_Channel channel) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    float voltage_uV = readHeatFluxVoltage(channel);
    
    // φ = U / So  [W/m²]  (sin corrección de temperatura)
    float heatFlux = voltage_uV / _heatFluxCal[ch].So;
    
    _lastHeatFlux = heatFlux;
    return heatFlux;
}

float ADS1220_NTC_Extended::readHeatFluxFiltered(ADS1220_Channel channel, float sensorTemp) {
    float heatFlux = readHeatFlux(channel, sensorTemp);
    return updateMovingAverage(channel, heatFlux);
}

float ADS1220_NTC_Extended::readThermalPower(ADS1220_Channel channel, float sensorTemp) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    float heatFlux = readHeatFlux(channel, sensorTemp);
    
    // Φ = φ * A  [W]
    float thermalPower = heatFlux * _heatFluxCal[ch].sensorArea;
    
    _lastThermalPower = thermalPower;
    return thermalPower;
}

float ADS1220_NTC_Extended::readThermalPowerFiltered(ADS1220_Channel channel, float sensorTemp) {
    float heatFlux = readHeatFluxFiltered(channel, sensorTemp);
    
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    float thermalPower = heatFlux * _heatFluxCal[ch].sensorArea;
    
    _lastThermalPower = thermalPower;
    return thermalPower;
}

// =============================================================================
// CONFIGURACIÓN DEL ADC
// =============================================================================

void ADS1220_NTC_Extended::setGain(uint8_t gain) {
    // Actualizar registro del hardware
    _reg0 = (_reg0 & 0xF1) | (gain & 0x0E);
    writeRegister(ADS1220_REG0, _reg0);
    
    // Guardar valor numérico de ganancia
    // Convertir bits de registro a valor numérico
    uint8_t gainBits = (gain >> 1) & 0x07;
    switch(gainBits) {
        case 0: _currentGain = 1; break;
        case 1: _currentGain = 2; break;
        case 2: _currentGain = 4; break;
        case 3: _currentGain = 8; break;
        case 4: _currentGain = 16; break;
        case 5: _currentGain = 32; break;
        case 6: _currentGain = 64; break;
        case 7: _currentGain = 128; break;
        default: _currentGain = 1; break;
    }
}

void ADS1220_NTC_Extended::setPGABypass(bool bypass) {
    // Bit 0 de Registro 0: PGA_BYPASS
    // 0 = PGA habilitado (default)
    // 1 = PGA deshabilitado (bypass)
    
    if (bypass) {
        _reg0 |= ADS1220_PGA_BYPASSED;   // Set bit 0 = 1
    } else {
        _reg0 &= ~ADS1220_PGA_BYPASSED;  // Clear bit 0 = 0
    }
    
    writeRegister(ADS1220_REG0, _reg0);
    
    // Nota: Al deshabilitar PGA, solo ganancias 1, 2, 4 están disponibles
    // Ganancias mayores requieren PGA habilitado
}

void ADS1220_NTC_Extended::setDataRate(uint8_t dataRate) {
    _reg1 = (_reg1 & 0x1F) | (dataRate & 0xE0);
    writeRegister(ADS1220_REG1, _reg1);
}

void ADS1220_NTC_Extended::setConversionMode(uint8_t mode) {
    _reg1 = (_reg1 & 0xFB) | (mode & 0x04);
    writeRegister(ADS1220_REG1, _reg1);
    startConversion();
}

// =============================================================================
// CONTROL
// =============================================================================

void ADS1220_NTC_Extended::startConversion() {
    sendCommand(ADS1220_CMD_START_SYNC);
}

bool ADS1220_NTC_Extended::dataReady() {
    if (_drdyPin != 255) {
        return (digitalRead(_drdyPin) == LOW);
    }
    return true;
}

void ADS1220_NTC_Extended::powerDown() {
    sendCommand(ADS1220_CMD_POWERDOWN);
}

void ADS1220_NTC_Extended::reset() {
    sendCommand(ADS1220_CMD_RESET);
    delay(1);
}

// =============================================================================
// DEBUG
// =============================================================================

void ADS1220_NTC_Extended::printRegisters(Stream* serial) {
    serial->println(F("=== ADS1220 Registros ==="));
    
    uint8_t reg0 = readRegister(ADS1220_REG0);
    uint8_t reg1 = readRegister(ADS1220_REG1);
    uint8_t reg2 = readRegister(ADS1220_REG2);
    uint8_t reg3 = readRegister(ADS1220_REG3);
    
    serial->print(F("Reg 0: 0x")); serial->print(reg0, HEX);
    serial->print(F(" (MUX:")); serial->print((reg0 >> 4) & 0x0F);
    serial->print(F(", GAIN:")); serial->print((reg0 >> 1) & 0x07);
    serial->print(F(", PGA_BYP:")); serial->print(reg0 & 0x01);
    serial->println(")");
    
    serial->print(F("Reg 1: 0x")); serial->print(reg1, HEX);
    serial->print(F(" (DR:")); serial->print((reg1 >> 5) & 0x07);
    serial->print(F(", MODE:")); serial->print((reg1 >> 3) & 0x03);
    serial->print(F(", CM:")); serial->print((reg1 >> 2) & 0x01);
    serial->println(")");
    
    serial->print(F("Reg 2: 0x")); serial->print(reg2, HEX);
    serial->print(F(" (VREF:")); serial->print((reg2 >> 6) & 0x03);
    serial->print(F(", IDAC:")); serial->print(reg2 & 0x07);
    serial->println(")");
    
    serial->print(F("Reg 3: 0x")); serial->print(reg3, HEX);
    serial->print(F(" (I1MUX:")); serial->print((reg3 >> 5) & 0x07);
    serial->print(F(", I2MUX:")); serial->print((reg3 >> 2) & 0x07);
    serial->println(")");
    
    serial->println();
}

void ADS1220_NTC_Extended::printLastReading(Stream* serial) {
    serial->println(F("=== Última Lectura ==="));
    serial->print(F("Canal: "));
    serial->println(_lastChannel == CHANNEL_0 ? "0 (AIN0-AIN1)" : "1 (AIN2-AIN3)");
    
    uint8_t ch = (_lastChannel == CHANNEL_0) ? 0 : 1;
    serial->print(F("Modo: "));
    serial->println(_channelMode[ch] == MODE_NTC ? "NTC" : "Heat Flux");
    
    serial->print(F("RAW: ")); serial->println(_lastRawReading);
    serial->print(F("Voltaje: ")); serial->print(_lastVoltage * 1000.0f, 4);
    serial->println(F(" mV"));
    
    if (_channelMode[ch] == MODE_NTC) {
        serial->print(F("Resistencia: ")); serial->print(_lastResistance / 1000.0f, 3);
        serial->println(F(" kΩ"));
        serial->print(F("Temperatura: ")); serial->print(_lastTemperature, 2);
        serial->println(F(" °C"));
    } else {
        serial->print(F("Flujo Térmico: ")); serial->print(_lastHeatFlux, 2);
        serial->println(F(" W/m²"));
        serial->print(F("Potencia Térmica: ")); serial->print(_lastThermalPower * 1000.0f, 3);
        serial->println(F(" mW"));
    }
    serial->println();
}

void ADS1220_NTC_Extended::printHeatFluxInfo(ADS1220_Channel channel, Stream* serial) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    serial->println(F("=== Calibración Heat Flux ==="));
    serial->print(F("Canal: ")); serial->println(ch);
    serial->print(F("So: ")); serial->print(_heatFluxCal[ch].So, 4);
    serial->println(F(" µV/(W/m²)"));
    serial->print(F("Sc: ")); serial->print(_heatFluxCal[ch].Sc, 6);
    serial->println(F(" (µV/(W/m²))/°C"));
    serial->print(F("To: ")); serial->print(_heatFluxCal[ch].To, 2);
    serial->println(F(" °C"));
    serial->print(F("Área: ")); serial->print(_heatFluxCal[ch].sensorArea * 1e6f, 2);
    serial->println(F(" mm²"));
    serial->println();
}

bool ADS1220_NTC_Extended::testCommunication() {
    uint8_t reg0 = readRegister(ADS1220_REG0);
    
    if (reg0 == 0xFF) {
        return false;
    }
    
    uint8_t originalValue = reg0;
    writeRegister(ADS1220_REG0, 0xAA);
    delayMicroseconds(100);
    
    uint8_t readback = readRegister(ADS1220_REG0);
    
    writeRegister(ADS1220_REG0, originalValue);
    delayMicroseconds(100);
    
    return (readback == 0xAA);
}

// =============================================================================
// FUNCIONES PRIVADAS SPI
// =============================================================================

void ADS1220_NTC_Extended::sendCommand(uint8_t cmd) {
    _spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    csLow();
    _spi->transfer(cmd);
    csHigh();
    _spi->endTransaction();
}

uint8_t ADS1220_NTC_Extended::readRegister(uint8_t reg) {
    _spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    csLow();
    
    _spi->transfer(ADS1220_CMD_RREG | ((reg & 0x03) << 2));
    uint8_t value = _spi->transfer(0x00);
    
    csHigh();
    _spi->endTransaction();
    
    return value;
}

void ADS1220_NTC_Extended::writeRegister(uint8_t reg, uint8_t value) {
    _spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    csLow();
    
    _spi->transfer(ADS1220_CMD_WREG | ((reg & 0x03) << 2));
    _spi->transfer(value);
    
    csHigh();
    _spi->endTransaction();
    
    delayMicroseconds(50);
}

int32_t ADS1220_NTC_Extended::readConversionData() {
    _spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    csLow();
    
    _spi->transfer(ADS1220_CMD_RDATA);
    
    uint8_t msb = _spi->transfer(0x00);
    uint8_t mid = _spi->transfer(0x00);
    uint8_t lsb = _spi->transfer(0x00);
    
    csHigh();
    _spi->endTransaction();
    
    int32_t result = ((uint32_t)msb << 16) | ((uint32_t)mid << 8) | (uint32_t)lsb;
    
    if (result & 0x00800000) {
        result |= 0xFF000000;
    }
    
    return result;
}

// =============================================================================
// FUNCIONES PRIVADAS DE CONFIGURACIÓN
// =============================================================================

void ADS1220_NTC_Extended::configureChannel(ADS1220_Channel channel) {
    uint8_t muxSetting = (channel == CHANNEL_0) ? ADS1220_MUX_AIN0_AIN1 : ADS1220_MUX_AIN2_AIN3;
    
    _reg0 = (_reg0 & 0x0F) | muxSetting;
    writeRegister(ADS1220_REG0, _reg0);
    
    delay(10);
}

void ADS1220_NTC_Extended::configureIDAC(ADS1220_Channel channel) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    if (_channelMode[ch] == MODE_NTC) {
        uint8_t idac_setting = _getCurrentRegisterSetting(_idacCurrent * 1e6f);
        _reg2 = (_reg2 & 0xF8) | idac_setting;
        writeRegister(ADS1220_REG2, _reg2);
        
        if (channel == CHANNEL_0) {
            _reg3 = (_reg3 & 0x1F) | (ADS1220_IDAC_AIN0 << 5);
        } else {
            _reg3 = (_reg3 & 0xE3) | (ADS1220_IDAC_AIN2 << 2);
        }
        writeRegister(ADS1220_REG3, _reg3);
    } else {
        _reg2 = (_reg2 & 0xF8) | ADS1220_IDAC_OFF;
        writeRegister(ADS1220_REG2, _reg2);
    }
}

uint8_t ADS1220_NTC_Extended::_getCurrentRegisterSetting(float desiredCurrent_uA) {
    if (desiredCurrent_uA < 5.0f) return ADS1220_IDAC_OFF;
    else if (desiredCurrent_uA < 30.0f) return ADS1220_IDAC_10UA;
    else if (desiredCurrent_uA < 75.0f) return ADS1220_IDAC_50UA;
    else if (desiredCurrent_uA < 175.0f) return ADS1220_IDAC_100UA;
    else if (desiredCurrent_uA < 375.0f) return ADS1220_IDAC_250UA;
    else if (desiredCurrent_uA < 750.0f) return ADS1220_IDAC_500UA;
    else if (desiredCurrent_uA < 1250.0f) return ADS1220_IDAC_1000UA;
    else return ADS1220_IDAC_1500UA;
}

// =============================================================================
// CÁLCULOS NTC
// =============================================================================

float ADS1220_NTC_Extended::rawToVoltage(int32_t rawValue) {
    // Fórmula correcta del datasheet: VIN = (Code × VREF) / (Gain × 2^23)
    // Donde 2^23 = 8,388,608 (usamos ADS1220_MAX_CODE = 8,388,607)
    float numerator = (float)rawValue * ADS1220_VREF;
    float denominator = (float)_currentGain * (float)ADS1220_MAX_CODE;
    return numerator / denominator;
}

float ADS1220_NTC_Extended::voltageToResistance(float voltage) {
    if (voltage <= 0.0f || _idacCurrent <= 0.0f) {
        return 0.0f;
    }
    return voltage / _idacCurrent;
}

float ADS1220_NTC_Extended::resistanceToTemperature(float resistance) {
    if (resistance <= 0.0f) {
        return -999.0f;
    }
    
    uint8_t ch = (_lastChannel == CHANNEL_0) ? 0 : 1;
    
    float lnR = log(resistance);
    float lnR3 = lnR * lnR * lnR;
    
    float invT = _ntc_A[ch] + (_ntc_B[ch] * lnR) + (_ntc_C[ch] * lnR3);
    float tempKelvin = 1.0f / invT;
    float tempCelsius = tempKelvin - 273.15f;
    
    return tempCelsius;
}

// =============================================================================
// CÁLCULOS HEAT FLUX
// =============================================================================

float ADS1220_NTC_Extended::voltageToHeatFlux(float voltage_uV, float sensitivity) {
    return voltage_uV / sensitivity;
}

float ADS1220_NTC_Extended::heatFluxToThermalPower(float heatFlux, float sensorArea) {
    return heatFlux * sensorArea;
}

// =============================================================================
// AUXILIARES
// =============================================================================

float ADS1220_NTC_Extended::updateMovingAverage(ADS1220_Channel channel, float newValue) {
    uint8_t ch = (channel == CHANNEL_0) ? 0 : 1;
    
    _tempBuffer[ch][_bufferIndex[ch]] = newValue;
    _bufferIndex[ch]++;
    
    if (_bufferIndex[ch] >= _avgSize) {
        _bufferIndex[ch] = 0;
        _bufferFull[ch] = true;
    }
    
    float sum = 0.0f;
    uint8_t count = _bufferFull[ch] ? _avgSize : _bufferIndex[ch];
    
    for (uint8_t i = 0; i < count; i++) {
        sum += _tempBuffer[ch][i];
    }
    
    return sum / (float)count;
}

void ADS1220_NTC_Extended::csLow() {
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
}

void ADS1220_NTC_Extended::csHigh() {
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
}

bool ADS1220_NTC_Extended::waitForData(uint16_t timeout_ms) {
    if (_drdyPin != 255) {
        uint32_t startTime = millis();
        while (digitalRead(_drdyPin) == HIGH) {
            if (millis() - startTime > timeout_ms) {
                return false;
            }
            delay(1);
        }
        return true;
    }
    
    delay(15);  // Para 90 SPS: ~11ms por conversión
    return true;
}















/* Juan, si leiste esto, mandame un msj con un emoji de ====calavera===*/