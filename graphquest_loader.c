#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_ITEMS 10
#define MAX_LINEA 512
#define MAX_NOMBRE 64
#define MAX_DESC 256
#define MAX_ESCENARIOS 100
#define TIEMPO_INICIAL 10
#define NUM_JUGADORES 2

typedef struct {
    char nombre[MAX_NOMBRE];
    int peso;
    int valor;
} Item;

typedef struct {
    int id;
    char nombre[MAX_NOMBRE];
    char descripcion[MAX_DESC];
    Item items[MAX_ITEMS];
    int num_items;
    int vecino_arriba;
    int vecino_abajo;
    int vecino_izquierda;
    int vecino_derecha;
    int es_final;
} Escenario;

typedef struct {
    int escenario_actual;
    int tiempo_restante;
    Item inventario[MAX_ITEMS];
    int num_inventario;
} Jugador;

/*
 * Funcion auxiliar para parsear una linea CSV.
 * Respeta campos entrecomillados para que comas internas se mantengan.
 */
int parse_csv_line(char *line, char *fields[], int max_fields) {
    int field = 0;
    char *p = line;
    while (*p && field < max_fields) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '"') {
            p++;  // Salta la comilla inicial
            fields[field++] = p;
            while (*p && *p != '"')
                p++;
            if (*p == '"') {
                *p = '\0';
                p++; // Salta la comilla de cierre
                if (*p == ',')
                    p++;  // Salta el separador
            }
        } else {
            fields[field++] = p;
            while (*p && *p != ',')
                p++;
            if (*p == ',') {
                *p = '\0';
                p++;
            }
        }
    }
    return field;
}

/*
 * Carga el laberinto del archivo CSV.
 * Formato: ID, Nombre, Descripcion, Items, Arriba, Abajo, Izquierda, Derecha, EsFinal.
 * Los items se separan por ";" y cada item se define con tres atributos, separados por coma.
 * Se convierte el indice (IDs inician en 1) a indice de array (inician en 0).
 */
Escenario* cargarLaberinto(const char* nombre_archivo, int* total_escenarios) {
    FILE* archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo.\n");
        return NULL;
    }
    Escenario* grafo = malloc(sizeof(Escenario) * MAX_ESCENARIOS);
    if (!grafo) {
        printf("Error al asignar memoria.\n");
        fclose(archivo);
        return NULL;
    }
    *total_escenarios = 0;
    char linea[MAX_LINEA];
    // Saltar encabezado
    if (!fgets(linea, sizeof(linea), archivo)) {
        printf("Archivo vacio.\n");
        fclose(archivo);
        return NULL;
    }
    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\r\n")] = '\0';
        char *campos[9];
        int num_campos = parse_csv_line(linea, campos, 9);
        if (num_campos < 9) {
            printf("Advertencia: linea mal formateada: %s\n", linea);
            continue;
        }
        Escenario esc;
        esc.num_items = 0;
        esc.id = atoi(campos[0]);
        strncpy(esc.nombre, campos[1], MAX_NOMBRE - 1);
        esc.nombre[MAX_NOMBRE - 1] = '\0';
        strncpy(esc.descripcion, campos[2], MAX_DESC - 1);
        esc.descripcion[MAX_DESC - 1] = '\0';
        // Procesamiento de items.
        if (strlen(campos[3]) > 0) {
            char items_copy[MAX_LINEA];
            strncpy(items_copy, campos[3], MAX_LINEA - 1);
            items_copy[MAX_LINEA - 1] = '\0';
            char *item_token = strtok(items_copy, ";");
            while (item_token != NULL && esc.num_items < MAX_ITEMS) {
                char *atributo;
                char *item_atributos[3];
                int idx = 0;
                atributo = strtok(item_token, ",");
                while (atributo != NULL && idx < 3) {
                    item_atributos[idx++] = atributo;
                    atributo = strtok(NULL, ",");
                }
                if (idx == 3) {
                    Item item;
                    strncpy(item.nombre, item_atributos[0], MAX_NOMBRE - 1);
                    item.nombre[MAX_NOMBRE - 1] = '\0';
                    item.peso = atoi(item_atributos[1]);
                    item.valor = atoi(item_atributos[2]);
                    esc.items[esc.num_items++] = item;
                }
                item_token = strtok(NULL, ";");
            }
        }
        // Lectura de conexiones.
        esc.vecino_arriba   = atoi(campos[4]);
        if (esc.vecino_arriba != -1)
            esc.vecino_arriba--;
        esc.vecino_abajo    = atoi(campos[5]);
        if (esc.vecino_abajo != -1)
            esc.vecino_abajo--;
        esc.vecino_izquierda = atoi(campos[6]);
        if (esc.vecino_izquierda != -1)
            esc.vecino_izquierda--;
        esc.vecino_derecha  = atoi(campos[7]);
        if (esc.vecino_derecha != -1)
            esc.vecino_derecha--;
        // Determinar si es escenario final.
        esc.es_final = (strcmp(campos[8], "Si") == 0) ? 1 : 0;
        grafo[*total_escenarios] = esc;
        (*total_escenarios)++;
    }
    fclose(archivo);
    return grafo;
}

/*
 * Muestra el estado actual del jugador: escenario, tiempo restante e inventario.
 */
void mostrar_estado(Jugador* jugador, Escenario* laberinto) {
    Escenario escenario_actual = laberinto[jugador->escenario_actual];
    printf("Estas en: %s (ID: %d)\n", escenario_actual.nombre, escenario_actual.id);
    printf("Tiempo restante: %d\n", jugador->tiempo_restante);
    printf("Inventario: ");
    if (jugador->num_inventario == 0)
        printf("Vacio");
    else {
        for (int i = 0; i < jugador->num_inventario; i++) {
            printf("%s ", jugador->inventario[i].nombre);
        }
    }
    printf("\n");
}

/*
 * Funcion para recoger un item.
 * Se muestra la lista de items disponibles y se permite al usuario ingresar
 * el numero del item a recoger o 0 para volver atras sin consumir accion ni tiempo.
 * Se retorna 1 si se completa la accion (item recogido) o 0 si se cancela.
 */
int tomar_item(Jugador* jugador, Escenario* escenario) {
    if (escenario->num_items == 0) {
        printf("No hay items disponibles en este escenario.\n");
        return 1;  // Se consume la accion pues no hay opciones.
    }
    printf("Items disponibles:\n");
    for (int i = 0; i < escenario->num_items; i++) {
        printf("%d. %s (peso: %d, valor: %d)\n", i + 1,
               escenario->items[i].nombre, escenario->items[i].peso, escenario->items[i].valor);
    }
    printf("Ingrese el numero del item a recoger, o 0 para volver atras: ");
    int opcion;
    if (scanf("%d", &opcion) != 1) {
        printf("Entrada invalida.\n");
        while (getchar() != '\n');
        return 0;
    }
    if (opcion == 0) {
        printf("Volviendo atras...\n");
        return 0; // Accion cancelada, no se consume tiempo ni se cuenta la accion.
    }
    if (opcion < 1 || opcion > escenario->num_items) {
        printf("Seleccion invalida.\n");
        return 0;
    }
    int idx = opcion - 1;
    Item item = escenario->items[idx];
    if (jugador->num_inventario < MAX_ITEMS) {
        jugador->inventario[jugador->num_inventario++] = item;
        printf("Recogiste %s.\n", item.nombre);
        // Remover el item del escenario para que no este disponible para otro jugador.
        for (int i = idx; i < escenario->num_items - 1; i++) {
            escenario->items[i] = escenario->items[i + 1];
        }
        escenario->num_items--;
        return 1;
    } else {
        printf("Tu inventario esta lleno.\n");
        return 1; // Se consume la accion aunque no se pudo recoger.
    }
}

/*
 * Permite al jugador descartar un item de su inventario.
 */
void descartar_item(Jugador* jugador) {
    if (jugador->num_inventario == 0) {
        printf("No tienes items para descartar.\n");
        return;
    }
    printf("Tus items:\n");
    for (int i = 0; i < jugador->num_inventario; i++) {
        printf("%d. %s (valor: %d)\n", i + 1,
               jugador->inventario[i].nombre, jugador->inventario[i].valor);
    }
    printf("Selecciona el numero del item a descartar: ");
    int opcion;
    if (scanf("%d", &opcion) != 1) {
        printf("Entrada invalida.\n");
        while (getchar() != '\n');
        return;
    }
    if (opcion < 1 || opcion > jugador->num_inventario) {
        printf("Seleccion invalida.\n");
        return;
    }
    printf("Descartaste %s.\n", jugador->inventario[opcion - 1].nombre);
    for (int i = opcion - 1; i < jugador->num_inventario - 1; i++) {
        jugador->inventario[i] = jugador->inventario[i + 1];
    }
    jugador->num_inventario--;
}

/*
 * Permite al jugador avanzar a un escenario vecino.
 */
void avanzar(Jugador* jugador, Escenario* escenarios, int total_escenarios) {
    Escenario* escenario_actual = &escenarios[jugador->escenario_actual];
    printf("Opciones de movimiento:\n");
    int opciones[4] = { -1, -1, -1, -1 };
    if (escenario_actual->vecino_arriba != -1)
        printf("1. Arriba -> %s\n", escenarios[escenario_actual->vecino_arriba].nombre);
    if (escenario_actual->vecino_abajo != -1)
        printf("2. Abajo -> %s\n", escenarios[escenario_actual->vecino_abajo].nombre);
    if (escenario_actual->vecino_izquierda != -1)
        printf("3. Izquierda -> %s\n", escenarios[escenario_actual->vecino_izquierda].nombre);
    if (escenario_actual->vecino_derecha != -1)
        printf("4. Derecha -> %s\n", escenarios[escenario_actual->vecino_derecha].nombre);
    opciones[0] = escenario_actual->vecino_arriba;
    opciones[1] = escenario_actual->vecino_abajo;
    opciones[2] = escenario_actual->vecino_izquierda;
    opciones[3] = escenario_actual->vecino_derecha;
    
    printf("Selecciona opcion de movimiento (numero): ");
    int opcion;
    if (scanf("%d", &opcion) != 1) {
        printf("Entrada invalida.\n");
        while(getchar() != '\n');
        return;
    }
    if (opcion < 1 || opcion > 4 || opciones[opcion - 1] == -1) {
        printf("Opcion invalida.\n");
        return;
    }
    jugador->escenario_actual = opciones[opcion - 1];
    printf("Avanzas hacia %s.\n", escenarios[jugador->escenario_actual].nombre);
}

/*
 * Verifica si la partida debe terminar:
 * Se termina si ambos jugadores se quedan sin tiempo o han llegado a un escenario final.
 */
int verificar_fin(Jugador jugadores[], Escenario* laberinto) {
    int ambosSinTiempo = (jugadores[0].tiempo_restante <= 0 && jugadores[1].tiempo_restante <= 0);
    int ambosLlegaronSalida = (laberinto[jugadores[0].escenario_actual].es_final &&
                               laberinto[jugadores[1].escenario_actual].es_final);
    return ambosSinTiempo || ambosLlegaronSalida;
}

/*
 * Calcula el puntaje total del inventario de un jugador.
 */
int calcular_puntaje(Jugador* jugador) {
    int puntaje = 0;
    for (int i = 0; i < jugador->num_inventario; i++) {
        puntaje += jugador->inventario[i].valor;
    }
    return puntaje;
}

int main() {
    int total_escenarios = 0;
    Escenario* laberinto = cargarLaberinto("graphquest.csv", &total_escenarios);
    if (!laberinto) {
        return 1;
    }
    
    // Inicializacion de dos jugadores con el primer escenario y tiempo inicial.
    Jugador jugadores[NUM_JUGADORES] = {
        { .escenario_actual = 0, .tiempo_restante = TIEMPO_INICIAL, .num_inventario = 0 },
        { .escenario_actual = 0, .tiempo_restante = TIEMPO_INICIAL, .num_inventario = 0 }
    };
    
    int finDeJuego = 0;
    int turno = 0;
    while (!verificar_fin(jugadores, laberinto) && !finDeJuego) {
        Jugador* jugador_actual = &jugadores[turno % NUM_JUGADORES];
        Escenario* escenario_actual = &laberinto[jugador_actual->escenario_actual];
        
        printf("\nTurno del Jugador %d\n", (turno % NUM_JUGADORES) + 1);
        mostrar_estado(jugador_actual, laberinto);
        
        int acciones = 0;
        printf("Cuantas acciones deseas realizar en este turno? (1 o 2): ");
        while (scanf("%d", &acciones) != 1 || (acciones != 1 && acciones != 2)) {
            printf("Entrada invalida. Ingresa 1 o 2: ");
            while(getchar() != '\n');
        }
        
        int accionesRealizadas = 0;
        while (accionesRealizadas < acciones && jugador_actual->tiempo_restante > 0 && !finDeJuego) {
            int opcion = 0;
            int valid = 0;
            // Menu de acciones y validacion (opciones 1 a 4)
            while (!valid) {
                printf("\nAcciones disponibles:\n");
                printf("1. Recoger item\n");
                printf("2. Descartar item\n");
                printf("3. Avanzar\n");
                printf("4. Finalizar partida\n");
                printf("Selecciona accion: ");
                if (scanf("%d", &opcion) != 1) {
                    printf("Entrada invalida.\n");
                    while(getchar() != '\n');
                    continue;
                }
                if (opcion < 1 || opcion > 4) {
                    printf("Opcion invalida. Intenta de nuevo.\n");
                    continue;
                }
                valid = 1;
            }
            
            if (opcion == 4) {
                printf("Partida finalizada manualmente.\n");
                finDeJuego = 1;
                break;
            }
            
            switch(opcion) {
                case 1:
                    // Si tomar_item retorna 0, significa que se cancelo la accion (boton volver)
                    if (!tomar_item(jugador_actual, escenario_actual))
                        continue;
                    break;
                case 2:
                    descartar_item(jugador_actual);
                    break;
                case 3:
                    avanzar(jugador_actual, laberinto, total_escenarios);
                    // Actualizar el puntero al escenario actual en caso de moverse.
                    escenario_actual = &laberinto[jugador_actual->escenario_actual];
                    break;
            }
            
            // Cada accion valida consume 1 unidad de tiempo
            jugador_actual->tiempo_restante--;
            accionesRealizadas++;
            if (jugador_actual->tiempo_restante <= 0) {
                printf("El Jugador %d se ha quedado sin tiempo.\n", (turno % NUM_JUGADORES) + 1);
                break;
            }
        }
        turno++;
    }
    
    printf("\nFin de la partida.\n");
    if (finDeJuego) {
        printf("Partida finalizada manualmente.\n");
    } else if (laberinto[jugadores[0].escenario_actual].es_final &&
               laberinto[jugadores[1].escenario_actual].es_final) {
        printf("Ambos jugadores han alcanzado el escenario final.\n");
        int puntaje_total = 0;
        for (int i = 0; i < NUM_JUGADORES; i++) {
            printf("\nJugador %d:\n", i + 1);
            if (jugadores[i].num_inventario == 0) {
                printf("  Inventario: Vacio\n");
            } else {
                printf("  Items:\n");
                for (int j = 0; j < jugadores[i].num_inventario; j++) {
                    printf("    - %s (valor: %d)\n", jugadores[i].inventario[j].nombre,
                           jugadores[i].inventario[j].valor);
                }
            }
            int puntaje = calcular_puntaje(&jugadores[i]);
            printf("  Puntaje: %d\n", puntaje);
            puntaje_total += puntaje;
        }
        printf("\nPuntaje total obtenido: %d\n", puntaje_total);
    } else {
        printf("La partida termino por falta de tiempo.\n");
    }
    
    free(laberinto);
    return 0;
}
