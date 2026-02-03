/*
 * SIMULADOR COMPLETO DE GESTION DE MEMORIA
 * Integra 4 Algoritmos: First Fit, Best Fit, Worst Fit, Buddy System
 * Compatible con ANSI C (C90)
 */

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
    int nivel;              /* Para Buddy System */
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
int max_particiones = 0;         /* Para Buddy System */
int algoritmo_seleccionado = 0;  /* 1=First Fit, 2=Best Fit, 3=Worst Fit, 4=Buddy */

/* ==================== PROTOTIPOS DE FUNCIONES ==================== */

void seleccionar_algoritmo(void);
void inicializar_memoria(void);
void agregar_proceso(int id, int tamano, int direccion);
int eliminar_proceso(int id);
Proceso* buscar_proceso(int id);
void agregar_proceso_terminado(int id, int tamano, int direccion);
float calcular_fragmentacion(int tamano_particion, int tamano_proceso);

/* Funciones para algoritmos clasicos */
void crear_proceso_first_fit(void);
void crear_proceso_best_fit(void);
void crear_proceso_worst_fit(void);

/* Funciones para Buddy System */
int es_potencia_de_2(int n);
int siguiente_potencia_de_2(int n);
int calcular_buddy(int indice, int tamano);
void dividir_bloque(int indice);
void fusionar_buddies(void);
void crear_proceso_buddy(void);
void cerrar_proceso_buddy(void);

/* Funciones generales */
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
    
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  SIMULADOR COMPLETO DE GESTION DE MEMORIA                     ║\n");
    printf("║  Algoritmos: First Fit, Best Fit, Worst Fit, Buddy System     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    /* Seleccionar algoritmo antes de inicializar */
    seleccionar_algoritmo();
    
    /* Inicializar memoria segun el algoritmo */
    inicializar_memoria();
    
    /* Menu principal */
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

/* ==================== SELECCION DE ALGORITMO ==================== */

void seleccionar_algoritmo(void) {
    printf("\n=== SELECCION DE ALGORITMO DE ASIGNACION ===\n\n");
    printf("1. Primer Ajuste (First Fit)\n");
    printf("   - Asigna en la PRIMERA particion libre que quepa\n");
    printf("   - Rapido, pero puede fragmentar\n\n");
    
    printf("2. Mejor Ajuste (Best Fit)\n");
    printf("   - Asigna en la particion mas PEQUENA que quepa\n");
    printf("   - Minimiza desperdicio, pero mas lento\n\n");
    
    printf("3. Peor Ajuste (Worst Fit)\n");
    printf("   - Asigna en la particion mas GRANDE disponible\n");
    printf("   - Deja huecos grandes, evita fragmentos pequenos\n\n");
    
    printf("4. Sistema de Companeros (Buddy System)\n");
    printf("   - Division recursiva en potencias de 2\n");
    printf("   - Fusion automatica de buddies libres\n\n");
    
    do {
        printf("Seleccione el algoritmo (1-4): ");
        scanf("%d", &algoritmo_seleccionado);
        
        if (algoritmo_seleccionado < 1 || algoritmo_seleccionado > 4) {
            printf("Error: Opcion invalida. Intente nuevamente.\n");
        }
    } while (algoritmo_seleccionado < 1 || algoritmo_seleccionado > 4);
    
    printf("\n>>> Algoritmo seleccionado: ");
    switch(algoritmo_seleccionado) {
        case 1: printf("Primer Ajuste (First Fit)\n"); break;
        case 2: printf("Mejor Ajuste (Best Fit)\n"); break;
        case 3: printf("Peor Ajuste (Worst Fit)\n"); break;
        case 4: printf("Sistema de Companeros (Buddy System)\n"); break;
    }
}

/* ==================== INICIALIZACION DE MEMORIA ==================== */

void inicializar_memoria(void) {
    int i;
    int temp;
    
    printf("\n=== INICIALIZACION DEL SISTEMA DE MEMORIA ===\n");
    
    /* Solicitar tamano total */
    do {
        printf("Ingrese el tamano total de memoria (KB): ");
        scanf("%d", &tamano_total_memoria);
        if (tamano_total_memoria <= 0) {
            printf("Error: El tamano debe ser mayor a 0.\n");
        }
    } while (tamano_total_memoria <= 0);
    
    /* Para Buddy System, ajustar a potencia de 2 */
    if (algoritmo_seleccionado == 4) {
        if (!es_potencia_de_2(tamano_total_memoria)) {
            temp = siguiente_potencia_de_2(tamano_total_memoria);
            printf("Advertencia: %d no es potencia de 2. Se ajustara a %d KB.\n", 
                   tamano_total_memoria, temp);
            tamano_total_memoria = temp;
        }
        printf("\n>>> Memoria total: %d KB (potencia de 2)\n", tamano_total_memoria);
    }
    
    /* Para algoritmos clasicos, solicitar tamano de particion */
    if (algoritmo_seleccionado != 4) {
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
        
        num_particiones = tamano_total_memoria / tamano_particion;
        
        if (num_particiones == 0) {
            printf("Error: No se pueden crear particiones con estos valores.\n");
            exit(1);
        }
        
        printf("\n>>> Se crearan %d particiones de %d KB cada una.\n", 
               num_particiones, tamano_particion);
    }
    
    /* Asignar memoria para el vector de RAM */
    memoria = (int *)malloc(tamano_total_memoria * sizeof(int));
    if (memoria == NULL) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }
    
    /* Inicializar vector de memoria */
    for (i = 0; i < tamano_total_memoria; i++) {
        memoria[i] = -1;
    }
    
    /* Crear tabla de particiones segun el algoritmo */
    if (algoritmo_seleccionado == 4) {
        /* Buddy System: tabla dinamica grande */
        max_particiones = tamano_total_memoria * 2;
        tabla_particiones = (Particion *)malloc(max_particiones * sizeof(Particion));
        if (tabla_particiones == NULL) {
            printf("Error: No se pudo asignar memoria para la tabla de particiones.\n");
            free(memoria);
            exit(1);
        }
        
        /* Inicializar todas como invalidas */
        for (i = 0; i < max_particiones; i++) {
            tabla_particiones[i].id = -1;
            tabla_particiones[i].id_proceso = -1;
            tabla_particiones[i].direccion_inicio = -1;
            tabla_particiones[i].tamano = 0;
            tabla_particiones[i].estado = -1;
            tabla_particiones[i].nivel = -1;
        }
        
        /* Crear particion inicial */
        tabla_particiones[0].id = 0;
        tabla_particiones[0].id_proceso = -1;
        tabla_particiones[0].direccion_inicio = 0;
        tabla_particiones[0].tamano = tamano_total_memoria;
        tabla_particiones[0].estado = 0;
        tabla_particiones[0].nivel = 0;
        num_particiones = 1;
        
        printf(">>> Sistema Buddy inicializado con un bloque de %d KB.\n", tamano_total_memoria);
    } else {
        /* Algoritmos clasicos: particiones fijas */
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
            tabla_particiones[i].nivel = 0;
        }
        
        printf(">>> Memoria inicializada exitosamente.\n");
    }
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

/* ==================== ALGORITMOS CLASICOS ==================== */

void crear_proceso_first_fit(void) {
    int tamano_proceso;
    int particion_asignada;
    int i;
    int dir_inicio;
    float fragmentacion;
    
    tamano_proceso = (rand() % tamano_particion) + 1;
    
    printf("\n--- CREANDO NUEVO PROCESO (FIRST FIT) ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamano requerido: %d KB\n", tamano_proceso);
    
    /* Buscar PRIMERA particion libre que quepa */
    particion_asignada = -1;
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == 0 && 
            tabla_particiones[i].tamano >= tamano_proceso) {
            particion_asignada = i;
            break;  /* Primera que sirve */
        }
    }
    
    if (particion_asignada == -1) {
        printf("ERROR: No hay particiones disponibles.\n");
        return;
    }
    
    /* Asignar proceso */
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

void crear_proceso_best_fit(void) {
    int tamano_proceso;
    int mejor_particion;
    int menor_desperdicio;
    int desperdicio;
    int i;
    int dir_inicio;
    float fragmentacion;
    
    tamano_proceso = (rand() % tamano_particion) + 1;
    
    printf("\n--- CREANDO NUEVO PROCESO (BEST FIT) ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamano requerido: %d KB\n", tamano_proceso);
    
    /* Buscar particion con MENOR desperdicio */
    mejor_particion = -1;
    menor_desperdicio = tamano_total_memoria + 1;
    
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == 0 && 
            tabla_particiones[i].tamano >= tamano_proceso) {
            desperdicio = tabla_particiones[i].tamano - tamano_proceso;
            if (desperdicio < menor_desperdicio) {
                menor_desperdicio = desperdicio;
                mejor_particion = i;
            }
        }
    }
    
    if (mejor_particion == -1) {
        printf("ERROR: No hay particiones disponibles.\n");
        return;
    }
    
    /* Asignar proceso */
    tabla_particiones[mejor_particion].estado = 1;
    tabla_particiones[mejor_particion].id_proceso = contador_procesos;
    
    dir_inicio = tabla_particiones[mejor_particion].direccion_inicio;
    for (i = 0; i < tamano_proceso; i++) {
        memoria[dir_inicio + i] = contador_procesos;
    }
    
    agregar_proceso(contador_procesos, tamano_proceso, dir_inicio);
    
    fragmentacion = calcular_fragmentacion(tamano_particion, tamano_proceso);
    
    printf(">>> PROCESO CREADO EXITOSAMENTE <<<\n");
    printf("    Asignado a la particion: %d (menor desperdicio: %d KB)\n", 
           mejor_particion, menor_desperdicio);
    printf("    Direccion de inicio: %d\n", dir_inicio);
    printf("    Fragmentacion interna: %.2f%%\n", fragmentacion);
    
    contador_procesos++;
}

void crear_proceso_worst_fit(void) {
    int tamano_proceso;
    int peor_particion;
    int mayor_espacio;
    int i;
    int dir_inicio;
    float fragmentacion;
    
    tamano_proceso = (rand() % tamano_particion) + 1;
    
    printf("\n--- CREANDO NUEVO PROCESO (WORST FIT) ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamano requerido: %d KB\n", tamano_proceso);
    
    /* Buscar particion MAS GRANDE disponible */
    peor_particion = -1;
    mayor_espacio = -1;
    
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == 0 && 
            tabla_particiones[i].tamano >= tamano_proceso &&
            tabla_particiones[i].tamano > mayor_espacio) {
            mayor_espacio = tabla_particiones[i].tamano;
            peor_particion = i;
        }
    }
    
    if (peor_particion == -1) {
        printf("ERROR: No hay particiones disponibles.\n");
        return;
    }
    
    /* Asignar proceso */
    tabla_particiones[peor_particion].estado = 1;
    tabla_particiones[peor_particion].id_proceso = contador_procesos;
    
    dir_inicio = tabla_particiones[peor_particion].direccion_inicio;
    for (i = 0; i < tamano_proceso; i++) {
        memoria[dir_inicio + i] = contador_procesos;
    }
    
    agregar_proceso(contador_procesos, tamano_proceso, dir_inicio);
    
    fragmentacion = calcular_fragmentacion(tamano_particion, tamano_proceso);
    
    printf(">>> PROCESO CREADO EXITOSAMENTE <<<\n");
    printf("    Asignado a la particion: %d (particion mas grande)\n", peor_particion);
    printf("    Direccion de inicio: %d\n", dir_inicio);
    printf("    Fragmentacion interna: %.2f%%\n", fragmentacion);
    
    contador_procesos++;
}

/* ==================== BUDDY SYSTEM - FUNCIONES AUXILIARES ==================== */

int es_potencia_de_2(int n) {
    if (n <= 0) {
        return 0;
    }
    return (n & (n - 1)) == 0;
}

int siguiente_potencia_de_2(int n) {
    int potencia;
    
    potencia = 1;
    while (potencia < n) {
        potencia *= 2;
    }
    return potencia;
}

int calcular_buddy(int indice, int tamano) {
    int i;
    int dir_actual;
    int dir_buddy;
    
    dir_actual = tabla_particiones[indice].direccion_inicio;
    
    if ((dir_actual / tamano) % 2 == 0) {
        dir_buddy = dir_actual + tamano;
    } else {
        dir_buddy = dir_actual - tamano;
    }
    
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado != -1 &&
            tabla_particiones[i].direccion_inicio == dir_buddy &&
            tabla_particiones[i].tamano == tamano) {
            return i;
        }
    }
    
    return -1;
}

void dividir_bloque(int indice) {
    int nuevo_tamano;
    int hijo_izq_idx;
    int hijo_der_idx;
    
    if (num_particiones >= max_particiones - 2) {
        printf("Error: No hay espacio para mas particiones.\n");
        return;
    }
    
    nuevo_tamano = tabla_particiones[indice].tamano / 2;
    
    hijo_izq_idx = num_particiones;
    tabla_particiones[hijo_izq_idx].id = hijo_izq_idx;
    tabla_particiones[hijo_izq_idx].id_proceso = -1;
    tabla_particiones[hijo_izq_idx].direccion_inicio = tabla_particiones[indice].direccion_inicio;
    tabla_particiones[hijo_izq_idx].tamano = nuevo_tamano;
    tabla_particiones[hijo_izq_idx].estado = 0;
    tabla_particiones[hijo_izq_idx].nivel = tabla_particiones[indice].nivel + 1;
    num_particiones++;
    
    hijo_der_idx = num_particiones;
    tabla_particiones[hijo_der_idx].id = hijo_der_idx;
    tabla_particiones[hijo_der_idx].id_proceso = -1;
    tabla_particiones[hijo_der_idx].direccion_inicio = tabla_particiones[indice].direccion_inicio + nuevo_tamano;
    tabla_particiones[hijo_der_idx].tamano = nuevo_tamano;
    tabla_particiones[hijo_der_idx].estado = 0;
    tabla_particiones[hijo_der_idx].nivel = tabla_particiones[indice].nivel + 1;
    num_particiones++;
    
    tabla_particiones[indice].estado = -1;
}

void fusionar_buddies(void) {
    int i;
    int buddy_idx;
    int fusionado;
    
    do {
        fusionado = 0;
        
        for (i = 0; i < num_particiones; i++) {
            if (tabla_particiones[i].estado != 0) {
                continue;
            }
            
            buddy_idx = calcular_buddy(i, tabla_particiones[i].tamano);
            
            if (buddy_idx != -1 && tabla_particiones[buddy_idx].estado == 0) {
                tabla_particiones[i].estado = -1;
                tabla_particiones[buddy_idx].estado = -1;
                
                if (tabla_particiones[i].direccion_inicio < tabla_particiones[buddy_idx].direccion_inicio) {
                    tabla_particiones[i].tamano *= 2;
                    tabla_particiones[i].estado = 0;
                    tabla_particiones[i].nivel--;
                } else {
                    tabla_particiones[buddy_idx].tamano *= 2;
                    tabla_particiones[buddy_idx].estado = 0;
                    tabla_particiones[buddy_idx].nivel--;
                }
                
                fusionado = 1;
                break;
            }
        }
    } while (fusionado);
}

/* ==================== BUDDY SYSTEM - CREAR PROCESO ==================== */

void crear_proceso_buddy(void) {
    int tamano_proceso;
    int tamano_requerido;
    int i;
    int mejor_particion;
    int menor_tamano;
    int dir_inicio;
    float fragmentacion;
    
    tamano_proceso = (rand() % (tamano_total_memoria / 4)) + 1;
    tamano_requerido = siguiente_potencia_de_2(tamano_proceso);
    
    printf("\n--- CREANDO NUEVO PROCESO (BUDDY SYSTEM) ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamano solicitado: %d KB\n", tamano_proceso);
    printf("Tamano asignado (potencia de 2): %d KB\n", tamano_requerido);
    
    mejor_particion = -1;
    menor_tamano = tamano_total_memoria + 1;
    
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == 0 && 
            tabla_particiones[i].tamano >= tamano_requerido &&
            tabla_particiones[i].tamano < menor_tamano) {
            mejor_particion = i;
            menor_tamano = tabla_particiones[i].tamano;
        }
    }
    
    if (mejor_particion == -1) {
        printf("ERROR: No hay bloques disponibles.\n");
        return;
    }
    
    while (tabla_particiones[mejor_particion].tamano > tamano_requerido) {
        dividir_bloque(mejor_particion);
        
        for (i = 0; i < num_particiones; i++) {
            if (tabla_particiones[i].estado == 0 &&
                tabla_particiones[i].direccion_inicio == tabla_particiones[mejor_particion].direccion_inicio &&
                tabla_particiones[i].tamano == tabla_particiones[mejor_particion].tamano / 2) {
                mejor_particion = i;
                break;
            }
        }
    }
    
    tabla_particiones[mejor_particion].estado = 1;
    tabla_particiones[mejor_particion].id_proceso = contador_procesos;
    
    dir_inicio = tabla_particiones[mejor_particion].direccion_inicio;
    for (i = 0; i < tamano_proceso; i++) {
        memoria[dir_inicio + i] = contador_procesos;
    }
    
    agregar_proceso(contador_procesos, tamano_proceso, dir_inicio);
    
    fragmentacion = calcular_fragmentacion(tamano_requerido, tamano_proceso);
    
    printf(">>> PROCESO CREADO EXITOSAMENTE <<<\n");
    printf("    Asignado a la particion: %d\n", mejor_particion);
    printf("    Direccion de inicio: %d\n", dir_inicio);
    printf("    Tamano del bloque: %d KB\n", tabla_particiones[mejor_particion].tamano);
    printf("    Fragmentacion interna: %.2f%%\n", fragmentacion);
    
    contador_procesos++;
}

/* ==================== FUNCION CREAR PROCESO (DISPATCHER) ==================== */

void crear_proceso(void) {
    switch(algoritmo_seleccionado) {
        case 1:
            crear_proceso_first_fit();
            break;
        case 2:
            crear_proceso_best_fit();
            break;
        case 3:
            crear_proceso_worst_fit();
            break;
        case 4:
            crear_proceso_buddy();
            break;
        default:
            printf("Error: Algoritmo no valido.\n");
    }
}

/* ==================== CERRAR PROCESO ==================== */

void cerrar_proceso(void) {
    int id_proceso;
    Proceso *proceso;
    int particion_encontrada;
    int i;
    int dir_inicio;
    int tamano_proceso;
    int tamano_bloque;
    
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
    
    tamano_bloque = tabla_particiones[particion_encontrada].tamano;
    
    tabla_particiones[particion_encontrada].estado = 0;
    tabla_particiones[particion_encontrada].id_proceso = -1;
    
    for (i = 0; i < tamano_bloque; i++) {
        if (memoria[dir_inicio + i] == id_proceso) {
            memoria[dir_inicio + i] = -1;
        }
    }
    
    agregar_proceso_terminado(id_proceso, tamano_proceso, dir_inicio);
    eliminar_proceso(id_proceso);
    
    printf(">>> PROCESO %d CERRADO EXITOSAMENTE <<<\n", id_proceso);
    printf("    Particion %d liberada.\n", particion_encontrada);
    
    /* Si es Buddy System, intentar fusion */
    if (algoritmo_seleccionado == 4) {
        printf("    Tamano del bloque liberado: %d KB\n", tamano_bloque);
        printf(">>> Intentando fusionar buddies libres...\n");
        fusionar_buddies();
        printf(">>> Fusion completada.\n");
    }
}

/* ==================== FUNCIONES DE VISUALIZACION ==================== */

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

void mostrar_tabla_particiones(void) {
    int i;
    char *estado_str;
    Proceso *proc;
    float frag;
    
    printf("\n==================== TABLA DE PARTICIONES ====================\n");
    
    if (algoritmo_seleccionado == 4) {
        printf("%-8s %-12s %-18s %-12s %-12s %-8s %-15s\n", 
               "Part ID", "Estado", "Dir. Inicio (KB)", "Tamano (KB)", "ID Proceso", "Nivel", "Fragment. (%)");
    } else {
        printf("%-8s %-12s %-18s %-12s %-12s %-15s\n", 
               "Part ID", "Estado", "Dir. Inicio (KB)", "Tamano (KB)", "ID Proceso", "Fragment. (%)");
    }
    printf("---------------------------------------------------------------\n");
    
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == -1) {
            continue;
        }
        
        estado_str = tabla_particiones[i].estado == 0 ? "Libre" : "Ocupada";
        
        if (tabla_particiones[i].estado == 1) {
            proc = buscar_proceso(tabla_particiones[i].id_proceso);
            frag = 0.0;
            if (proc != NULL) {
                frag = calcular_fragmentacion(tabla_particiones[i].tamano, 
                                              proc->tamano_requerido);
            }
            
            if (algoritmo_seleccionado == 4) {
                printf("%-8d %-12s %-18d %-12d %-12d %-8d %-15.2f\n",
                       tabla_particiones[i].id, estado_str,
                       tabla_particiones[i].direccion_inicio,
                       tabla_particiones[i].tamano,
                       tabla_particiones[i].id_proceso,
                       tabla_particiones[i].nivel, frag);
            } else {
                printf("%-8d %-12s %-18d %-12d %-12d %-15.2f\n",
                       tabla_particiones[i].id, estado_str,
                       tabla_particiones[i].direccion_inicio,
                       tabla_particiones[i].tamano,
                       tabla_particiones[i].id_proceso, frag);
            }
        } else {
            if (algoritmo_seleccionado == 4) {
                printf("%-8d %-12s %-18d %-12d %-12s %-8d %-15s\n",
                       tabla_particiones[i].id, estado_str,
                       tabla_particiones[i].direccion_inicio,
                       tabla_particiones[i].tamano, "---",
                       tabla_particiones[i].nivel, "---");
            } else {
                printf("%-8d %-12s %-18d %-12d %-12s %-15s\n",
                       tabla_particiones[i].id, estado_str,
                       tabla_particiones[i].direccion_inicio,
                       tabla_particiones[i].tamano, "---", "---");
            }
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
            printf("[%d] ", memoria[i]);
        }
    }
    
    printf("\n==============================================\n");
}

void mostrar_procesos_terminados(void) {
    Proceso *actual;
    int contador;
    
    printf("\n========== PROCESOS TERMINADOS ==========\n");
    printf("%-10s %-15s %-20s\n", "ID", "Tamano (KB)", "Direccion Inicio");
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
