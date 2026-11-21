# Proposal

## Why

El financiamiento de vehículos se lleva hoy en papel o en hojas de cálculo sueltas: no hay registro único de clientes, los préstamos se recalculan a mano y no queda constancia de qué cuotas pagó cada comprador. Eso produce saldos inconsistentes, préstamos duplicados y pérdida del historial cuando un expediente se descarta.

Este change introduce un programa de consola en C que centraliza clientes, préstamos con interés plano y el reporte de cuotas con saldos, para que el estado de cada préstamo se derive siempre de los mismos datos y no de un cálculo manual.

## What Changes

- **Clientes**: alta, listado, búsqueda, modificación y eliminación. La cédula dominicana se valida con dígito verificador, se normaliza y puede corregirse; los préstamos siguen asociados. La eliminación se bloquea si el cliente tiene préstamos.
- **Préstamos**: creación con valor del vehículo, avance del cliente, meses, tasa anual mayor que cero y meses de gracia; el monto financiado se calcula. Modificación, calendario de cuotas y eliminación.
- **Cálculo de interés plano en enteros**: montos y cuotas sin decimales. El residuo de la división se reparte en las primeras cuotas de amortización, y las cuotas de gracia pagan solo interés.
- **Pagos secuenciales**: solo se registra la cuota pendiente más antigua, una por operación, con su fecha. No se permite adelantar cuotas.
- **Bloqueo de préstamos con historial**: un préstamo con al menos un pago no se puede eliminar; si no tiene pagos, la eliminación exige advertencia y deja registro en una bitácora de avisos.
- **Reportes en TUI**: préstamos por cliente, calendario de pagos con saldos y cuotas vencidas, dibujados con marcos ANSI, paginado y navegación por teclado.
- **Persistencia en archivos de texto delimitados**: `clientes.txt`, `prestamos.txt`, `pagos.txt` y `avisos.txt`.

## Capabilities

### New Capabilities

- `clientes`: gestión del padrón de compradores (alta, consulta, modificación y eliminación con bloqueo por préstamos asociados), con identificación única por cédula.
- `prestamos`: creación y mantenimiento de préstamos de vehículos, cálculo de interés plano en enteros, generación del calendario de cuotas y registro secuencial de pagos.
- `reportes`: consultas de solo lectura en TUI sobre préstamos por cliente, calendario de cuotas con saldos y cuotas vencidas.

### Modified Capabilities

Ninguna: el proyecto no tiene specs previas.

## Impact

- Código nuevo en C para consola, sin dependencias externas, compilable en Code::Blocks.
- Persistencia en cuatro archivos de texto en el directorio de ejecución; se crean vacíos en el primer arranque.
- Requiere consola Windows con soporte de secuencias ANSI (modo terminal virtual) para los reportes.
- No modifica servicios, APIs ni datos existentes: el repositorio no contiene código de aplicación todavía.
