# 📐 Clasificador de Figuras Geométricas con OpenCV y Android

Esta aplicación móvil permite al usuario dibujar figuras geométricas a mano alzada y clasificarlas automáticamente mediante técnicas de visión por computador. El sistema está desarrollado en **Java + C++** para Android, e implementa descriptores clásicos de forma como los **Momentos Invariantes de Hu** y la **Firma de la Figura (Shape Signature)**.

El procesamiento de imagen se realiza con **OpenCV** a nivel nativo, aprovechando la eficiencia de C++ para calcular características y realizar la comparación contra un conjunto de referencias. La clasificación final se muestra en tiempo real a través de una interfaz intuitiva.

> Este proyecto fue desarrollado para reforzar conocimientos en análisis de formas, procesamiento de imágenes, y desarrollo de aplicaciones móviles con integración nativa.

## ✅ Características principales
- 🎨 Interfaz interactiva que permite dibujar figuras a mano alzada.
- ⚙️ Clasificación automática en tiempo real.
- 🧠 Uso de descriptores clásicos de forma:
  - Momentos Invariantes de Hu
  - Shape Signature (Firma de la Figura)
- 🔬 Procesamiento de imagen implementado con OpenCV en C++.
- 🔁 Comparación contra un conjunto de referencias utilizando distancia Euclídea.
- 📲 Compatible con dispositivos Android (API 21+).
- 🧩 Integración de código nativo mediante JNI y NDK.
- 📁 Archivos de referencia embebidos en los assets de la aplicación.

## 🏗️ Arquitectura general del sistema

La aplicación está compuesta por dos partes principales: el frontend móvil desarrollado en Java (Android SDK) y el backend de procesamiento implementado en C++ con OpenCV, comunicados mediante JNI (Java Native Interface).

### 🔄 Flujo de funcionamiento

1. El usuario traza una figura en pantalla mediante el componente `DrawingView`.
2. El trazo se convierte a un `Bitmap` y se envía al código nativo C++ a través de JNI.
3. En C++, se realiza el preprocesamiento de la imagen: conversión a escala de grises, binarización e identificación de contornos.
4. Se extraen los descriptores seleccionados por el usuario:
   - Momentos Invariantes de Hu
   - Firma de la Figura (Shape Signature)
5. El descriptor resultante se compara contra un conjunto de referencias previamente cargado desde los assets.
6. Se calcula la distancia Euclídea y se determina la clase más cercana.
7. El resultado se devuelve a Java y se muestra en la interfaz gráfica.

Este enfoque permite combinar la facilidad del desarrollo móvil en Android con la eficiencia de procesamiento de imágenes en C++.

## 🗂️ Estructura del proyecto

A continuación se muestra la estructura principal del proyecto Android, organizada por módulos y carpetas clave:
app
├── assets
│   ├── hu_moments_reference.csv
│   └── shape_signature_reference.csv
├── cpp
│   ├── CMakeLists.txt
│   └── native-lib.cpp
├── java
│   └── com
│       └── example
│           └── app
│               ├── DrawingView.java
│               └── MainActivity.java
└── res
    ├── drawable
    │   ├── draw_area_background.xml
    │   ├── ic_launcher_background.xml
    │   └── ic_launcher_foreground.xml
    ├── layout
    │   └── activity_main.xml
    ├── values
    │   ├── colors.xml
    │   ├── strings.xml
    │   └── themes.xml
    ├── values-night
    │   └── themes.xml
    └── xml
        ├── backup_rules.xml
        └── data_extraction_rules.xml

## 🛠️ Requisitos del sistema y herramientas necesarias

Para compilar, ejecutar y modificar esta aplicación, se requiere el siguiente entorno de desarrollo:

### 🧰 Entorno de desarrollo

- **Android Studio** Narwall (latest version)
- **Gradle** (incluido con Android Studio)
- **Android SDK** versión mínima: API 21 (Android 5.0)
- **Android NDK** r21 o superior
- **CMake** (integrado en Android Studio)

### 📦 Dependencias

- **OpenCV 4.12.0** compilado para Android (incluido como dependencia en el módulo nativo)
- Soporte para **JNI (Java Native Interface)**
- Herramientas de build para código nativo (`ndk-build`, `cmake`, `ninja`)

### 📱 Dispositivo o emulador

- Dispositivo físico con Android 5.0+ o emulador con soporte para gráficos básicos
- Se recomienda pantalla táctil para una mejor experiencia al trazar figuras

## 🧪 Modo de uso

La aplicación está diseñada para ser intuitiva y fácil de usar. A continuación se describen los pasos básicos para interactuar con el sistema de clasificación:

### ✍️ 1. Dibujar una figura

En la pantalla principal, el usuario puede trazar libremente una figura geométrica sobre un área de dibujo de fondo negro. La figura debe representar un **círculo**, **cuadrado** o **triángulo**, preferiblemente de forma cerrada y centrada.

### 🧭 2. Seleccionar el descriptor

Debajo del área de dibujo se encuentra un menú desplegable (`Spinner`) donde se puede elegir el descriptor que se usará para clasificar la figura:

- **Momentos de Hu**  
- **Firma de la Figura (Shape Signature)**

Cada descriptor ofrece una manera distinta de analizar la forma del trazo.

### 🔍 3. Analizar la figura

Presiona el botón **“Analizar”**. La aplicación realizará lo siguiente:

- Convertirá el dibujo a una imagen binaria.
- Extraerá el descriptor seleccionado.
- Comparará el resultado contra referencias almacenadas.
- Mostrará en pantalla la clase detectada (ej. “Círculo”).

### ♻️ 4. Limpiar el área de dibujo

Si se desea probar otra figura, el botón **“Limpiar”** borra completamente el trazo actual y permite volver a empezar.

---

⚠️ **Nota**: los mejores resultados se obtienen con figuras bien definidas, cerradas y centradas. Trazos incompletos o muy irregulares pueden afectar la precisión del sistema.

