/*
 * Practica
 *Integrantes: Rodrigo Checo Agudo y Diego Aimar Calahuana Choque
 *Fecha: 29 de enero de 2026
 *Descripcion: Simulador de gestion de memoria particionada fija
 *Materia: Taller de Sistemas Operativos
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ==================== ESTRUCTURAS ==================== */

/**
 * Estructura que representa una particion de memoria
 */
typedef struct {
    int id;                 /* ID unico de la particion */
    int id_proceso;         /* ID del proceso asignado (-1 si esta libre) */
    int direccion_inicio;   /* Direccion de inicio en memoria */
    int tamano;             /* Tamano de la particion */
    int estado;             /* 0 = Libre, 1 = Ocupada */
} Particion;

/**
 * Estructura que representa un proceso (nodo de lista enlazada)
 */
typedef struct Proceso {
    int id;                     /* ID unico del proceso */
    int tamano_requerido;       /* Tamano que requiere el proceso */
    int direccion_asignada;     /* Direccion donde fue asignado */
    struct Proceso *siguiente;  /* Puntero al siguiente nodo */
} Proceso;

/* ==================== VARIABLES GLOBALES ==================== */

int *memoria = NULL;              /* Vector que simula la RAM */
Particion *tabla_particiones = NULL;  /* Arreglo de particiones */
Proceso *lista_procesos = NULL;   /* Lista enlazada de procesos */
int num_particiones = 0;          /* Numero total de particiones */
int tamano_total_memoria = 0;     /* Tamano total de memoria */
int tamano_particion = 0;         /* Tamano de cada particion */
int contador_procesos = 1;        /* Contador para asignar IDs a procesos */

/* ==================== PROTOTIPOS DE FUNCIONES ==================== */

void agregar_proceso(int id, int tamano, int direccion);
int eliminar_proceso(int id);
Proceso* buscar_proceso(int id);
void inicializar_memoria(void);
float calcular_fragmentacion(int tamano_particion, int tamano_proceso);
void crear_proceso(void);
void cerrar_proceso(void);
void mostrar_tabla_procesos(void);
void mostrar_tabla_particiones(void);
void mostrar_memoria(void);
void liberar_recursos(void);

/* ==================== FUNCIONES DE LISTA ENLAZADA ==================== */

/**
 * Agrega un nuevo proceso a la lista enlazada
 */
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
    
    /* Si la lista esta vacia, el nuevo proceso es el primero */
    if (lista_procesos == NULL) {
        lista_procesos = nuevo;
    } else {
        /* Agregar al final de la lista */
        actual = lista_procesos;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo;
    }
}

/**
 * Elimina un proceso de la lista enlazada por su ID
 */
int eliminar_proceso(int id) {
    Proceso *actual;
    Proceso *anterior;
    Proceso *temp;
    
    if (lista_procesos == NULL) {
        return 0;  /* Lista vacia */
    }
    
    actual = lista_procesos;
    anterior = NULL;
    
    /* Buscar el proceso en la lista */
    while (actual != NULL && actual->id != id) {
        anterior = actual;
        actual = actual->siguiente;
    }
    
    /* Si no se encontro el proceso */
    if (actual == NULL) {
        return 0;
    }
    
    /* Si es el primer nodo */
    if (anterior == NULL) {
        lista_procesos = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }
    
    temp = actual;
    free(temp);
    return 1;  /* Eliminacion exitosa */
}

/**
 * Busca un proceso en la lista por su ID
 */
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

/* ==================== FUNCIONES DE GESTION DE MEMORIA ==================== */

/**
 * Inicializa el sistema de memoria particionada
 */
void inicializar_memoria(void) {
    int i;
    
    printf("\n=== INICIALIZACION DEL SISTEMA DE MEMORIA ===\n");
    
    /* Solicitar tamano total de memoria */
    do {
        printf("Ingrese el tamano total de memoria (KB): ");
        scanf("%d", &tamano_total_memoria);
        if (tamano_total_memoria <= 0) {
            printf("Error: El tamano debe ser mayor a 0.\n");
        }
    } while (tamano_total_memoria <= 0);
    
    /* Solicitar tamano de cada particion */
    do {
        printf("Ingrese el tamano de cada particion (KB): ");
        scanf("%d", &tamano_particion);
        if (tamano_particion <= 0) {
            printf("Error: El tamano debe ser mayor a 0.\n");
        } else if (tamano_particion > tamano_total_memoria) {
            printf("Error: El tamano de particion no puede ser mayor que la memoria total.\n");
            tamano_particion = 0;
        }
    } while (tamano_particion <= 0);
    
    /* Calcular numero de particiones */
    num_particiones = tamano_total_memoria / tamano_particion;
    
    if (num_particiones == 0) {
        printf("Error: No se pueden crear particiones con estos valores.\n");
        exit(1);
    }
    
    printf("\n>>> Se crearan %d particiones de %d KB cada una.\n", 
           num_particiones, tamano_particion);
    
    /* Asignar memoria para el vector de RAM */
    memoria = (int *)malloc(tamano_total_memoria * sizeof(int));
    if (memoria == NULL) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }
    
    /* Inicializar vector de memoria con -1 (indica espacio libre) */
    for (i = 0; i < tamano_total_memoria; i++) {
        memoria[i] = -1;
    }
    
    /* Crear tabla de particiones */
    tabla_particiones = (Particion *)malloc(num_particiones * sizeof(Particion));
    if (tabla_particiones == NULL) {
        printf("Error: No se pudo asignar memoria para la tabla de particiones.\n");
        free(memoria);
        exit(1);
    }
    
    /* Inicializar cada particion */
    for (i = 0; i < num_particiones; i++) {
        tabla_particiones[i].id = i;
        tabla_particiones[i].id_proceso = -1;  /* -1 indica libre */
        tabla_particiones[i].direccion_inicio = i * tamano_particion;
        tabla_particiones[i].tamano = tamano_particion;
        tabla_particiones[i].estado = 0;  /* 0 = Libre */
    }
    
    printf(">>> Memoria inicializada exitosamente.\n");
}

/**
 * Calcula el porcentaje de fragmentacion interna de una particion
 */
float calcular_fragmentacion(int tamano_particion, int tamano_proceso) {
    int fragmentacion;
    
    if (tamano_proceso == 0) {
        return 0.0;
    }
    
    fragmentacion = tamano_particion - tamano_proceso;
    return (fragmentacion * 100.0) / tamano_particion;
}

/**
 * Crea un nuevo proceso usando el algoritmo Primer Ajuste
 */
void crear_proceso(void) {
    int tamano_proceso;
    int particion_asignada;
    int i;
    int dir_inicio;
    float fragmentacion;
    
    /* Generar tamano aleatorio para el proceso (entre 1 y el tamano de particion) */
    tamano_proceso = (rand() % tamano_particion) + 1;
    
    printf("\n--- CREANDO NUEVO PROCESO ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamano requerido: %d KB\n", tamano_proceso);
    
    /* Algoritmo Primer Ajuste: buscar la primera particion libre donde quepa */
    particion_asignada = -1;
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == 0 && 
            tabla_particiones[i].tamano >= tamano_proceso) {
            particion_asignada = i;
            break;  /* Primer ajuste: tomar la primera que sirva */
        }
    }
    
    /* Verificar si se encontro una particion */
    if (particion_asignada == -1) {
        printf("ERROR: No hay particiones disponibles para el proceso.\n");
        printf("       El proceso no pudo ser creado.\n");
        return;
    }
    
    /* Asignar el proceso a la particion */
    tabla_particiones[particion_asignada].estado = 1;  /* Ocupada */
    tabla_particiones[particion_asignada].id_proceso = contador_procesos;
    
    /* Actualizar el vector de memoria con el ID del proceso */
    dir_inicio = tabla_particiones[particion_asignada].direccion_inicio;
    for (i = 0; i < tamano_proceso; i++) {
        memoria[dir_inicio + i] = contador_procesos;
    }
    
    /* Agregar proceso a la lista enlazada */
    agregar_proceso(contador_procesos, tamano_proceso, dir_inicio);
    
    /* Calcular fragmentacion interna */
    fragmentacion = calcular_fragmentacion(tamano_particion, tamano_proceso);
    
    printf(">>> PROCESO CREADO EXITOSAMENTE <<<\n");
    printf("    Asignado a la particion: %d\n", particion_asignada);
    printf("    Direccion de inicio: %d\n", dir_inicio);
    printf("    Fragmentacion interna: %.2f%%\n", fragmentacion);
    
    contador_procesos++;  /* Incrementar contador para el proximo proceso */
}

/**
 * Cierra un proceso y libera la particion
 */
void cerrar_proceso(void) {
    int id_proceso;
    Proceso *proceso;
    int particion_encontrada;
    int i;
    int dir_inicio;
    
    printf("\n--- CERRAR PROCESO ---\n");
    printf("Ingrese el ID del proceso a cerrar: ");
    scanf("%d", &id_proceso);
    
    /* Buscar el proceso en la lista */
    proceso = buscar_proceso(id_proceso);
    if (proceso == NULL) {
        printf("ERROR: No existe un proceso con ID %d.\n", id_proceso);
        return;
    }
    
    /* Buscar la particion que contiene el proceso */
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
    
    /* Liberar la particion */
    tabla_particiones[particion_encontrada].estado = 0;  /* Libre */
    tabla_particiones[particion_encontrada].id_proceso = -1;
    
    /* Limpiar el vector de memoria */
    dir_inicio = tabla_particiones[particion_encontrada].direccion_inicio;
    for (i = 0; i < tamano_particion; i++) {
        if (memoria[dir_inicio + i] == id_proceso) {
            memoria[dir_inicio + i] = -1;  /* Marcar como libre */
        }
    }
    
    /* Eliminar el proceso de la lista enlazada */
    eliminar_proceso(id_proceso);
    
    printf(">>> PROCESO %d CERRADO EXITOSAMENTE <<<\n", id_proceso);
    printf("    Particion %d liberada.\n", particion_encontrada);
}

/**
 * Muestra la tabla de procesos activos
 */
void mostrar_tabla_procesos(void) {
    Proceso *actual;
    
    printf("\n========== TABLA DE PROCESOS ==========\n");
    printf("%-10s %-15s %-20s\n", "ID", "Tamano (KB)", "Direccion Inicio");
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

/**
 * Muestra la tabla de particiones con fragmentacion
 */
void mostrar_tabla_particiones(void) {
    int i;
    char *estado_str;
    Proceso *proc;
    float frag;
    
    printf("\n==================== TABLA DE PARTICIONES ====================\n");
    printf("%-8s %-12s %-18s %-12s %-12s %-15s\n", 
           "Part ID", "Estado", "Dir. Inicio (KB)", "Tamano (KB)", "ID Proceso", "Fragment. (%)");
    printf("---------------------------------------------------------------\n");
    
    for (i = 0; i < num_particiones; i++) {
        estado_str = tabla_particiones[i].estado == 0 ? "Libre" : "Ocupada";
        
        if (tabla_particiones[i].estado == 1) {
            /* Calcular fragmentacion para particiones ocupadas */
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

/**
 * Muestra visualmente el vector de memoria
 */
void mostrar_memoria(void) {
    int i;
    
    printf("\n========== VISUALIZACION DE MEMORIA ==========\n");
    printf("Leyenda: [-1] = Libre, [N] = Proceso ID N\n");
    printf("----------------------------------------------\n");
    
    for (i = 0; i < tamano_total_memoria; i++) {
        if (i % 20 == 0 && i > 0) {
            printf("\n");  /* Salto de linea cada 20 elementos */
        }
        
        if (memoria[i] == -1) {
            printf("[ -- ] ");
        } else {
            printf("[P%-3d] ", memoria[i]);
        }
    }
    
    printf("\n==============================================\n");
}

/**
 * Libera toda la memoria asignada dinamicamente
 */
void liberar_recursos(void) {
    Proceso *actual;
    Proceso *temp;
    
    /* Liberar lista de procesos */
    actual = lista_procesos;
    while (actual != NULL) {
        temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    
    /* Liberar vector de memoria y tabla de particiones */
    if (memoria != NULL) {
        free(memoria);
    }
    if (tabla_particiones != NULL) {
        free(tabla_particiones);
    }
    
    printf("\n>>> Recursos liberados. Programa finalizado.\n");
}

/* ==================== FUNCION PRINCIPAL ==================== */

int main(void) {
    int opcion;
    
    /* Inicializar generador de numeros aleatorios */
    srand((unsigned int)time(NULL));
    
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  SIMULADOR DE GESTION DE MEMORIA PARTICIONADA FIJA     ║\n");
    printf("║  Algoritmo: Primer Ajuste (First Fit)                  ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    /* Inicializar el sistema de memoria */
    inicializar_memoria();
    
    /* Menu interactivo */
    do {
        printf("\n\n╔════════════════ MENU PRINCIPAL ════════════════╗\n");
        printf("║  1. Crear Proceso                              ║\n");
        printf("║  2. Cerrar Proceso                             ║\n");
        printf("║  3. Ver Tabla de Procesos                      ║\n");
        printf("║  4. Ver Tabla de Particiones                   ║\n");
        printf("║  5. Ver Memoria Asignada                       ║\n");
        printf("║  6. Ver Todas las Tablas                       ║\n");
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
                mostrar_tabla_procesos();
                mostrar_tabla_particiones();
                mostrar_memoria();
                break;
            case 0:
                printf("\nSaliendo del programa...\n");
                break;
            default:
                printf("\nOpcion invalida. Intente nuevamente.\n");
        }
        
    } while(opcion != 0);
    
    /* Liberar recursos antes de salir */
    liberar_recursos();
    
    return 0;
}
