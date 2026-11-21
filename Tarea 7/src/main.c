#include <stdio.h>
#include <string.h>

#include "archivos.h"
#include "clientes.h"
#include "model.h"
#include "prestamos.h"
#include "reportes.h"
#include "tui.h"
#include "util.h"

/* ------------------------------------------------------------------------- */
/* Apoyo                                                                     */
/* ------------------------------------------------------------------------- */

static int guardar(const Datos *d)
{
    if (!archivos_guardar_todo(d)) {
        printf("ERROR: no se pudieron guardar los archivos de datos.\n");
        return 0;
    }
    return 1;
}

static void mostrar_error(const char *mensaje)
{
    printf("ERROR: %s\n", mensaje);
}

static const Cliente *elegir_cliente(const Datos *d)
{
    long id = util_leer_entero("Id del cliente (0 = cancelar)", 0, 999999999L);
    const Cliente *c;

    if (id == 0) {
        return NULL;
    }
    c = clientes_por_id(d, id);
    if (!c) {
        printf("No existe un cliente con id %ld.\n", id);
    }
    return c;
}

static const Prestamo *elegir_prestamo(const Datos *d)
{
    long id = util_leer_entero("Id del prestamo (0 = cancelar)", 0, 999999999L);
    const Prestamo *p;

    if (id == 0) {
        return NULL;
    }
    p = prestamos_por_id(d, id);
    if (!p) {
        printf("No existe un prestamo con id %ld.\n", id);
    }
    return p;
}

static void mostrar_reporte(Reporte *r)
{
    if (!r) {
        return;
    }
    tui_mostrar_reporte(r);
    reporte_liberar(r);
}

/* ------------------------------------------------------------------------- */
/* Clientes                                                                  */
/* ------------------------------------------------------------------------- */

static void clientes_alta(Datos *d)
{
    Cliente c;
    char err[UTIL_ERR_LEN];
    long id;

    memset(&c, 0, sizeof c);
    printf("\n--- REGISTRAR CLIENTE ---\n");
    util_leer_texto("Cedula (000-0000000-0)", c.cedula, sizeof c.cedula);
    util_leer_texto("Nombre", c.nombre, sizeof c.nombre);
    util_leer_texto("Apellido", c.apellido, sizeof c.apellido);
    util_leer_texto("Telefono", c.telefono, sizeof c.telefono);
    util_leer_texto("Direccion", c.direccion, sizeof c.direccion);

    id = clientes_crear(d, &c, err, sizeof err);
    if (id < 0) {
        mostrar_error(err);
        return;
    }
    if (guardar(d)) {
        printf("Cliente registrado con id %ld.\n", id);
    }
}

static void clientes_listar(const Datos *d)
{
    char busqueda[64];

    util_leer_linea_libre("Buscar por cedula, nombre o apellido (vacio = todos)",
                          busqueda, sizeof busqueda);
    mostrar_reporte(reporte_clientes(d, busqueda));
}

static void clientes_modificar(Datos *d)
{
    const Cliente *c = elegir_cliente(d);
    Cliente nuevo;
    char err[UTIL_ERR_LEN];

    if (!c) {
        return;
    }
    nuevo = *c;
    printf("Actual: %s %s | %s | %s | %s\n", c->nombre, c->apellido, c->cedula,
           c->telefono, c->direccion);
    util_leer_texto_def("Cedula", nuevo.cedula, sizeof nuevo.cedula, c->cedula);
    util_leer_texto_def("Nombre", nuevo.nombre, sizeof nuevo.nombre, c->nombre);
    util_leer_texto_def("Apellido", nuevo.apellido, sizeof nuevo.apellido,
                        c->apellido);
    util_leer_texto_def("Telefono", nuevo.telefono, sizeof nuevo.telefono,
                        c->telefono);
    util_leer_texto_def("Direccion", nuevo.direccion, sizeof nuevo.direccion,
                        c->direccion);

    if (!clientes_actualizar(d, c->id, &nuevo, err, sizeof err)) {
        mostrar_error(err);
        return;
    }
    if (guardar(d)) {
        printf("Cliente actualizado. Sus prestamos siguen asociados.\n");
    }
}

static void clientes_eliminar_menu(Datos *d)
{
    const Cliente *c = elegir_cliente(d);
    char err[UTIL_ERR_LEN];
    int prestamos;

    if (!c) {
        return;
    }
    prestamos = clientes_contar_prestamos(d, c->id);
    if (prestamos > 0) {
        printf("No se puede eliminar: el cliente tiene %d prestamo(s) "
               "asociado(s).\n",
               prestamos);
        return;
    }
    printf("Cliente: %s %s | %s\n", c->nombre, c->apellido, c->cedula);
    if (!util_confirmar("Eliminar este cliente?")) {
        printf("Operacion cancelada.\n");
        return;
    }
    if (!clientes_eliminar(d, c->id, err, sizeof err)) {
        mostrar_error(err);
        return;
    }
    if (guardar(d)) {
        printf("Cliente eliminado.\n");
    }
}

static void menu_clientes(Datos *d)
{
    for (;;) {
        long opcion;

        printf("\n=== CLIENTES ===\n");
        printf("1. Registrar cliente\n");
        printf("2. Listar o buscar clientes\n");
        printf("3. Modificar cliente\n");
        printf("4. Eliminar cliente\n");
        printf("5. Volver al menu principal\n");
        opcion = util_leer_entero("Opcion", 1, 5);

        switch (opcion) {
        case 1:
            clientes_alta(d);
            break;
        case 2:
            clientes_listar(d);
            break;
        case 3:
            clientes_modificar(d);
            break;
        case 4:
            clientes_eliminar_menu(d);
            break;
        default:
            return;
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Prestamos                                                                 */
/* ------------------------------------------------------------------------- */

static void prestamos_alta(Datos *d)
{
    const Cliente *c = elegir_cliente(d);
    Prestamo p;
    char err[UTIL_ERR_LEN];
    char hoy[FECHA_LEN];
    long id;

    if (!c) {
        return;
    }
    memset(&p, 0, sizeof p);
    p.id_cliente = c->id;
    util_hoy_iso(hoy, sizeof hoy);

    printf("\n--- REGISTRAR PRESTAMO PARA %s %s ---\n", c->nombre,
           c->apellido);
    util_leer_texto("Descripcion del vehiculo", p.vehiculo, sizeof p.vehiculo);
    p.valor = util_leer_entero("Valor del vehiculo", -1000000000000LL,
                               1000000000000LL);
    p.avance = util_leer_entero("Avance del cliente", -1000000000000LL,
                                1000000000000LL);
    p.meses = (int)util_leer_entero("Meses del prestamo (1-120)", -10, 100000);
    p.tasa = (int)util_leer_entero("Tasa anual en porcentaje (1-100)", -100,
                                   100000);
    p.gracia = (int)util_leer_entero("Meses de gracia de capital", -10, 100000);
    util_leer_texto_def("Fecha de inicio (YYYY-MM-DD)", p.inicio,
                        sizeof p.inicio, hoy);

    id = prestamos_crear(d, &p, err, sizeof err);
    if (id < 0) {
        mostrar_error(err);
        return;
    }
    if (!guardar(d)) {
        return;
    }
    printf("Prestamo registrado con id %ld. Financiado: ", id);
    {
        char monto[32];
        util_formato_monto(prestamo_financiado(&p), monto, sizeof monto);
        printf("%s\n", monto);
    }
    mostrar_reporte(reporte_calendario(d, prestamos_por_id(d, id)));
}

static void prestamos_modificar(Datos *d)
{
    const Prestamo *p = elegir_prestamo(d);
    Prestamo nuevo;
    char err[UTIL_ERR_LEN];
    int pagos;

    if (!p) {
        return;
    }
    nuevo = *p;
    pagos = prestamo_pagos(d, p->id);

    if (pagos > 0) {
        printf("El prestamo tiene %d pago(s): solo puede cambiar la "
               "descripcion del vehiculo.\n",
               pagos);
        util_leer_texto_def("Descripcion del vehiculo", nuevo.vehiculo,
                            sizeof nuevo.vehiculo, p->vehiculo);
    } else {
        printf("\n--- MODIFICAR PRESTAMO #%ld ---\n", p->id);
        util_leer_texto_def("Descripcion del vehiculo", nuevo.vehiculo,
                            sizeof nuevo.vehiculo, p->vehiculo);
        nuevo.valor = util_leer_entero_def("Valor del vehiculo",
                                           -1000000000000LL,
                                           1000000000000LL, p->valor);
        nuevo.avance = util_leer_entero_def("Avance del cliente",
                                            -1000000000000LL,
                                            1000000000000LL, p->avance);
        nuevo.meses = (int)util_leer_entero_def("Meses (1-120)", -10, 100000,
                                                p->meses);
        nuevo.tasa = (int)util_leer_entero_def("Tasa anual (1-100)", -100,
                                               100000, p->tasa);
        nuevo.gracia = (int)util_leer_entero_def("Meses de gracia", -10, 100000,
                                                 p->gracia);
        util_leer_texto_def("Fecha de inicio (YYYY-MM-DD)", nuevo.inicio,
                            sizeof nuevo.inicio, p->inicio);
    }

    if (!prestamos_actualizar(d, p->id, &nuevo, err, sizeof err)) {
        mostrar_error(err);
        return;
    }
    if (guardar(d)) {
        printf("Prestamo actualizado.\n");
        mostrar_reporte(reporte_calendario(d, prestamos_por_id(d, p->id)));
    }
}

static void prestamos_pagar(Datos *d)
{
    const Prestamo *p = elegir_prestamo(d);
    char err[UTIL_ERR_LEN];
    int pagadas;
    int siguiente;
    long num;
    Cuota *cuotas;
    int cantidad = 0;
    long long monto = 0;

    if (!p) {
        return;
    }
    pagadas = prestamo_pagos(d, p->id);
    if (pagadas >= p->meses) {
        printf("El prestamo esta cancelado: todas las cuotas estan pagadas.\n");
        return;
    }
    siguiente = pagadas + 1;
    cuotas = prestamo_calendario(p, &cantidad);
    if (cuotas && siguiente <= cantidad) {
        monto = cuotas[siguiente - 1].cuota;
    }
    prestamo_liberar_calendario(cuotas);

    {
        char monto_txt[32];
        util_formato_monto(monto, monto_txt, sizeof monto_txt);
        printf("Pago pendiente: cuota %d de %d, monto %s.\n", siguiente,
               p->meses, monto_txt);
    }
    num = util_leer_entero_def("Numero de cuota a pagar", 1, p->meses,
                               siguiente);
    if (!util_confirmar("Registrar el pago ahora?")) {
        printf("Operacion cancelada.\n");
        return;
    }
    if (!prestamos_registrar_pago(d, p->id, (int)num, err, sizeof err)) {
        mostrar_error(err);
        return;
    }
    if (guardar(d)) {
        printf("Pago de la cuota %ld registrado con la fecha del dia.\n", num);
    }
}

static void prestamos_eliminar_menu(Datos *d)
{
    const Prestamo *p = elegir_prestamo(d);
    char err[UTIL_ERR_LEN];
    int pagos;

    if (!p) {
        return;
    }
    pagos = prestamo_pagos(d, p->id);
    if (pagos > 0) {
        printf("No se puede eliminar: el prestamo tiene %d pago(s) "
               "registrados y debe conservarse el historial.\n",
               pagos);
        return;
    }
    printf("\nADVERTENCIA: se eliminara el prestamo #%ld (%s) por falta de "
           "pagos.\nEsta operacion no se puede deshacer y quedara registrada "
           "en la bitacora.\n",
           p->id, p->vehiculo);
    if (!util_confirmar_si("Confirma la eliminacion?")) {
        printf("Operacion cancelada. El prestamo se conserva.\n");
        return;
    }
    if (!prestamos_eliminar(d, p->id, err, sizeof err)) {
        mostrar_error(err);
        return;
    }
    if (guardar(d)) {
        printf("Prestamo eliminado. Se registro el aviso en la bitacora.\n");
    }
}

static void menu_prestamos(Datos *d)
{
    for (;;) {
        long opcion;

        printf("\n=== PRESTAMOS ===\n");
        printf("1. Registrar prestamo\n");
        printf("2. Listar prestamos\n");
        printf("3. Modificar prestamo\n");
        printf("4. Registrar pago\n");
        printf("5. Consultar calendario de cuotas\n");
        printf("6. Eliminar prestamo sin pagos\n");
        printf("7. Volver al menu principal\n");
        opcion = util_leer_entero("Opcion", 1, 7);

        switch (opcion) {
        case 1:
            prestamos_alta(d);
            break;
        case 2:
            mostrar_reporte(reporte_prestamos(d));
            break;
        case 3:
            prestamos_modificar(d);
            break;
        case 4:
            prestamos_pagar(d);
            break;
        case 5: {
            const Prestamo *p = elegir_prestamo(d);
            if (p) {
                mostrar_reporte(reporte_calendario(d, p));
            }
            break;
        }
        case 6:
            prestamos_eliminar_menu(d);
            break;
        default:
            return;
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Reportes                                                                  */
/* ------------------------------------------------------------------------- */

static void reportes_clientes(const Datos *d)
{
    char busqueda[64];

    util_leer_linea_libre("Buscar por cedula, nombre o apellido (vacio = todos)",
                          busqueda, sizeof busqueda);
    mostrar_reporte(reporte_clientes(d, busqueda));
}

static void reportes_prestamos_cliente(const Datos *d)
{
    const Cliente *c = elegir_cliente(d);

    if (c) {
        mostrar_reporte(reporte_prestamos_por_cliente(d, c));
    }
}

static void reportes_calendario(const Datos *d)
{
    const Prestamo *p = elegir_prestamo(d);

    if (p) {
        mostrar_reporte(reporte_calendario(d, p));
    }
}

static void menu_reportes(const Datos *d)
{
    for (;;) {
        long opcion;

        printf("\n=== REPORTES ===\n");
        printf("1. Clientes (listado o busqueda)\n");
        printf("2. Prestamos por cliente\n");
        printf("3. Calendario de cuotas con saldos\n");
        printf("4. Cuotas vencidas\n");
        printf("5. Bitacora de avisos\n");
        printf("6. Volver al menu principal\n");
        opcion = util_leer_entero("Opcion", 1, 6);

        switch (opcion) {
        case 1:
            reportes_clientes(d);
            break;
        case 2:
            reportes_prestamos_cliente(d);
            break;
        case 3:
            reportes_calendario(d);
            break;
        case 4:
            mostrar_reporte(reporte_vencidas(d));
            break;
        case 5:
            mostrar_reporte(reporte_avisos(d));
            break;
        default:
            return;
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Principal                                                                 */
/* ------------------------------------------------------------------------- */

int main(void)
{
    Datos datos;

    tui_iniciar();
    if (!archivos_cargar(&datos)) {
        printf("ERROR: no se pudieron leer los archivos de datos.\n");
        datos_liberar(&datos);
        return 1;
    }

    for (;;) {
        long opcion;

        printf("\n=== GESTION DE PRESTAMOS DE VEHICULOS ===\n");
        printf("1. Clientes\n");
        printf("2. Prestamos\n");
        printf("3. Reportes\n");
        printf("4. Salir\n");
        opcion = util_leer_entero("Opcion", 1, 4);

        switch (opcion) {
        case 1:
            menu_clientes(&datos);
            break;
        case 2:
            menu_prestamos(&datos);
            break;
        case 3:
            menu_reportes(&datos);
            break;
        default:
            datos_liberar(&datos);
            tui_terminar();
            printf("Hasta luego.\n");
            return 0;
        }
    }
}
