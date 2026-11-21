#include "reportes.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clientes.h"
#include "prestamos.h"
#include "util.h"

#define ANCHO_TEXTO 76

/* ------------------------------------------------------------------------- */
/* Construccion del reporte                                                  */
/* ------------------------------------------------------------------------- */

Reporte *reporte_crear(const char *titulo)
{
    Reporte *r = calloc(1, sizeof *r);
    if (!r) {
        fprintf(stderr, "Error fatal: sin memoria.\n");
        exit(1);
    }
    snprintf(r->titulo, sizeof r->titulo, "%s", titulo ? titulo : "");
    return r;
}

static void formatear(char *destino, size_t n, const char *fmt, va_list ap)
{
    vsnprintf(destino, n, fmt, ap);
}

void reporte_encabezado(Reporte *r, const char *fmt, ...)
{
    va_list ap;

    if (r->n_encabezado >= REPORTE_MAX_ENCABEZADO) {
        return;
    }
    va_start(ap, fmt);
    formatear(r->encabezado[r->n_encabezado], REPORTE_MAX_TEXTO, fmt, ap);
    va_end(ap);
    r->n_encabezado++;
}

void reporte_columnas(Reporte *r, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    formatear(r->columnas, sizeof r->columnas, fmt, ap);
    va_end(ap);
    r->hay_columnas = 1;
}

void reporte_fila(Reporte *r, const char *fmt, ...)
{
    va_list ap;
    char *linea;

    if (r->n_filas == r->cap_filas) {
        int nueva = r->cap_filas ? r->cap_filas * 2 : 32;
        char **p = realloc(r->filas, (size_t)nueva * sizeof *p);
        if (!p) {
            fprintf(stderr, "Error fatal: sin memoria.\n");
            exit(1);
        }
        r->filas = p;
        r->cap_filas = nueva;
    }
    linea = malloc(REPORTE_MAX_TEXTO);
    if (!linea) {
        fprintf(stderr, "Error fatal: sin memoria.\n");
        exit(1);
    }
    va_start(ap, fmt);
    formatear(linea, REPORTE_MAX_TEXTO, fmt, ap);
    va_end(ap);
    r->filas[r->n_filas++] = linea;
}

void reporte_totales(Reporte *r, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    formatear(r->totales, sizeof r->totales, fmt, ap);
    va_end(ap);
    r->hay_totales = 1;
}

void reporte_liberar(Reporte *r)
{
    int i;

    if (!r) {
        return;
    }
    for (i = 0; i < r->n_filas; i++) {
        free(r->filas[i]);
    }
    free(r->filas);
    free(r);
}

/* ------------------------------------------------------------------------- */
/* Alineacion por columnas fijas                                             */
/* ------------------------------------------------------------------------- */

static void fila_vacia(char *buf)
{
    memset(buf, ' ', ANCHO_TEXTO);
    buf[ANCHO_TEXTO] = '\0';
}

static void poner_izq(char *buf, int pos, const char *texto)
{
    int largo = (int)strlen(texto);

    if (pos < 0) {
        pos = 0;
    }
    if (pos >= ANCHO_TEXTO) {
        return;
    }
    if (pos + largo > ANCHO_TEXTO) {
        largo = ANCHO_TEXTO - pos;
    }
    if (largo > 0) {
        memcpy(buf + pos, texto, (size_t)largo);
    }
}

static void poner_der(char *buf, int fin, const char *texto)
{
    poner_izq(buf, fin - (int)strlen(texto), texto);
}

static void agregar_linea_armada(Reporte *r, const char *buf)
{
    reporte_fila(r, "%s", buf);
}

/* ------------------------------------------------------------------------- */
/* Reporte: clientes                                                         */
/* ------------------------------------------------------------------------- */

Reporte *reporte_clientes(const Datos *d, const char *busqueda)
{
    Reporte *r = reporte_crear("CLIENTES");
    char columnas[ANCHO_TEXTO + 1];
    char normalizada[CEDULA_LEN];
    int es_cedula = 0;
    size_t i;
    int encontrados = 0;

    if (busqueda && busqueda[0]) {
        es_cedula = util_cedula_normalizar(busqueda, normalizada,
                                          sizeof normalizada);
        reporte_encabezado(r, "BUSQUEDA: %s", busqueda);
    } else {
        reporte_encabezado(r, "LISTADO COMPLETO DEL PADRON");
    }

    fila_vacia(columnas);
    poner_der(columnas, 4, "ID");
    poner_izq(columnas, 7, "CEDULA");
    poner_izq(columnas, 21, "CLIENTE");
    poner_izq(columnas, 42, "TELEFONO");
    poner_izq(columnas, 56, "DIRECCION");
    reporte_columnas(r, "%s", columnas);

    for (i = 0; i < d->clientes.len; i++) {
        const Cliente *c = &d->clientes.items[i];
        char fila[ANCHO_TEXTO + 1];
        char id[16];
        char nombre[NOMBRE_COMPLETO_LEN];

        if (busqueda && busqueda[0]) {
            int coincide;
            if (es_cedula) {
                coincide = strcmp(c->cedula, normalizada) == 0;
            } else {
                coincide = util_buscar_sin_mayus(c->nombre, busqueda) != NULL ||
                           util_buscar_sin_mayus(c->apellido, busqueda) != NULL;
            }
            if (!coincide) {
                continue;
            }
        }

        snprintf(id, sizeof id, "%ld", c->id);
        snprintf(nombre, sizeof nombre, "%s %s", c->nombre, c->apellido);
        fila_vacia(fila);
        poner_der(fila, 4, id);
        poner_izq(fila, 7, c->cedula);
        poner_izq(fila, 21, nombre);
        poner_izq(fila, 42, c->telefono);
        poner_izq(fila, 56, c->direccion);
        agregar_linea_armada(r, fila);
        encontrados++;
    }

    if (encontrados == 0) {
        reporte_fila(r, "NO SE ENCONTRARON CLIENTES CON ESE CRITERIO.");
    } else {
        reporte_totales(r, "TOTAL: %d cliente(s)", encontrados);
    }
    return r;
}

/* ------------------------------------------------------------------------- */
/* Reporte: todos los prestamos (auxiliar para elegir un id)                 */
/* ------------------------------------------------------------------------- */

Reporte *reporte_prestamos(const Datos *d)
{
    Reporte *r = reporte_crear("PRESTAMOS REGISTRADOS");
    char columnas[ANCHO_TEXTO + 1];
    size_t i;
    int total = 0;
    long long suma_financiado = 0;
    long long suma_saldo = 0;

    reporte_encabezado(r, "LISTADO GENERAL. USE EL ID PARA CONSULTAR O MODIFICAR.");

    fila_vacia(columnas);
    poner_der(columnas, 3, "N");
    poner_izq(columnas, 5, "CLIENTE");
    poner_izq(columnas, 23, "VEHICULO");
    poner_der(columnas, 53, "FINANCIADO");
    poner_der(columnas, 60, "CTAS");
    poner_der(columnas, 67, "PAG");
    poner_der(columnas, 76, "SALDO");
    reporte_columnas(r, "%s", columnas);

    for (i = 0; i < d->prestamos.len; i++) {
        const Prestamo *p = &d->prestamos.items[i];
        const Cliente *c = clientes_por_id(d, p->id_cliente);
        char fila[ANCHO_TEXTO + 1];
        char id[16], cliente[NOMBRE_COMPLETO_LEN], cuotas[16], pagadas[16];
        char financiado[32], saldo[32];
        int pagos;

        pagos = prestamo_pagos(d, p->id);
        snprintf(id, sizeof id, "%ld", p->id);
        if (c) {
            snprintf(cliente, sizeof cliente, "%s %s", c->nombre, c->apellido);
        } else {
            snprintf(cliente, sizeof cliente, "CLIENTE DESCONOCIDO");
        }
        snprintf(cuotas, sizeof cuotas, "%d", p->meses);
        snprintf(pagadas, sizeof pagadas, "%d", pagos);
        util_formato_monto(prestamo_financiado(p), financiado,
                           sizeof financiado);
        util_formato_monto(prestamo_saldo_pendiente(d, p), saldo,
                           sizeof saldo);

        fila_vacia(fila);
        poner_der(fila, 3, id);
        poner_izq(fila, 5, cliente);
        poner_izq(fila, 23, p->vehiculo);
        poner_der(fila, 53, financiado);
        poner_der(fila, 60, cuotas);
        poner_der(fila, 67, pagadas);
        poner_der(fila, 76, saldo);
        agregar_linea_armada(r, fila);

        total++;
        suma_financiado += prestamo_financiado(p);
        suma_saldo += prestamo_saldo_pendiente(d, p);
    }

    if (total == 0) {
        reporte_fila(r, "NO HAY PRESTAMOS REGISTRADOS.");
    } else {
        char fila[ANCHO_TEXTO + 1];
        char financiado[32], saldo[32];
        util_formato_monto(suma_financiado, financiado, sizeof financiado);
        util_formato_monto(suma_saldo, saldo, sizeof saldo);

        fila_vacia(fila);
        poner_izq(fila, 0, "TOTALES");
        poner_der(fila, 53, financiado);
        poner_der(fila, 76, saldo);
        reporte_totales(r, "%s", fila);
    }
    return r;
}

/* ------------------------------------------------------------------------- */
/* Reporte: prestamos por cliente                                            */
/* ------------------------------------------------------------------------- */

Reporte *reporte_prestamos_por_cliente(const Datos *d, const Cliente *cliente)
{
    Reporte *r = reporte_crear("PRESTAMOS POR CLIENTE");
    char columnas[ANCHO_TEXTO + 1];
    size_t i;
    int total = 0;
    long long suma_financiado = 0;
    long long suma_cuotas = 0;
    long long suma_pagadas = 0;
    long long suma_saldo = 0;

    reporte_encabezado(r, "CLIENTE: %s %s", cliente->nombre, cliente->apellido);
    reporte_encabezado(r, "CEDULA: %s   TELEFONO: %s", cliente->cedula,
                       cliente->telefono);
    reporte_encabezado(r, "DIRECCION: %s", cliente->direccion);

    fila_vacia(columnas);
    poner_der(columnas, 3, "N");
    poner_izq(columnas, 5, "VEHICULO");
    poner_der(columnas, 37, "FINANCIADO");
    poner_der(columnas, 44, "CUOTAS");
    poner_der(columnas, 52, "PAGADAS");
    poner_der(columnas, 63, "SALDO");
    poner_izq(columnas, 65, "ESTADO");
    reporte_columnas(r, "%s", columnas);

    for (i = 0; i < d->prestamos.len; i++) {
        const Prestamo *p = &d->prestamos.items[i];
        char fila[ANCHO_TEXTO + 1];
        char id[16], financiado[32], cuotas[16], pagadas[16], saldo[32];
        int pagos;

        if (p->id_cliente != cliente->id) {
            continue;
        }
        pagos = prestamo_pagos(d, p->id);
        snprintf(id, sizeof id, "%ld", p->id);
        util_formato_monto(prestamo_financiado(p), financiado,
                           sizeof financiado);
        snprintf(cuotas, sizeof cuotas, "%d", p->meses);
        snprintf(pagadas, sizeof pagadas, "%d", pagos);
        util_formato_monto(prestamo_saldo_pendiente(d, p), saldo, sizeof saldo);

        fila_vacia(fila);
        poner_der(fila, 3, id);
        poner_izq(fila, 5, p->vehiculo);
        poner_der(fila, 37, financiado);
        poner_der(fila, 44, cuotas);
        poner_der(fila, 52, pagadas);
        poner_der(fila, 63, saldo);
        poner_izq(fila, 65, prestamo_cancelado(d, p) ? "CANCELADO" : "ACTIVO");
        agregar_linea_armada(r, fila);

        total++;
        suma_financiado += prestamo_financiado(p);
        suma_cuotas += p->meses;
        suma_pagadas += pagos;
        suma_saldo += prestamo_saldo_pendiente(d, p);
    }

    if (total == 0) {
        reporte_fila(r, "ESTE CLIENTE NO TIENE PRESTAMOS REGISTRADOS.");
    } else {
        char fila[ANCHO_TEXTO + 1];
        char financiado[32], cuotas[16], pagadas[16], saldo[32];
        util_formato_monto(suma_financiado, financiado, sizeof financiado);
        snprintf(cuotas, sizeof cuotas, "%lld", suma_cuotas);
        snprintf(pagadas, sizeof pagadas, "%lld", suma_pagadas);
        util_formato_monto(suma_saldo, saldo, sizeof saldo);

        fila_vacia(fila);
        poner_izq(fila, 0, "TOTALES");
        poner_der(fila, 37, financiado);
        poner_der(fila, 44, cuotas);
        poner_der(fila, 52, pagadas);
        poner_der(fila, 63, saldo);
        reporte_totales(r, "%s", fila);
    }
    return r;
}

/* ------------------------------------------------------------------------- */
/* Reporte: calendario de cuotas con saldos                                  */
/* ------------------------------------------------------------------------- */

Reporte *reporte_calendario(const Datos *d, const Prestamo *p)
{
    Reporte *r = reporte_crear("CALENDARIO DE CUOTAS");
    const Cliente *cliente = clientes_por_id(d, p->id_cliente);
    char columnas[ANCHO_TEXTO + 1];
    char valor[32], avance[32], financiado[32], interes[32];
    Cuota *cuotas;
    int cantidad = 0;
    int i;
    long long total_cuota = 0;
    long long total_interes = 0;
    long long total_capital = 0;

    reporte_encabezado(r, "PRESTAMO #%ld   CLIENTE: %s %s   %s", p->id,
                       cliente ? cliente->nombre : "?",
                       cliente ? cliente->apellido : "",
                       cliente ? cliente->cedula : "-");
    util_formato_monto(p->valor, valor, sizeof valor);
    util_formato_monto(p->avance, avance, sizeof avance);
    util_formato_monto(prestamo_financiado(p), financiado, sizeof financiado);
    reporte_encabezado(r, "%s   VALOR %s   AVANCE %s   FINANCIADO %s",
                       p->vehiculo, valor, avance, financiado);
    util_formato_monto(prestamo_interes(p), interes, sizeof interes);
    reporte_encabezado(r, "%d CUOTAS   %d%% ANUAL   GRACIA %d   INTERES %s/MES",
                       p->meses, p->tasa, p->gracia, interes);

    fila_vacia(columnas);
    poner_der(columnas, 3, "MES");
    poner_izq(columnas, 5, "VENCE");
    poner_der(columnas, 24, "CUOTA");
    poner_der(columnas, 34, "INTERES");
    poner_der(columnas, 44, "CAPITAL");
    poner_der(columnas, 54, "SALDO");
    poner_izq(columnas, 57, "ESTADO");
    reporte_columnas(r, "%s", columnas);

    cuotas = prestamo_calendario(p, &cantidad);
    if (!cuotas) {
        reporte_fila(r, "NO SE PUDO GENERAR EL CALENDARIO.");
        return r;
    }

    for (i = 0; i < cantidad; i++) {
        const Cuota *c = &cuotas[i];
        char fila[ANCHO_TEXTO + 1];
        char vence[16], cuota[32], interes_txt[32], capital[32], saldo[32];
        char mes[8];
        EstadoCuota estado = prestamo_estado(d, p->id, c->num, c->vence);

        snprintf(mes, sizeof mes, "%d", c->num);
        util_formato_fecha(c->vence, vence, sizeof vence);
        util_formato_monto(c->cuota, cuota, sizeof cuota);
        util_formato_monto(c->interes, interes_txt, sizeof interes_txt);
        util_formato_monto(c->capital, capital, sizeof capital);
        util_formato_monto(c->saldo, saldo, sizeof saldo);

        fila_vacia(fila);
        poner_der(fila, 3, mes);
        poner_izq(fila, 5, vence);
        poner_der(fila, 24, cuota);
        poner_der(fila, 34, interes_txt);
        poner_der(fila, 44, capital);
        poner_der(fila, 54, saldo);
        poner_izq(fila, 57, estado_texto(estado));
        agregar_linea_armada(r, fila);

        total_cuota += c->cuota;
        total_interes += c->interes;
        total_capital += c->capital;
    }
    prestamo_liberar_calendario(cuotas);

    {
        char fila[ANCHO_TEXTO + 1];
        char cuota[32], interes_txt[32], capital[32], saldo[32];
        util_formato_monto(total_cuota, cuota, sizeof cuota);
        util_formato_monto(total_interes, interes_txt, sizeof interes_txt);
        util_formato_monto(total_capital, capital, sizeof capital);
        util_formato_monto(0, saldo, sizeof saldo);

        fila_vacia(fila);
        poner_izq(fila, 0, "TOTALES");
        poner_der(fila, 24, cuota);
        poner_der(fila, 34, interes_txt);
        poner_der(fila, 44, capital);
        poner_der(fila, 54, saldo);
        reporte_totales(r, "%s", fila);
    }
    return r;
}

/* ------------------------------------------------------------------------- */
/* Reporte: cuotas vencidas                                                  */
/* ------------------------------------------------------------------------- */

Reporte *reporte_vencidas(const Datos *d)
{
    Reporte *r = reporte_crear("CUOTAS VENCIDAS");
    char columnas[ANCHO_TEXTO + 1];
    char hoy[FECHA_LEN];
    char hoy_txt[16];
    size_t i;
    int total = 0;
    long long total_monto = 0;

    util_hoy_iso(hoy, sizeof hoy);
    util_formato_fecha(hoy, hoy_txt, sizeof hoy_txt);
    reporte_encabezado(r, "CUOTAS SIN PAGO CON VENCIMIENTO ANTERIOR A %s",
                       hoy_txt);

    fila_vacia(columnas);
    poner_izq(columnas, 0, "CLIENTE");
    poner_izq(columnas, 21, "CEDULA");
    poner_der(columnas, 39, "PREST");
    poner_der(columnas, 45, "CTA");
    poner_izq(columnas, 47, "VENCE");
    poner_der(columnas, 67, "MONTO");
    reporte_columnas(r, "%s", columnas);

    for (i = 0; i < d->clientes.len; i++) {
        const Cliente *c = &d->clientes.items[i];
        size_t j;

        for (j = 0; j < d->prestamos.len; j++) {
            const Prestamo *p = &d->prestamos.items[j];
            Cuota *cuotas;
            int cantidad = 0;
            int k;

            if (p->id_cliente != c->id) {
                continue;
            }
            cuotas = prestamo_calendario(p, &cantidad);
            if (!cuotas) {
                continue;
            }
            for (k = 0; k < cantidad; k++) {
                const Cuota *cu = &cuotas[k];
                char fila[ANCHO_TEXTO + 1];
                char cliente_txt[NOMBRE_COMPLETO_LEN];
                char prest[16], cta[8], vence[16], monto[32];

                if (prestamo_cuota_pagada(d, p->id, cu->num) ||
                    util_comparar_fechas(cu->vence, hoy) >= 0) {
                    continue;
                }
                snprintf(cliente_txt, sizeof cliente_txt, "%s %s", c->nombre,
                         c->apellido);
                snprintf(prest, sizeof prest, "%ld", p->id);
                snprintf(cta, sizeof cta, "%d", cu->num);
                util_formato_fecha(cu->vence, vence, sizeof vence);
                util_formato_monto(cu->cuota, monto, sizeof monto);

                fila_vacia(fila);
                poner_izq(fila, 0, cliente_txt);
                poner_izq(fila, 21, c->cedula);
                poner_der(fila, 39, prest);
                poner_der(fila, 45, cta);
                poner_izq(fila, 47, vence);
                poner_der(fila, 67, monto);
                agregar_linea_armada(r, fila);

                total++;
                total_monto += cu->cuota;
            }
            prestamo_liberar_calendario(cuotas);
        }
    }

    if (total == 0) {
        reporte_fila(r, "NO HAY CUOTAS VENCIDAS.");
    } else {
        char fila[ANCHO_TEXTO + 1];
        char monto[32];
        util_formato_monto(total_monto, monto, sizeof monto);
        fila_vacia(fila);
        poner_izq(fila, 0, "TOTAL VENCIDO");
        poner_der(fila, 67, monto);
        reporte_totales(r, "%s", fila);
    }
    return r;
}

/* ------------------------------------------------------------------------- */
/* Reporte: bitacora de avisos                                               */
/* ------------------------------------------------------------------------- */

Reporte *reporte_avisos(const Datos *d)
{
    Reporte *r = reporte_crear("BITACORA DE AVISOS");
    char columnas[ANCHO_TEXTO + 1];
    size_t i;

    reporte_encabezado(r, "REGISTRO DE PRESTAMOS ELIMINADOS (SOLO ANEXADO)");

    fila_vacia(columnas);
    poner_izq(columnas, 0, "FECHA Y HORA");
    poner_der(columnas, 25, "PREST");
    poner_izq(columnas, 27, "CLIENTE");
    poner_izq(columnas, 46, "CEDULA");
    poner_der(columnas, 69, "FINANCIADO");
    reporte_columnas(r, "%s", columnas);

    if (d->avisos.len == 0) {
        reporte_fila(r, "SIN AVISOS REGISTRADOS.");
    }

    for (i = 0; i < d->avisos.len; i++) {
        const Aviso *a = &d->avisos.items[i];
        char fila[ANCHO_TEXTO + 1];
        char id[16], monto[32], detalle[160];

        snprintf(id, sizeof id, "%ld", a->id_prestamo);
        util_formato_monto(a->financiado, monto, sizeof monto);

        fila_vacia(fila);
        poner_izq(fila, 0, a->fecha_hora);
        poner_der(fila, 25, id);
        poner_izq(fila, 27, a->nombre);
        poner_izq(fila, 46, a->cedula);
        poner_der(fila, 69, monto);
        agregar_linea_armada(r, fila);

        snprintf(detalle, sizeof detalle,
                 "   Vehiculo: %s   Cuotas: %d   Motivo: %s", a->vehiculo,
                 a->meses, a->motivo);
        reporte_fila(r, "%s", detalle);
    }

    if (d->avisos.len > 0) {
        reporte_totales(r, "TOTAL DE AVISOS: %d", (int)d->avisos.len);
    }
    return r;
}
