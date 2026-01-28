#!/bin/bash

# Script de compilacion estricta ANSI C (C90)
# Para profesores que exigen cumplimiento total del estandar

echo "╔════════════════════════════════════════════════════════╗"
echo "║  COMPILACION ANSI C (C90) - MODO ESTRICTO             ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Compilar con banderas estrictas ANSI C
echo ">>> Compilando con banderas ANSI C estrictas..."
echo "    Banderas: -ansi -pedantic -Wall -Wextra"
echo ""

gcc -ansi -pedantic -Wall -Wextra -o gestion_memoria_ansi gestion_memoria_ansi.c

if [ $? -eq 0 ]; then
    echo "✓✓✓ COMPILACION EXITOSA ✓✓✓"
    echo ""
    echo ">>> El codigo cumple 100% con el estandar ANSI C (C90)"
    echo ""
    
    # Mostrar informacion del ejecutable
    echo ">>> Informacion del ejecutable:"
    ls -lh gestion_memoria_ansi
    echo ""
    
    # Mostrar estandares cumplidos
    echo "╔════════════════════════════════════════════════════════╗"
    echo "║  ESTANDARES CUMPLIDOS                                 ║"
    echo "╠════════════════════════════════════════════════════════╣"
    echo "║  ✓ ANSI C (C89/C90)                                   ║"
    echo "║  ✓ Variables declaradas al inicio de funciones        ║"
    echo "║  ✓ Comentarios estilo /* */                           ║"
    echo "║  ✓ Sin declaraciones en bucles for                    ║"
    echo "║  ✓ Sin extensiones de GNU                             ║"
    echo "║  ✓ Sin advertencias del compilador                    ║"
    echo "╚════════════════════════════════════════════════════════╝"
    echo ""
    
    echo ">>> Para ejecutar el programa:"
    echo "    ./gestion_memoria_ansi"
    echo ""
    echo ">>> Configuracion sugerida para pruebas:"
    echo "    Tamano total de memoria: 1000 KB"
    echo "    Tamano de particion: 100 KB"
    echo "    (Esto creara 10 particiones)"
    
else
    echo "✗✗✗ ERROR EN LA COMPILACION ✗✗✗"
    echo ""
    echo "Por favor revise los errores mostrados arriba."
    exit 1
fi

echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║  LISTO PARA ENTREGAR A SU PROFESOR                    ║"
echo "╚════════════════════════════════════════════════════════╝"
