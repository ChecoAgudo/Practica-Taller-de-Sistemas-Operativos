/*
 * SIMULADOR DE GESTION DE MEMORIA - SISTEMA DE COMPANEROS (BUDDY SYSTEM)
 * Compatible con ANSI C (C90)
 * 
 * Caracteristicas:
 * - La memoria se divide recursivamente en potencias de 2 (buddies)
 * - Al asignar: se divide el bloque hasta encontrar el tamano optimo
 * - Al liberar: se fusionan buddies libres adyacentes
 * - Memoria inicial debe ser potencia de 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ==================== ESTRUCTURAS ==================== */

typedef struct {
    int id;                 /* ID unico de la particion */
    int id_proceso;         /* ID del proceso asignado (-1 si libre) */
    int direccion_inicio;   /* Direccion de inicio en memoria */
    int tamano;             /* Tamano de la particion (potencia de 2) */
    int estado;             /* 0 = Libre, 1 = Ocupada */
    int nivel;              /* Nivel en el arbol binario (0 = raiz) */
} Particion;

typedef struct Proceso {
    int id;                     /* ID unico del proceso */
    int tamano_requerido;       /* Tamano que requiere el proceso */
    int direccion_asignada;     /* Direccion donde fue asignado */
    struct Proceso *siguiente;  /* Puntero al siguiente nodo */
} Proceso;

/* ==================== VARIABLES GLOBALES ==================== */

int *memoria = NULL;
Particion *tabla_particiones = NULL;
Proceso *lista_procesos = NULL;
Proceso *lista_procesos_terminados = NULL;
int num_particiones = 0;         /* Numero ACTUAL de particiones (dinamico) */
int tamano_total_memoria = 0;
int tamano_particion = 0;        /* Ya no usado en buddy, pero mantenido por compatibilidad */
int contador_procesos = 1;
int max_particiones = 0;         /* Maximo numero de particiones posibles */

/* ==================== PROTOTIPOS DE FUNCIONES ==================== */

void inicializar_memoria(void);
void agregar_proceso(int id, int tamano, int direccion);
int eliminar_proceso(int id);
Proceso* buscar_proceso(int id);
void agregar_proceso_terminado(int id, int tamano, int direccion);
float calcular_fragmentacion(int tamano_particion, int tamano_proceso);
int es_potencia_de_2(int n);
int siguiente_potencia_de_2(int n);
int calcular_buddy(int indice, int tamano);
void dividir_bloque(int indice);
void fusionar_buddies(void);
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
    printf("║  SIMULADOR DE SISTEMA DE COMPANEROS (BUDDY SYSTEM)    ║\n");
    printf("║  Algoritmo: Division Recursiva y Fusion de Bloques    ║\n");
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
    int temp;
    
    printf("\n=== INICIALIZACION DEL SISTEMA DE MEMORIA (BUDDY SYSTEM) ===\n");
    
    /* Solicitar tamano total de memoria (debe ser potencia de 2) */
    do {
        printf("Ingrese el tamano total de memoria en KB (debe ser potencia de 2, ej: 512, 1024, 2048): ");
        scanf("%d", &tamano_total_memoria);
        
        if (tamano_total_memoria <= 0) {
            printf("Error: El tamano debe ser mayor a 0.\n");
        } else if (!es_potencia_de_2(tamano_total_memoria)) {
            temp = siguiente_potencia_de_2(tamano_total_memoria);
            printf("Advertencia: %d no es potencia de 2. Se ajustara a %d KB.\n", 
                   tamano_total_memoria, temp);
            tamano_total_memoria = temp;
            break;
        }
    } while (tamano_total_memoria <= 0 || !es_potencia_de_2(tamano_total_memoria));
    
    printf("\n>>> Memoria total: %d KB (potencia de 2)\n", tamano_total_memoria);
    
    /* Calcular numero maximo de particiones posibles */
    /* En el peor caso: toda la memoria dividida en bloques de 1 KB */
    max_particiones = tamano_total_memoria * 2;  /* Suficiente para el arbol binario */
    
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
    tabla_particiones = (Particion *)malloc(max_particiones * sizeof(Particion));
    if (tabla_particiones == NULL) {
        printf("Error: No se pudo asignar memoria para la tabla de particiones.\n");
        free(memoria);
        exit(1);
    }
    
    /* Inicializar todas las particiones como invalidas */
    for (i = 0; i < max_particiones; i++) {
        tabla_particiones[i].id = -1;
        tabla_particiones[i].id_proceso = -1;
        tabla_particiones[i].direccion_inicio = -1;
        tabla_particiones[i].tamano = 0;
        tabla_particiones[i].estado = -1;  /* -1 = no existe */
        tabla_particiones[i].nivel = -1;
    }
    
    /* Crear la particion inicial (toda la memoria como un solo bloque) */
    tabla_particiones[0].id = 0;
    tabla_particiones[0].id_proceso = -1;
    tabla_particiones[0].direccion_inicio = 0;
    tabla_particiones[0].tamano = tamano_total_memoria;
    tabla_particiones[0].estado = 0;  /* Libre */
    tabla_particiones[0].nivel = 0;   /* Nivel raiz */
    
    num_particiones = 1;  /* Solo una particion al inicio */
    
    printf(">>> Sistema Buddy inicializado con un bloque de %d KB.\n", tamano_total_memoria);
    printf(">>> Las particiones se dividiran dinamicamente segun la demanda.\n");
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

/* ==================== FUNCIONES AUXILIARES BUDDY SYSTEM ==================== */

/*
 * Verifica si un numero es potencia de 2
 */
int es_potencia_de_2(int n) {
    if (n <= 0) {
        return 0;
    }
    return (n & (n - 1)) == 0;
}

/*
 * Calcula la siguiente potencia de 2 mayor o igual a n
 */
int siguiente_potencia_de_2(int n) {
    int potencia;
    
    potencia = 1;
    while (potencia < n) {
        potencia *= 2;
    }
    return potencia;
}

/*
 * Calcula el indice del buddy (companero) de un bloque
 * Formula: buddy_index = indice XOR (tamano / tamano_minimo)
 * Simplificado: buscamos el bloque hermano en el arbol
 */
int calcular_buddy(int indice, int tamano) {
    int i;
    int dir_actual;
    int dir_buddy;
    
    dir_actual = tabla_particiones[indice].direccion_inicio;
    
    /* El buddy esta en la misma direccion +/- tamano */
    if ((dir_actual / tamano) % 2 == 0) {
        dir_buddy = dir_actual + tamano;  /* Buddy a la derecha */
    } else {
        dir_buddy = dir_actual - tamano;  /* Buddy a la izquierda */
    }
    
    /* Buscar el buddy en la tabla de particiones */
    for (i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado != -1 &&
            tabla_particiones[i].direccion_inicio == dir_buddy &&
            tabla_particiones[i].tamano == tamano) {
            return i;
        }
    }
    
    return -1;  /* No se encontro el buddy */
}

/*
 * Divide un bloque en dos buddies (mitades iguales)
 */
void dividir_bloque(int indice) {
    int nuevo_tamano;
    int hijo_izq_idx;
    int hijo_der_idx;
    
    if (num_particiones >= max_particiones - 2) {
        printf("Error: No hay espacio para mas particiones.\n");
        return;
    }
    
    nuevo_tamano = tabla_particiones[indice].tamano / 2;
    
    /* Crear hijo izquierdo */
    hijo_izq_idx = num_particiones;
    tabla_particiones[hijo_izq_idx].id = hijo_izq_idx;
    tabla_particiones[hijo_izq_idx].id_proceso = -1;
    tabla_particiones[hijo_izq_idx].direccion_inicio = tabla_particiones[indice].direccion_inicio;
    tabla_particiones[hijo_izq_idx].tamano = nuevo_tamano;
    tabla_particiones[hijo_izq_idx].estado = 0;  /* Libre */
    tabla_particiones[hijo_izq_idx].nivel = tabla_particiones[indice].nivel + 1;
    num_particiones++;
    
    /* Crear hijo derecho */
    hijo_der_idx = num_particiones;
    tabla_particiones[hijo_der_idx].id = hijo_der_idx;
    tabla_particiones[hijo_der_idx].id_proceso = -1;
    tabla_particiones[hijo_der_idx].direccion_inicio = tabla_particiones[indice].direccion_inicio + nuevo_tamano;
    tabla_particiones[hijo_der_idx].tamano = nuevo_tamano;
    tabla_particiones[hijo_der_idx].estado = 0;  /* Libre */
    tabla_particiones[hijo_der_idx].nivel = tabla_particiones[indice].nivel + 1;
    num_particiones++;
    
    /* Marcar el bloque padre como dividido (estado -1) */
    tabla_particiones[indice].estado = -1;
}

/*
 * Fusiona buddies libres adyacentes recursivamente
 */
void fusionar_buddies(void) {
    int i;
    int buddy_idx;
    int fusionado;
    
    /* Repetir hasta que no haya mas fusiones posibles */
    do {
        fusionado = 0;
        
        for (i = 0; i < num_particiones; i++) {
            /* Solo considerar bloques libres y validos */
            if (tabla_particiones[i].estado != 0) {
                continue;
            }
            
            /* Buscar su buddy */
            buddy_idx = calcular_buddy(i, tabla_particiones[i].tamano);
            
            /* Si el buddy existe y esta libre, fusionar */
            if (buddy_idx != -1 && tabla_particiones[buddy_idx].estado == 0) {
                /* Marcar ambos como no existentes */
                tabla_particiones[i].estado = -1;
                tabla_particiones[buddy_idx].estado = -1;
                
                /* Crear bloque fusionado (el de menor direccion) */
                if (tabla_particiones[i].direccion_inicio < tabla_particiones[buddy_idx].direccion_inicio) {
                    tabla_particiones[i].tamano *= 2;
                    tabla_particiones[i].estado = 0;  /* Libre */
                    tabla_particiones[i].nivel--;
                } else {
                    tabla_particiones[buddy_idx].tamano *= 2;
                    tabla_particiones[buddy_idx].estado = 0;  /* Libre */
                    tabla_particiones[buddy_idx].nivel--;
                }
                
                fusionado = 1;
                break;  /* Reiniciar la busqueda */
            }
        }
    } while (fusionado);
}

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
    int tamano_requerido;
    int i;
    int mejor_particion;
    int menor_tamano;
    int dir_inicio;
    float fragmentacion;
    
    /* Generar tamano aleatorio */
    tamano_proceso = (rand() % (tamano_total_memoria / 4)) + 1;
    
    /* Calcular la potencia de 2 mas pequena que contiene al proceso */
    tamano_requerido = siguiente_potencia_de_2(tamano_proceso);
    
    printf("\n--- CREANDO NUEVO PROCESO ---\n");
    printf("Proceso ID: %d\n", contador_procesos);
    printf("Tamano solicitado: %d KB\n", tamano_proceso);
    printf("Tamano asignado (potencia de 2): %d KB\n", tamano_requerido);
    
    /* Buscar bloque libre mas pequeno que quepa */
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
        printf("ERROR: No hay bloques disponibles para el proceso.\n");
        return;
    }
    
    /* Dividir el bloque hasta que tenga el tamano exacto requerido */
    while (tabla_particiones[mejor_particion].tamano > tamano_requerido) {
        dividir_bloque(mejor_particion);
        
        /* Despues de dividir, el hijo izquierdo tiene el mismo inicio */
        /* Buscar el nuevo hijo izquierdo */
        for (i = 0; i < num_particiones; i++) {
            if (tabla_particiones[i].estado == 0 &&
                tabla_particiones[i].direccion_inicio == tabla_particiones[mejor_particion].direccion_inicio &&
                tabla_particiones[i].tamano == tabla_particiones[mejor_particion].tamano / 2) {
                mejor_particion = i;
                break;
            }
        }
    }
    
    /* Asignar el proceso al bloque */
    tabla_particiones[mejor_particion].estado = 1;  /* Ocupada */
    tabla_particiones[mejor_particion].id_proceso = contador_procesos;
    
    /* Actualizar vector de memoria */
    dir_inicio = tabla_particiones[mejor_particion].direccion_inicio;
    for (i = 0; i < tamano_proceso; i++) {
        memoria[dir_inicio + i] = contador_procesos;
    }
    
    /* Agregar proceso a la lista */
    agregar_proceso(contador_procesos, tamano_proceso, dir_inicio);
    
    /* Calcular fragmentacion interna */
    fragmentacion = calcular_fragmentacion(tamano_requerido, tamano_proceso);
    
    printf(">>> PROCESO CREADO EXITOSAMENTE <<<\n");
    printf("    Asignado a la particion: %d\n", mejor_particion);
    printf("    Direccion de inicio: %d\n", dir_inicio);
    printf("    Tamano del bloque: %d KB\n", tabla_particiones[mejor_particion].tamano);
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
    
    tamano_bloque = tabla_particiones[particion_encontrada].tamano;
    
    /* Liberar la particion */
    tabla_particiones[particion_encontrada].estado = 0;  /* Libre */
    tabla_particiones[particion_encontrada].id_proceso = -1;
    
    /* Limpiar el vector de memoria */
    for (i = 0; i < tamano_bloque; i++) {
        if (memoria[dir_inicio + i] == id_proceso) {
            memoria[dir_inicio + i] = -1;
        }
    }
    
    /* Agregar a la lista de procesos terminados */
    agregar_proceso_terminado(id_proceso, tamano_proceso, dir_inicio);
    
    /* Eliminar de la lista de procesos activos */
    eliminar_proceso(id_proceso);
    
    printf(">>> PROCESO %d CERRADO EXITOSAMENTE <<<\n", id_proceso);
    printf("    Particion %d liberada.\n", particion_encontrada);
    printf("    Tamano del bloque liberado: %d KB\n", tamano_bloque);
    
    /* Intentar fusionar buddies libres */
    printf(">>> Intentando fusionar buddies libres...\n");
    fusionar_buddies();
    printf(">>> Fusion completada.\n");
}

/* ==================== FUNCIONES DE VISUALIZACION ==================== */

void mostrar_tabla_procesos(void) {
    Proceso *actual;
    
    printf("\n========== TABLA DE PROCESOS ==========\n");
    printf("%-10s %-15s %-20s\n", "ID", "Tamanio (KB)", "Direccion Inicio");
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
    
    printf("\n==================== TABLA DE PARTICIONES (BUDDY SYSTEM) ====================\n");
    printf("%-8s %-12s %-18s %-12s %-12s %-8s %-15s\n", 
           "Part ID", "Estado", "Dir. Inicio (KB)", "Tamanio (KB)", "ID Proceso", "Nivel", "Fragment. (%)");
    printf("------------------------------------------------------------------------------\n");
    
    for (i = 0; i < num_particiones; i++) {
        /* Solo mostrar particiones validas */
        if (tabla_particiones[i].estado == -1) {
            continue;
        }
        
        if (tabla_particiones[i].estado == 0) {
            estado_str = "Libre";
        } else {
            estado_str = "Ocupada";
        }
        
        if (tabla_particiones[i].estado == 1) {
            proc = buscar_proceso(tabla_particiones[i].id_proceso);
            frag = 0.0;
            if (proc != NULL) {
                frag = calcular_fragmentacion(tabla_particiones[i].tamano, 
                                              proc->tamano_requerido);
            }
            
            printf("%-8d %-12s %-18d %-12d %-12d %-8d %-15.2f\n",
                   tabla_particiones[i].id,
                   estado_str,
                   tabla_particiones[i].direccion_inicio,
                   tabla_particiones[i].tamano,
                   tabla_particiones[i].id_proceso,
                   tabla_particiones[i].nivel,
                   frag);
        } else {
            printf("%-8d %-12s %-18d %-12d %-12s %-8d %-15s\n",
                   tabla_particiones[i].id,
                   estado_str,
                   tabla_particiones[i].direccion_inicio,
                   tabla_particiones[i].tamano,
                   "---",
                   tabla_particiones[i].nivel,
                   "---");
        }
    }
    printf("==============================================================================\n");
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
    printf("%-10s %-15s %-20s\n", "ID", "Tamanio (KB)", "Direccion Inicio");
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
