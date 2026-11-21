#ifndef CLIENTES_H
#define CLIENTES_H

#include "model.h"

const Cliente *clientes_por_id(const Datos *d, long id);
const Cliente *clientes_por_cedula(const Datos *d, const char *cedula);
int clientes_indice_por_id(const Datos *d, long id);
int clientes_contar_prestamos(const Datos *d, long id_cliente);

long clientes_crear(Datos *d, const Cliente *nuevo, char *err, size_t errn);
int clientes_actualizar(Datos *d, long id, const Cliente *nuevo, char *err,
                        size_t errn);
int clientes_eliminar(Datos *d, long id, char *err, size_t errn);

#endif
