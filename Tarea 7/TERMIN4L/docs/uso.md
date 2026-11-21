# Guía de uso

El programa se ejecuta desde una consola. Todos los montos son enteros en pesos
dominicanos (RD$) y se muestran con separador de miles: `800.000`. No se usan
decimales en ninguna parte del sistema.

Al arrancar, el programa busca en el directorio actual los archivos
`clientes.txt`, `prestamos.txt`, `pagos.txt` y `avisos.txt`. Si no existen, los
crea vacíos. No hay que preparar nada antes de la primera ejecución.

## Menú principal

```
1. Clientes
2. Prestamos
3. Reportes
4. Salir
```

Cada opción se elige escribiendo el número y presionando `Enter`.

## Clientes

| Opción | Qué hace |
| --- | --- |
| 1. Registrar cliente | Pide cédula, nombre, apellido, teléfono y dirección. La cédula debe tener 11 dígitos con dígito verificador válido y se guarda con el formato `000-0000000-0`. |
| 2. Listar o buscar clientes | Muestra el padrón en pantalla. Si se escribe un texto, busca por cédula exacta, por nombre o por apellido sin distinguir mayúsculas. Dejar vacío lista todos. |
| 3. Modificar cliente | Pide el id, muestra los datos actuales y permite cambiarlos. `Enter` conserva el valor actual. La cédula nueva se valida y no puede repetirse. Los préstamos siguen asociados aunque cambie la cédula. |
| 4. Eliminar cliente | Pide el id y confirma con `S`/`N`. Se bloquea si el cliente tiene préstamos e indica cuántos. |
| 5. Volver al menú principal | Regresa sin cambiar nada. |

Ejemplo de cédula válida: `001-1234567-3`. También se acepta sin guiones
(`00112345673`) y el programa la normaliza.

## Préstamos

| Opción | Qué hace |
| --- | --- |
| 1. Registrar préstamo | Pide el id del cliente, la descripción del vehículo, valor, avance, meses, tasa anual, meses de gracia y fecha de inicio (`AAAA-MM-DD`). El financiado se calcula como valor menos avance y se muestra el calendario al terminar. |
| 2. Listar préstamos | Muestra todos los préstamos con su id, cliente, vehículo, financiado, cuotas, pagos y saldo. |
| 3. Modificar préstamo | Sin pagos permite cambiar todos los campos financieros y recalcula el calendario. Con al menos un pago solo permite cambiar la descripción del vehículo. |
| 4. Registrar pago | Pide el id y el número de cuota (por defecto, la siguiente pendiente), muestra el monto y pide confirmación. Solo se puede pagar la cuota pendiente con el número más bajo. |
| 5. Consultar calendario | Muestra cuota, vencimiento, monto, interés, capital, saldo y estado (`PAGADA`, `VENCIDA` o `PENDIENTE`). |
| 6. Eliminar préstamo sin pagos | Solo se permite si no hay pagos. Muestra una advertencia de "falta de pagos" y exige escribir `SI`. La eliminación queda registrada en la bitácora. |
| 7. Volver al menú principal | Regresa sin cambiar nada. |

### Reglas de validación

- Valor del vehículo mayor que cero.
- Avance mayor o igual a cero y menor que el valor.
- Meses entre 1 y 120.
- Tasa anual mayor que cero y hasta 100.
- Meses de gracia entre 0 y meses menos 1.
- Fecha de inicio válida en formato `AAAA-MM-DD`.

### Interés plano y gracia

El interés mensual es constante:

```
interes = (financiado * tasa_anual + 600) / 1200
```

Durante las cuotas de gracia el cliente paga solo el interés, el capital no baja
y el saldo se mantiene. El capital se reparte entre los meses restantes
(`meses - gracia`); si la división no es exacta, las primeras cuotas de
amortización reciben una unidad adicional.

Ejemplo: `800.000` a 36 meses al 18% con 2 meses de gracia produce un interés de
`12.000` por mes, 14 cuotas de capital `23.530` y 20 de `23.529`. Los totales
son siempre `1.232.000` en cuotas, `432.000` en intereses y `800.000` en capital.

## Reportes y navegación TUI

| Opción | Qué hace |
| --- | --- |
| 1. Clientes | Listado o búsqueda por cédula, nombre o apellido. |
| 2. Préstamos por cliente | Préstamos del cliente con cuotas, pagadas, saldo y estado `ACTIVO`/`CANCELADO`. |
| 3. Calendario de cuotas | Calendario completo con saldos y totales al pie. |
| 4. Cuotas vencidas | Cuotas sin pago cuya fecha ya pasó, agrupadas por cliente y préstamo. |
| 5. Bitácora de avisos | Préstamos eliminados por falta de pagos, con fecha y hora. Es de solo lectura. |
| 6. Volver al menú principal | Regresa sin cambiar nada. |

Los reportes usan un marco de 80 columnas y hasta 18 filas de datos por página.
Las teclas son:

- `N`: página siguiente.
- `P`: página anterior.
- `Q`: salir del reporte y volver al menú.

El alto de página se ajusta al alto de la consola para que el marco no se corte.
Si la consola no admite secuencias ANSI, los marcos se ven como texto plano y el
programa sigue funcionando. Recorrer un reporte no modifica ningún archivo.

## Archivos de datos

Los archivos son de texto y usan `|` como separador. No se recomienda editarlos a
mano: el programa calcula los ids como `maximo + 1` y reescribe el archivo
completo en cada operación.

```
clientes.txt   id|cedula|nombre|apellido|telefono|direccion
prestamos.txt  id|id_cliente|vehiculo|valor|avance|meses|tasa|gracia|inicio
pagos.txt      id_prestamo|num_mes|fecha_pago
avisos.txt     fecha_hora|id_prestamo|cedula|nombre|vehiculo|financiado|meses|motivo
```

`inicio` y `fecha_pago` usan `AAAA-MM-DD`; `fecha_hora` usa
`AAAA-MM-DD HH:MM:SS`. `avisos.txt` es de solo anexado: las entradas no se
modifican ni se eliminan desde el programa, aunque el préstamo o el cliente ya no
existan.

## Consejos

- Ejecutar siempre el programa en el mismo directorio para ver los mismos datos.
- No abrir los archivos de datos en un editor mientras el programa está en
  ejecución.
- Las fechas de vencimiento se calculan sumando meses y recortando al último día
  del mes: `2026-01-31` vence el `2026-02-28`; en año bisiesto, `2028-01-31`
  vence el `2028-02-29`.
- El estado `VENCIDA` se evalúa contra la fecha y hora del sistema al consultar;
  no se guarda en disco.
