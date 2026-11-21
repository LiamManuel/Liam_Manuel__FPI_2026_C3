#ifndef PRESTAMOS_H
#define PRESTAMOS_H

#include "model.h"

typedef enum {
    ESTADO_PENDIENTE = 0,
    ESTADO_PAGADA,
    ESTADO_VENCIDA
} EstadoCuota;

/* Calculos derivados: nada de esto se persiste. */
long long prestamo_financiado(const Prestamo *p);
long long prestamo_interes(const Prestamo *p);
int prestamo_meses_amort(const Prestamo *p);
Cuota *prestamo_calendario(const Prestamo *p, int *cantidad);
void prestamo_liberar_calendario(Cuota *cuotas);

/* Consultas sobre pagos registrados. */
const Prestamo *prestamos_por_id(const Datos *d, long id);
int prestamo_pagos(const Datos *d, long id_prestamo);
int prestamo_cuota_pagada(const Datos *d, long id_prestamo, int num);
int prestamo_siguiente_cuota(const Datos *d, const Prestamo *p);
int prestamo_cancelado(const Datos *d, const Prestamo *p);
long long prestamo_saldo_pendiente(const Datos *d, const Prestamo *p);
EstadoCuota prestamo_estado(const Datos *d, long id_prestamo, int num,
                            const char *vence);
const char *estado_texto(EstadoCuota estado);

/* Operaciones. */
int prestamo_validar(const Prestamo *p, char *err, size_t errn);
long prestamos_crear(Datos *d, const Prestamo *nuevo, char *err, size_t errn);
int prestamos_actualizar(Datos *d, long id, const Prestamo *nuevo, char *err,
                         size_t errn);
int prestamos_registrar_pago(Datos *d, long id, int num_cuota, char *err,
                             size_t errn);
int prestamos_eliminar(Datos *d, long id, char *err, size_t errn);

#endif
