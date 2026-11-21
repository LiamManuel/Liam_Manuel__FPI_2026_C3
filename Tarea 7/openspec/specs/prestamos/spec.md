# prestamos Specification

## Purpose

Gestiona los préstamos de vehículos con interés plano en montos enteros: su creación, el período de gracia, el calendario de cuotas, el registro secuencial de pagos y las reglas que preservan el historial del comprador.

## Requirements

### Requirement: Creación de préstamo

El sistema SHALL crear un préstamo para un cliente existente a partir del valor del vehículo, el avance del cliente, la cantidad de meses, la tasa de interés anual, los meses de gracia, la fecha de inicio y la descripción del vehículo. El monto financiado SHALL calcularse como valor del vehículo menos avance y no se ingresa manualmente.

#### Scenario: Creación exitosa

- **WHEN** el operador registra un préstamo para un cliente existente con valor `1000000`, avance `200000`, `36` meses, tasa `18` y sin gracia
- **THEN** el sistema guarda el préstamo con monto financiado `800000` y muestra el calendario de 36 cuotas

#### Scenario: Cliente inexistente

- **WHEN** el operador intenta crear un préstamo para un cliente que no está en el padrón
- **THEN** el sistema rechaza la creación e informa que el cliente no existe

### Requirement: Validación de los datos del préstamo

El sistema SHALL rechazar un préstamo con valor menor o igual a cero, avance negativo o mayor o igual al valor, meses fuera de 1 a 120, tasa menor o igual a cero o mayor a 100, o meses de gracia negativos o mayores o iguales a los meses. El rechazo SHALL indicar el campo inválido sin guardar nada.

#### Scenario: Avance igual al valor del vehículo

- **WHEN** el operador registra un préstamo con valor `500000` y avance `500000`
- **THEN** el sistema rechaza la creación e informa que el avance debe ser menor que el valor del vehículo

#### Scenario: Avance negativo

- **WHEN** el operador registra un préstamo con avance `-1`
- **THEN** el sistema rechaza la creación e informa que el avance no puede ser negativo

#### Scenario: Meses fuera de rango

- **WHEN** el operador registra un préstamo con `0` meses o con `121` meses
- **THEN** el sistema rechaza la creación e informa que los meses deben estar entre 1 y 120

#### Scenario: Tasa no positiva

- **WHEN** el operador registra un préstamo con tasa anual `0` o `-5`
- **THEN** el sistema rechaza la creación e informa que la tasa debe ser mayor que cero

#### Scenario: Tasa demasiado alta

- **WHEN** el operador registra un préstamo con tasa anual `101`
- **THEN** el sistema rechaza la creación e informa que la tasa no puede superar 100

#### Scenario: Gracia que consume todas las cuotas

- **WHEN** el operador registra un préstamo de 36 meses con `36` meses de gracia
- **THEN** el sistema rechaza la creación e informa que la gracia debe ser menor que los meses del préstamo

### Requirement: Período de gracia de capital

El sistema SHALL aceptar hasta dos dígitos de meses de gracia, entre 0 y meses−1. Durante cada cuota de gracia el cliente SHALL pagar solo el interés mensual, con capital cero y saldo sin variación, y la amortización del capital SHALL comenzar en la cuota siguiente a la gracia.

#### Scenario: Dos cuotas de gracia

- **WHEN** el operador consulta el calendario de `800000` a 36 meses al 18% con 2 meses de gracia
- **THEN** las cuotas 1 y 2 muestran cuota `12000`, capital `0` y saldo `800000`, y la cuota 3 es la primera que amortiza capital

#### Scenario: Sin gracia

- **WHEN** el operador consulta un préstamo con `0` meses de gracia
- **THEN** la cuota 1 ya amortiza capital

#### Scenario: La gracia no altera los totales

- **WHEN** se compara un préstamo de `800000` a 36 meses al 18% con y sin 2 meses de gracia
- **THEN** ambos muestran el mismo total de intereses `432000` y el mismo total de capital `800000`, solo cambia el reparto del capital entre las cuotas

### Requirement: Cálculo del interés plano en enteros

El sistema SHALL calcular el interés mensual como el redondeo al entero más cercano de `financiado * tasa_anual / 1200`, y SHALL mantenerlo constante durante todo el préstamo, incluidas las cuotas de gracia. Todos los montos del préstamo, las cuotas y los reportes SHALL expresarse como enteros, sin decimales.

#### Scenario: Interés constante sin decimales

- **WHEN** el operador consulta el calendario de un préstamo de `800000` al `18` por ciento anual
- **THEN** cada una de las 36 cuotas muestra un interés de `12000` y ningún monto contiene decimales

### Requirement: Reparto del residuo entre las cuotas de amortización

El sistema SHALL dividir el monto financiado entre los meses de amortización (los meses del préstamo menos los meses de gracia) usando división entera, y SHALL repartir el residuo agregando una unidad de capital a cada una de las primeras cuotas de amortización hasta agotarlo.

#### Scenario: Residuo repartido en las primeras cuotas de amortización

- **WHEN** el operador consulta el calendario de `800000` a 36 meses al 18% con 2 meses de gracia
- **THEN** las cuotas 3 a 16 muestran capital `23530` y cuota `35530`, las cuotas 17 a 36 muestran capital `23529` y cuota `35529`, y la suma de capital es `800000`

#### Scenario: División exacta sin residuo

- **WHEN** el monto financiado es divisible exactamente entre los meses de amortización
- **THEN** todas las cuotas de amortización muestran el mismo capital y la misma cuota

### Requirement: Calendario de cuotas con saldos

El sistema SHALL generar el calendario del préstamo con una fila por mes que incluya número de cuota, fecha de vencimiento, cuota, interés, capital y saldo posterior al pago. La fecha de vencimiento SHALL ser la fecha de inicio más el número de cuota, ajustada al último día del mes cuando ese día no exista, y el saldo de la última cuota SHALL ser cero.

#### Scenario: Vencimientos mensuales

- **WHEN** se crea un préstamo de 36 meses con fecha de inicio `2026-01-15`
- **THEN** el calendario muestra 36 cuotas con vencimientos mensuales desde `2026-02-15` hasta `2029-01-15` y saldo final `0`

#### Scenario: Inicio a fin de mes

- **WHEN** se crea un préstamo con fecha de inicio `2026-01-31`
- **THEN** la primera cuota vence el `2026-02-28` y la segunda el `2026-03-31`

#### Scenario: Saldo que se mantiene y luego decrece

- **WHEN** el operador consulta el calendario de un préstamo con gracia
- **THEN** el saldo se mantiene igual durante las cuotas de gracia, luego es el saldo anterior menos el capital de cada cuota, y nunca aumenta

### Requirement: Estado de cada cuota

El sistema SHALL determinar el estado de cada cuota al consultarla: `PAGADA` si tiene un pago registrado, `VENCIDA` si su fecha de vencimiento es anterior a la fecha actual y no tiene pago, o `PENDIENTE` en cualquier otro caso.

#### Scenario: Cuota vencida sin pago

- **WHEN** la fecha actual es posterior al vencimiento de una cuota sin pago
- **THEN** el calendario muestra esa cuota como `VENCIDA`

#### Scenario: Cuota futura sin pago

- **WHEN** la fecha actual es anterior al vencimiento de una cuota sin pago
- **THEN** el calendario muestra esa cuota como `PENDIENTE`

#### Scenario: Cuota pagada con vencimiento pasado

- **WHEN** una cuota tiene pago registrado aunque su vencimiento ya pasó
- **THEN** el calendario muestra esa cuota como `PAGADA`

### Requirement: Registro de pago secuencial por número de cuota

El sistema SHALL registrar una sola cuota por operación, siempre la cuota pendiente con el número más bajo: primero la 1ra, luego la 2da, luego la 3ra, luego la 4ta y así sucesivamente, junto con la fecha del pago. SHALL rechazar el intento de pagar una cuota posterior mientras exista una anterior sin pagar, y SHALL rechazar un nuevo pago cuando todas las cuotas estén pagadas.

#### Scenario: Pago de la primera cuota

- **WHEN** el operador registra un pago en un préstamo cuya primera cuota está pendiente
- **THEN** el sistema marca la cuota 1 como `PAGADA` y la siguiente cuota pendiente pasa a ser la 2

#### Scenario: Pago en orden hasta la cuarta

- **WHEN** el operador registra tres pagos consecutivos y luego consulta el préstamo
- **THEN** las cuotas 1, 2 y 3 están `PAGADA` y la elegible para el siguiente pago es la 4

#### Scenario: Intento de adelantar una cuota

- **WHEN** el operador intenta pagar la cuota 4 mientras la cuota 1 sigue pendiente
- **THEN** el sistema rechaza la operación, informa que debe pagarse primero la cuota 1 y no registra ningún pago

#### Scenario: Préstamo totalmente pagado

- **WHEN** el operador intenta registrar un pago en un préstamo cuyas 36 cuotas ya están pagadas
- **THEN** el sistema rechaza la operación e informa que el préstamo está cancelado

### Requirement: Modificación de préstamo

El sistema SHALL permitir modificar valor del vehículo, avance, meses, tasa, meses de gracia y fecha de inicio de un préstamo sin pagos, recalculando su calendario. Si el préstamo tiene al menos un pago, SHALL permitir modificar solo la descripción del vehículo y SHALL rechazar cambios en los campos financieros.

#### Scenario: Recalcular un préstamo sin pagos

- **WHEN** el operador cambia la tasa de un préstamo sin pagos de `18` a `24`
- **THEN** el sistema guarda la nueva tasa y muestra el calendario recalculado con el nuevo interés mensual

#### Scenario: Agregar gracia a un préstamo sin pagos

- **WHEN** el operador cambia los meses de gracia de un préstamo sin pagos de `0` a `2`
- **THEN** el sistema recalcula el calendario y las cuotas 1 y 2 pasan a ser de solo interés

#### Scenario: Intento de cambiar meses con pagos registrados

- **WHEN** el operador intenta cambiar los meses de un préstamo que ya tiene un pago
- **THEN** el sistema rechaza el cambio e informa que solo puede modificar la descripción del vehículo

#### Scenario: Cambio de descripción con pagos registrados

- **WHEN** el operador corrige la descripción del vehículo de un préstamo que ya tiene pagos
- **THEN** el sistema guarda la nueva descripción sin alterar el calendario ni los pagos existentes

### Requirement: Eliminación de préstamo sin pagos

El sistema SHALL eliminar un préstamo únicamente cuando no tenga pagos registrados. Antes de eliminar SHALL mostrar una advertencia de que el préstamo se elimina por falta de pagos y que la operación no se puede deshacer, y SHALL exigir que el operador confirme escribiendo `SI`.

#### Scenario: Confirmación de la eliminación

- **WHEN** el operador confirma con `SI` la eliminación de un préstamo sin pagos
- **THEN** el sistema elimina el préstamo, deja de listarlo y lo reporta como eliminado

#### Scenario: Cancelación de la advertencia

- **WHEN** el operador responde algo distinto de `SI` en la advertencia de eliminación
- **THEN** el sistema conserva el préstamo sin cambios y vuelve al menú

### Requirement: Bloqueo de eliminación con historial de pagos

El sistema SHALL rechazar la eliminación de un préstamo que tenga al menos un pago registrado, e SHALL informar que existe historial de pagos que debe conservarse.

#### Scenario: Préstamo con un pago

- **WHEN** el operador intenta eliminar un préstamo que tiene una cuota pagada
- **THEN** el sistema rechaza la eliminación, muestra cuántos pagos existen y conserva el préstamo y sus pagos intactos

### Requirement: Bitácora de avisos de eliminación

El sistema SHALL registrar en una bitácora de solo anexado cada préstamo eliminado, con fecha y hora, identificador del préstamo, cédula y nombre del cliente, descripción del vehículo, monto financiado, cantidad de cuotas y el motivo `falta de pagos`. La bitácora SHALL conservarse aunque el préstamo ya no exista.

#### Scenario: Registro del aviso

- **WHEN** el sistema elimina el préstamo `12` de un cliente por falta de pagos
- **THEN** la bitácora contiene una entrada con la fecha y hora de la operación, el préstamo `12`, los datos del cliente, los montos y el motivo `falta de pagos`

#### Scenario: Consulta posterior del aviso

- **WHEN** el operador consulta la bitácora de avisos después de eliminar el préstamo
- **THEN** el sistema muestra las entradas registradas, incluida la del préstamo eliminado

### Requirement: Persistencia entre ejecuciones

El sistema SHALL conservar clientes, préstamos, pagos y avisos entre ejecuciones, y SHALL crear los archivos de almacenamiento vacíos en el primer arranque cuando no existan.

#### Scenario: Continuidad tras cerrar el programa

- **WHEN** el operador registra un pago, cierra el programa y lo vuelve a abrir
- **THEN** el pago sigue registrado y la cuota continúa mostrándose como `PAGADA`

#### Scenario: Primer arranque sin archivos

- **WHEN** el programa se ejecuta en un directorio sin archivos de datos
- **THEN** crea los archivos vacíos, inicia con los listados vacíos y no reporta errores
