#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>

/* Limites de campo: incluyen el terminador nulo. */
#define CEDULA_LEN        14   /* 000-0000000-0 */
#define NOMBRE_LEN        41
#define APELLIDO_LEN      41
#define TELEFONO_LEN      16
#define DIRECCION_LEN     81
#define VEHICULO_LEN      61
#define FECHA_LEN         11   /* YYYY-MM-DD */
#define FECHA_HORA_LEN    20   /* YYYY-MM-DD HH:MM:SS */
#define MOTIVO_LEN        32
#define NOMBRE_COMPLETO_LEN (NOMBRE_LEN + APELLIDO_LEN)

typedef struct {
    long id;
    char cedula[CEDULA_LEN];
    char nombre[NOMBRE_LEN];
    char apellido[APELLIDO_LEN];
    char telefono[TELEFONO_LEN];
    char direccion[DIRECCION_LEN];
} Cliente;

typedef struct {
    long id;
    long id_cliente;
    char vehiculo[VEHICULO_LEN];
    long long valor;
    long long avance;
    int meses;
    int tasa;                /* porcentaje anual, 1..100 */
    int gracia;              /* meses de gracia de capital, 0..meses-1 */
    char inicio[FECHA_LEN];
} Prestamo;

typedef struct {
    long id_prestamo;
    int num_mes;
    char fecha[FECHA_LEN];
} Pago;

typedef struct {
    char fecha_hora[FECHA_HORA_LEN];
    long id_prestamo;
    char cedula[CEDULA_LEN];
    char nombre[NOMBRE_COMPLETO_LEN];
    char vehiculo[VEHICULO_LEN];
    long long financiado;
    int meses;
    char motivo[MOTIVO_LEN];
} Aviso;

/* Cuota calculada: nunca se persiste. */
typedef struct {
    int num;
    char vence[FECHA_LEN];
    long long cuota;
    long long interes;
    long long capital;
    long long saldo;
    int pagada;
} Cuota;

typedef struct { Cliente *items; size_t len, cap; } Clientes;
typedef struct { Prestamo *items; size_t len, cap; } Prestamos;
typedef struct { Pago *items; size_t len, cap; } Pagos;
typedef struct { Aviso *items; size_t len, cap; } Avisos;

typedef struct {
    Clientes clientes;
    Prestamos prestamos;
    Pagos pagos;
    Avisos avisos;
} Datos;

#endif
