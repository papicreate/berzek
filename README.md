# Papiweb desarrollos informaticos
# Berzek (SFML) — Build instructions C++

Este repositorio contiene `berzek.cpp`, un clon simple de Berzerk usando SFML.

Requisitos
- CMake 3.16+
- Compilador C++ con soporte C++17 (MSVC, g++, clang)
- SFML 2.5 (librerías `graphics`, `window`, `system`)
- Una fuente TTF llamada `font.ttf` colocada al lado del ejecutable (puedes descargar Liberation Sans u otra TTF)

Linux (ejemplo Ubuntu/Debian)

1. Instala dependencias:

```bash
sudo apt update
sudo apt install -y build-essential cmake libsfml-dev
```

2. Compilar:

```bash
cmake -S . -B build
cmake --build build --config Release
```

3. Ejecutar (coloca una `font.ttf` junto al ejecutable `build/berzek` si no existe):

```bash
cp /ruta/a/font.ttf build/
./build/berzek
```

Windows (Visual Studio + vcpkg) — recomendado

1. Instala vcpkg y SFML (ejemplo):

```powershell
# en PowerShell (ejemplo):
# clonar vcpkg y bootstrap
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
# instalar SFML
.\vcpkg\vcpkg.exe install sfml:x64-windows
```

2. Genera y compila con CMake pasando el toolchain de vcpkg:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/ruta/a/vcpkg/scripts/buildsystems/vcpkg.cmake -A x64
cmake --build build --config Release
```

3. Copia `font.ttf` al directorio del ejecutable `build/Release` y asegúrate de copiar las DLLs de SFML (o usa la variante estática) junto al `.exe`.

Windows (MinGW)

1. Instala MinGW y las librerías SFML compiladas para MinGW.
2. Genera con el generador MinGW de CMake:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --config Release
```

Notas sobre dependencias en Windows
- Si usas las builds dinámicas de SFML, necesitas colocar las DLLs (sfml-graphics-2.dll, sfml-window-2.dll, sfml-system-2.dll, etc.) en el mismo directorio que el `.exe`.
- Para enlazado estático ajusta las opciones de SFML y define SFML_STATIC si compilas estáticamente.

Problemas comunes
- "No se encuentra SFML": instala SFML (vcpkg o paquetes precompilados) y/o pasa `-DCMAKE_TOOLCHAIN_FILE` para vcpkg.
- Ventana no aparece en contenedores remotos: la app abre una ventana nativa; si trabajas por SSH debes usar reenvío X11 o ejecutar en una máquina con servidor gráfico.

Consejo
- Descarga una fuente TTF (por ejemplo Liberation Sans en Google Fonts), renómbrala `font.ttf` y colócala junto al ejecutable.# berzek
