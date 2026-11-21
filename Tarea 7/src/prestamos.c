#include "prestamos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archivos.h"
#include "clientes.h"
#include "util.h"

long long prestamo_financiado(const Prestamo *p)
{
    return p->valor - p->avance;
}

long long prestamo_interes(const Prestamo *p)
{
    return (prestamo_financiado(p) * (long long)p->tasa + 600) / 1200;
}

int prestamo_meses_amort(const Prestamo *p)
{
    return p->meses - p->gracia;
}

Cuota *prestamo_calendario(const Prestamo *p, int *cantidad)
{
    Cuota *cuotas;
    long long financiado;
    long long interes;
    long long base;
    long long residuo;
    long long saldo;
    int meses_amort;
    int k;

    if (cantidad) {
        *cantidad = 0;
    }
    if (!p || p->meses <= 0) {
        return NULL;
    }
    meses_amort = prestamo_meses_amort(p);
    if (meses_amort <= 0) {
        return NULL;
    }
    cuotas = calloc((size_t)p->meses, sizeof *cuotas);
    if (!cuotas) {
        return NULL;
    }

    financiado = prestamo_financiado(p);
    interes = prestamo_interes(p);
    base = financiado / meses_amort;
    residuo = financiado % meses_amort;
    saldo = financiado;

    for (k = 1; k <= p->meses; k++) {
        Cuota *c = &cuotas[k - 1];
        c->num = k;
        if (!util_sumar_meses(p->inicio, k, c->vence, sizeof c->vence)) {
            snprintf(c->vence, sizeof c->vence, "%s", p->inicio);
        }
        c->interes = interes;
        if (k <= p->gracia) {
            c->capital = 0;
        } else {
            long long posicion = k - p->gracia;
            c->capital = base + (posicion <= residuo ? 1 : 0);
        }
        c->cuota = c->capital + c->interes;
        saldo -= c->capital;
        c->saldo = saldo;
        c->pagada = 0;
    }
    if (cantidad) {
        *cantidad = p->meses;
    }
    return cuotas;
}

void prestamo_liberar_calendario(Cuota *cuotas)
{
    free(cuotas);
}

const Prestamo *prestamos_por_id(const Datos *d, long id)
{
    size_t i;

    for (i = 0; i < d->prestamos.len; i++) {
        if (d->prestamos.items[i].id == id) {
            return &d->prestamos.items[i];
        }
    }
    return NULL;
}

int prestamo_pagos(const Datos *d, long id_prestamo)
{
    int total = 0;
    size_t i;

    for (i = 0; i < d->pagos.len; i++) {
        if (d->pagos.items[i].id_prestamo == id_prestamo) {
            total++;
        }
    }
    return total;
}

int prestamo_cuota_pagada(const Datos *d, long id_prestamo, int num)
{
    size_t i;

    for (i = 0; i < d->pagos.len; i++) {
        if (d->pagos.items[i].id_prestamo == id_prestamo &&
            d->pagos.items[i].num_mes == num) {
            return 1;
        }
    }
    return 0;
}

int prestamo_siguiente_cuota(const Datos *d, const Prestamo *p)
{
    int pagadas = prestamo_pagos(d, p->id);
    if (pagadas >= p->meses) {
        return 0;
    }
    return pagadas + 1;
}

int prestamo_cancelado(const Datos *d, const Prestamo *p)
{
    return prestamo_pagos(d, p->id) >= p->meses;
}

long long prestamo_saldo_pendiente(const Datos *d, const Prestamo *p)
{
    Cuota *cuotas;
    long long pagado = 0;
    int cantidad = 0;
    int i;

    cuotas = prestamo_calendario(p, &cantidad);
    if (!cuotas) {
        return prestamo_financiado(p);
    }
    for (i = 0; i < cantidad; i++) {
        if (prestamo_cuota_pagada(d, p->id, cuotas[i].num)) {
            pagado += cuotas[i].capital;
        }
    }
    prestamo_liberar_calendario(cuotas);
    return prestamo_financiado(p) - pagado;
}

EstadoCuota prestamo_estado(const Datos *d, long id_prestamo, int num,
                            const char *vence)
{
    char hoy[FECHA_LEN];

    if (prestamo_cuota_pagada(d, id_prestamo, num)) {
        return ESTADO_PAGADA;
    }
    util_hoy_iso(hoy, sizeof hoy);
    return util_comparar_fechas(vence, hoy) < 0 ? ESTADO_VENCIDA
                                                : ESTADO_PENDIENTE;
}

const char *estado_texto(EstadoCuota estado)
{
    switch (estado) {
    case ESTADO_PAGADA:
        return "PAGADA";
    case ESTADO_VENCIDA:
        return "VENCIDA";
    default:
        return "PENDIENTE";
    }
}

int prestamo_validar(const Prestamo *p, char *err, size_t errn)
{
    if (!p->vehiculo[0]) {
        snprintf(err, errn, "La descripcion del vehiculo no puede quedar vacia.");
        return 0;
    }
    if (p->valor <= 0) {
        snprintf(err, errn, "El valor del vehiculo debe ser mayor que cero.");
        return 0;
    }
    if (p->avance < 0) {
        snprintf(err, errn, "El avance no puede ser negativo.");
        return 0;
    }
    if (p->avance >= p->valor) {
        snprintf(err, errn, "El avance debe ser menor que el valor del vehiculo.");
        return 0;
    }
    if (p->meses < 1 || p->meses > 120) {
        snprintf(err, errn, "Los meses deben estar entre 1 y 120.");
        return 0;
    }
    if (p->tasa <= 0) {
        snprintf(err, errn, "La tasa debe ser mayor que cero.");
        return 0;
    }
    if (p->tasa > 100) {
        snprintf(err, errn, "La tasa no puede superar 100.");
        return 0;
    }
    if (p->gracia < 0) {
        snprintf(err, errn, "Los meses de gracia no pueden ser negativos.");
        return 0;
    }
    if (p->gracia >= p->meses) {
        snprintf(err, errn, "La gracia debe ser menor que los meses del prestamo.");
        return 0;
    }
    if (!util_fecha_valida(p->inicio)) {
        snprintf(err, errn, "La fecha de inicio no es valida (YYYY-MM-DD).");
        return 0;
    }
    return 1;
}

long prestamos_crear(Datos *d, const Prestamo *nuevo, char *err, size_t errn)
{
    Prestamo p = *nuevo;

    if (!prestamo_validar(&p, err, errn)) {
        return -1;
    }
    if (!clientes_por_id(d, p.id_cliente)) {
        snprintf(err, errn, "El cliente con id %ld no existe.", p.id_cliente);
        return -1;
    }
    p.id = datos_siguiente_id_prestamos(d);
    datos_agregar_prestamo(d, &p);
    return p.id;
}

int prestamos_actualizar(Datos *d, long id, const Prestamo *nuevo, char *err,
                         size_t errn)
{
    size_t i;
    Prestamo *actual = NULL;

    for (i = 0; i < d->prestamos.len; i++) {
        if (d->prestamos.items[i].id == id) {
            actual = &d->prestamos.items[i];
            break;
        }
    }
    if (!actual) {
        snprintf(err, errn, "El prestamo %ld no existe.", id);
        return 0;
    }

    if (prestamo_pagos(d, id) > 0) {
        if (nuevo->valor != actual->valor || nuevo->avance != actual->avance ||
            nuevo->meses != actual->meses || nuevo->tasa != actual->tasa ||
            nuevo->gracia != actual->gracia ||
            strcmp(nuevo->inicio, actual->inicio) != 0) {
            snprintf(err, errn,
                     "El prestamo tiene pagos registrados: solo puede "
                     "modificar la descripcion del vehiculo.");
            return 0;
        }
        if (!nuevo->vehiculo[0]) {
            snprintf(err, errn,
                     "La descripcion del vehiculo no puede quedar vacia.");
            return 0;
        }
        snprintf(actual->vehiculo, sizeof actual->vehiculo, "%s",
                 nuevo->vehiculo);
        return 1;
    }

    if (!prestamo_validar(nuevo, err, errn)) {
        return 0;
    }
    {
        long id_cliente = actual->id_cliente;
        Prestamo copia = *nuevo;
        copia.id = id;
        copia.id_cliente = id_cliente;
        *actual = copia;
    }
    return 1;
}

int prestamos_registrar_pago(Datos *d, long id, int num_cuota, char *err,
                             size_t errn)
{
    const Prestamo *p = prestamos_por_id(d, id);
    int pagadas;
    int siguiente;
    Pago pago;

    if (!p) {
        snprintf(err, errn, "El prestamo %ld no existe.", id);
        return 0;
    }
    pagadas = prestamo_pagos(d, id);
    if (pagadas >= p->meses) {
        snprintf(err, errn,
                 "El prestamo esta cancelado: todas las cuotas estan pagadas.");
        return 0;
    }
    siguiente = pagadas + 1;
    if (num_cuota != siguiente) {
        snprintf(err, errn, "Debe pagarse primero la cuota %d.", siguiente);
        return 0;
    }
    if (prestamo_cuota_pagada(d, id, num_cuota)) {
        snprintf(err, errn, "La cuota %d ya esta pagada.", num_cuota);
        return 0;
    }

    memset(&pago, 0, sizeof pago);
    pago.id_prestamo = id;
    pago.num_mes = num_cuota;
    util_hoy_iso(pago.fecha, sizeof pago.fecha);
    datos_agregar_pago(d, &pago);
    return 1;
}

int prestamos_eliminar(Datos *d, long id, char *err, size_t errn)
{
    size_t i;
    size_t indice = (size_t)-1;
    const Cliente *cliente;
    const Prestamo *p;
    Aviso aviso;

    for (i = 0; i < d->prestamos.len; i++) {
        if (d->prestamos.items[i].id == id) {
            indice = i;
            break;
        }
    }
    if (indice == (size_t)-1) {
        snprintf(err, errn, "El prestamo %ld no existe.", id);
        return 0;
    }
    p = &d->prestamos.items[indice];
    if (prestamo_pagos(d, id) > 0) {
        snprintf(err, errn,
                 "No se puede eliminar: el prestamo tiene %d pago(s) "
                 "registrados y debe conservarse el historial.",
                 prestamo_pagos(d, id));
        return 0;
    }

    cliente = clientes_por_id(d, p->id_cliente);
    memset(&aviso, 0, sizeof aviso);
    util_ahora_texto(aviso.fecha_hora, sizeof aviso.fecha_hora);
    aviso.id_prestamo = id;
    snprintf(aviso.cedula, sizeof aviso.cedula, "%s",
             cliente ? cliente->cedula : "-");
    if (cliente) {
        snprintf(aviso.nombre, sizeof aviso.nombre, "%s %s", cliente->nombre,
                 cliente->apellido);
    } else {
        snprintf(aviso.nombre, sizeof aviso.nombre, "CLIENTE DESCONOCIDO");
    }
    snprintf(aviso.vehiculo, sizeof aviso.vehiculo, "%s", p->vehiculo);
    aviso.financiado = prestamo_financiado(p);
    aviso.meses = p->meses;
    snprintf(aviso.motivo, sizeof aviso.motivo, "falta de pagos");
    datos_agregar_aviso(d, &aviso);

    datos_eliminar_prestamo(d, indice);
    return 1;
}
