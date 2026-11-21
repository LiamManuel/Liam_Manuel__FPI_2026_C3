# Design

## Context

Ver `proposal.md` para la motivación. El repositorio no tiene código de aplicación: el change crea el programa completo desde cero.

Restricciones que condicionan el diseño:

- C de consola, sin librerías externas, compilable en Code::Blocks (MinGW) sobre Windows.
- Montos enteros, sin decimales, en todo el sistema.
- Cuatro archivos de texto como única persistencia.
- Reportes dibujados con secuencias ANSI; el resto del programa usa entrada secuencial por teclado.
- Volumen esperado pequeño (decenas de clientes, cientos de préstamos): todo cabe en memoria y no hace falta indexación ni consultas Incrementales.

## Goals / Non-Goals

**Goals:**

- Una sola fuente de verdad: el calendario, los saldos y los estados se derivan de los datos guardados, nunca se persisten.
- Escrituras resistentes a interrupciones: guardar no debe dejar un archivo a medias.
- Que el reporte de saldos sea siempre coherente con los pagos, sin huecos posibles.
- Código modular y legible, defendible en una presentación académica.

**Non-Goals:**

- Mora, recargos, abonos parciales, refinanciamiento o múltiples monedas.
- Concurrencia, usuarios, permisos o acceso remoto.
- TUI para los menús de alta y edición (solo reportes).
- Exportación de reportes a archivo o impresión.

## Decisions

### Archivos de texto delimitados, cargados en memoria

Cada entidad vive en su archivo (`clientes.txt`, `prestamos.txt`, `pagos.txt`, `avisos.txt`) con campos separados por `|` y sin encabezado. Al iniciar se leen completos a arreglos dinámicos; al guardar se reescribe el archivo completo.

- **Por qué**: inspeccionable con un editor de texto, sin dependencias, y el volumen no justifica otra cosa.
- **Alternativas**: structs binarios con `fwrite` (ilegibles y frágiles ante cambios de struct) y SQLite (agrega una librería y un esquema que el proyecto no necesita).

### Escritura atómica por archivo temporal

Cada guardado escribe `<archivo>.tmp`, cierra, y reemplaza el original con `remove` + `rename`. Si el proceso muere a mitad de escritura, el archivo original queda intacto.

- **Por qué**: con reescritura completa, un corte de energía a mitad de `fprintf` perdería todo el archivo.
- **Alternativa**: append por operación (más rápido, pero no resuelve modificaciones ni borrados).

### Todo lo derivado se calcula, nada se guarda

`prestamos.txt` guarda solo los datos de entrada del préstamo. Capital, interés, cuota, saldo y estado se calculan al vuelo a partir de la tasa, los meses y los pagos registrados.

- **Por qué**: elimina la clase de error más común —un calendario guardado que ya no coincide con el préstamo— y hace que editar un préstamo sin pagos no requiera migrar datos.

### Cédula normalizada y préstamos enlazados por id interno

La cédula se valida y normaliza en una sola función antes de tocar el padrón: se quitan los guiones, se exigen 11 dígitos numéricos, se comprueba el dígito verificador con el algoritmo módulo 10 y se guarda como `000-0000000-0`. `prestamos.txt` guarda el `id` del cliente, nunca la cédula.

- **Por qué**: corregir una cédula mal tecleada actualiza un solo campo de `clientes.txt`; los préstamos, los pagos y los reportes quedan consistentes sin reescribir nada, porque apuntan al mismo id.
- **Alternativa descartada**: guardar la cédula dentro del préstamo. Toda corrección obligaría a reescribir préstamos y pagos, con riesgo de dejar registros apuntando a una cédula inexistente.
- **Alternativa descartada**: cédula inmutable. Obligaría a borrar y recrear el cliente, lo que el propio sistema bloquea cuando ya hay préstamos.
- **Formato**: la búsqueda aplica la misma normalización, así que `00112345673` y `001-1234567-3` encuentran al mismo cliente.

### Período de gracia de capital

Los meses de gracia son cuotas donde el cliente paga solo el interés: el capital no baja y el saldo se mantiene. La amortización se reparte entre los meses restantes (`meses - gracia`), y la gracia se valida entre 0 y `meses - 1` para que exista al menos una cuota de capital.

- **Por qué**: es la acepción estándar de período de gracia en financiamiento de vehículos —se difiere el capital, no el costo del dinero— y con interés plano no infla la deuda: los totales del préstamo son idénticos a los de un préstamo sin gracia, solo cambia el reparto.
- **Alternativa descartada**: gracia total (no pagar nada y capitalizar el interés). Agranda la deuda, rompe la constancia del interés y complica la explicación del saldo.
- **Alternativa descartada**: solo desplazar la primera fecha de vencimiento sin cobrar interés durante la espera. Regala el costo del financiamiento y desalinea el calendario del mes en que el vehículo se entrega al cliente.

### Aritmética entera con reparto de residuo

```
interes_mensual = (financiado * tasa + 600) / 1200    /* redondeo half-up, enteros */
meses_amort     = meses - gracia
capital_base    = financiado / meses_amort
residuo         = financiado % meses_amort

j = k - gracia                                  /* posición dentro de la amortización */

capital(k) = 0                                            si k <= gracia
             capital_base + 1   si 1 <= j <= residuo
             capital_base       si j >  residuo

cuota(k)   = interes_mensual  si k <= gracia
             capital(k) + interes_mensual   si k >  gracia

saldo(k)   = saldo(k-1) - capital(k)     con saldo(0) = financiado
```

Se usa `long long` en productos y acumulados (`financiado * tasa` y totales de reporte) porque un valor de vehículo alto por 100 supera el rango de `int`.

- **Por qué**: repartir el sobrante en las primeras cuotas de amortización evita el salto visible de cargar todo a la última (p. ej. 2.805 contra 2.777) y garantiza que la suma de capital sea exactamente el financiado.
- **Alternativa descartada**: última cuota ajustada; produce cuotas notoriamente distintas y complica la lectura del reporte.

### Pagos secuenciales identificados por número de cuota

`pagos.txt` guarda `id_prestamo|num_mes|fecha_pago`. La cuota elegible es siempre `pagos_registrados + 1`. No se admiten huecos ni varias cuotas por operación.

- **Por qué**: el saldo teórico del calendario y el saldo real coinciden siempre; no hay que decidir qué mostrar cuando falta una cuota intermedia.
- **Alternativa descartada**: pagos por monto libre, que exigiría prorratear capital e interés y abriría la puerta a inconsistencias.

### Fechas como texto, cálculo con `struct tm`

En archivos las fechas son `YYYY-MM-DD`; en memoria se convierten a `struct tm`. El vencimiento de la cuota `k` se obtiene sumando `k` meses y recortando el día al último del mes destino cuando no existe (31/01 → 28/02), con `mktime` para normalizar.

- **Por qué**: legible en el archivo y sin dependencias adicionales.
- **Nota**: el estado `VENCIDA` se evalúa contra la fecha del sistema al momento de consultar, por lo que un mismo calendario cambia de aspecto con el paso del tiempo sin que se modifique ningún archivo.

### TUI ANSI puro, ancho fijo de 80 columnas

Los reportes se dibujan con marcos ASCII (`+ - |`), cursor posicionado por secuencias ANSI y limpieza de pantalla. Se habilita el modo terminal virtual de Windows en el arranque (`SetConsoleMode` sobre `ENABLE_VIRTUAL_TERMINAL_PROCESSING`) y se fija la página de códigos UTF-8.

- **Por qué**: cero dependencias y un solo ejecutable; los bordes ASCII evitan problemas de ancho con fuentes que no renderizan bien los caracteres de caja.
- **Alternativa descartada**: PDCurses, que obliga a descargar y enlazar una librería en Code::Blocks y no aporta nada a este alcance.
- **Navegación**: lectura con `_getch()`, sin `Enter` y sin parsear flechas; `N`, `P` y `Q` son suficientes para paginar. Si la consola no soporta ANSI, los marcos se ven como texto plano y el programa sigue siendo usable.

### Ancho fijo y paginación de hasta 18 filas

Ancho total 80 columnas y hasta 18 filas de datos por página, con encabezado de contexto arriba y totales y teclas abajo. La altura de página se recorta consultando el alto de la consola, para que el marco no se corte en ventanas de 25 líneas.

- **Por qué**: 18 filas es el máximo que cabe con encabezado y totales en una consola alta; en una de 25 líneas hay que bajar la página o el borde superior se pierde al desplazarse.
- **Trade-off**: la paginación no es idéntica en todas las consolas; el contenido y las teclas sí lo son.

### Módulos separados por responsabilidad

```
src/
  main.c        arranque, menús, despacho de opciones
  archivos.c/h  carga, guardado atómico, rutas
  clientes.c/h  alta, búsqueda, modificación, eliminación
  prestamos.c/h cálculo del calendario, pagos, modificación, eliminación
  reportes.c/h  consultas de solo lectura
  tui.c/h       marcos ANSI, paginación, formato de montos y fechas
  util.c/h      entrada validada, fechas, cadenas
```

- **Por qué**: cada archivo se puede explicar y probar por separado; `tui.c` concentra todo el código de pantalla.

### Entrada validada por línea

Toda lectura usa `fgets` más parseo explícito (`strtol`, `sscanf` controlado). Se descartan entradas con `|` para no romper el formato, se truncan a la longitud del campo y se repite la pregunta ante un valor inválido.

- **Por qué**: `scanf("%s")` no lee nombres con espacios y desborda buffers; `scanf("%d")` deja residuos en el buffer que desalinean las preguntas siguientes.

## Risks / Trade-offs

- **Reescritura completa en cada guardado** → aceptable con cientos de registros; si el volumen crece, el costo es lineal y perceptible. Se mitiga manteniendo un solo guardado por operación de menú.
- **Colisión de identificadores** → el id se calcula como `max(id) + 1` al cargar; si se edita un archivo a mano con huecos, se reutiliza el máximo. Se documenta que los archivos no se editan manualmente.
- **Desbordamiento en `int`** → se usa `long long` en toda multiplicación monetaria y en los totales; los montos se validan contra un máximo razonable.
- **Codificación en consola Windows** → se fija UTF-8 y se evita depender de acentos en los menús; los nombres con acentos se guardan tal cual y se muestran con la página de códigos correcta.
- **Cambios de fecha del sistema** → los estados `VENCIDA`/`PENDIENTE` dependen del reloj; una fecha mal configurada produce reportes engañosos. Es inherente al requisito y se documenta en la ayuda del menú.
- **Eliminar un préstamo sin pagos no conserva sus datos financieros** → la bitácora guarda cliente, vehículo, financiado, cuotas y motivo, que es el mínimo para auditoría; no se guarda el calendario completo.
- **División por cero** → `meses` se valida entre 1 y 120 y la gracia entre 0 y `meses - 1` antes de calcular `meses - gracia`, así siempre hay al menos una cuota de amortización.

## Migration Plan

No hay datos ni código previos. En el primer arranque el programa crea los cuatro archivos vacíos si no existen y arranca con listados vacíos. Revertir consiste en eliminar el ejecutable y los cuatro archivos generados; no hay estado compartido.

## Open Questions

Ninguna: las decisiones pendientes (borrado bloqueado, pagos en orden, aviso en bitácora, formato TUI) quedaron resueltas antes de escribir este diseño.
