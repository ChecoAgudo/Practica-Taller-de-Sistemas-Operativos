================================================================================
    SIMULADOR DE MEMORIA PARTICIONADA - TODO LO QUE NECESITAS EN UN ARCHIVO
================================================================================

📋 INSTRUCCIONES RÁPIDAS:
-------------------------
1. Copia TODO el código de abajo
2. Guárdalo en un archivo: memoria_particionada.c
3. Compila: gcc -Wall -o memoria_particionada memoria_particionada.c
4. Ejecuta: ./memoria_particionada
5. Usa memoria=4096 y partición=512
6. Crea procesos (opción 1), mira tablas (opción 4), cierra procesos (opción 2)


================================================================================
                     CÓDIGO COMPLETO - COPIA TODO DE AQUÍ ABAJO
================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========== CONSTANTES ==========
#define MIN_MEMORIA_PROCESO 64
#define MAX_MEMORIA_PROCESO 512

// Estados de partición
#define LIBRE 0
#define OCUPADA 1

// ========== ESTRUCTURAS ==========

// Nodo para la lista enlazada de procesos
typedef struct NodoProceso {
    int id_proceso;
    int memoria_requerida;
    int posicion_memoria;
    struct NodoProceso *siguiente;
} NodoProceso;

// Estructura para cada partición
typedef struct {
    int id_particion;
    int id_proceso;           // -1 si está libre
    int direccion_memoria;
    int tamano;
    int estado;               // LIBRE=0, OCUPADA=1
} Particion;

// ========== VARIABLES GLOBALES ==========
int *memoria = NULL;                      // Vector que simula la memoria
int tamano_memoria = 0;                   // Tamaño total de memoria
int tamano_particion = 0;                 // Tamaño de cada partición
int num_particiones = 0;                  // Número total de particiones
Particion *tabla_particiones = NULL;      // Array de particiones
NodoProceso *lista_procesos = NULL;       // Lista de procesos activos
NodoProceso *lista_terminados = NULL;     // Lista de procesos terminados
int contador_procesos = 1;                // ID autoincremental para procesos

// ========== PROTOTIPOS DE FUNCIONES ==========
void inicializar_sistema();
void liberar_sistema();
void crear_proceso();
void cerrar_proceso();
void ver_tabla_procesos();
void ver_tabla_particiones();
void ver_memoria_asignada();
void ver_procesos_terminados();
void menu();
int asignar_particion_primer_ajuste();
NodoProceso* buscar_proceso(int id);
void agregar_proceso_lista(NodoProceso **lista, int id, int mem_req, int pos);
void eliminar_proceso_lista(int id);

// ========== FUNCIÓN PRINCIPAL ==========
int main() {
    int opcion;
    
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  SIMULADOR DE MEMORIA PARTICIONADA ESTATICA FIJA      ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    // Solicitar parámetros
    printf("Ingrese el tamaño total de memoria (ej: 4096): ");
    if (scanf("%d", &tamano_memoria) != 1 || tamano_memoria <= 0) {
        printf("Error: valor inválido. Usando 4096 por defecto.\n");
        tamano_memoria = 4096;
    }
    
    printf("Ingrese el tamaño de cada partición (ej: 512): ");
    if (scanf("%d", &tamano_particion) != 1 || tamano_particion <= 0) {
        printf("Error: valor inválido. Usando 512 por defecto.\n");
        tamano_particion = 512;
    }
    
    // Inicializar el sistema
    inicializar_sistema();
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  SISTEMA INICIALIZADO CORRECTAMENTE                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("→ Memoria total: %d bytes\n", tamano_memoria);
    printf("→ Tamaño de partición: %d bytes\n", tamano_particion);
    printf("→ Número de particiones: %d\n\n", num_particiones);
    
    // Bucle principal del menú
    do {
        menu();
        printf("→ Seleccione una opción: ");
        
        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n'); // Limpiar buffer
            opcion = -1;
        }
        
        printf("\n");
        
        switch(opcion) {
            case 1:
                crear_proceso();
                break;
            case 2:
                cerrar_proceso();
                break;
            case 3:
                ver_tabla_procesos();
                break;
            case 4:
                ver_tabla_particiones();
                break;
            case 5:
                ver_memoria_asignada();
                break;
            case 6:
                ver_procesos_terminados();
                break;
            case 0:
                printf("Saliendo del simulador...\n");
                break;
            default:
                printf("⚠ Opción inválida. Intente nuevamente.\n");
        }
        
        printf("\n");
        printf("Presione Enter para continuar...");
        while (getchar() != '\n'); // Limpiar buffer
        getchar(); // Esperar Enter
        
    } while(opcion != 0);
    
    // Liberar toda la memoria antes de salir
    liberar_sistema();
    
    return 0;
}

// ========== IMPLEMENTACIÓN DE FUNCIONES ==========

// Inicializar el sistema de memoria
void inicializar_sistema() {
    srand(time(NULL)); // Semilla para números aleatorios
    
    // Calcular número de particiones
    num_particiones = tamano_memoria / tamano_particion;
    
    // Crear vector de memoria (reservar memoria dinámica)
    memoria = (int *)malloc(tamano_memoria * sizeof(int));
    if (memoria == NULL) {
        printf("Error: no se pudo reservar memoria.\n");
        exit(1);
    }
    
    // Inicializar vector de memoria en -1 (todo libre)
    for (int i = 0; i < tamano_memoria; i++) {
        memoria[i] = -1;
    }
    
    // Crear tabla de particiones
    tabla_particiones = (Particion *)malloc(num_particiones * sizeof(Particion));
    if (tabla_particiones == NULL) {
        printf("Error: no se pudo reservar memoria para particiones.\n");
        exit(1);
    }
    
    // Inicializar tabla de particiones
    for (int i = 0; i < num_particiones; i++) {
        tabla_particiones[i].id_particion = i;
        tabla_particiones[i].id_proceso = -1;
        tabla_particiones[i].direccion_memoria = i * tamano_particion;
        tabla_particiones[i].tamano = tamano_particion;
        tabla_particiones[i].estado = LIBRE;
    }
    
    // Las listas de procesos inician en NULL
    lista_procesos = NULL;
    lista_terminados = NULL;
}

// Liberar toda la memoria reservada
void liberar_sistema() {
    // Liberar vector de memoria
    if (memoria != NULL) {
        free(memoria);
    }
    
    // Liberar tabla de particiones
    if (tabla_particiones != NULL) {
        free(tabla_particiones);
    }
    
    // Liberar lista de procesos activos
    NodoProceso *actual = lista_procesos;
    while (actual != NULL) {
        NodoProceso *temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    
    // Liberar lista de procesos terminados
    actual = lista_terminados;
    while (actual != NULL) {
        NodoProceso *temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
}

// Crear un nuevo proceso
void crear_proceso() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  CREAR NUEVO PROCESO                                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    // Generar tamaño de memoria requerido aleatoriamente
    int memoria_requerida = MIN_MEMORIA_PROCESO + 
                           (rand() % (MAX_MEMORIA_PROCESO - MIN_MEMORIA_PROCESO + 1));
    
    printf("→ ID del proceso: %d\n", contador_procesos);
    printf("→ Memoria requerida: %d bytes\n", memoria_requerida);
    
    // Verificar si la memoria requerida excede el tamaño de partición
    if (memoria_requerida > tamano_particion) {
        printf("\n❌ ERROR: No se puede ejecutar el programa.\n");
        printf("   La memoria requerida (%d bytes) excede el tamaño de partición (%d bytes).\n", 
               memoria_requerida, tamano_particion);
        return;
    }
    
    // Buscar partición libre usando algoritmo Primer Ajuste
    int particion_asignada = asignar_particion_primer_ajuste();
    
    if (particion_asignada == -1) {
        printf("\n❌ ERROR: Memoria llena.\n");
        printf("   No hay particiones libres disponibles.\n");
        return;
    }
    
    // Asignar proceso a la partición (cambiar -1 por ID del proceso)
    tabla_particiones[particion_asignada].id_proceso = contador_procesos;
    tabla_particiones[particion_asignada].estado = OCUPADA;
    
    // Llenar el vector de memoria con el ID del proceso
    int direccion = tabla_particiones[particion_asignada].direccion_memoria;
    for (int i = direccion; i < direccion + memoria_requerida; i++) {
        memoria[i] = contador_procesos;
    }
    
    // Agregar proceso a la lista enlazada de procesos
    agregar_proceso_lista(&lista_procesos, contador_procesos, 
                         memoria_requerida, direccion);
    
    // Calcular fragmentación interna
    int fragmentacion = tamano_particion - memoria_requerida;
    float fragmentacion_porcentaje = (fragmentacion * 100.0) / tamano_particion;
    
    printf("\n✅ Proceso %d creado exitosamente\n", contador_procesos);
    printf("   ├─ Asignado a partición: %d\n", particion_asignada);
    printf("   ├─ Dirección de memoria: %d\n", direccion);
    printf("   ├─ Memoria utilizada: %d bytes\n", memoria_requerida);
    printf("   └─ Fragmentación interna: %d bytes (%.2f%%)\n", 
           fragmentacion, fragmentacion_porcentaje);
    
    contador_procesos++;
}

// Algoritmo Primer Ajuste: buscar primera partición libre
int asignar_particion_primer_ajuste() {
    for (int i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].estado == LIBRE) {
            return i; // Retorna índice de la primera partición libre
        }
    }
    return -1; // No hay particiones libres
}

// Cerrar un proceso
void cerrar_proceso() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  CERRAR PROCESO                                        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    int id_proceso;
    
    printf("→ Ingrese el ID del proceso a cerrar: ");
    if (scanf("%d", &id_proceso) != 1) {
        printf("⚠ Entrada inválida.\n");
        while (getchar() != '\n');
        return;
    }
    
    // Buscar el proceso en la lista de procesos activos
    NodoProceso *proceso = buscar_proceso(id_proceso);
    
    if (proceso == NULL) {
        printf("\n❌ ERROR: Proceso %d no encontrado en la tabla de procesos.\n", id_proceso);
        return;
    }
    
    // Buscar la partición que contiene el proceso
    int particion_encontrada = -1;
    for (int i = 0; i < num_particiones; i++) {
        if (tabla_particiones[i].id_proceso == id_proceso) {
            particion_encontrada = i;
            break;
        }
    }
    
    if (particion_encontrada == -1) {
        printf("\n❌ ERROR: No se encontró la partición del proceso.\n");
        return;
    }
    
    // Guardar datos antes de eliminar
    int direccion = tabla_particiones[particion_encontrada].direccion_memoria;
    int mem_req = proceso->memoria_requerida;
    int pos = proceso->posicion_memoria;
    
    // 1. En el vector memoria asignar -1 (liberar memoria)
    for (int i = direccion; i < direccion + mem_req; i++) {
        memoria[i] = -1;
    }
    
    // 2. Actualizar la partición, marcándola como libre
    tabla_particiones[particion_encontrada].id_proceso = -1;
    tabla_particiones[particion_encontrada].estado = LIBRE;
    
    // 3. Agregar a lista de procesos terminados (opcional)
    agregar_proceso_lista(&lista_terminados, id_proceso, mem_req, pos);
    
    // 4. Borrar de la tabla de procesos (liberar nodo de lista enlazada)
    eliminar_proceso_lista(id_proceso);
    
    printf("\n✅ Proceso %d cerrado exitosamente\n", id_proceso);
    printf("   ├─ Partición liberada: %d\n", particion_encontrada);
    printf("   └─ Memoria liberada: %d bytes\n", mem_req);
}

// Buscar un proceso en la lista de procesos activos
NodoProceso* buscar_proceso(int id) {
    NodoProceso *actual = lista_procesos;
    while (actual != NULL) {
        if (actual->id_proceso == id) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

// Agregar un proceso a una lista enlazada
void agregar_proceso_lista(NodoProceso **lista, int id, int mem_req, int pos) {
    // Crear nuevo nodo
    NodoProceso *nuevo = (NodoProceso *)malloc(sizeof(NodoProceso));
    if (nuevo == NULL) {
        printf("Error: no se pudo reservar memoria para el proceso.\n");
        return;
    }
    
    // Llenar datos del nodo
    nuevo->id_proceso = id;
    nuevo->memoria_requerida = mem_req;
    nuevo->posicion_memoria = pos;
    
    // Insertar al inicio de la lista
    nuevo->siguiente = *lista;
    *lista = nuevo;
}

// Eliminar un proceso de la lista de procesos activos
void eliminar_proceso_lista(int id) {
    NodoProceso *actual = lista_procesos;
    NodoProceso *anterior = NULL;
    
    // Buscar el nodo a eliminar
    while (actual != NULL && actual->id_proceso != id) {
        anterior = actual;
        actual = actual->siguiente;
    }
    
    if (actual == NULL) {
        return; // No encontrado
    }
    
    // Eliminar el nodo
    if (anterior == NULL) {
        // Es el primer nodo
        lista_procesos = actual->siguiente;
    } else {
        // Está en medio o al final
        anterior->siguiente = actual->siguiente;
    }
    
    free(actual);
}

// Ver tabla de procesos activos
void ver_tabla_procesos() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  TABLA DE PROCESOS ACTIVOS                             ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    if (lista_procesos == NULL) {
        printf("   No hay procesos activos.\n");
        return;
    }
    
    printf("┌────────────┬─────────────────────┬─────────────────────┐\n");
    printf("│ ID Proceso │ Memoria Requerida   │ Posición Memoria    │\n");
    printf("├────────────┼─────────────────────┼─────────────────────┤\n");
    
    NodoProceso *actual = lista_procesos;
    int contador = 0;
    while (actual != NULL) {
        printf("│ %-10d │ %-19d │ %-19d │\n", 
               actual->id_proceso, 
               actual->memoria_requerida, 
               actual->posicion_memoria);
        actual = actual->siguiente;
        contador++;
    }
    
    printf("└────────────┴─────────────────────┴─────────────────────┘\n");
    printf("\nTotal de procesos activos: %d\n", contador);
}

// Ver tabla de particiones con fragmentación
void ver_tabla_particiones() {
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║  TABLA DE PARTICIONES                                                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n\n");
    
    printf("┌────────┬────────────┬──────────────┬─────────┬─────────┬──────────────────┐\n");
    printf("│ ID     │ ID Proceso │ Dir. Memoria │ Tamaño  │ Estado  │ Frag. Interna    │\n");
    printf("│ Part.  │            │              │         │         │ (%%)             │\n");
    printf("├────────┼────────────┼──────────────┼─────────┼─────────┼──────────────────┤\n");
    
    int total_fragmentacion = 0;
    int particiones_ocupadas = 0;
    
    for (int i = 0; i < num_particiones; i++) {
        printf("│ %-6d │ ", tabla_particiones[i].id_particion);
        
        if (tabla_particiones[i].estado == OCUPADA) {
            // Buscar el proceso para calcular fragmentación
            NodoProceso *proceso = buscar_proceso(tabla_particiones[i].id_proceso);
            
            int fragmentacion_bytes = 0;
            float fragmentacion_porcentaje = 0.0;
            
            if (proceso != NULL) {
                fragmentacion_bytes = tabla_particiones[i].tamano - proceso->memoria_requerida;
                fragmentacion_porcentaje = (fragmentacion_bytes * 100.0) / tabla_particiones[i].tamano;
                total_fragmentacion += fragmentacion_bytes;
            }
            
            printf("%-10d │ %-12d │ %-7d │ %-7s │ %6d (%.2f%%) │\n",
                   tabla_particiones[i].id_proceso,
                   tabla_particiones[i].direccion_memoria,
                   tabla_particiones[i].tamano,
                   "OCUPADA",
                   fragmentacion_bytes,
                   fragmentacion_porcentaje);
            
            particiones_ocupadas++;
        } else {
            printf("%-10s │ %-12d │ %-7d │ %-7s │ %-16s │\n",
                   "-",
                   tabla_particiones[i].direccion_memoria,
                   tabla_particiones[i].tamano,
                   "LIBRE",
                   "N/A");
        }
    }
    
    printf("└────────┴────────────┴──────────────┴─────────┴─────────┴──────────────────┘\n");
    
    // Resumen
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  RESUMEN                                               ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("→ Total de particiones: %d\n", num_particiones);
    printf("→ Particiones ocupadas: %d\n", particiones_ocupadas);
    printf("→ Particiones libres: %d\n", num_particiones - particiones_ocupadas);
    printf("→ Total fragmentación interna: %d bytes\n", total_fragmentacion);
    
    if (particiones_ocupadas > 0) {
        float frag_promedio = (total_fragmentacion * 100.0) / (particiones_ocupadas * tamano_particion);
        printf("→ Fragmentación interna promedio: %.2f%%\n", frag_promedio);
    }
}

// Ver memoria asignada visualmente
void ver_memoria_asignada() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  MEMORIA ASIGNADA                                      ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    printf("Mostrando memoria en bloques de 64 bytes\n\n");
    
    int bloques_por_linea = 16;
    int contador = 0;
    
    for (int i = 0; i < tamano_memoria; i += 64) {
        int proceso_id = memoria[i];
        
        if (proceso_id == -1) {
            printf("[LIBRE] ");
        } else {
            printf("[P%-4d] ", proceso_id);
        }
        
        contador++;
        if (contador % bloques_por_linea == 0) {
            printf("\n");
        }
    }
    printf("\n");
    
    // Estadísticas
    int memoria_ocupada = 0;
    for (int i = 0; i < tamano_memoria; i++) {
        if (memoria[i] != -1) {
            memoria_ocupada++;
        }
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  ESTADÍSTICAS                                          ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("→ Memoria total: %d bytes\n", tamano_memoria);
    printf("→ Memoria ocupada: %d bytes (%.2f%%)\n", 
           memoria_ocupada, 
           (memoria_ocupada * 100.0) / tamano_memoria);
    printf("→ Memoria libre: %d bytes (%.2f%%)\n", 
           tamano_memoria - memoria_ocupada,
           ((tamano_memoria - memoria_ocupada) * 100.0) / tamano_memoria);
}

// Ver procesos terminados
void ver_procesos_terminados() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  PROCESOS TERMINADOS                                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    if (lista_terminados == NULL) {
        printf("   No hay procesos terminados.\n");
        return;
    }
    
    printf("┌────────────┬─────────────────────┬─────────────────────┐\n");
    printf("│ ID Proceso │ Memoria Requerida   │ Posición Memoria    │\n");
    printf("├────────────┼─────────────────────┼─────────────────────┤\n");
    
    NodoProceso *actual = lista_terminados;
    int contador = 0;
    while (actual != NULL) {
        printf("│ %-10d │ %-19d │ %-19d │\n", 
               actual->id_proceso, 
               actual->memoria_requerida, 
               actual->posicion_memoria);
        actual = actual->siguiente;
        contador++;
    }
    
    printf("└────────────┴─────────────────────┴─────────────────────┘\n");
    printf("\nTotal de procesos terminados: %d\n", contador);
}

// Mostrar menú principal
void menu() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                    MENÚ PRINCIPAL                      ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  1. Crear Proceso                                      ║\n");
    printf("║  2. Cerrar Proceso                                     ║\n");
    printf("║  3. Ver Tabla de Procesos                              ║\n");
    printf("║  4. Ver Tabla de Particiones                           ║\n");
    printf("║  5. Ver Memoria Asignada                               ║\n");
    printf("║  6. Ver Procesos Terminados                            ║\n");
    printf("║  0. Salir                                              ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
}


================================================================================
                                FIN DEL CÓDIGO
================================================================================

✅ ESO ES TODO! Ya tienes todo lo necesario.

RECUERDA:
- Es UN solo archivo .c (todo incluido)
- No necesitas crear clases ni dividir en partes
- Las estructuras son como clases pero sin métodos
- Variables globales conectan todo
- Compila con: gcc -Wall -o memoria_particionada memoria_particionada.c

¡LISTO PARA USAR!
