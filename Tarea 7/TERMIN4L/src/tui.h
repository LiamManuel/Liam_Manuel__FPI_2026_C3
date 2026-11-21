#ifndef TUI_H
#define TUI_H

#include "reportes.h"

/* Prepara la consola: UTF-8 y modo terminal virtual, con retroceso silencioso. */
void tui_iniciar(void);
void tui_terminar(void);

/* Alto visible de la consola en filas (25 si no se puede consultar). */
int tui_alto_consola(void);

/* Lee una tecla sin Enter cuando la entrada es una consola. */
char tui_leer_tecla(void);

/* Dibuja el reporte paginado con N/P/Q. */
void tui_mostrar_reporte(const Reporte *r);

#endif
