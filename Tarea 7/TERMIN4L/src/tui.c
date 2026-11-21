#include "tui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#define TUI_ANCHO      80
#define TUI_INTERIOR   78   /* columnas entre los dos bordes verticales */
#define TUI_TEXTO      76   /* ancho util dentro del marco con relleno */
#define TUI_FILAS_MAX  18

static int g_ansi = 0;

/* ------------------------------------------------------------------------- */
/* Arranque de consola                                                       */
/* ------------------------------------------------------------------------- */

void tui_iniciar(void)
{
#ifdef _WIN32
    HANDLE salida = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (salida != INVALID_HANDLE_VALUE && GetConsoleMode(salida, &modo)) {
        if (SetConsoleMode(salida,
                           modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            g_ansi = 1;
        }
    }
#else
    g_ansi = 1;
#endif
}

void tui_terminar(void)
{
    if (g_ansi) {
        printf("\033[0m");
        fflush(stdout);
    }
}

int tui_alto_consola(void)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE salida = GetStdHandle(STD_OUTPUT_HANDLE);

    if (salida != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(salida, &info)) {
        int alto = (int)(info.srWindow.Bottom - info.srWindow.Top + 1);
        if (alto > 0) {
            return alto;
        }
    }
#else
    const char *lineas = getenv("LINES");
    if (lineas) {
        int alto = atoi(lineas);
        if (alto > 0) {
            return alto;
        }
    }
#endif
    return 25;
}

char tui_leer_tecla(void)
{
#ifdef _WIN32
    if (_isatty(_fileno(stdin))) {
        int c = _getch();
        return c < 0 ? 'q' : (char)c;
    }
#endif
    {
        int c;
        do {
            c = getchar();
        } while (c == '\r' || c == '\n');
        return c < 0 ? 'q' : (char)c;
    }
}

/* ------------------------------------------------------------------------- */
/* Dibujo del marco                                                          */
/* ------------------------------------------------------------------------- */

static void borde(void)
{
    int i;

    putchar('+');
    for (i = 0; i < TUI_INTERIOR; i++) {
        putchar('-');
    }
    puts("+");
}

/* Longitud en bytes de la secuencia UTF-8 que empieza en c. */
static int bytes_utf8(unsigned char c)
{
    if (c < 0x80) {
        return 1;
    }
    if ((c & 0xE0) == 0xC0) {
        return 2;
    }
    if ((c & 0xF0) == 0xE0) {
        return 3;
    }
    if ((c & 0xF8) == 0xF0) {
        return 4;
    }
    return 1;
}

/*
 * Escribe una linea de contenido de 78 columnas visibles. Los caracteres
 * UTF-8 cuentan como una columna, de modo que el borde derecho no se mueve
 * aunque el texto tenga acentos.
 */
static void linea_texto(const char *texto)
{
    const char *p = texto ? texto : "";
    int columnas = 0;

    printf("| ");
    while (*p && columnas < TUI_TEXTO) {
        size_t restantes = strlen(p);
        int bytes = bytes_utf8((unsigned char)*p);

        if ((size_t)bytes > restantes) {
            bytes = (int)restantes;
        }
        fwrite(p, 1, (size_t)bytes, stdout);
        p += bytes;
        columnas++;
    }
    while (columnas < TUI_TEXTO) {
        putchar(' ');
        columnas++;
    }
    puts(" |");
}

static void pie_pagina(int pagina, int paginas)
{
    char pie[TUI_TEXTO + 1];

    snprintf(pie, sizeof pie, "PAGINA %d/%d   N: SIGUIENTE   P: ANTERIOR   Q: SALIR",
             pagina, paginas);
    linea_texto(pie);
}

/* ------------------------------------------------------------------------- */
/* Reporte paginado                                                          */
/* ------------------------------------------------------------------------- */

void tui_mostrar_reporte(const Reporte *r)
{
    int filas_por_pagina;
    int paginas;
    int pagina = 0;
    int fijos;
    int total;
    int i;

    if (!r) {
        return;
    }
    total = r->n_filas > 0 ? r->n_filas : 1;

    /* borde + titulo + separador, contexto, columnas, totales y pie */
    fijos = 3 + r->n_encabezado + 3;
    if (r->hay_columnas) {
        fijos += 2;
    }
    if (r->hay_totales) {
        fijos += 1;
    }

    /* Se deja una fila libre para que el ultimo salto de linea no desplace
       el marco cuando la consola tiene exactamente el alto de la pagina. */
    filas_por_pagina = tui_alto_consola() - fijos - 1;
    if (filas_por_pagina > TUI_FILAS_MAX) {
        filas_por_pagina = TUI_FILAS_MAX;
    }
    if (filas_por_pagina < 4) {
        filas_por_pagina = 4;
    }
    paginas = (total + filas_por_pagina - 1) / filas_por_pagina;
    if (paginas < 1) {
        paginas = 1;
    }

    for (;;) {
        int inicio = pagina * filas_por_pagina;
        char tecla;

        if (g_ansi) {
            printf("\033[2J\033[H");
        } else if (pagina > 0) {
            putchar('\n');
        }

        borde();
        linea_texto(r->titulo);
        borde();
        for (i = 0; i < r->n_encabezado; i++) {
            linea_texto(r->encabezado[i]);
        }
        if (r->hay_columnas) {
            linea_texto(r->columnas);
            borde();
        }
        for (i = 0; i < filas_por_pagina; i++) {
            int indice = inicio + i;
            if (indice < r->n_filas) {
                linea_texto(r->filas[indice]);
            } else if (indice == 0) {
                linea_texto("SIN DATOS.");
            } else {
                linea_texto("");
            }
        }
        if (r->hay_totales) {
            linea_texto(r->totales);
        }
        borde();
        pie_pagina(pagina + 1, paginas);
        borde();
        fflush(stdout);

        tecla = tui_leer_tecla();
        if (tecla == 'q' || tecla == 'Q' || tecla == 27) {
            break;
        }
        if ((tecla == 'n' || tecla == 'N') && pagina + 1 < paginas) {
            pagina++;
        } else if ((tecla == 'p' || tecla == 'P') && pagina > 0) {
            pagina--;
        }
    }
}
