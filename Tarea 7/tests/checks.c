#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define CREAR_DIR(p) _mkdir(p)
#define BORRAR_DIR(p) _rmdir(p)
#else
#include <sys/stat.h>
#include <unistd.h>
#define CREAR_DIR(p) mkdir(p, 0777)
#define BORRAR_DIR(p) rmdir(p)
#endif

#include "archivos.h"
#include "clientes.h"
#include "prestamos.h"
#include "reportes.h"
#include "util.h"

static int pruebas = 0;
static int fallos = 0;

#define CHECK(cond)                                                           \
    do {                                                                      \
        pruebas++;                                                            \
        if (!(cond)) {                                                        \
            printf("FALLO linea %d: %s\n", __LINE__, #cond);                  \
            fallos++;                                                         \
        }                                                                     \
    } while (0)

static void llenar_cliente(Cliente *c, const char *cedula, const char *nombre,
                           const char *apellido)
{
    memset(c, 0, sizeof *c);
    snprintf(c->cedula, sizeof c->cedula, "%s", cedula);
    snprintf(c->nombre, sizeof c->nombre, "%s", nombre);
    snprintf(c->apellido, sizeof c->apellido, "%s", apellido);
    snprintf(c->telefono, sizeof c->telefono, "809-555-0101");
    snprintf(c->direccion, sizeof c->direccion, "Calle Duarte 10");
}

static Prestamo prestamo_base(int gracia, int meses)
{
    Prestamo p;

    memset(&p, 0, sizeof p);
    p.valor = 1000000;
    p.avance = 200000;
    p.meses = meses;
    p.tasa = 18;
    p.gracia = gracia;
    snprintf(p.inicio, sizeof p.inicio, "2026-01-15");
    snprintf(p.vehiculo, sizeof p.vehiculo, "Toyota Corolla 2020");
    return p;
}

/* ------------------------------------------------------------------------- */
/* Utilidades                                                                */
/* ------------------------------------------------------------------------- */

static void probar_formatos(void)
{
    char texto[64];

    util_formato_monto(800000, texto, sizeof texto);
    CHECK(strcmp(texto, "800.000") == 0);
    util_formato_monto(0, texto, sizeof texto);
    CHECK(strcmp(texto, "0") == 0);
    util_formato_monto(1234567, texto, sizeof texto);
    CHECK(strcmp(texto, "1.234.567") == 0);
    util_formato_monto(-2500, texto, sizeof texto);
    CHECK(strcmp(texto, "-2.500") == 0);

    util_formato_fecha("2026-02-28", texto, sizeof texto);
    CHECK(strcmp(texto, "28/02/2026") == 0);
}

static void probar_fechas(void)
{
    char texto[16];

    CHECK(util_sumar_meses("2026-01-31", 1, texto, sizeof texto));
    CHECK(strcmp(texto, "2026-02-28") == 0);
    CHECK(util_sumar_meses("2028-01-31", 1, texto, sizeof texto));
    CHECK(strcmp(texto, "2028-02-29") == 0);
    CHECK(util_sumar_meses("2026-01-15", 36, texto, sizeof texto));
    CHECK(strcmp(texto, "2029-01-15") == 0);
    CHECK(util_sumar_meses("2026-01-31", 2, texto, sizeof texto));
    CHECK(strcmp(texto, "2026-03-31") == 0);

    CHECK(util_fecha_valida("2026-02-28"));
    CHECK(util_fecha_valida("2028-02-29"));
    CHECK(!util_fecha_valida("2026-02-29"));
    CHECK(!util_fecha_valida("2026-13-01"));
    CHECK(!util_fecha_valida("2026-00-10"));
    CHECK(!util_fecha_valida("15/01/2026"));
}

static void probar_parseo(void)
{
    long long valor = 0;

    CHECK(util_parse_entero("abc", &valor) == 0);
    CHECK(util_parse_entero("12abc", &valor) == 0);
    CHECK(util_parse_entero("12 ", &valor) == 1 && valor == 12);
    CHECK(util_parse_entero("-5", &valor) == 1 && valor == -5);
    CHECK(util_parse_entero("0", &valor) == 1 && valor == 0);
    CHECK(util_parse_entero("999999999999999999999999", &valor) == 0);
}

static void probar_cedula(void)
{
    char normalizada[CEDULA_LEN];

    CHECK(util_cedula_normalizar("00112345673", normalizada,
                                 sizeof normalizada));
    CHECK(strcmp(normalizada, "001-1234567-3") == 0);
    CHECK(util_cedula_normalizar("001-1234567-3", normalizada,
                                 sizeof normalizada));
    CHECK(strcmp(normalizada, "001-1234567-3") == 0);
    CHECK(!util_cedula_normalizar("001-1234567-8", normalizada,
                                  sizeof normalizada));
    CHECK(!util_cedula_normalizar("001-123456-3", normalizada,
                                  sizeof normalizada));
    CHECK(!util_cedula_normalizar("001-1234567-A", normalizada,
                                  sizeof normalizada));
    CHECK(!util_cedula_normalizar("", normalizada, sizeof normalizada));
}

/* ------------------------------------------------------------------------- */
/* Calculo del prestamo                                                      */
/* ------------------------------------------------------------------------- */

static void probar_calendario(void)
{
    Prestamo p = prestamo_base(0, 36);
    Cuota *cuotas;
    int cantidad = 0;
    int i;
    long long total_cuota = 0;
    long long total_interes = 0;
    long long total_capital = 0;

    CHECK(prestamo_financiado(&p) == 800000);
    CHECK(prestamo_interes(&p) == 12000);
    CHECK(prestamo_meses_amort(&p) == 36);

    cuotas = prestamo_calendario(&p, &cantidad);
    CHECK(cuotas != NULL && cantidad == 36);
    if (cuotas && cantidad == 36) {
        CHECK(cuotas[0].capital == 22223);
        CHECK(cuotas[7].capital == 22223);
        CHECK(cuotas[8].capital == 22222);
        CHECK(cuotas[35].capital == 22222);
        CHECK(cuotas[35].saldo == 0);
        CHECK(strcmp(cuotas[0].vence, "2026-02-15") == 0);
        CHECK(strcmp(cuotas[35].vence, "2029-01-15") == 0);
        for (i = 0; i < cantidad; i++) {
            total_cuota += cuotas[i].cuota;
            total_interes += cuotas[i].interes;
            total_capital += cuotas[i].capital;
        }
        CHECK(total_cuota == 1232000);
        CHECK(total_interes == 432000);
        CHECK(total_capital == 800000);
    }
    prestamo_liberar_calendario(cuotas);

    p = prestamo_base(2, 36);
    CHECK(prestamo_meses_amort(&p) == 34);
    cuotas = prestamo_calendario(&p, &cantidad);
    CHECK(cuotas != NULL && cantidad == 36);
    total_cuota = total_interes = total_capital = 0;
    if (cuotas && cantidad == 36) {
        CHECK(cuotas[0].capital == 0 && cuotas[0].cuota == 12000);
        CHECK(cuotas[1].capital == 0 && cuotas[1].cuota == 12000);
        CHECK(cuotas[0].saldo == 800000 && cuotas[1].saldo == 800000);
        CHECK(cuotas[2].capital == 23530 && cuotas[2].cuota == 35530);
        CHECK(cuotas[15].capital == 23530 && cuotas[15].cuota == 35530);
        CHECK(cuotas[16].capital == 23529 && cuotas[16].cuota == 35529);
        CHECK(cuotas[35].capital == 23529 && cuotas[35].saldo == 0);
        for (i = 0; i < cantidad; i++) {
            total_cuota += cuotas[i].cuota;
            total_interes += cuotas[i].interes;
            total_capital += cuotas[i].capital;
            if (i > 0) {
                CHECK(cuotas[i].saldo <= cuotas[i - 1].saldo);
            }
        }
        CHECK(total_cuota == 1232000);
        CHECK(total_interes == 432000);
        CHECK(total_capital == 800000);
    }
    prestamo_liberar_calendario(cuotas);

    p = prestamo_base(0, 36);
    snprintf(p.inicio, sizeof p.inicio, "2026-01-31");
    cuotas = prestamo_calendario(&p, &cantidad);
    CHECK(cuotas != NULL && cantidad == 36);
    if (cuotas && cantidad == 36) {
        CHECK(strcmp(cuotas[0].vence, "2026-02-28") == 0);
        CHECK(strcmp(cuotas[1].vence, "2026-03-31") == 0);
    }
    prestamo_liberar_calendario(cuotas);

    {
        Datos d;
        Reporte *r;

        memset(&d, 0, sizeof d);
        r = reporte_calendario(&d, &p);
        CHECK(r != NULL);
        if (r) {
            CHECK(r->n_filas == 36);
            reporte_liberar(r);
        }
    }
}

static void probar_busqueda(void)
{
    Datos d;
    Cliente c;
    char err[UTIL_ERR_LEN];
    Reporte *r;

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "001-1234567-3", "Ana", "Perez");
    clientes_crear(&d, &c, err, sizeof err);
    llenar_cliente(&c, "402-1234567-8", "Luis", "Diaz");
    clientes_crear(&d, &c, err, sizeof err);

    r = reporte_clientes(&d, "per");
    CHECK(r != NULL && r->n_filas == 1);
    if (r && r->n_filas == 1) {
        CHECK(strstr(r->filas[0], "Perez") != NULL);
    }
    reporte_liberar(r);

    r = reporte_clientes(&d, "00112345673");
    CHECK(r != NULL && r->n_filas == 1);
    if (r && r->n_filas == 1) {
        CHECK(strstr(r->filas[0], "001-1234567-3") != NULL);
    }
    reporte_liberar(r);

    r = reporte_clientes(&d, "inexistente");
    CHECK(r != NULL && r->n_filas == 1);
    if (r && r->n_filas == 1) {
        CHECK(strstr(r->filas[0], "NO SE ENCONTRARON") != NULL);
    }
    reporte_liberar(r);

    datos_liberar(&d);
}

/* ------------------------------------------------------------------------- */
/* Clientes y prestamos en memoria                                           */
/* ------------------------------------------------------------------------- */

static void probar_clientes(void)
{
    Datos d;
    Cliente c;
    Prestamo p;
    char err[UTIL_ERR_LEN];
    long id;
    long id_prestamo;

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "001-1234567-3", "Ana", "Perez");
    id = clientes_crear(&d, &c, err, sizeof err);
    CHECK(id == 1);
    CHECK(d.clientes.len == 1);

    CHECK(clientes_crear(&d, &c, err, sizeof err) == -1);
    CHECK(d.clientes.len == 1);

    llenar_cliente(&c, "001-1234567-8", "Luis", "Diaz");
    CHECK(clientes_crear(&d, &c, err, sizeof err) == -1);

    CHECK(clientes_por_cedula(&d, "00112345673") ==
          clientes_por_id(&d, id));
    CHECK(clientes_por_id(&d, 99) == NULL);

    p = prestamo_base(0, 6);
    p.id_cliente = id;
    p.valor = 500000;
    p.avance = 100000;
    id_prestamo = prestamos_crear(&d, &p, err, sizeof err);
    CHECK(id_prestamo == 1);
    p = prestamo_base(0, 6);
    p.id_cliente = id;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == 2);
    CHECK(clientes_contar_prestamos(&d, id) == 2);

    CHECK(clientes_eliminar(&d, id, err, sizeof err) == 0);
    CHECK(strstr(err, "2 prestamo") != NULL);
    CHECK(d.clientes.len == 1);

    CHECK(prestamos_eliminar(&d, id_prestamo, err, sizeof err) == 1);
    CHECK(prestamos_eliminar(&d, 2, err, sizeof err) == 1);
    CHECK(d.prestamos.len == 0);
    CHECK(d.avisos.len == 2);
    if (d.avisos.len == 2) {
        CHECK(strcmp(d.avisos.items[0].motivo, "falta de pagos") == 0);
        CHECK(strcmp(d.avisos.items[0].cedula, "001-1234567-3") == 0);
        CHECK(d.avisos.items[0].financiado == 400000);
        CHECK(d.avisos.items[0].meses == 6);
        CHECK(strcmp(d.avisos.items[1].motivo, "falta de pagos") == 0);
    }

    CHECK(clientes_eliminar(&d, id, err, sizeof err) == 1);
    CHECK(d.clientes.len == 0);

    datos_liberar(&d);
}

static void probar_validaciones(void)
{
    Datos d;
    Cliente c;
    Prestamo p;
    char err[UTIL_ERR_LEN];

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "001-1234567-3", "Ana", "Perez");
    clientes_crear(&d, &c, err, sizeof err);

    p = prestamo_base(0, 36);
    p.id_cliente = 1;
    p.meses = 0;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(0, 121);
    p.id_cliente = 1;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(0, 36);
    p.id_cliente = 1;
    p.tasa = 0;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(0, 36);
    p.id_cliente = 1;
    p.tasa = 101;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(36, 36);
    p.id_cliente = 1;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(0, 36);
    p.id_cliente = 1;
    p.avance = -1;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(0, 36);
    p.id_cliente = 1;
    p.avance = p.valor;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    p = prestamo_base(0, 36);
    p.id_cliente = 99;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == -1);
    CHECK(d.prestamos.len == 0);

    p = prestamo_base(0, 36);
    p.id_cliente = 1;
    CHECK(prestamos_crear(&d, &p, err, sizeof err) == 1);

    datos_liberar(&d);
}

static void probar_pagos(void)
{
    Datos d;
    Cliente c;
    Prestamo p;
    char err[UTIL_ERR_LEN];
    long id;
    long id_prestamo;

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "402-1234567-8", "Maria", "Santos");
    id = clientes_crear(&d, &c, err, sizeof err);
    CHECK(id == 1);

    p = prestamo_base(0, 4);
    p.id_cliente = id;
    id_prestamo = prestamos_crear(&d, &p, err, sizeof err);
    CHECK(id_prestamo == 1);

    CHECK(prestamos_registrar_pago(&d, id_prestamo, 2, err, sizeof err) == 0);
    CHECK(d.pagos.len == 0);
    CHECK(prestamo_siguiente_cuota(&d, &d.prestamos.items[0]) == 1);

    CHECK(prestamos_registrar_pago(&d, id_prestamo, 1, err, sizeof err) == 1);
    CHECK(prestamos_registrar_pago(&d, id_prestamo, 2, err, sizeof err) == 1);
    CHECK(prestamos_registrar_pago(&d, id_prestamo, 3, err, sizeof err) == 1);
    CHECK(prestamo_siguiente_cuota(&d, &d.prestamos.items[0]) == 4);
    CHECK(prestamo_cuota_pagada(&d, id_prestamo, 1));
    CHECK(prestamo_cuota_pagada(&d, id_prestamo, 2));
    CHECK(prestamo_cuota_pagada(&d, id_prestamo, 3));
    CHECK(!prestamo_cuota_pagada(&d, id_prestamo, 4));

    CHECK(prestamos_registrar_pago(&d, id_prestamo, 4, err, sizeof err) == 1);
    CHECK(prestamo_cancelado(&d, &d.prestamos.items[0]));
    CHECK(prestamos_registrar_pago(&d, id_prestamo, 4, err, sizeof err) == 0);
    CHECK(d.pagos.len == 4);

    CHECK(prestamos_eliminar(&d, id_prestamo, err, sizeof err) == 0);
    CHECK(d.prestamos.len == 1);
    CHECK(d.pagos.len == 4);

    datos_liberar(&d);
}

static void probar_modificacion(void)
{
    Datos d;
    Cliente c;
    Cliente nuevo;
    Prestamo p;
    Prestamo cambio;
    char err[UTIL_ERR_LEN];
    long id;
    long id_prestamo;
    Cuota *cuotas;
    int cantidad = 0;

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "001-1234567-3", "Ana", "Perez");
    id = clientes_crear(&d, &c, err, sizeof err);

    p = prestamo_base(0, 12);
    p.id_cliente = id;
    id_prestamo = prestamos_crear(&d, &p, err, sizeof err);
    CHECK(id_prestamo == 1);

    /* Cambio de cedula: los prestamos siguen ligados al id interno. */
    nuevo = *clientes_por_id(&d, id);
    snprintf(nuevo.cedula, sizeof nuevo.cedula, "402-1234567-8");
    CHECK(clientes_actualizar(&d, id, &nuevo, err, sizeof err) == 1);
    CHECK(strcmp(clientes_por_id(&d, id)->cedula, "402-1234567-8") == 0);
    CHECK(d.prestamos.items[0].id_cliente == id);

    llenar_cliente(&c, "402-1234567-8", "Otra", "Persona");
    CHECK(clientes_crear(&d, &c, err, sizeof err) == -1);

    /* Cambio de tasa y gracia sin pagos: recalcula el calendario. */
    cambio = *prestamos_por_id(&d, id_prestamo);
    cambio.tasa = 24;
    cambio.gracia = 2;
    CHECK(prestamos_actualizar(&d, id_prestamo, &cambio, err, sizeof err) ==
          1);
    CHECK(prestamos_por_id(&d, id_prestamo)->tasa == 24);
    CHECK(prestamos_por_id(&d, id_prestamo)->gracia == 2);
    CHECK(prestamo_interes(prestamos_por_id(&d, id_prestamo)) == 16000);
    cuotas = prestamo_calendario(prestamos_por_id(&d, id_prestamo), &cantidad);
    CHECK(cuotas != NULL && cantidad == 12);
    if (cuotas && cantidad == 12) {
        CHECK(cuotas[0].capital == 0 && cuotas[0].cuota == 16000);
        CHECK(cuotas[1].capital == 0);
        CHECK(cuotas[2].capital > 0);
    }
    prestamo_liberar_calendario(cuotas);

    /* Con un pago, solo se permite cambiar la descripcion. */
    CHECK(prestamos_registrar_pago(&d, id_prestamo, 1, err, sizeof err) == 1);
    cambio = *prestamos_por_id(&d, id_prestamo);
    cambio.meses = 24;
    CHECK(prestamos_actualizar(&d, id_prestamo, &cambio, err, sizeof err) == 0);
    cambio = *prestamos_por_id(&d, id_prestamo);
    snprintf(cambio.vehiculo, sizeof cambio.vehiculo, "Toyota Corolla 2021");
    CHECK(prestamos_actualizar(&d, id_prestamo, &cambio, err, sizeof err) == 1);
    CHECK(strcmp(prestamos_por_id(&d, id_prestamo)->vehiculo,
                 "Toyota Corolla 2021") == 0);
    CHECK(d.pagos.len == 1);

    datos_liberar(&d);
}

static void probar_estados(void)
{
    Datos d;
    Cliente c;
    Prestamo p;
    char err[UTIL_ERR_LEN];
    long id;
    long pasado;
    long futuro;

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "001-1234567-3", "Ana", "Perez");
    id = clientes_crear(&d, &c, err, sizeof err);

    p = prestamo_base(0, 12);
    p.id_cliente = id;
    snprintf(p.inicio, sizeof p.inicio, "2020-01-01");
    pasado = prestamos_crear(&d, &p, err, sizeof err);
    p = prestamo_base(0, 12);
    p.id_cliente = id;
    snprintf(p.inicio, sizeof p.inicio, "2035-01-01");
    futuro = prestamos_crear(&d, &p, err, sizeof err);

    CHECK(prestamo_estado(&d, pasado, 1, "2020-02-01") == ESTADO_VENCIDA);
    CHECK(prestamo_estado(&d, futuro, 1, "2035-02-01") == ESTADO_PENDIENTE);
    CHECK(prestamos_registrar_pago(&d, pasado, 1, err, sizeof err) == 1);
    CHECK(prestamo_estado(&d, pasado, 1, "2020-02-01") == ESTADO_PAGADA);

    datos_liberar(&d);
}

static void probar_reportes(void)
{
    Datos d;
    Cliente c;
    Prestamo p;
    char err[UTIL_ERR_LEN];
    long ana;
    long luis;
    long sin_prestamos;
    Reporte *r;
    int i;

    memset(&d, 0, sizeof d);
    llenar_cliente(&c, "001-1234567-3", "Ana", "Perez");
    ana = clientes_crear(&d, &c, err, sizeof err);
    llenar_cliente(&c, "402-1234567-8", "Luis", "Diaz");
    luis = clientes_crear(&d, &c, err, sizeof err);
    llenar_cliente(&c, "001-0000000-9", "Rosa", "Nunez");
    sin_prestamos = clientes_crear(&d, &c, err, sizeof err);

    p = prestamo_base(0, 12);
    p.id_cliente = ana;
    snprintf(p.inicio, sizeof p.inicio, "2020-01-01");
    prestamos_crear(&d, &p, err, sizeof err);
    p = prestamo_base(0, 12);
    p.id_cliente = ana;
    snprintf(p.inicio, sizeof p.inicio, "2035-01-01");
    prestamos_crear(&d, &p, err, sizeof err);
    p = prestamo_base(0, 12);
    p.id_cliente = luis;
    snprintf(p.inicio, sizeof p.inicio, "2020-01-01");
    prestamos_crear(&d, &p, err, sizeof err);

    r = reporte_vencidas(&d);
    CHECK(r != NULL && r->n_filas == 24 && r->hay_totales);
    for (i = 0; r && i < r->n_filas; i++) {
        CHECK(strstr(r->filas[i], "2035") == NULL);
    }
    reporte_liberar(r);

    r = reporte_prestamos_por_cliente(&d, clientes_por_id(&d, ana));
    CHECK(r != NULL && r->n_filas == 2 && r->hay_totales);
    reporte_liberar(r);

    r = reporte_prestamos_por_cliente(&d, clientes_por_id(&d, sin_prestamos));
    CHECK(r != NULL && r->n_filas == 1 && !r->hay_totales);
    if (r && r->n_filas == 1) {
        CHECK(strstr(r->filas[0], "NO TIENE PRESTAMOS") != NULL);
    }
    reporte_liberar(r);

    datos_liberar(&d);
}

/* ------------------------------------------------------------------------- */
/* Persistencia                                                              */
/* ------------------------------------------------------------------------- */

static void borrar_archivos_prueba(void)
{
    remove("build/testdata/clientes.txt");
    remove("build/testdata/prestamos.txt");
    remove("build/testdata/pagos.txt");
    remove("build/testdata/avisos.txt");
}

static void probar_archivos(void)
{
    Datos d;
    Datos e;
    Cliente c;
    char err[UTIL_ERR_LEN];
    int i;
    const char *cedulas[3] = {"001-1234567-3", "402-1234567-8",
                              "001-0000000-9"};

    CREAR_DIR("build");
    CREAR_DIR("build/testdata");
    borrar_archivos_prueba();
    archivos_usar_directorio("build/testdata");

    memset(&d, 0, sizeof d);
    CHECK(archivos_cargar(&d));
    CHECK(d.clientes.len == 0);
    CHECK(d.prestamos.len == 0);

    for (i = 0; i < 3; i++) {
        llenar_cliente(&c, cedulas[i], "Cliente", "Prueba");
        CHECK(clientes_crear(&d, &c, err, sizeof err) == i + 1);
    }
    CHECK(archivos_guardar_todo(&d));
    datos_liberar(&d);

    memset(&e, 0, sizeof e);
    CHECK(archivos_cargar(&e));
    CHECK(e.clientes.len == 3);
    if (e.clientes.len == 3) {
        CHECK(strcmp(e.clientes.items[0].cedula, "001-1234567-3") == 0);
        CHECK(strcmp(e.clientes.items[1].cedula, "402-1234567-8") == 0);
        CHECK(strcmp(e.clientes.items[2].cedula, "001-0000000-9") == 0);
        CHECK(e.clientes.items[0].id == 1);
        CHECK(e.clientes.items[2].id == 3);
    }
    datos_liberar(&e);

    borrar_archivos_prueba();
    BORRAR_DIR("build/testdata");
    archivos_usar_directorio(NULL);
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    probar_formatos();
    probar_fechas();
    probar_parseo();
    probar_cedula();
    probar_calendario();
    probar_busqueda();
    probar_clientes();
    probar_validaciones();
    probar_pagos();
    probar_modificacion();
    probar_estados();
    probar_reportes();
    probar_archivos();

    printf("\n%d prueba(s), %d fallo(s).\n", pruebas, fallos);
    if (fallos == 0) {
        printf("TODO EN VERDE\n");
        return 0;
    }
    return 1;
}
