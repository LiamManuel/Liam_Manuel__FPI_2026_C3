#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include "model.h"

#define ARCH_CLIENTES  "clientes.txt"
#define ARCH_PRESTAMOS "prestamos.txt"
#define ARCH_PAGOS     "pagos.txt"
#define ARCH_AVISOS    "avisos.txt"

/* Directorio de trabajo de los archivos ("" o NULL = directorio actual). */
void archivos_usar_directorio(const char *directorio);

int archivos_cargar(Datos *d);
void datos_liberar(Datos *d);

void datos_agregar_cliente(Datos *d, const Cliente *c);
void datos_agregar_prestamo(Datos *d, const Prestamo *p);
void datos_agregar_pago(Datos *d, const Pago *p);
void datos_agregar_aviso(Datos *d, const Aviso *a);

void datos_eliminar_cliente(Datos *d, size_t indice);
void datos_eliminar_prestamo(Datos *d, size_t indice);

long datos_siguiente_id_clientes(const Datos *d);
long datos_siguiente_id_prestamos(const Datos *d);

int archivos_guardar_clientes(const Datos *d);
int archivos_guardar_prestamos(const Datos *d);
int archivos_guardar_pagos(const Datos *d);
int archivos_guardar_avisos(const Datos *d);
int archivos_guardar_todo(const Datos *d);

#endif
