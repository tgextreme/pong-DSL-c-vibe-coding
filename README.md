# Pong Game con SDL2

Un juego clásico de Pong implementado en C++ usando SDL2, con menú principal, modo multijugador, IA inteligente y soporte de audio.

## 🎮 Características

- **Menú Principal Interactivo**: Navega entre opciones con las flechas
- **Modo vs IA**: Juega contra una inteligencia artificial adaptable
- **Modo Multijugador**: Juego local para dos jugadores
- **Sistema de Audio**: Música de fondo activable/desactivable
- **Física Realística**: Efectos de rebote según el punto de impacto
- **Interfaz Visual**: Menús y marcadores visuales
- **Controles Intuitivos**: Fácil de aprender y jugar

## 🎯 Modos de Juego

### 1. Vs IA (Un Jugador)
- Juega contra la computadora
- IA con dificultad balanceada (80% velocidad)
- Solo reacciona cuando la pelota se acerca
- Incluye zona muerta para movimientos suaves

### 2. Multijugador (Dos Jugadores)
- Juego local en la misma computadora
- Cada jugador controla su propia paleta
- Perfecto para competir con amigos

## 🎮 Controles

### Menú Principal
- **Flechas ↑↓**: Navegar opciones
- **ENTER**: Seleccionar opción
- **ESC**: Salir del juego

### Modo vs IA
- **W/S**: Mover tu paleta (arriba/abajo)
- **IA**: Se mueve automáticamente
- **M**: Activar/desactivar música
- **ESC**: Volver al menú

### Modo Multijugador
- **Jugador 1**: W (arriba), S (abajo)
- **Jugador 2**: Flechas ↑↓ (arriba/abajo)
- **M**: Activar/desactivar música
- **ESC**: Volver al menú

## 🔊 Sistema de Audio

- **Música de Fondo**: Activable desde el menú o durante el juego
- **Toggle con M**: Presiona M durante el juego para activar/desactivar
- **Estado Persistente**: La configuración se mantiene entre partidas

## 💻 Requisitos del Sistema

- Linux (Ubuntu/Debian)
- SDL2 development libraries
- SDL2_mixer development libraries  
- C++ compiler (g++)
- Make

## 🚀 Instalación

### Instalar dependencias

```bash
# Opción 1: Usar el Makefile
make install-deps

# Opción 2: Instalación manual
sudo apt update
sudo apt install -y libsdl2-dev libsdl2-mixer-dev build-essential pkg-config
```

### Compilar el juego

```bash
make
```

### Ejecutar el juego

```bash
make run
# o directamente:
./pong
```

## 🎯 Cómo Jugar

### Inicio
1. Ejecuta el juego
2. Usa las flechas ↑↓ para navegar en el menú
3. Presiona ENTER para seleccionar una opción

### Modo vs IA
1. Selecciona "1. Juegar vs IA"
2. Usa W/S para controlar tu paleta (izquierda)
3. La IA controla automáticamente la paleta derecha
4. ¡Intenta marcar más puntos que la máquina!

### Modo Multijugador
1. Selecciona "2. Multijugador"
2. Jugador 1: Usa W/S (paleta izquierda)
3. Jugador 2: Usa flechas ↑↓ (paleta derecha)
4. ¡Compite con tu amigo!

### Música
- Activa/desactiva desde el menú (opción 3)
- O presiona M durante cualquier partida
- El estado se mantiene entre partidas

## 📊 Sistema de Puntuación

- El marcador se muestra tanto en pantalla como en consola
- Puntos se muestran como pequeños cuadrados en la parte superior
- El juego continúa indefinidamente - ¡sigue jugando hasta que quieras parar!

## 🛠️ Estructura del Código

- `main.cpp`: Contiene toda la lógica del juego
  - **Clase `AudioManager`**: Maneja el sistema de audio
    - `init()`: Inicializa SDL_mixer
    - `toggleMusic()`: Activa/desactiva música
    - `cleanup()`: Limpia recursos de audio
  - **Clase `Paddle`**: Maneja las paletas (jugador e IA)
    - `update()`: Control manual del jugador
    - `updateAI()`: Lógica de inteligencia artificial
  - **Clase `Ball`**: Maneja la física de la pelota
  - **Clase `Game`**: Controla el bucle principal y estados del juego
    - `renderMenu()`: Dibuja el menú principal
    - `renderGame()`: Dibuja el juego en curso
    - `handleMenuEvents()`: Maneja input del menú
    - `handleGameEvents()`: Maneja input durante el juego

## ⚙️ Personalización

### Ajustar Dificultad de la IA
Edita la variable `difficulty` en el método `updateAI()`:
```cpp
float difficulty = 0.8f; // Cambia entre 0.1 (muy fácil) a 1.0 (muy difícil)
```

### Cambiar Velocidades
En las constantes del inicio del archivo:
```cpp
const float PADDLE_SPEED = 300.0f; // Velocidad de paletas
const float BALL_SPEED = 250.0f;   // Velocidad de pelota
```

### Modificar Tamaños
```cpp
const int PADDLE_WIDTH = 20;    // Ancho de paletas
const int PADDLE_HEIGHT = 100;  // Alto de paletas  
const int BALL_SIZE = 15;       // Tamaño de pelota
```

## 🧹 Limpieza

```bash
make clean
```

## 🚀 Características Técnicas

- **Framerate**: ~60 FPS con SDL_Delay(16)
- **Resolución**: 800x600 pixels
- **Audio**: SDL2_mixer para soporte de música
- **Físicas**: Colisiones con efecto según punto de impacto
- **Renderizado**: SDL2 con aceleración por hardware
- **Estados**: Sistema de menú y modos de juego
- **Cross-platform**: Preparado para Linux (fácilmente portable)

## 🐛 Solución de Problemas

### El juego no compila
```bash
# Verifica que tengas las dependencias
pkg-config --modversion sdl2
pkg-config --libs sdl2

# Reinstala dependencias si es necesario
make install-deps
```

### No hay audio
- El audio es opcional - el juego funciona sin él
- Verifica que SDL2_mixer esté instalado: `dpkg -l | grep libsdl2-mixer`

### Problemas de rendimiento
- El juego está optimizado pero si tienes problemas, cierra otras aplicaciones
- Verifica que tengas aceleración gráfica habilitada

¡Disfruta jugando Pong! 🎮🏓
# pong-DSL-c-vibe-coding
