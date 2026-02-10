# Guía para Subir la Librería a GitHub

Esta guía te ayudará paso a paso a crear y gestionar tu repositorio en GitHub.

## 📋 Tabla de Contenidos
1. [Requisitos Previos](#requisitos-previos)
2. [Crear Cuenta en GitHub](#crear-cuenta-en-github)
3. [Crear el Repositorio](#crear-el-repositorio)
4. [Subir los Archivos](#subir-los-archivos)
5. [Mantener el Repositorio](#mantener-el-repositorio)
6. [Colaboración](#colaboración)

## 1. Requisitos Previos

### Instalar Git
**Windows:**
- Descarga Git desde: https://git-scm.com/download/win
- Ejecuta el instalador y sigue las instrucciones

**Mac:**
- Abre Terminal y ejecuta:
```bash
git --version
```
- Si no está instalado, macOS te pedirá instalarlo automáticamente

**Linux:**
```bash
sudo apt-get install git  # Ubuntu/Debian
```

### Configurar Git (primera vez)
Abre la terminal/command prompt y ejecuta:
```bash
git config --global user.name "Tu Nombre"
git config --global user.email "tu.email@ejemplo.com"
```

## 2. Crear Cuenta en GitHub

1. Ve a https://github.com
2. Click en "Sign up" (Registrarse)
3. Completa el formulario:
   - Username (usuario único)
   - Email
   - Contraseña
4. Verifica tu email
5. Completa tu perfil (opcional pero recomendado)

## 3. Crear el Repositorio

### Opción A: Desde la Interfaz Web (Más fácil)

1. **Inicia sesión en GitHub**

2. **Crear nuevo repositorio:**
   - Click en el botón "+" en la esquina superior derecha
   - Selecciona "New repository"

3. **Configurar el repositorio:**
   ```
   Repository name: ADS1220_NTC_Extended
   Description: Extended library for ADS1220 supporting NTC thermistors and heat flux sensors
   
   ☑️ Public (recomendado para librerías)
   ☐ Add a README file (NO marcar, ya lo tenemos)
   ☐ Add .gitignore (NO marcar, ya lo tenemos)
   ☑️ Choose a license: MIT License
   ```

4. **Click en "Create repository"**

### Opción B: Desde la Línea de Comandos

Después de crear el repositorio vacío en GitHub, continúa con la siguiente sección.

## 4. Subir los Archivos

### Paso 1: Preparar tu repositorio local

1. **Navega a la carpeta de tu librería:**
```bash
cd ruta/a/ADS1220_NTC_Extended
```

2. **Inicializar Git (si no está inicializado):**
```bash
git init
```

3. **Agregar todos los archivos:**
```bash
git add .
```

4. **Verificar qué archivos se agregarán:**
```bash
git status
```

Deberías ver:
```
On branch main

Changes to be committed:
  new file:   .gitignore
  new file:   LICENSE
  new file:   README.md
  new file:   keywords.txt
  new file:   library.properties
  new file:   docs/images/connection_gskin.png
  new file:   docs/images/connection_ntc.png
  new file:   examples/Simple_NTC_Reading/README.md
  new file:   examples/Simple_NTC_Reading/Simple_NTC_Reading.ino
  new file:   examples/gSKIN_HeatFlux_Sensor/README.md
  new file:   examples/gSKIN_HeatFlux_Sensor/gSKIN_HeatFlux_Sensor.ino
  new file:   src/ADS1220_NTC_Extended.cpp
  new file:   src/ADS1220_NTC_Extended.h
```

5. **Hacer el primer commit:**
```bash
git commit -m "Initial commit: v2.0.0 - Add NTC and Heat Flux sensor support"
```

### Paso 2: Conectar con GitHub

1. **Agregar el repositorio remoto:**
```bash
git remote add origin https://github.com/TU_USUARIO/ADS1220_NTC_Extended.git
```

Reemplaza `TU_USUARIO` con tu nombre de usuario de GitHub.

2. **Verificar la conexión:**
```bash
git remote -v
```

Deberías ver:
```
origin  https://github.com/TU_USUARIO/ADS1220_NTC_Extended.git (fetch)
origin  https://github.com/TU_USUARIO/ADS1220_NTC_Extended.git (push)
```

### Paso 3: Subir los archivos

1. **Renombrar la rama principal (si es necesario):**
```bash
git branch -M main
```

2. **Subir los archivos:**
```bash
git push -u origin main
```

3. **Autenticación:**
   - GitHub te pedirá tu usuario y contraseña
   - **IMPORTANTE:** Para la contraseña, debes usar un Personal Access Token (PAT)

### Crear Personal Access Token (PAT)

Si es tu primera vez subiendo código:

1. Ve a GitHub → Settings (Configuración)
2. Developer settings → Personal access tokens → Tokens (classic)
3. Click "Generate new token (classic)"
4. Configuración:
   ```
   Note: Git access for ADS1220_NTC_Extended
   Expiration: 90 days (o más)
   Scopes: ☑️ repo (marca todos los repo)
   ```
5. Click "Generate token"
6. **IMPORTANTE:** Copia el token y guárdalo en un lugar seguro
7. Usa este token como contraseña cuando Git te lo pida

### Verificar

1. Ve a tu repositorio en GitHub:
   ```
   https://github.com/TU_USUARIO/ADS1220_NTC_Extended
   ```

2. Deberías ver todos tus archivos listados

## 5. Mantener el Repositorio

### Hacer Cambios y Actualizaciones

Cuando modifiques archivos:

```bash
# 1. Ver qué archivos cambiaron
git status

# 2. Agregar archivos modificados
git add archivo_modificado.cpp
# O agregar todos los cambios:
git add .

# 3. Hacer commit con mensaje descriptivo
git commit -m "Fix: Corregir cálculo de temperatura en modo NTC"

# 4. Subir cambios a GitHub
git push
```

### Crear Versiones (Tags/Releases)

Para versiones importantes:

```bash
# 1. Crear tag
git tag -a v2.0.1 -m "Version 2.0.1: Minor bug fixes"

# 2. Subir tag a GitHub
git push origin v2.0.1

# 3. Crear Release en GitHub (interfaz web)
```

En GitHub:
1. Ve a tu repositorio
2. Click en "Releases" → "Create a new release"
3. Selecciona el tag v2.0.1
4. Agrega título y descripción de cambios
5. Puedes adjuntar archivos ZIP si quieres
6. Click "Publish release"

### Estructura de Mensajes de Commit

Usa mensajes claros y descriptivos:

```bash
# Nuevas características
git commit -m "Add: Soporte para sensor XYZ"

# Correcciones de bugs
git commit -m "Fix: Resolver problema de lectura en Canal 1"

# Mejoras de documentación
git commit -m "Docs: Actualizar ejemplo de calibración"

# Cambios de rendimiento
git commit -m "Perf: Optimizar filtro de media móvil"

# Refactorización
git commit -m "Refactor: Simplificar función de lectura"
```

## 6. Colaboración

### Recibir Contribuciones

1. **Pull Requests:**
   - Otros usuarios pueden hacer "fork" de tu repo
   - Hacer cambios en su fork
   - Enviar un "Pull Request" a tu repo
   - Tú revisas y aceptas/rechazas

2. **Issues (Reportes de bugs):**
   - Los usuarios pueden reportar problemas
   - Ir a: https://github.com/TU_USUARIO/ADS1220_NTC_Extended/issues

### Proteger la Rama Main

Configuración recomendada:

1. Ve a Settings → Branches
2. Add rule para `main`
3. Configuración sugerida:
   ```
   ☑️ Require pull request reviews before merging
   ☑️ Require status checks to pass before merging
   ```

## 📚 Comandos Git Útiles

### Consultar Estado
```bash
git status              # Ver estado actual
git log                # Ver historial de commits
git log --oneline      # Historial resumido
git diff               # Ver cambios no commiteados
```

### Deshacer Cambios
```bash
git checkout archivo.cpp    # Descartar cambios en un archivo
git reset HEAD archivo.cpp  # Quitar archivo del staging
git reset --soft HEAD~1     # Deshacer último commit (mantener cambios)
git reset --hard HEAD~1     # Deshacer último commit (eliminar cambios)
```

### Ramas (Branches)
```bash
git branch                    # Listar ramas
git branch nueva-rama         # Crear rama
git checkout nueva-rama       # Cambiar a rama
git checkout -b nueva-rama    # Crear y cambiar a rama
git merge otra-rama           # Fusionar rama
git branch -d rama-antigua    # Eliminar rama
```

### Actualizar desde GitHub
```bash
git pull                # Traer cambios desde GitHub
git fetch              # Descargar cambios sin fusionar
```

## 🆘 Solución de Problemas

### Error: "Permission denied (publickey)"
**Solución:** Configurar SSH o usar HTTPS con PAT

### Error: "Failed to push some refs"
**Solución:**
```bash
git pull origin main --rebase
git push origin main
```

### Eliminar archivos que no deberían estar
```bash
# 1. Agregar al .gitignore
echo "archivo_secreto.txt" >> .gitignore

# 2. Eliminar del repositorio (pero mantener local)
git rm --cached archivo_secreto.txt

# 3. Commit
git commit -m "Remove: Eliminar archivo secreto"

# 4. Push
git push
```

### Conflictos al hacer merge
```bash
# 1. Git marca los archivos con conflictos
# 2. Abre los archivos y resuelve manualmente
# 3. Después de resolver:
git add archivo_resuelto.cpp
git commit -m "Merge: Resolver conflicto en archivo"
```

## 📖 Recursos Adicionales

- [GitHub Docs](https://docs.github.com/)
- [Git Book (Español)](https://git-scm.com/book/es/v2)
- [GitHub Learning Lab](https://lab.github.com/)
- [Visualizing Git](http://git-school.github.io/visualizing-git/)

## ✅ Checklist Final

Antes de publicar tu repositorio:

- [ ] README.md completo y claro
- [ ] LICENSE incluido (MIT)
- [ ] Ejemplos funcionales y documentados
- [ ] library.properties con información correcta
- [ ] .gitignore apropiado
- [ ] Diagramas de conexión incluidos
- [ ] keywords.txt para Arduino IDE
- [ ] Código comentado y limpio
- [ ] Sin archivos personales o sensibles
- [ ] URL del repositorio actualizada en library.properties

## 🎉 ¡Listo!

Tu librería ahora está disponible públicamente en GitHub. Los usuarios pueden:
- Clonar el repositorio
- Reportar issues
- Hacer contribuciones
- Usar en sus propios proyectos

---

**¿Necesitas ayuda?**
Si tienes problemas, puedes:
1. Buscar en Stack Overflow
2. Leer la documentación de GitHub
3. Preguntar en los foros de Arduino
4. Abrir un issue en tu propio repositorio
