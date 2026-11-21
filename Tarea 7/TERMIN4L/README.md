# Gestión de préstamos de vehículos

Programa de consola en C para administrar clientes y préstamos de vehículos con
interés plano, pagos secuenciales y reportes en pantalla. No usa librerías
externas: solo la biblioteca estándar de C y la API de consola de Windows.

## Características

- Alta, modificación, búsqueda y eliminación de clientes con cédula dominicana
  validada (11 dígitos y dígito verificador módulo 10).
- Préstamos con valor del vehículo, avance, meses, tasa anual, meses de gracia
  de capital y fecha de inicio.
- Interés plano en montos enteros, sin decimales, con reparto del residuo entre
  las primeras cuotas de amortización.
- Calendario de cuotas con vencimientos, interés, capital, saldo y estado
  (`PAGADA`, `VENCIDA`, `PENDIENTE`).
- Pagos estrictamente secuenciales: una cuota por operación, siempre la pendiente
  con el número más bajo.
- Eliminación de préstamos solo sin pagos, con advertencia, confirmación `SI` y
  registro en una bitácora de solo anexado.
- Reportes TUI de 80 columnas con paginación por `N`, `P` y `Q`.
- Persistencia en cuatro archivos de texto delimitados por `|`.

## Requisitos

- Windows con MinGW (el incluido en Code::Blocks sirve).
- `gcc` compatible con C11.
- `mingw32-make` (incluido con Code::Blocks) o `make` de GNU.

Si `gcc` no está en el `PATH`, se puede usar la ruta completa del toolchain de
Code::Blocks:

```
C:\Program Files\CodeBlocks\MinGW\bin
```

## Compilación

Desde la carpeta del proyecto, en PowerShell, se agrega el toolchain de
Code::Blocks al `PATH` de la sesión y se compila:

```powershell
$env:Path = "C:\Program Files\CodeBlocks\MinGW\bin;" + $env:Path
mingw32-make
```

Si `gcc` y `mingw32-make` ya están en el `PATH`, basta con la última línea. En
Linux o macOS con GNU Make:

```bash
make
```

El binario se genera como `gestion.exe` en Windows o `gestion` en Linux/macOS.
La compilación usa `-std=c11 -Wall -Wextra -Wpedantic -O2` y no debe producir
advertencias.

### Code::Blocks

1. Crear un proyecto de consola vacío.
2. Agregar todos los archivos `.c` y `.h` de `src/`.
3. En las opciones del proyecto, usar el compilador GNU GCC con el estándar C11.
4. Compilar y ejecutar. El directorio de trabajo debe ser la carpeta del
   proyecto para que los archivos de datos queden junto al ejecutable.

## Ejecución

```powershell
.\gestion.exe
```

En Linux o macOS:

```bash
./gestion
```

El programa crea en el directorio actual los archivos `clientes.txt`,
`prestamos.txt`, `pagos.txt` y `avisos.txt` si no existen. La guía completa de
menús, validaciones y navegación está en [docs/uso.md](docs/uso.md).

## Pruebas

```powershell
mingw32-make test
```

O, en Linux/macOS:

```bash
make test
```

Las pruebas de `tests/checks.c` cubren formatos de montos y fechas, cédula
dominicana, cálculo de interés y residuo, gracia, calendario, pagos
secuenciales, validaciones, modificación, estados, reportes y persistencia.
Terminan con `TODO EN VERDE` y código de salida `0`.

Limpieza de binarios:

```powershell
mingw32-make clean
```

## Archivos de datos

Todos son texto plano separado por `|`:

```
clientes.txt   id|cedula|nombre|apellido|telefono|direccion
prestamos.txt  id|id_cliente|vehiculo|valor|avance|meses|tasa|gracia|inicio
pagos.txt      id_prestamo|num_mes|fecha_pago
avisos.txt     fecha_hora|id_prestamo|cedula|nombre|vehiculo|financiado|meses|motivo
```

Los montos se guardan como enteros. Las fechas de los préstamos y los pagos usan
`AAAA-MM-DD`; la bitácora usa `AAAA-MM-DD HH:MM:SS`. `avisos.txt` solo recibe
entradas nuevas: el programa nunca las modifica ni las borra.

Los cuatro archivos de datos y los binarios generados están ignorados por Git.

## Estructura

```
src/
  main.c        menús y despacho de opciones
  model.h       structs Cliente, Prestamo, Pago, Aviso y Cuota
  archivos.c/h  carga, guardado atómico y arreglos dinámicos
  clientes.c/h  alta, búsqueda, modificación y eliminación de clientes
  prestamos.c/h calendario, validaciones, pagos y eliminación
  reportes.c/h  construcción de reportes de solo lectura
  tui.c/h       marcos ANSI, paginación y lectura de teclas
  util.c/h      entrada validada, fechas, cadenas y cédula
tests/
  checks.c      aserciones de utilidades, negocio y persistencia
docs/
  uso.md        manual de uso
```

## Alcance

Incluye gestión de clientes, préstamos con interés plano y gracia de capital,
pagos secuenciales, bitácora de eliminaciones y reportes en pantalla.

No incluye exportación de reportes a archivo, impresión, multiusuario,
autenticación, cobros por mora, pagos parciales ni abonos anticipados. Los
montos son enteros: no se admiten centavos.
