#include "clientes.h"

#include <stdio.h>
#include <string.h>

#include "archivos.h"
#include "util.h"

int clientes_indice_por_id(const Datos *d, long id)
{
    size_t i;

    for (i = 0; i < d->clientes.len; i++) {
        if (d->clientes.items[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

const Cliente *clientes_por_id(const Datos *d, long id)
{
    int i = clientes_indice_por_id(d, id);
    return i < 0 ? NULL : &d->clientes.items[i];
}

const Cliente *clientes_por_cedula(const Datos *d, const char *cedula)
{
    char normalizada[CEDULA_LEN];
    size_t i;
    int valida;

    if (!cedula || !cedula[0]) {
        return NULL;
    }
    valida = util_cedula_normalizar(cedula, normalizada, sizeof normalizada);
    for (i = 0; i < d->clientes.len; i++) {
        const Cliente *c = &d->clientes.items[i];
        if (valida ? strcmp(c->cedula, normalizada) == 0
                   : strcmp(c->cedula, cedula) == 0) {
            return c;
        }
    }
    return NULL;
}

int clientes_contar_prestamos(const Datos *d, long id_cliente)
{
    int total = 0;
    size_t i;

    for (i = 0; i < d->prestamos.len; i++) {
        if (d->prestamos.items[i].id_cliente == id_cliente) {
            total++;
        }
    }
    return total;
}

static int campos_completos(const Cliente *c, char *err, size_t errn)
{
    if (!c->nombre[0]) {
        snprintf(err, errn, "El nombre no puede quedar vacio.");
        return 0;
    }
    if (!c->apellido[0]) {
        snprintf(err, errn, "El apellido no puede quedar vacio.");
        return 0;
    }
    if (!c->telefono[0]) {
        snprintf(err, errn, "El telefono no puede quedar vacio.");
        return 0;
    }
    if (!c->direccion[0]) {
        snprintf(err, errn, "La direccion no puede quedar vacia.");
        return 0;
    }
    return 1;
}

static int preparar_cedula(const Datos *d, const char *entrada, long excepto_id,
                           char *destino, size_t n, char *err, size_t errn)
{
    const Cliente *otro;

    if (!entrada || !entrada[0]) {
        snprintf(err, errn, "La cedula es obligatoria.");
        return 0;
    }
    if (!util_cedula_normalizar(entrada, destino, n)) {
        snprintf(err, errn,
                 "La cedula no es valida: debe tener 11 digitos con digito "
                 "verificador correcto (000-0000000-0).");
        return 0;
    }
    otro = clientes_por_cedula(d, destino);
    if (otro && otro->id != excepto_id) {
        snprintf(err, errn, "La cedula %s ya esta registrada.", destino);
        return 0;
    }
    return 1;
}

long clientes_crear(Datos *d, const Cliente *nuevo, char *err, size_t errn)
{
    Cliente c = *nuevo;

    if (!preparar_cedula(d, c.cedula, 0, c.cedula, sizeof c.cedula, err, errn)) {
        return -1;
    }
    if (!campos_completos(&c, err, errn)) {
        return -1;
    }
    c.id = datos_siguiente_id_clientes(d);
    datos_agregar_cliente(d, &c);
    return c.id;
}

int clientes_actualizar(Datos *d, long id, const Cliente *nuevo, char *err,
                        size_t errn)
{
    Cliente c = *nuevo;
    Cliente *actual;
    int indice = clientes_indice_por_id(d, id);

    if (indice < 0) {
        snprintf(err, errn, "El cliente con id %ld no existe.", id);
        return 0;
    }
    if (!preparar_cedula(d, c.cedula, id, c.cedula, sizeof c.cedula, err, errn)) {
        return 0;
    }
    if (!campos_completos(&c, err, errn)) {
        return 0;
    }
    actual = &d->clientes.items[indice];
    c.id = actual->id;
    *actual = c;
    return 1;
}

int clientes_eliminar(Datos *d, long id, char *err, size_t errn)
{
    int indice = clientes_indice_por_id(d, id);
    int prestamos;

    if (indice < 0) {
        snprintf(err, errn, "El cliente con id %ld no existe.", id);
        return 0;
    }
    prestamos = clientes_contar_prestamos(d, id);
    if (prestamos > 0) {
        snprintf(err, errn,
                 "No se puede eliminar: el cliente tiene %d prestamo(s) "
                 "asociado(s).",
                 prestamos);
        return 0;
    }
    datos_eliminar_cliente(d, (size_t)indice);
    return 1;
}
