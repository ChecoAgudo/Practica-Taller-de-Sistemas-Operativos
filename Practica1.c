#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ==================== ESTRUCTURAS ==================== */

typedef struct {
    int id;
    int id_proceso;
    int direccion_inicio;
    int tamano;
    int estado;
} Particion;

typedef struct Proceso {
    int id;
    int tamano_requerido;
    int direccion_asignada;
    struct Proceso *siguiente;
} Proceso;

/* ==================== VARIABLES GLOBALES ==================== */

int *memoria = NULL;
Particion *tabla_particiones = NULL;
Proceso *lista_procesos = NULL;
Proceso *lista_procesos_terminados = NULL;
int num_particiones = 0;
int tamano_total_memoria = 0;
int tamano_particion = 0;
int contador_procesos = 1;

/* ==================== PROTOTIPOS DE FUNCIONES ==================== */

void inicializar_memoria(void);
void agregar_proceso(int id, int tamano, int direccion);
int eliminar_proceso(int id);
Proceso* buscar_proceso(int id);
void agregar_proceso_terminado(int id, int tamano, int direccion);
float calcular_fragmentacion(int tamano_particion, int tamano_proceso);
void crear_proceso(void);
void cerrar_proceso(void);
void mostrar_tabla_procesos(void);
void mostrar_tabla_particiones(void);
void mostrar_memoria(void);
void mostrar_procesos_terminados(void);
void liberar_recursos(void);

/* ==================== FUNCION PRINCIPAL ==================== */

int main(void) {
    int opcion;
    
    srand((unsigned int)time(NULL));
    
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  SIMULADOR DE GESTION DE MEMORIA PARTICIONADA FIJA     ║\n");
    printf("║  Algoritmo: Primer Ajuste (First Fit)                  ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    inicializar_memoria();
    
    do {
        printf("\n\n╔════════════════ MENU PRINCIPAL ════════════════╗\n");
        printf("║  1. Crear Proceso                              ║\n");
        printf("║  2. Cerrar Proceso                             ║\n");
        printf("║  3. Ver Tabla de Procesos                      ║\n");
        printf("║  4. Ver Tabla de Particiones                   ║\n");
        printf("║  5. Ver Memoria Asignada                       ║\n");
        printf("║  6. Ver Procesos Terminados                    ║\n");
        printf("║  0. Salir                                      ║\n");
        printf("╚════════════════════════════════════════════════╝\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);
        
        switch(opcion) {
            case 1:
                crear_proceso();
                break;
            case 2:
                cerrar_proceso();
                break;
            case 3:
                mostrar_tabla_procesos();
                break;
            case 4:
                mostrar_tabla_particiones();
                break;
            case 5:
                mostrar_memoria();
                break;
            case 6:
                mostrar_procesos_terminados();
                break;
            case 0:
                printf("\nSaliendo del programa...\n");
                break;
            default:
                printf("\nOpcion invalida. Intente nuevamente.\n");
        }
        
    } while(opcion != 0);
    
    liberar_recursos();
    
    return 0;
}

/* ==================== FUNCION DE INICIALIZACION ==================== */

void inicializar_memoria(void) {
    int i;
    
    printf("\n=== INICIALIZACION DEL SISTEMA DE MEMORIA ===\n");
    
    do {
        printf("Ingrese el tamaño total de memoria (KB): ");
        scanf("%d", &tamano_total_memoria);
        if (tamano_total_memoria <= 0) {
            printf("Error: El tamaño debe ser mayor a 0.\n");
        }
    } while (tamano_total_memoria <= 0);
    
    do {
        printf("Ingrese el tamaño de cada particion (KB): ");
        scanf("%d", &tamano_particion);
        if (tamano_particion <= 0) {
            printf("Error: El tamaño debe ser mayor a 0.\n");
        } else if (tamano_particion > tamano_total_memoria) {
            printf("Error: El tamaño de particion no puede ser mayor que la memoria total.\n");
            tamano_particion = 0;
        }
    } while (tamano_particion <= 0);
    
    num_particiones = tamano_total_memoria / tamano_particion;
    
    if (num_particiones == 0) {
        printf("Error: No se pueden crear particiones con estos valores.\n");
        exit(1);
    }
    
    printf("\n>>> Se crearan %d particiones de %d KB cada una.\n", 
           num_particiones, tamano_particion);
    
    memoria = (int *)malloc(tamano_total_memoria * sizeof(int));
    if (memoria == NULL) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }
    
    for (i = 0; i < tamano_total_memoria; i++) {
        memoria[i] = -1;
    }
    
    tabla_particiones = (Particion *)malloc(num_particiones * sizeof(Particion));
    if (tabla_particiones == NULL) {
        printf("Error: No se pudo asignar memoria para la tabla de particiones.\n");
        free(memoria);
        exit(1);
    }
    
    for (i = 0; i < num_particiones; i++) {
        tabla_particiones[i].id = i;
        tabla_particiones[i].id_proceso = -1;
        tabla_particiones[i].direccion_inicio = i * tamano_particion;
        tabla_particiones[i].tamano = tamano_particion;
        tabla_particiones[i].estado = 0;
    }
    
    printf(">>> Memoria inicializada exitosamente.\n");
}

/* ==================== FUNCIONES DE LISTA ENLAZADA ==================== */

void agregar_proceso(int id, int tamano, int direccion) {
    Proceso *nuevo;
    Proceso *actual;
    
    nuevo = (Proceso *)malloc(sizeof(Proceso));
    if (nuevo == NULL) {
        printf("Error: No se pudo asignar memoria para el proceso.\n");
        return;
    }
    
    nuevo->id = id;
    nuevo->tamano_requerido = tamano;
    nuevo->direccion_asignada = direccion;
    nuevo->siguiente = NULL;
    
    if (lista_procesos == NULL) {
        lista_procesos = nuevo;
    } else {
        actual = lista_procesos;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo;
    }
}

int eliminar_proceso(int id) {
    Proceso *actual;
    Proceso *anterior;
    Proceso *temp;
    
    if (lista_procesos == NULL) {
        return 0;
    }
    
    actual = lista_procesos;
    anterior = NULL;
    
    while (actual != NULL && actual->id != id) {
        anterior = actual;
        actual = actual->siguiente;
    }
    
    if (actual == NULL) {
        return 0;
    }
    
    if (anterior == NULL) {
        lista_procesos = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }
    
    temp = actual;
    free(temp);
    return 1;
}

Proceso* buscar_proceso(int id) {
    Proceso *actual;
    
    actual = lista_procesos;
    while (actual != NULL) {
        if (actual->id == id) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

void agregar_proceso_terminado(int id, int tamano, int direccion) {
    Proceso *nuevo;
    Proceso *actual;
    
    nuevo = (Proceso *)malloc(sizeof(Proceso));
    if (nuevo == NULL) {
        printf("Error: No se pudo asignar memoria para el proceso terminado.\n");
        return;
    }
    
    nuevo->id = id;
    nuevo->tamano_requerido = tamano;
    nuevo->direccion_asignada = direccion;
    nuevo->siguiente = NULL;
    
    if (lista_procesos_terminados == NULL) {
        lista_procesos_terminados = nuevo;
    } else {
        actual = lista_procesos_terminados;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo;
    }
}

/* ==================== FUNCIONES AUXILIARES ==================== */

float calcular_fragmentacion(int tamano_particion, int tamano_proceso) {
    int fragmentacion;
    
    if (tamano_proceso == 0) {
        return 0.0;
    }
    
    fragmentacion = tamano_particion - tamano_proceso;
    return (fragmentacion * 100.0) / tamano_particion;
}

/* ==================== FUNCIONES DE OPERACIONES ==================== */

void crear_proceso(void) {
    int tamano_proceso;
    int particion_asignada;
    int i;
    int dir_inicio;
    float fragmentacion;
    /*Esta linea es la que define el tamaño maximo del proceso aleatorio*/
    tamano_proceso = (rand() % (tamano_particion+0)) + 1;
    
    printf("\n--- CREANDO NUEVO PROCESO ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamaño requerido: %d KB\n", (tamano_proceso));
    
    particion_asignada = -1;
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == 0 && 
            tabla_particiones[i].tamano >= tamano_proceso) {
            particion_asignada = i;
            break;
        }
    }
    
    if (particion_asignada == -1) {
        printf("ERROR: No hay particiones disponibles para el proceso.\n");
        printf("       El proceso no pudo ser creado.\n");
        return;
    }
    
    tabla_particiones[particion_asignada].estado = 1;
    tabla_particiones[particion_asignada].id_proceso = contador_procesos;
    
    dir_inicio = tabla_particiones[particion_asignada].direccion_inicio;
    for (i = 0; i < tamano_proceso; i++) {
        memoria[dir_inicio + i] = contador_procesos;
    }
    
    agregar_proceso(contador_procesos, tamano_proceso, dir_inicio);
    
    fragmentacion = calcular_fragmentacion(tamano_particion, tamano_proceso);
    
    printf(">>> PROCESO CREADO EXITOSAMENTE <<<\n");
    printf("    Asignado a la particion: %d\n", particion_asignada);
    printf("    Direccion de inicio: %d\n", dir_inicio);
    printf("    Fragmentacion interna: %.2f%%\n", fragmentacion);
    
    contador_procesos++;
}

void cerrar_proceso(void) {
    int id_proceso;
    Proceso *proceso;
    int particion_encontrada;
    int i;
    int dir_inicio;
    int tamano_proceso;
    
    printf("\n--- CERRAR PROCESO ---\n");
    printf("Ingrese el ID del proceso a cerrar: ");
    scanf("%d", &id_proceso);
    
    proceso = buscar_proceso(id_proceso);
    if (proceso == NULL) {
        printf("ERROR: No existe un proceso con ID %d.\n", id_proceso);
        return;
    }
    
    tamano_proceso = proceso->tamano_requerido;
    dir_inicio = proceso->direccion_asignada;
    
    particion_encontrada = -1;
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].id_proceso == id_proceso) {
            particion_encontrada = i;
            break;
        }
    }
    
    if (particion_encontrada == -1) {
        printf("ERROR: No se encontro la particion del proceso.\n");
        return;
    }
    
    tabla_particiones[particion_encontrada].estado = 0;
    tabla_particiones[particion_encontrada].id_proceso = -1;
    
    for (i = 0; i < tamano_particion; i++) {
        if (memoria[dir_inicio + i] == id_proceso) {
            memoria[dir_inicio + i] = -1;
        }
    }
    
    agregar_proceso_terminado(id_proceso, tamano_proceso, dir_inicio);
    
    eliminar_proceso(id_proceso);
    
    printf(">>> PROCESO %d CERRADO EXITOSAMENTE <<<\n", id_proceso);
    printf("    Particion %d liberada.\n", particion_encontrada);
}

/* ==================== FUNCIONES DE VISUALIZACION ==================== */

void mostrar_tabla_procesos(void) {
    Proceso *actual;
    
    printf("\n========== TABLA DE PROCESOS ==========\n");
    printf("%-10s %-15s %-20s\n", "ID", "Tamaño (KB)", "Direccion Inicio");
    printf("--------------------------------------------------\n");
    
    if (lista_procesos == NULL) {
        printf("No hay procesos activos.\n");
    } else {
        actual = lista_procesos;
        while (actual != NULL) {
            printf("%-10d %-15d %-20d\n", 
                   actual->id, 
                   actual->tamano_requerido, 
                   actual->direccion_asignada);
            actual = actual->siguiente;
        }
    }
    printf("=======================================\n");
}

void mostrar_tabla_particiones(void) {
    int i;
    char *estado_str;
    Proceso *proc;
    float frag;
    
    printf("\n==================== TABLA DE PARTICIONES ====================\n");
    printf("%-8s %-12s %-18s %-12s %-12s %-15s\n", 
           "Part ID", "Estado", "Dir. Inicio (KB)", "Tamaño (KB)", "ID Proceso", "Fragment. (%)");
    printf("---------------------------------------------------------------\n");
    
    for (i = 0; i < num_particiones; i++) {
        estado_str = tabla_particiones[i].estado == 0 ? "Libre" : "Ocupada";
        
        if (tabla_particiones[i].estado == 1) {
            proc = buscar_proceso(tabla_particiones[i].id_proceso);
            frag = 0.0;
            if (proc != NULL) {
                frag = calcular_fragmentacion(tabla_particiones[i].tamano, 
                                              proc->tamano_requerido);
            }
            
            printf("%-8d %-12s %-18d %-12d %-12d %-15.2f\n",
                   tabla_particiones[i].id,
                   estado_str,
                   tabla_particiones[i].direccion_inicio,
                   tabla_particiones[i].tamano,
                   tabla_particiones[i].id_proceso,
                   frag);
        } else {
            printf("%-8d %-12s %-18d %-12d %-12s %-15s\n",
                   tabla_particiones[i].id,
                   estado_str,
                   tabla_particiones[i].direccion_inicio,
                   tabla_particiones[i].tamano,
                   "---",
                   "---");
        }
    }
    printf("===============================================================\n");
}

void mostrar_memoria(void) {
    int i;
    
    printf("\n========== VISUALIZACION DE MEMORIA ==========\n");
    printf("Leyenda: [0] = Libre, [N] = Proceso ID N\n");
    printf("----------------------------------------------\n");
    
    for (i = 0; i < tamano_total_memoria; i++) {
        if (i % 20 == 0 && i > 0) {
            printf("\n");
        }
        
        if (memoria[i] == -1) {
            printf("[0] ");
        } else {
            printf("[1] ", memoria[i]);
        }
    }
    
    printf("\n==============================================\n");
}

void mostrar_procesos_terminados(void) {
    Proceso *actual;
    int contador;
    
    printf("\n========== PROCESOS TERMINADOS ==========\n");
    printf("%-10s %-15s %-20s\n", "ID", "Tamaño (KB)", "Direccion Inicio");
    printf("--------------------------------------------------\n");
    
    if (lista_procesos_terminados == NULL) {
        printf("No hay procesos terminados.\n");
    } else {
        actual = lista_procesos_terminados;
        contador = 0;
        while (actual != NULL) {
            printf("%-10d %-15d %-20d\n", 
                   actual->id, 
                   actual->tamano_requerido, 
                   actual->direccion_asignada);
            actual = actual->siguiente;
            contador++;
        }
        printf("\nTotal de procesos terminados: %d\n", contador);
    }
    printf("=========================================\n");
}

/* ==================== FUNCION DE LIMPIEZA ==================== */

void liberar_recursos(void) {
    Proceso *actual;
    Proceso *temp;
    
    actual = lista_procesos;
    while (actual != NULL) {
        temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    
    actual = lista_procesos_terminados;
    while (actual != NULL) {
        temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    
    if (memoria != NULL) {
        free(memoria);
    }
    if (tabla_particiones != NULL) {
        free(tabla_particiones);
    }
    
    printf("\n>>> Recursos liberados. Programa finalizado.\n");
}