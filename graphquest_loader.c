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

int parse_csv_line(char *line, char *fields[], int max_fields) {
    int field = 0;
    char *p = line;
    while (*p && field < max_fields) {
        // Omitir espacios en blanco iniciales
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '"') {
            // Campo entrecomillado
            p++;  // saltar la comilla inicial
            fields[field++] = p;
            // Buscar la comilla de cierre
            while (*p && *p != '"') p++;
            if (*p == '"') {
                *p = '\0';  // terminar el campo
                p++;        // saltar la comilla de cierre
                if (*p == ',') p++;  // saltar la coma separadora
            }
        } else {
            // Campo sin comillas
            fields[field++] = p;
            while (*p && *p != ',') p++;
            if (*p == ',') {
                *p = '\0';
                p++;
            }
        }
    }
    return field;
}


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

    // Saltar la línea de encabezado
    if (!fgets(linea, sizeof(linea), archivo)) {
        printf("El archivo esta vacio o no se pudo leer la linea.\n");
        fclose(archivo);
        return NULL;
    }

    while (fgets(linea, sizeof(linea), archivo)) {
        // Eliminar fin de línea
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

        // Procesar el campo de ítems.
        // Si el campo tiene contenido, se separa por ";" y luego cada ítem se divide en sus atributos (separados por ",")
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

        // Conversión de vecinos: restar 1 para pasarlo a índice (si el valor es distinto de -1)
        esc.vecino_arriba   = atoi(campos[4]);
        if (esc.vecino_arriba != -1) esc.vecino_arriba--;
        esc.vecino_abajo    = atoi(campos[5]);
        if (esc.vecino_abajo != -1) esc.vecino_abajo--;
        esc.vecino_izquierda= atoi(campos[6]);
        if (esc.vecino_izquierda != -1) esc.vecino_izquierda--;
        esc.vecino_derecha  = atoi(campos[7]);
        if (esc.vecino_derecha != -1) esc.vecino_derecha--;

        // Determinar si es escenario final (campo 9)
        esc.es_final = (strcmp(campos[8], "Si") == 0) ? 1 : 0;

        grafo[*total_escenarios] = esc;
        (*total_escenarios)++;
    }

    fclose(archivo);
    return grafo;
}


void describir_escenarios(Escenario* grafo, int total) {
    printf("\nEscenarios disponibles:\n");
    for (int i = 0; i < total; i++) {
        Escenario e = grafo[i];
        printf("[%d] %s: %s\n", e.id, e.nombre, e.descripcion);

        if (e.num_items > 0) {
            printf("Items disponibles:\n");
            for (int j = 0; j < e.num_items; j++) {
                printf("     - %s (peso: %d, valor: %d)\n", 
                       e.items[j].nombre, e.items[j].peso, e.items[j].valor);
            }
        }

        printf("Conexiones:");
        if (e.vecino_arriba != -1)
            printf(" arriba -> %s", grafo[e.vecino_arriba].nombre);
        if (e.vecino_abajo != -1)
            printf(" abajo -> %s", grafo[e.vecino_abajo].nombre);
        if (e.vecino_izquierda != -1)
            printf(" izquierda -> %s", grafo[e.vecino_izquierda].nombre);
        if (e.vecino_derecha != -1)
            printf(" derecha -> %s", grafo[e.vecino_derecha].nombre);
        printf("\n\n");
    }
}


void mostrar_estado(Escenario* grafo, Jugador* jugador) {
    Escenario actual = grafo[jugador->escenario_actual];

    printf("\nEscenario actual: %s\n", actual.nombre);
    printf("%s\n", actual.descripcion);
    printf("Tiempo restante: %d\n", jugador->tiempo_restante);
    printf("Inventario: %d item(s), Peso total: ", jugador->num_inventario);

    int peso_total = 0, puntaje = 0;
    for (int i = 0; i < jugador->num_inventario; i++) {
        peso_total += jugador->inventario[i].peso;
        puntaje += jugador->inventario[i].valor;
    }
    printf("%d, Puntaje: %d\n", peso_total, puntaje);

    // Mostrar ítems disponibles en el escenario actual
    printf("Items disponibles en este escenario:\n");
    if (actual.num_items > 0) {
        for (int i = 0; i < actual.num_items; i++) {
            printf(" - %s (Peso: %d, Valor: %d)\n",
                   actual.items[i].nombre,
                   actual.items[i].peso,
                   actual.items[i].valor);
        }
    } else {
        printf(" - No hay items disponibles en este escenario.\n");
    }

    // Mostrar acciones posibles
    printf("Acciones posibles desde este escenario:");
    if (actual.vecino_arriba != -1) printf(" arriba");
    if (actual.vecino_abajo != -1) printf(" abajo");
    if (actual.vecino_izquierda != -1) printf(" izquierda");
    if (actual.vecino_derecha != -1) printf(" derecha");
    printf("\n");
}



void mostrar_menu_juego() {
    printf("\n===== Menu del Juego =====\n");
    printf("1. Recoger Item(s)\n");
    printf("2. Descartar Item(s)\n");
    printf("3. Avanzar en una Direccion\n");
    printf("4. Reiniciar Partida\n");
    printf("5. Salir del Juego\n");
    printf("Seleccione una opcion: ");
}


void jugar_partida(Escenario* grafo, int total) {
    Jugador jugador;
    jugador.escenario_actual = 0;
    jugador.tiempo_restante = TIEMPO_INICIAL;
    jugador.num_inventario = 0;

    int opcion;
    do {
        mostrar_estado(grafo, &jugador);
        mostrar_menu_juego();
        scanf("%d", &opcion);
        getchar(); // limpiar buffer

        switch (opcion) {
            case 1: {
                printf("Selecciona item(s) para recoger:\n");
                for (int i = 0; i < grafo[jugador.escenario_actual].num_items; i++) {
                    printf("[%d] %s (Peso: %d, Valor: %d)\n", i, 
                           grafo[jugador.escenario_actual].items[i].nombre, 
                           grafo[jugador.escenario_actual].items[i].peso, 
                           grafo[jugador.escenario_actual].items[i].valor);
                }
                printf("Ingresa los indices de los items a recoger (separados por espacio, -1 para terminar): ");
                int index;
                while (scanf("%d", &index) && index != -1) {
                    if (index >= 0 && index < grafo[jugador.escenario_actual].num_items) {
                        if (jugador.num_inventario < MAX_ITEMS) {
                            jugador.inventario[jugador.num_inventario++] = grafo[jugador.escenario_actual].items[index];
                            printf("Item %s recogido.\n", grafo[jugador.escenario_actual].items[index].nombre);
                        } else {
                            printf("Inventario lleno, no se puede recoger mas items.\n");
                        }
                    } else {
                        printf("Indice invalido.\n");
                    }
                }
                jugador.tiempo_restante -= 1;
                break;
            }
            case 2: {
                printf("Selecciona item(s) para descartar:\n");
                for (int i = 0; i < jugador.num_inventario; i++) {
                    printf("[%d] %s (Peso: %d, Valor: %d)\n", i, 
                           jugador.inventario[i].nombre, 
                           jugador.inventario[i].peso, 
                           jugador.inventario[i].valor);
                }
                printf("Ingresa los indices de los ítems a descartar (separados por espacio, -1 para terminar): ");

                int index;
                while (scanf("%d", &index) && index != -1) {
                    if (index >= 0 && index < jugador.num_inventario) {
                        printf("Item %s descartado.\n", jugador.inventario[index].nombre);
                        // Descartar el ítem
                        for (int j = index; j < jugador.num_inventario - 1; j++) {
                            jugador.inventario[j] = jugador.inventario[j + 1];
                        }
                        jugador.num_inventario--;
                    } else {
                        printf("Indice invalido.\n");
                    }
                }
                jugador.tiempo_restante -= 1;
                break;
            }
            case 3: {
                printf("Selecciona una direccion para avanzar:\n");
                printf("1. Arriba\n2. Abajo\n3. Izquierda\n4. Derecha\n");
                int direccion;
                scanf("%d", &direccion);
                int nuevo_escenario = -1;

                switch (direccion) {
                    case 1: nuevo_escenario = grafo[jugador.escenario_actual].vecino_arriba; break;
                    case 2: nuevo_escenario = grafo[jugador.escenario_actual].vecino_abajo; break;
                    case 3: nuevo_escenario = grafo[jugador.escenario_actual].vecino_izquierda; break;
                    case 4: nuevo_escenario = grafo[jugador.escenario_actual].vecino_derecha; break;
                    default: printf("Direccion invalida.\n"); continue;
                }

                if (nuevo_escenario != -1) {
                    jugador.escenario_actual = nuevo_escenario;
                    int peso_total = 0;
                    for (int i = 0; i < jugador.num_inventario; i++) {
                        peso_total += jugador.inventario[i].peso;
                    }
                    jugador.tiempo_restante -= (int)ceil((peso_total + 1) / 10.0);
                    if (jugador.tiempo_restante <= 0) {
                        printf("\n¡Se acabo el tiempo! Has perdido.\n");
                        return;
                    }
                } else {
                    printf("No puedes avanzar en esa direccion.\n");
                }
                break;
            }
            case 4:
                jugador.escenario_actual = 0;
                jugador.tiempo_restante = TIEMPO_INICIAL;
                jugador.num_inventario = 0;
                printf("Partida reiniciada.\n");
                break;
            case 5:
                printf("Saliendo del juego...\n");
                break;
            default:
                printf("Opcion invalida.\n");
        }

        if (jugador.tiempo_restante <= 0) {
            printf("\n¡Se acabo el tiempo! Has perdido.\n");
            break;
        }

    } while (opcion != 5);
}



int main() {
    int total_escenarios = 0;
    Escenario* grafo = cargarLaberinto("graphquest.csv", &total_escenarios);

    if (!grafo) {
        return 1;
    }

    printf("Laberinto cargado con %d escenarios.\n", total_escenarios);
    describir_escenarios(grafo, total_escenarios);

    int opcion;
    do {
        printf("\n====== Menu Principal ======\n");
        printf("1. Iniciar Partida\n");
        printf("2. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                jugar_partida(grafo, total_escenarios);
                break;
            case 2:
                printf("Hasta la proxima, aventurero!\n");
                break;
            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 2);

    free(grafo);
    return 0;
}
