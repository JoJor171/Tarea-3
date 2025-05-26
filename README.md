GraphQuest: Juego de Aventura en Laberinto 

---------------------------------------------------------------------------------------------------------------
DESCRIPCION:

GraphQuest es un juego de aventura en C donde dos jugadores exploran un laberinto cargado desde un archivo CSV. 
Cada escenario contiene conexiones a otros escenarios y puede tener ítems con valores de puntos. Los jugadores 
deben moverse, recolectar y gestionar ítems, todo mientras administran su tiempo limitado. El juego finaliza 
cuando uno de los jugadores alcanza el escenario final o ambos agotan su tiempo.Este proyecto tiene como propósito 
trabajar con estructuras, lectura de archivos, lógica de juego y manejo de memoria en C, simulando un entorno tipo RPG.

COMO COMPILAR Y EJECUTAR:

El juego está desarrollado en C y se puede compilar y ejecutar en cualquier sistema con soporte para el compilador GCC.

REQUISITOS PREVIOS:

-Tener instalado Visual Studio Code.

-Instalar la extensión C/C++ de Microsoft.

-Contar con un compilador de C, como gcc.

-En Windows, puedes instalar MinGW o usar WSL.


---------------------------------------------------------------------------------------------------------------
PASOS PARA COMPILAR Y EJECUTAR:
Descarga y descomprime el proyecto en una carpeta local.

ABRE EL PROYECTO CON VISUAL STUDIO:

Ve a Archivo > Abrir carpeta... y selecciona la carpeta del proyecto.

Coloca el archivo de escenarios CSV (graphquest.csv) en la misma carpeta.

COMPILA EL PROGRAMA:

Abre la terminal integrada (Terminal > Nueva terminal).

EJECUTA EL SIGUIENTE COMANDO PARA COMPILAR:  
    gcc graphquest.c -o graphquest

EJECUTA EL JUEGO CON EL COMANDO:

./graphquest



---------------------------------------------------------------------------------------------------------------
FUNCIONALIDADES IMPLEMENTADAS CORRECTAMENTE:

Carga de escenarios e ítems desde un archivo CSV.

Representación del laberinto con múltiples conexiones por escenario.

Movimiento libre de los jugadores entre escenarios conectados.

Recolección y descarte de ítems con gestión de inventario.

Sistema de puntaje individual por jugador.

Tiempo limitado por jugador (disminuye por acción).

Finalización automática del juego si:

Un jugador alcanza el escenario final.

Ambos jugadores agotan su tiempo.


PROBLEMAS CONOCIDOS:
Puede fallar si el archivo CSV está mal formateado o tiene más ítems que el máximo permitido (MAX_ITEMS).

---------------------------------------------------------------------------------------------------------------
A MEJORAR:
-Mejor manejo de errores en la lectura del archivo CSV.
-Inclusión de animaciones o interfaz gráfica básica.

---------------------------------------------------------------------------------------------------------------
EJEMPLO DE USO :


INICIO DEL JUEGO :
Bienvenidos a GraphQuest: Juego de Aventura en Laberinto
Ingrese el ID del escenario inicial (0 a 99): 0
Jugador 1 y Jugador 2 han comenzado en el escenario 0

MENU DE ACCIONES :

Turno de Jugador 1. Tiempo restante: 10
1) Moverse a un escenario conectado
2) Ver escenario actual
3) Ver inventario
4) Recoger ítem
5) Descatar ítem
Ingrese una opción: 1

RECOLECCION DE ITEM : 

Escenario contiene los siguientes ítems:
1. Espada (valor: 50)
2. Pocion (valor: 20)
Seleccione el número de ítem a recoger: 1
Ítem 'Espada' añadido al inventario.

FIN DEL JUEGO :

Jugador 1 ha llegado al escenario final. ¡Juego terminado!
Puntaje final:
Jugador 1: 80 puntos
Jugador 2: 40 puntos


CONTRIBUCIONES : 

Jose Jorquera : 
Implementación de la estructura del laberinto y lectura desde CSV.

Lógica de movimiento entre escenarios y conexión de vecinos.

Gestión de ítems e inventario por jugador.

Rafael Cortes : 

Desarrollo del sistema de puntajes, turnos y control de tiempo.

Implementación del menú de acciones y reglas de finalización del juego.

Redacción del archivo README y validación de errores comunes.


