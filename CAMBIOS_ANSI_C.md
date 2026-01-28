# CAMBIOS REALIZADOS PARA CUMPLIR CON ANSI C (C90)

## Resumen de Modificaciones

Este documento detalla todos los cambios realizados al código original para cumplir estrictamente con el estándar ANSI C (C90).

---

## 1. DECLARACIÓN DE VARIABLES

### ❌ ANTES (C99/C11 - NO compatible con ANSI C):
```c
void crear_proceso(void) {
    int tamano_proceso = (rand() % tamano_particion) + 1;  /* ERROR: declaración al medio */
    
    printf("Proceso ID: %d\n", contador_procesos);
    
    int particion_asignada = -1;  /* ERROR: declaración después de código ejecutable */
    for (int i = 0; i < num_particiones; i++) {  /* ERROR: declaración en for */
        /* código */
    }
}
```

### ✅ DESPUÉS (ANSI C - Compatible):
```c
void crear_proceso(void) {
    /* TODAS las variables declaradas AL INICIO de la función */
    int tamano_proceso;
    int particion_asignada;
    int i;
    int dir_inicio;
    float fragmentacion;
    
    /* Ahora sí, código ejecutable */
    tamano_proceso = (rand() % tamano_particion) + 1;
    
    printf("Proceso ID: %d\n", contador_procesos);
    
    particion_asignada = -1;
    for (i = 0; i < num_particiones; i++) {
        /* código */
    }
}
```

---

## 2. COMENTARIOS

### ❌ ANTES (Estilo C99):
```c
// Este es un comentario de línea única
int x = 5;  // Comentario al final de línea
```

### ✅ DESPUÉS (Estilo ANSI C):
```c
/* Este es un comentario tradicional de C */
int x = 5;  /* Comentario al final de linea */
```

**REGLA**: En ANSI C (C89/C90) NO existen los comentarios `//`. Solo se permiten `/* */`.

---

## 3. BUCLES FOR

### ❌ ANTES (C99):
```c
for (int i = 0; i < n; i++) {
    /* código */
}
```

### ✅ DESPUÉS (ANSI C):
```c
int i;  /* Declaración al inicio de la función */

for (i = 0; i < n; i++) {
    /* código */
}
```

---

## 4. PROTOTIPOS DE FUNCIONES

En ANSI C es una buena práctica declarar todos los prototipos antes de las definiciones:

```c
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
```

---

## 5. CAST DE TIME(NULL)

### ❌ ANTES:
```c
srand(time(NULL));
```

### ✅ DESPUÉS:
```c
srand((unsigned int)time(NULL));
```

**RAZÓN**: `srand()` espera `unsigned int`, y `time()` devuelve `time_t`. El cast explícito evita advertencias en compilación estricta.

---

## 6. DECLARACIÓN DE main()

### ✅ ANSI C:
```c
int main(void) {
    /* código */
    return 0;
}
```

El `void` indica explícitamente que la función no toma argumentos.

---

## 7. LISTA COMPLETA DE CAMBIOS POR FUNCIÓN

### `agregar_proceso()`
```c
/* ANTES */
void agregar_proceso(int id, int tamano, int direccion) {
    Proceso *nuevo = (Proceso *)malloc(sizeof(Proceso));  /* ERROR */
    /* ... */
}

/* DESPUÉS */
void agregar_proceso(int id, int tamano, int direccion) {
    Proceso *nuevo;      /* Declaración al inicio */
    Proceso *actual;     /* Declaración al inicio */
    
    nuevo = (Proceso *)malloc(sizeof(Proceso));  /* Asignación después */
    /* ... */
}
```

### `eliminar_proceso()`
```c
/* Declaraciones movidas al inicio */
Proceso *actual;
Proceso *anterior;
Proceso *temp;
```

### `buscar_proceso()`
```c
/* Declaraciones movidas al inicio */
Proceso *actual;
```

### `inicializar_memoria()`
```c
/* Declaraciones movidas al inicio */
int i;
```

### `calcular_fragmentacion()`
```c
/* Declaraciones movidas al inicio */
int fragmentacion;
```

### `crear_proceso()`
```c
/* TODAS las variables al inicio */
int tamano_proceso;
int particion_asignada;
int i;
int dir_inicio;
float fragmentacion;
```

### `cerrar_proceso()`
```c
/* TODAS las variables al inicio */
int id_proceso;
Proceso *proceso;
int particion_encontrada;
int i;
int dir_inicio;
```

### `mostrar_tabla_procesos()`
```c
/* Declaraciones movidas al inicio */
Proceso *actual;
```

### `mostrar_tabla_particiones()`
```c
/* TODAS las variables al inicio */
int i;
char *estado_str;
Proceso *proc;
float frag;
```

### `mostrar_memoria()`
```c
/* Declaraciones movidas al inicio */
int i;
```

### `liberar_recursos()`
```c
/* Declaraciones movidas al inicio */
Proceso *actual;
Proceso *temp;
```

### `main()`
```c
/* Declaraciones movidas al inicio */
int opcion;
```

---

## 8. COMPILACIÓN ESTRICTA

### Comando de Compilación con ANSI C:
```bash
gcc -ansi -pedantic -Wall -Wextra -o gestion_memoria_ansi gestion_memoria_ansi.c
```

### Explicación de las banderas:
- **`-ansi`**: Activa el modo ANSI C (equivalente a `-std=c89`)
- **`-pedantic`**: Rechaza extensiones no estándar de GNU
- **`-Wall`**: Activa todas las advertencias comunes
- **`-Wextra`**: Activa advertencias adicionales

### Resultado:
```
✓ Compilación exitosa sin errores ni advertencias
✓ Cumple 100% con el estándar ANSI C (C90)
```

---

## 9. DIFERENCIAS CLAVE ENTRE C90 Y C99/C11

| Característica | ANSI C (C90) | C99/C11 |
|----------------|--------------|---------|
| Comentarios `//` | ❌ NO permitido | ✅ Permitido |
| Variables en medio del código | ❌ NO permitido | ✅ Permitido |
| Declaraciones en `for` | ❌ NO permitido | ✅ Permitido (`for(int i=0...)`) |
| `inline` | ❌ NO existe | ✅ Existe |
| `long long` | ❌ NO existe | ✅ Existe |
| Arrays de longitud variable | ❌ NO existe | ✅ Existe (VLA) |

---

## 10. VERIFICACIÓN DE CUMPLIMIENTO

### ✅ Checklist ANSI C:
- [x] Todas las variables declaradas al inicio de cada función
- [x] Ninguna declaración dentro de bucles `for`
- [x] Ninguna declaración después de código ejecutable
- [x] Todos los comentarios en formato `/* */`
- [x] Cast explícito de `time(NULL)`
- [x] Prototipos de funciones declarados
- [x] `main()` con `void` explícito
- [x] Compila con `-ansi -pedantic` sin errores ni advertencias

---

## 11. VENTAJAS DE ESTE CÓDIGO

✅ **Portabilidad**: Funciona en compiladores antiguos y modernos
✅ **Compatibilidad**: Compatible con sistemas embebidos y legacy
✅ **Estándares académicos**: Cumple con requisitos de profesores estrictos
✅ **Claridad**: Las variables al inicio hacen más fácil ver el alcance
✅ **Profesionalismo**: Demuestra conocimiento de estándares históricos

---

## 12. PRUEBA DE COMPILACIÓN

```bash
# Compilación estricta ANSI C
gcc -ansi -pedantic -Wall -Wextra -o gestion_memoria_ansi gestion_memoria_ansi.c

# Ejecución
./gestion_memoria_ansi

# Configuración sugerida:
# Tamaño total: 1000 KB
# Tamaño partición: 100 KB
```

---

## 13. NOTAS FINALES

- **ANSI C (C89/C90)** fue el primer estándar oficial de C (1989-1990)
- Es el estándar más portable y compatible
- Muchos profesores lo exigen para enseñar fundamentos sin "azúcar sintáctica"
- El código resultante es **100% compatible** con compiladores modernos y antiguos

---

**IMPORTANTE**: Este código cumple ESTRICTAMENTE con ANSI C (C90) y compila sin errores ni advertencias con las banderas más estrictas de GCC.

---

## Autor
Código adaptado para cumplir con ANSI C (C90)
Sistemas Operativos - Gestión de Memoria
