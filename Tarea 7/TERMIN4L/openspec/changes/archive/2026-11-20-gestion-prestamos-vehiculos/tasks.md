# Tasks

## 1. Estructura y persistencia

- [x] 1.1 Crear `src/` con los módulos de `design.md` y un `Makefile` (`gcc -std=c11 -Wall -Wextra`); verificar que `make` compila sin advertencias y genera el ejecutable
- [x] 1.2 Definir los structs `Cliente`, `Prestamo`, `Pago` y `Aviso` con sus límites de campo en un encabezado común; verificar que compila y que `sizeof` de cada struct es estable entre compilaciones
- [x] 1.3 Implementar la carga de los cuatro archivos delimitados por `|` en arreglos dinámicos y el guardado atómico vía `.tmp` + `rename`; verificar con un programa de prueba que guarda 3 clientes, cierra, recarga y obtiene los mismos 3 registros
- [x] 1.4 Crear los archivos vacíos en el primer arranque cuando no existen; verificar ejecutando el binario en un directorio vacío y comprobando que aparecen los cuatro archivos sin mensajes de error
- [x] 1.5 Ignorar en `git status` los archivos de datos generados y el ejecutable; verificar que `git status` queda limpio tras ejecutar el programa

## 2. Utilidades de entrada, fechas y dinero

- [x] 2.1 Implementar lectura de línea con `fgets`, truncado al tamaño del campo, rechazo de `|` y repetición ante valor inválido; verificar con entradas vacías, demasiado largas y con separador
- [x] 2.2 Implementar el parseo de enteros con `strtol` y detección de desbordamiento y de texto sobrante; verificar con `abc`, `12abc`, `-5`, `0` y un valor mayor a `INT_MAX`
- [x] 2.3 Implementar el formateo de montos con separador de miles y sin decimales, y el de fechas `DD/MM/AAAA`; verificar que `800000` se muestra `800.000` y que `0` se muestra `0`
- [x] 2.4 Implementar la suma de meses sobre `struct tm` con ajuste al último día del mes; verificar `2026-01-31 + 1 mes = 2026-02-28` y `2028-01-31 + 1 mes = 2028-02-29`
- [x] 2.5 Implementar el cálculo de interés, capital base y residuo con `long long`, descontando los meses de gracia del divisor; verificar con `800000`, `36` meses, `18%` y `2` de gracia que produce 14 cuotas de capital `23530`, 20 de `23529`, interés `12000` y suma de capital `800000`
- [x] 2.6 Agregar `tests/checks.c` con aserciones de las utilidades y un objetivo `make test`; verificar que pasa en verde y que falla al alterar a propósito un valor esperado
- [x] 2.7 Implementar la validación y normalización de cédula dominicana: 11 dígitos, formato `000-0000000-0` y dígito verificador módulo 10; verificar que `00112345673` se normaliza a `001-1234567-3` y que `001-1234567-8`, `001-123456-3` y `001-1234567-A` se rechazan

## 3. Clientes

- [x] 3.1 Implementar el alta con validación de campos obligatorios, cédula única en formato dominicano y asignación de id; verificar que un alta válida aparece en `clientes.txt` y que una cédula repetida o inválida se rechaza sin escribir
- [x] 3.2 Implementar listado, búsqueda por cédula exacta y por nombre o apellido parcial sin distinguir mayúsculas; verificar que `per` encuentra `Perez`, que buscar `00112345673` encuentra la cédula `001-1234567-3` y que un texto inexistente informa cero resultados
- [x] 3.3 Implementar la modificación de cédula, nombre, apellido, teléfono y dirección con validación de formato y unicidad de la cédula nueva; verificar que corregir la cédula de un cliente conserva sus préstamos asociados y que una cédula ya ocupada se rechaza sin cambios
- [x] 3.4 Implementar la eliminación con confirmación y el bloqueo cuando el cliente tiene préstamos; verificar que un cliente con dos préstamos se rechaza indicando `2` y que uno sin préstamos desaparece de `clientes.txt`
- [x] 3.5 Documentar el menú de clientes en `docs/uso.md` y verificar que cada opción descrita existe en pantalla

## 4. Préstamos y calendario

- [x] 4.1 Implementar el alta de préstamo con cliente existente, cálculo de financiado y validación de valor, avance, meses, tasa y meses de gracia; verificar los rechazos de la spec (avance igual o negativo, meses `0`/`121`, tasa `0`/`101`, gracia igual a los meses) y el alta válida con financiado `800000`
- [x] 4.2 Implementar el período de gracia de capital: cuotas 1 a `gracia` con capital `0`, cuota igual al interés y saldo constante; verificar que con gracia `2` las cuotas 1 y 2 no amortizan y la 3 sí
- [x] 4.3 Implementar la generación del calendario con cuota, interés, capital, saldo y vencimientos; verificar que el saldo se mantiene durante la gracia, que la última cuota deja saldo `0` y que nunca aumenta entre filas
- [x] 4.4 Verificar los totales del calendario de `800000` a `36` meses al `18%` con y sin `2` meses de gracia: cuotas `1232000`, intereses `432000`, capital `800000` en ambos casos; agregar los casos a `tests/checks.c`
- [x] 4.5 Implementar la modificación de campos financieros, incluida la gracia, cuando no hay pagos, con recálculo del calendario; verificar que cambiar la tasa de `18` a `24` y la gracia de `0` a `2` altera el calendario
- [x] 4.6 Implementar el bloqueo de campos financieros cuando hay al menos un pago y permitir solo la descripción del vehículo; verificar ambos caminos contra un préstamo con una cuota pagada

## 5. Pagos y estados

- [x] 5.1 Implementar el registro de pago de la cuota pendiente con el número más bajo (1ra, luego 2da, 3ra, 4ta) con la fecha del día; verificar que tras tres pagos las cuotas 1, 2 y 3 quedan `PAGADA` y la elegible es la 4
- [x] 5.2 Bloquear el intento de pagar una cuota posterior y el pago cuando todas están pagadas; verificar que el archivo `pagos.txt` no cambia en ambos rechazos
- [x] 5.3 Implementar la derivación de estados `PAGADA`, `VENCIDA` y `PENDIENTE` contra la fecha del sistema; verificar los tres casos con cuotas de fechas pasadas y futuras
- [x] 5.4 Verificar la continuidad tras reiniciar el programa: registrar un pago, cerrar, reabrir y comprobar que la cuota sigue `PAGADA` y que no se puede volver a pagar

## 6. Eliminación, bitácora y avisos

- [x] 6.1 Implementar la eliminación de préstamo sin pagos con advertencia de "falta de pagos" y confirmación literal `SI`; verificar que responder otra cosa conserva el préstamo
- [x] 6.2 Bloquear la eliminación cuando existe al menos un pago e informar cuántos; verificar que el préstamo y sus pagos quedan intactos
- [x] 6.3 Implementar la bitácora append-only de avisos con fecha y hora, cliente, vehículo, financiado, cuotas y motivo; verificar que la entrada sobrevive al reinicio y a la desaparición del préstamo
- [x] 6.4 Implementar la consulta de la bitácora desde el menú; verificar que muestra las entradas más recientes y que no permite editarlas

## 7. Reportes en TUI

- [x] 7.1 Activar el modo terminal virtual de Windows y la página de códigos UTF-8 al arrancar, con retroceso silencioso si no están disponibles; verificar que los marcos ANSI se dibujan y que el programa no falla en una consola sin soporte
- [x] 7.2 Implementar el motor de pantalla de 80 columnas: marco, encabezado de contexto, tabla de columnas alineadas, pie de totales y contador de página
- [x] 7.3 Implementar la paginación con `N`, `P` y `Q` leyendo con `_getch()`; verificar un calendario de 36 cuotas en páginas de 18 filas
- [x] 7.4 Implementar el reporte de préstamos por cliente con cuotas pagadas, pendientes, saldo y estado `ACTIVO`/`CANCELADO`; verificar un cliente con dos préstamos y uno sin préstamos
- [x] 7.5 Implementar el reporte de calendario con saldos y totales; verificar visualmente el caso de `800000`, `36` meses y `18%`
- [x] 7.6 Implementar el reporte de cuotas vencidas agrupado por cliente y préstamo; verificar con dos clientes con cuotas pasadas y con un caso sin cuotas vencidas
- [x] 7.7 Verificar que recorrer un reporte completo no altera ningún archivo de datos comparando su contenido antes y después

## 8. Integración y entrega

- [x] 8.1 Integrar el menú principal y los submenús de clientes, préstamos y reportes; verificar cada opción de punta a punta en una sesión completa
- [x] 8.2 Ejecutar una prueba de extremo a extremo guionizada: alta de cliente, préstamo, tres pagos, consulta de calendario, intento de eliminar el préstamo e intento de borrar el cliente; verificar que cada caso produce el resultado de las specs
- [x] 8.3 Completar `README.md` con compilación, ejecución, formato de los archivos de datos y alcance; verificar que los comandos documentados funcionan tal como están escritos
- [x] 8.4 Ejecutar `make` y `make test` en limpio (`make clean` previo) y verificar cero advertencias y todas las aserciones en verde

## Workflow follow-up

- Validar el change con `openspec validate gestion-prestamos-vehiculos --strict`.
- Archivar el change cuando la implementación y las verificaciones estén completas.
