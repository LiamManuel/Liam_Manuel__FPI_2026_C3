#ifndef REPORTES_H
#define REPORTES_H

#include "model.h"

#define REPORTE_MAX_ENCABEZADO 6
#define REPORTE_MAX_TEXTO 81

typedef struct {
    char titulo[REPORTE_MAX_TEXTO];
    char encabezado[REPORTE_MAX_ENCABEZADO][REPORTE_MAX_TEXTO];
    int n_encabezado;
    char columnas[REPORTE_MAX_TEXTO];
    int hay_columnas;
    char **filas;
    int n_filas;
    int cap_filas;
    char totales[REPORTE_MAX_TEXTO];
    int hay_totales;
} Reporte;

Reporte *reporte_crear(const char *titulo);
void reporte_encabezado(Reporte *r, const char *fmt, ...);
void reporte_columnas(Reporte *r, const char *fmt, ...);
void reporte_fila(Reporte *r, const char *fmt, ...);
void reporte_totales(Reporte *r, const char *fmt, ...);
void reporte_liberar(Reporte *r);

Reporte *reporte_clientes(const Datos *d, const char *busqueda);
Reporte *reporte_prestamos(const Datos *d);
Reporte *reporte_prestamos_por_cliente(const Datos *d, const Cliente *cliente);
Reporte *reporte_calendario(const Datos *d, const Prestamo *prestamo);
Reporte *reporte_vencidas(const Datos *d);
Reporte *reporte_avisos(const Datos *d);

#endif
