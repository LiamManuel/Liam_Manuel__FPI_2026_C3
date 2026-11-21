#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define UTIL_ERR_LEN 200

/* --- Entrada por teclado ------------------------------------------------ */

void util_leer_texto(const char *prompt, char *buf, size_t n);
void util_leer_texto_def(const char *prompt, char *buf, size_t n,
                         const char *defecto);
void util_leer_linea_libre(const char *prompt, char *buf, size_t n);
void util_pausa(void);
long long util_leer_entero(const char *prompt, long long min, long long max);
long long util_leer_entero_def(const char *prompt, long long min, long long max,
                               long long defecto);
int util_parse_entero(const char *texto, long long *salida);
int util_confirmar(const char *prompt);
int util_confirmar_si(const char *prompt);

/* --- Formatos ------------------------------------------------------------ */

void util_formato_monto(long long valor, char *out, size_t n);
void util_formato_fecha(const char *iso, char *out, size_t n);

/* --- Fechas -------------------------------------------------------------- */

void util_hoy_iso(char *out, size_t n);
void util_ahora_texto(char *out, size_t n);
int util_fecha_valida(const char *iso);
int util_sumar_meses(const char *iso, int meses, char *out, size_t n);
int util_comparar_fechas(const char *a, const char *b);

/* --- Cedula dominicana --------------------------------------------------- */

int util_cedula_normalizar(const char *entrada, char *salida, size_t n);
int util_cedula_valida(const char *entrada);

/* --- Cadenas ------------------------------------------------------------- */

int util_contiene_char(const char *texto, char c);
const char *util_buscar_sin_mayus(const char *texto, const char *aguja);
void util_mayusculas(char *texto);
void util_trim(char *texto);

#endif
