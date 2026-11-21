#include "archivos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ------------------------------------------------------------------------- */
/* Rutas y escritura atomica                                                 */
/* ------------------------------------------------------------------------- */

static char g_directorio[512] = "";

void archivos_usar_directorio(const char *directorio)
{
    size_t largo;

    if (!directorio) {
        g_directorio[0] = '\0';
        return;
    }
    snprintf(g_directorio, sizeof g_directorio, "%s", directorio);
    largo = strlen(g_directorio);
    while (largo > 0 &&
           (g_directorio[largo - 1] == '/' || g_directorio[largo - 1] == '\\')) {
        g_directorio[--largo] = '\0';
    }
}

static void ruta_de(char *out, size_t n, const char *archivo)
{
    if (g_directorio[0]) {
        snprintf(out, n, "%s/%s", g_directorio, archivo);
    } else {
        snprintf(out, n, "%s", archivo);
    }
}

static int existe_archivo(const char *archivo)
{
    char ruta[600];
    FILE *f;

    ruta_de(ruta, sizeof ruta, archivo);
    f = fopen(ruta, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static FILE *abrir_temporal(const char *archivo, char *final, size_t nf,
                            char *temporal, size_t nt)
{
    ruta_de(final, nf, archivo);
    snprintf(temporal, nt, "%s.tmp", final);
    return fopen(temporal, "wb");
}

static int cerrar_temporal(FILE *f, const char *final, const char *temporal)
{
    if (!f) {
        return 0;
    }
    if (fclose(f) != 0) {
        remove(temporal);
        return 0;
    }
#ifdef _WIN32
    if (!MoveFileExA(temporal, final, MOVEFILE_REPLACE_EXISTING)) {
        remove(temporal);
        return 0;
    }
#else
    remove(final);
    if (rename(temporal, final) != 0) {
        remove(temporal);
        return 0;
    }
#endif
    return 1;
}

/* ------------------------------------------------------------------------- */
/* Arreglos dinamicos                                                        */
/* ------------------------------------------------------------------------- */

static void asegurar(void **items, size_t *cap, size_t len, size_t tam)
{
    size_t nueva;
    void *p;

    if (len < *cap) {
        return;
    }
    nueva = *cap ? *cap * 2 : 8;
    p = realloc(*items, nueva * tam);
    if (!p) {
        fprintf(stderr, "Error fatal: sin memoria.\n");
        exit(1);
    }
    *items = p;
    *cap = nueva;
}

void datos_agregar_cliente(Datos *d, const Cliente *c)
{
    asegurar((void **)&d->clientes.items, &d->clientes.cap, d->clientes.len,
             sizeof(Cliente));
    d->clientes.items[d->clientes.len++] = *c;
}

void datos_agregar_prestamo(Datos *d, const Prestamo *p)
{
    asegurar((void **)&d->prestamos.items, &d->prestamos.cap,
             d->prestamos.len, sizeof(Prestamo));
    d->prestamos.items[d->prestamos.len++] = *p;
}

void datos_agregar_pago(Datos *d, const Pago *p)
{
    asegurar((void **)&d->pagos.items, &d->pagos.cap, d->pagos.len,
             sizeof(Pago));
    d->pagos.items[d->pagos.len++] = *p;
}

void datos_agregar_aviso(Datos *d, const Aviso *a)
{
    asegurar((void **)&d->avisos.items, &d->avisos.cap, d->avisos.len,
             sizeof(Aviso));
    d->avisos.items[d->avisos.len++] = *a;
}

void datos_eliminar_cliente(Datos *d, size_t indice)
{
    if (indice >= d->clientes.len) {
        return;
    }
    memmove(&d->clientes.items[indice], &d->clientes.items[indice + 1],
            (d->clientes.len - indice - 1) * sizeof(Cliente));
    d->clientes.len--;
}

void datos_eliminar_prestamo(Datos *d, size_t indice)
{
    if (indice >= d->prestamos.len) {
        return;
    }
    memmove(&d->prestamos.items[indice], &d->prestamos.items[indice + 1],
            (d->prestamos.len - indice - 1) * sizeof(Prestamo));
    d->prestamos.len--;
}

long datos_siguiente_id_clientes(const Datos *d)
{
    long maximo = 0;
    size_t i;

    for (i = 0; i < d->clientes.len; i++) {
        if (d->clientes.items[i].id > maximo) {
            maximo = d->clientes.items[i].id;
        }
    }
    return maximo + 1;
}

long datos_siguiente_id_prestamos(const Datos *d)
{
    long maximo = 0;
    size_t i;

    for (i = 0; i < d->prestamos.len; i++) {
        if (d->prestamos.items[i].id > maximo) {
            maximo = d->prestamos.items[i].id;
        }
    }
    return maximo + 1;
}

void datos_liberar(Datos *d)
{
    free(d->clientes.items);
    free(d->prestamos.items);
    free(d->pagos.items);
    free(d->avisos.items);
    memset(d, 0, sizeof *d);
}

/* ------------------------------------------------------------------------- */
/* Carga                                                                     */
/* ------------------------------------------------------------------------- */

static void copiar(char *destino, size_t n, const char *origen)
{
    snprintf(destino, n, "%s", origen ? origen : "");
}

static int dividir(char *linea, char **campos, int maximo)
{
    int n = 0;
    char *p;

    campos[n++] = linea;
    for (p = linea; *p; p++) {
        if (*p == '|') {
            *p = '\0';
            if (n < maximo) {
                campos[n++] = p + 1;
            }
        }
    }
    return n;
}

static void quitar_salto(char *linea)
{
    linea[strcspn(linea, "\r\n")] = '\0';
}

static int cargar_clientes(Datos *d)
{
    char ruta[600];
    char linea[1024];
    FILE *f;

    ruta_de(ruta, sizeof ruta, ARCH_CLIENTES);
    f = fopen(ruta, "rb");
    if (!f) {
        return existe_archivo(ARCH_CLIENTES) ? 0 : 1;
    }
    while (fgets(linea, sizeof linea, f)) {
        char *c[6];
        Cliente cl;
        quitar_salto(linea);
        if (linea[0] == '\0' || dividir(linea, c, 6) < 6) {
            continue;
        }
        memset(&cl, 0, sizeof cl);
        cl.id = strtol(c[0], NULL, 10);
        copiar(cl.cedula, sizeof cl.cedula, c[1]);
        copiar(cl.nombre, sizeof cl.nombre, c[2]);
        copiar(cl.apellido, sizeof cl.apellido, c[3]);
        copiar(cl.telefono, sizeof cl.telefono, c[4]);
        copiar(cl.direccion, sizeof cl.direccion, c[5]);
        datos_agregar_cliente(d, &cl);
    }
    fclose(f);
    return 1;
}

static int cargar_prestamos(Datos *d)
{
    char ruta[600];
    char linea[1024];
    FILE *f;

    ruta_de(ruta, sizeof ruta, ARCH_PRESTAMOS);
    f = fopen(ruta, "rb");
    if (!f) {
        return existe_archivo(ARCH_PRESTAMOS) ? 0 : 1;
    }
    while (fgets(linea, sizeof linea, f)) {
        char *c[9];
        Prestamo p;
        quitar_salto(linea);
        if (linea[0] == '\0' || dividir(linea, c, 9) < 9) {
            continue;
        }
        memset(&p, 0, sizeof p);
        p.id = strtol(c[0], NULL, 10);
        p.id_cliente = strtol(c[1], NULL, 10);
        copiar(p.vehiculo, sizeof p.vehiculo, c[2]);
        p.valor = strtoll(c[3], NULL, 10);
        p.avance = strtoll(c[4], NULL, 10);
        p.meses = (int)strtol(c[5], NULL, 10);
        p.tasa = (int)strtol(c[6], NULL, 10);
        p.gracia = (int)strtol(c[7], NULL, 10);
        copiar(p.inicio, sizeof p.inicio, c[8]);
        datos_agregar_prestamo(d, &p);
    }
    fclose(f);
    return 1;
}

static int cargar_pagos(Datos *d)
{
    char ruta[600];
    char linea[1024];
    FILE *f;

    ruta_de(ruta, sizeof ruta, ARCH_PAGOS);
    f = fopen(ruta, "rb");
    if (!f) {
        return existe_archivo(ARCH_PAGOS) ? 0 : 1;
    }
    while (fgets(linea, sizeof linea, f)) {
        char *c[3];
        Pago p;
        quitar_salto(linea);
        if (linea[0] == '\0' || dividir(linea, c, 3) < 3) {
            continue;
        }
        memset(&p, 0, sizeof p);
        p.id_prestamo = strtol(c[0], NULL, 10);
        p.num_mes = (int)strtol(c[1], NULL, 10);
        copiar(p.fecha, sizeof p.fecha, c[2]);
        datos_agregar_pago(d, &p);
    }
    fclose(f);
    return 1;
}

static int cargar_avisos(Datos *d)
{
    char ruta[600];
    char linea[1024];
    FILE *f;

    ruta_de(ruta, sizeof ruta, ARCH_AVISOS);
    f = fopen(ruta, "rb");
    if (!f) {
        return existe_archivo(ARCH_AVISOS) ? 0 : 1;
    }
    while (fgets(linea, sizeof linea, f)) {
        char *c[8];
        Aviso a;
        quitar_salto(linea);
        if (linea[0] == '\0' || dividir(linea, c, 8) < 8) {
            continue;
        }
        memset(&a, 0, sizeof a);
        copiar(a.fecha_hora, sizeof a.fecha_hora, c[0]);
        a.id_prestamo = strtol(c[1], NULL, 10);
        copiar(a.cedula, sizeof a.cedula, c[2]);
        copiar(a.nombre, sizeof a.nombre, c[3]);
        copiar(a.vehiculo, sizeof a.vehiculo, c[4]);
        a.financiado = strtoll(c[5], NULL, 10);
        a.meses = (int)strtol(c[6], NULL, 10);
        copiar(a.motivo, sizeof a.motivo, c[7]);
        datos_agregar_aviso(d, &a);
    }
    fclose(f);
    return 1;
}

int archivos_cargar(Datos *d)
{
    memset(d, 0, sizeof *d);
    if (!cargar_clientes(d) || !cargar_prestamos(d) || !cargar_pagos(d) ||
        !cargar_avisos(d)) {
        return 0;
    }
    /* Primer arranque: crear los archivos que falten. */
    if (!existe_archivo(ARCH_CLIENTES) && !archivos_guardar_clientes(d)) {
        return 0;
    }
    if (!existe_archivo(ARCH_PRESTAMOS) && !archivos_guardar_prestamos(d)) {
        return 0;
    }
    if (!existe_archivo(ARCH_PAGOS) && !archivos_guardar_pagos(d)) {
        return 0;
    }
    if (!existe_archivo(ARCH_AVISOS) && !archivos_guardar_avisos(d)) {
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------- */
/* Guardado                                                                  */
/* ------------------------------------------------------------------------- */

int archivos_guardar_clientes(const Datos *d)
{
    char final[600], temporal[620];
    FILE *f = abrir_temporal(ARCH_CLIENTES, final, sizeof final, temporal,
                             sizeof temporal);
    size_t i;

    if (!f) {
        return 0;
    }
    for (i = 0; i < d->clientes.len; i++) {
        const Cliente *c = &d->clientes.items[i];
        fprintf(f, "%ld|%s|%s|%s|%s|%s\n", c->id, c->cedula, c->nombre,
                c->apellido, c->telefono, c->direccion);
    }
    return cerrar_temporal(f, final, temporal);
}

int archivos_guardar_prestamos(const Datos *d)
{
    char final[600], temporal[620];
    FILE *f = abrir_temporal(ARCH_PRESTAMOS, final, sizeof final, temporal,
                             sizeof temporal);
    size_t i;

    if (!f) {
        return 0;
    }
    for (i = 0; i < d->prestamos.len; i++) {
        const Prestamo *p = &d->prestamos.items[i];
        fprintf(f, "%ld|%ld|%s|%lld|%lld|%d|%d|%d|%s\n", p->id, p->id_cliente,
                p->vehiculo, p->valor, p->avance, p->meses, p->tasa, p->gracia,
                p->inicio);
    }
    return cerrar_temporal(f, final, temporal);
}

int archivos_guardar_pagos(const Datos *d)
{
    char final[600], temporal[620];
    FILE *f = abrir_temporal(ARCH_PAGOS, final, sizeof final, temporal,
                             sizeof temporal);
    size_t i;

    if (!f) {
        return 0;
    }
    for (i = 0; i < d->pagos.len; i++) {
        const Pago *p = &d->pagos.items[i];
        fprintf(f, "%ld|%d|%s\n", p->id_prestamo, p->num_mes, p->fecha);
    }
    return cerrar_temporal(f, final, temporal);
}

int archivos_guardar_avisos(const Datos *d)
{
    char final[600], temporal[620];
    FILE *f = abrir_temporal(ARCH_AVISOS, final, sizeof final, temporal,
                             sizeof temporal);
    size_t i;

    if (!f) {
        return 0;
    }
    for (i = 0; i < d->avisos.len; i++) {
        const Aviso *a = &d->avisos.items[i];
        fprintf(f, "%s|%ld|%s|%s|%s|%lld|%d|%s\n", a->fecha_hora,
                a->id_prestamo, a->cedula, a->nombre, a->vehiculo, a->financiado,
                a->meses, a->motivo);
    }
    return cerrar_temporal(f, final, temporal);
}

int archivos_guardar_todo(const Datos *d)
{
    int ok = 1;
    ok = archivos_guardar_clientes(d) && ok;
    ok = archivos_guardar_prestamos(d) && ok;
    ok = archivos_guardar_pagos(d) && ok;
    ok = archivos_guardar_avisos(d) && ok;
    return ok;
}
