#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------------- */
/* Internos                                                                  */
/* ------------------------------------------------------------------------- */

static void descartar_resto_linea(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* descartar */
    }
}

static int leer_linea(char *buf, size_t n)
{
    if (!fgets(buf, (int)n, stdin)) {
        return 0;
    }
    if (!strchr(buf, '\n')) {
        descartar_resto_linea();
    }
    util_trim(buf);
    return 1;
}

void util_trim(char *texto)
{
    char *ini = texto;
    size_t largo;

    while (*ini && isspace((unsigned char)*ini)) {
        ini++;
    }
    if (ini != texto) {
        memmove(texto, ini, strlen(ini) + 1);
    }
    largo = strlen(texto);
    while (largo > 0 && isspace((unsigned char)texto[largo - 1])) {
        texto[--largo] = '\0';
    }
}

int util_contiene_char(const char *texto, char c)
{
    return texto && strchr(texto, c) != NULL;
}

void util_mayusculas(char *texto)
{
    for (; texto && *texto; texto++) {
        *texto = (char)toupper((unsigned char)*texto);
    }
}

const char *util_buscar_sin_mayus(const char *texto, const char *aguja)
{
    size_t largo;

    if (!texto || !aguja) {
        return NULL;
    }
    largo = strlen(aguja);
    if (largo == 0) {
        return texto;
    }
    for (; *texto; texto++) {
        size_t i = 0;
        while (i < largo && texto[i] &&
               tolower((unsigned char)texto[i]) ==
                   tolower((unsigned char)aguja[i])) {
            i++;
        }
        if (i == largo) {
            return texto;
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* Entrada por teclado                                                       */
/* ------------------------------------------------------------------------- */

void util_leer_texto(const char *prompt, char *buf, size_t n)
{
    char linea[512];

    for (;;) {
        printf("%s: ", prompt);
        fflush(stdout);
        if (!leer_linea(linea, sizeof linea)) {
            printf("\n");
            exit(0);
        }
        if (linea[0] == '\0') {
            printf("  Este campo no puede quedar vacio.\n");
            continue;
        }
        if (util_contiene_char(linea, '|')) {
            printf("  El caracter '|' no esta permitido.\n");
            continue;
        }
        if (strlen(linea) >= n) {
            printf("  Aviso: el texto se recorto a %d caracteres.\n", (int)(n - 1));
        }
        snprintf(buf, n, "%s", linea);
        return;
    }
}

void util_leer_texto_def(const char *prompt, char *buf, size_t n,
                         const char *defecto)
{
    char linea[512];

    for (;;) {
        printf("%s [%s]: ", prompt, defecto);
        fflush(stdout);
        if (!leer_linea(linea, sizeof linea)) {
            printf("\n");
            exit(0);
        }
        if (linea[0] == '\0') {
            snprintf(buf, n, "%s", defecto);
            return;
        }
        if (util_contiene_char(linea, '|')) {
            printf("  El caracter '|' no esta permitido.\n");
            continue;
        }
        if (strlen(linea) >= n) {
            printf("  Aviso: el texto se recorto a %d caracteres.\n", (int)(n - 1));
        }
        snprintf(buf, n, "%s", linea);
        return;
    }
}

void util_leer_linea_libre(const char *prompt, char *buf, size_t n)
{
    char linea[512];

    printf("%s: ", prompt);
    fflush(stdout);
    if (!leer_linea(linea, sizeof linea)) {
        printf("\n");
        exit(0);
    }
    snprintf(buf, n, "%s", linea);
}

void util_pausa(void)
{
    char linea[32];

    printf("\nPresione Enter para continuar...");
    fflush(stdout);
    if (!fgets(linea, sizeof linea, stdin)) {
        printf("\n");
    }
}

int util_parse_entero(const char *texto, long long *salida)
{
    char *fin = NULL;
    long long valor;

    if (!texto) {
        return 0;
    }
    errno = 0;
    valor = strtoll(texto, &fin, 10);
    if (fin == texto || errno == ERANGE) {
        return 0;
    }
    while (*fin && isspace((unsigned char)*fin)) {
        fin++;
    }
    if (*fin != '\0') {
        return 0;
    }
    if (salida) {
        *salida = valor;
    }
    return 1;
}

long long util_leer_entero(const char *prompt, long long min, long long max)
{
    char linea[128];
    long long valor;

    for (;;) {
        printf("%s: ", prompt);
        fflush(stdout);
        if (!leer_linea(linea, sizeof linea)) {
            printf("\n");
            exit(0);
        }
        if (util_parse_entero(linea, &valor) && valor >= min && valor <= max) {
            return valor;
        }
        printf("  Valor invalido: escriba un numero entero entre %lld y %lld.\n",
               min, max);
    }
}

long long util_leer_entero_def(const char *prompt, long long min, long long max,
                               long long defecto)
{
    char linea[128];
    long long valor;

    for (;;) {
        printf("%s [%lld]: ", prompt, defecto);
        fflush(stdout);
        if (!leer_linea(linea, sizeof linea)) {
            printf("\n");
            exit(0);
        }
        if (linea[0] == '\0') {
            return defecto;
        }
        if (util_parse_entero(linea, &valor) && valor >= min && valor <= max) {
            return valor;
        }
        printf("  Valor invalido: escriba un numero entero entre %lld y %lld.\n",
               min, max);
    }
}

int util_confirmar(const char *prompt)
{
    char linea[32];

    for (;;) {
        printf("%s (S/N): ", prompt);
        fflush(stdout);
        if (!leer_linea(linea, sizeof linea)) {
            printf("\n");
            exit(0);
        }
        if (linea[0] == 's' || linea[0] == 'S') {
            return 1;
        }
        if (linea[0] == 'n' || linea[0] == 'N') {
            return 0;
        }
        printf("  Responda S o N.\n");
    }
}

int util_confirmar_si(const char *prompt)
{
    char linea[32];

    printf("%s\nEscriba SI para confirmar: ", prompt);
    fflush(stdout);
    if (!leer_linea(linea, sizeof linea)) {
        printf("\n");
        exit(0);
    }
    util_mayusculas(linea);
    return strcmp(linea, "SI") == 0;
}

/* ------------------------------------------------------------------------- */
/* Formatos                                                                  */
/* ------------------------------------------------------------------------- */

void util_formato_monto(long long valor, char *out, size_t n)
{
    char digitos[32];
    int largo;
    size_t pos = 0;
    int negativo = valor < 0;
    unsigned long long magnitud;
    int i;

    if (n == 0) {
        return;
    }
    magnitud = negativo ? (unsigned long long)(-(valor + 1)) + 1ULL
                        : (unsigned long long)valor;
    snprintf(digitos, sizeof digitos, "%llu", magnitud);
    largo = (int)strlen(digitos);

    if (negativo && pos + 1 < n) {
        out[pos++] = '-';
    }
    for (i = 0; i < largo && pos + 1 < n; i++) {
        if (i > 0 && (largo - i) % 3 == 0 && pos + 1 < n) {
            out[pos++] = '.';
        }
        if (pos + 1 < n) {
            out[pos++] = digitos[i];
        }
    }
    out[pos] = '\0';
}

void util_formato_fecha(const char *iso, char *out, size_t n)
{
    int y, m, d;

    if (iso && strlen(iso) >= 10 && sscanf(iso, "%4d-%2d-%2d", &y, &m, &d) == 3) {
        snprintf(out, n, "%02d/%02d/%04d", d, m, y);
    } else {
        snprintf(out, n, "%s", iso ? iso : "");
    }
}

/* ------------------------------------------------------------------------- */
/* Fechas                                                                    */
/* ------------------------------------------------------------------------- */

static int bisiesto(int anio)
{
    return (anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0);
}

static int dias_del_mes(int anio, int mes)
{
    static const int dias[12] = {31, 28, 31, 30, 31, 30,
                                 31, 31, 30, 31, 30, 31};
    if (mes == 2 && bisiesto(anio)) {
        return 29;
    }
    return dias[mes - 1];
}

void util_hoy_iso(char *out, size_t n)
{
    time_t ahora = time(NULL);
    struct tm *local = localtime(&ahora);

    if (!local) {
        snprintf(out, n, "1970-01-01");
        return;
    }
    strftime(out, n, "%Y-%m-%d", local);
}

void util_ahora_texto(char *out, size_t n)
{
    time_t ahora = time(NULL);
    struct tm *local = localtime(&ahora);

    if (!local) {
        snprintf(out, n, "1970-01-01 00:00:00");
        return;
    }
    strftime(out, n, "%Y-%m-%d %H:%M:%S", local);
}

int util_fecha_valida(const char *iso)
{
    int y, m, d;

    if (!iso || strlen(iso) != 10 || iso[4] != '-' || iso[7] != '-') {
        return 0;
    }
    if (sscanf(iso, "%4d-%2d-%2d", &y, &m, &d) != 3) {
        return 0;
    }
    if (m < 1 || m > 12 || d < 1) {
        return 0;
    }
    return d <= dias_del_mes(y, m);
}

int util_sumar_meses(const char *iso, int meses, char *out, size_t n)
{
    int y, m, d, total, anio, mes, ultimo;

    if (!util_fecha_valida(iso)) {
        snprintf(out, n, "%s", iso ? iso : "");
        return 0;
    }
    sscanf(iso, "%4d-%2d-%2d", &y, &m, &d);

    total = y * 12 + (m - 1) + meses;
    anio = total / 12;
    mes = total % 12 + 1;
    if (mes <= 0) { /* no ocurre con meses >= 0, por robustez */
        anio -= 1;
        mes += 12;
    }
    ultimo = dias_del_mes(anio, mes);
    if (d > ultimo) {
        d = ultimo;
    }
    snprintf(out, n, "%04d-%02d-%02d", anio, mes, d);
    return 1;
}

int util_comparar_fechas(const char *a, const char *b)
{
    int r = strcmp(a, b);
    return r < 0 ? -1 : (r > 0 ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
/* Cedula dominicana                                                         */
/* ------------------------------------------------------------------------- */

int util_cedula_normalizar(const char *entrada, char *salida, size_t n)
{
    char digitos[12];
    int cuantos = 0;
    int suma = 0;
    int verificador;
    const char *p;

    if (!entrada || !salida || n < 14) { /* "000-0000000-0" + NUL */
        return 0;
    }
    for (p = entrada; *p; p++) {
        if (*p == '-' || *p == ' ') {
            continue;
        }
        if (*p < '0' || *p > '9') {
            return 0;
        }
        if (cuantos >= 11) {
            return 0;
        }
        digitos[cuantos++] = *p;
    }
    if (cuantos != 11) {
        return 0;
    }
    digitos[11] = '\0';

    for (p = digitos; p < digitos + 10; p++) {
        int valor = (*p - '0') * (((p - digitos) % 2 == 1) ? 2 : 1);
        if (valor > 9) {
            valor -= 9;
        }
        suma += valor;
    }
    verificador = (10 - (suma % 10)) % 10;
    if (verificador != digitos[10] - '0') {
        return 0;
    }
    snprintf(salida, n, "%.3s-%.7s-%c", digitos, digitos + 3, digitos[10]);
    return 1;
}

int util_cedula_valida(const char *entrada)
{
    char normalizada[14];
    return util_cedula_normalizar(entrada, normalizada, sizeof normalizada);
}
