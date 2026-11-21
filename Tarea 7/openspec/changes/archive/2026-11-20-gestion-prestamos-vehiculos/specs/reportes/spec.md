# Spec Delta

## Purpose

Presenta en pantalla, con formato de terminal y en modo de solo lectura, el estado de los préstamos por cliente y el detalle mensual de cuotas y saldos que necesita el operador para cobrar y rendir cuentas.

## ADDED Requirements

### Requirement: Reporte de préstamos por cliente

El sistema SHALL mostrar, para un cliente seleccionado, todos sus préstamos con la descripción del vehículo, el monto financiado, la cantidad de cuotas, las cuotas pagadas, las cuotas pendientes, el saldo pendiente y el estado del préstamo (`ACTIVO` o `CANCELADO`).

#### Scenario: Cliente con varios préstamos

- **WHEN** el operador pide el reporte de un cliente que tiene dos préstamos
- **THEN** el sistema muestra una fila por préstamo con sus cuotas pagadas, pendientes y saldo, y una fila de totales al pie

#### Scenario: Cliente sin préstamos

- **WHEN** el operador pide el reporte de un cliente registrado que no tiene préstamos
- **THEN** el sistema muestra los datos del cliente e informa que no tiene préstamos registrados

#### Scenario: Estado de un préstamo cancelado

- **WHEN** todas las cuotas de un préstamo están pagadas
- **THEN** el reporte muestra ese préstamo con estado `CANCELADO` y saldo pendiente `0`

### Requirement: Reporte de calendario de pagos con saldos

El sistema SHALL mostrar, para un préstamo seleccionado, una fila por cada mes con número de cuota, fecha de vencimiento, cuota, interés, capital, saldo posterior al pago y estado, además de una fila de totales con la suma de cuotas, intereses y capital.

#### Scenario: Calendario completo

- **WHEN** el operador abre el calendario de un préstamo de 36 cuotas
- **THEN** el reporte muestra las 36 filas con su estado y una fila de totales donde la suma de capital es igual al monto financiado

#### Scenario: Totales coherentes con el préstamo

- **WHEN** el calendario muestra un préstamo de `800000` al `18` por ciento anual a 36 meses
- **THEN** los totales indican `1232000` en cuotas, `432000` en intereses y `800000` en capital

### Requirement: Reporte de cuotas vencidas

El sistema SHALL listar las cuotas cuyo vencimiento es anterior a la fecha actual y que no tienen pago registrado, agrupadas por cliente y préstamo, indicando número de cuota, fecha de vencimiento y monto adeudado.

#### Scenario: Cuotas vencidas de varios clientes

- **WHEN** el operador abre el reporte de cuotas vencidas y existen cuotas sin pago con vencimiento pasado en dos clientes distintos
- **THEN** el sistema muestra ambas agrupaciones con sus cuotas, fechas y montos

#### Scenario: Sin cuotas vencidas

- **WHEN** el operador abre el reporte y no existe ninguna cuota vencida
- **THEN** el sistema muestra un mensaje indicando que no hay cuotas vencidas

### Requirement: Presentación en TUI

El sistema SHALL dibujar los reportes en la terminal con marcos, encabezado de contexto, columnas alineadas y numeración de montos con separador de miles y sin decimales. Cuando las filas no caben en la pantalla, SHALL paginar con las teclas `N` para la página siguiente, `P` para la anterior y `Q` para volver al menú.

#### Scenario: Reporte que excede una página

- **WHEN** el operador abre un calendario de 36 cuotas en una pantalla de 18 filas
- **THEN** el reporte se divide en páginas, muestra el número de página actual y `N` avanza a la siguiente

#### Scenario: Formato de los montos

- **WHEN** el reporte muestra el monto financiado `800000`
- **THEN** se dibuja como `800.000`, sin decimales ni símbolo de moneda dentro de la columna

#### Scenario: Regreso al menú

- **WHEN** el operador presiona `Q` en cualquier página de un reporte
- **THEN** la pantalla vuelve al menú de reportes sin modificar datos

### Requirement: Solo lectura

Los reportes SHALL ser de solo lectura y no SHALL modificar clientes, préstamos, pagos ni avisos, incluso si el operador recorre todas sus páginas.

#### Scenario: Recorrer un reporte no altera datos

- **WHEN** el operador abre un calendario, recorre todas sus páginas y vuelve al menú
- **THEN** los archivos de datos conservan exactamente el mismo contenido que antes de abrir el reporte
