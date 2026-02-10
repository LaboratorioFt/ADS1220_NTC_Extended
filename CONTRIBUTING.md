# Contribuyendo a ADS1220_NTC_Extended

¡Gracias por tu interés en contribuir! Este documento proporciona pautas para contribuir a este proyecto.

## 🤝 Cómo Contribuir

### Reportar Bugs

Si encuentras un bug, por favor abre un [issue](https://github.com/YOUR_USERNAME/ADS1220_NTC_Extended/issues) con:

1. **Título descriptivo**
2. **Descripción del problema:**
   - Comportamiento esperado
   - Comportamiento actual
   - Pasos para reproducir
3. **Información del sistema:**
   - Versión de la librería
   - Microcontrolador (ESP32, Arduino Mega, etc.)
   - Versión de Arduino IDE
   - Sensor usado (NTC modelo, gSKIN modelo)
4. **Código de ejemplo mínimo** que reproduce el problema
5. **Salida del Serial Monitor**
6. **Output de `printRegisters()` y `printLastReading()`** (si aplica)

### Solicitar Nuevas Características

Para solicitar una nueva característica:

1. Abre un [issue](https://github.com/YOUR_USERNAME/ADS1220_NTC_Extended/issues)
2. Usa el prefijo `[Feature Request]` en el título
3. Describe:
   - La característica que deseas
   - Por qué sería útil
   - Casos de uso específicos
   - Implementación propuesta (opcional)

### Contribuir Código

#### Fork y Pull Request

1. **Fork el repositorio**
   - Click en "Fork" en GitHub

2. **Clonar tu fork**
   ```bash
   git clone https://github.com/TU_USUARIO/ADS1220_NTC_Extended.git
   cd ADS1220_NTC_Extended
   ```

3. **Crear una rama para tu característica**
   ```bash
   git checkout -b feature/nombre-descriptivo
   ```
   
   Nomenclatura de ramas:
   - `feature/` - Nuevas características
   - `bugfix/` - Corrección de bugs
   - `docs/` - Mejoras de documentación
   - `refactor/` - Refactorización de código

4. **Hacer tus cambios**
   - Escribe código limpio y comentado
   - Sigue el estilo de código existente
   - Agrega documentación si es necesario

5. **Probar tus cambios**
   - Compila el código
   - Prueba en hardware real si es posible
   - Verifica que los ejemplos existentes sigan funcionando

6. **Commit tus cambios**
   ```bash
   git add .
   git commit -m "Add: Descripción clara del cambio"
   ```
   
   Formato de mensajes de commit:
   - `Add:` - Nueva característica
   - `Fix:` - Corrección de bug
   - `Docs:` - Cambios en documentación
   - `Refactor:` - Refactorización
   - `Test:` - Agregar pruebas
   - `Style:` - Cambios de formato

7. **Push a tu fork**
   ```bash
   git push origin feature/nombre-descriptivo
   ```

8. **Crear Pull Request**
   - Ve a tu fork en GitHub
   - Click "Compare & pull request"
   - Describe tus cambios detalladamente
   - Referencia issues relacionados (#número)

## 📝 Guías de Estilo

### Código C++

```cpp
// ✅ CORRECTO
void readTemperature(ADS1220_Channel channel) {
    float voltage = readVoltage(channel);  // Variable descriptiva
    float resistance = voltageToResistance(voltage);
    
    // Comentario explicativo de lógica compleja
    if (resistance > 0.0f) {
        return resistanceToTemperature(resistance);
    }
    return NAN;
}

// ❌ INCORRECTO
void rdTemp(ADS1220_Channel ch) {
    float v = rdVolt(ch);  // Nombres no descriptivos
    float r = v2r(v);
    if(r>0.0f)return r2t(r);  // Sin espacios, difícil de leer
    return NAN;
}
```

### Convenciones

1. **Nombres:**
   - Funciones: `camelCase` (ej: `readTemperature`)
   - Variables: `camelCase` (ej: `sensorTemp`)
   - Constantes: `UPPER_CASE` (ej: `MAX_BUFFER_SIZE`)
   - Clases: `PascalCase` (ej: `ADS1220_NTC_Extended`)

2. **Comentarios:**
   - Usa `//` para comentarios de una línea
   - Usa `/* */` para bloques de comentarios
   - Documenta funciones públicas con Doxygen
   ```cpp
   /**
    * @brief Descripción breve de la función
    * @param channel Canal a leer
    * @return Temperatura en °C
    */
   float readTemperature(ADS1220_Channel channel);
   ```

3. **Espaciado:**
   - Indentación: 4 espacios (no tabs)
   - Espacios alrededor de operadores: `a + b` no `a+b`
   - Una línea vacía entre funciones

4. **Llaves:**
   ```cpp
   // Estilo K&R (usado en este proyecto)
   if (condition) {
       // código
   } else {
       // código
   }
   ```

### Documentación

1. **README.md:**
   - Actualizar si agregas nuevas características
   - Mantener ejemplos actualizados
   - Agregar a la sección de API si creas nuevas funciones

2. **Comentarios en código:**
   - Explica el "por qué", no el "qué"
   - Documenta algoritmos complejos
   - Cita fuentes para ecuaciones

3. **Ejemplos:**
   - Cada ejemplo debe tener su propio README
   - Incluir diagrama de conexión si es necesario
   - Código comentado paso a paso

## 🧪 Pruebas

Antes de enviar un Pull Request:

1. **Compilación:**
   ```
   ✓ Compila sin errores
   ✓ Compila sin warnings
   ✓ Compila en ESP32
   ✓ Compila en Arduino Mega (si es posible)
   ```

2. **Funcionalidad:**
   ```
   ✓ Función nueva probada en hardware
   ✓ Ejemplos existentes siguen funcionando
   ✓ No rompe compatibilidad con versiones anteriores
   ```

3. **Documentación:**
   ```
   ✓ README actualizado
   ✓ Comentarios de código agregados
   ✓ keywords.txt actualizado (si hay nuevas funciones)
   ```

## 🎯 Áreas que Necesitan Ayuda

Contribuciones especialmente bienvenidas en:

- ✨ Soporte para nuevos sensores de flujo térmico
- ✨ Soporte para termopares (en desarrollo)
- 🐛 Correcciones de bugs reportados
- 📚 Mejoras en documentación
- 🧪 Tests y validación en diferentes hardware
- 🌐 Traducciones a otros idiomas
- 📊 Ejemplos adicionales

## ⚖️ Licencia

Al contribuir, aceptas que tus contribuciones serán licenciadas bajo la misma licencia MIT del proyecto.

## 💬 Preguntas

Si tienes preguntas sobre cómo contribuir:
- Abre un [issue](https://github.com/YOUR_USERNAME/ADS1220_NTC_Extended/issues) con el tag `question`
- Contacta al mantenedor del proyecto

## 🙏 Reconocimientos

Todos los contribuidores serán reconocidos en el README del proyecto.

---

¡Gracias por ayudar a mejorar ADS1220_NTC_Extended! 🎉
