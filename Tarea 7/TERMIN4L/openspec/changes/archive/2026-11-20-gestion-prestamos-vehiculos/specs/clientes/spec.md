# Spec Delta

## Purpose

Mantiene el padrón de compradores que solicitan financiamiento de vehículos, con identificación única por cédula dominicana validada y reglas que impiden perder el historial de préstamos asociados.

## ADDED Requirements

### Requirement: Formato de la cédula dominicana

El sistema SHALL aceptar únicamente cédulas dominicanas de 11 dígitos en formato `000-0000000-0` cuyo dígito verificador sea válido. SHALL aceptar la entrada con o sin guiones y SHALL guardarla siempre normalizada con guiones.

#### Scenario: Cédula válida normalizada

- **WHEN** el operador escribe la cédula `00112345673` sin guiones
- **THEN** el sistema la acepta y la guarda como `001-1234567-3`

#### Scenario: Dígito verificador incorrecto

- **WHEN** el operador escribe la cédula `001-1234567-8`, cuyo dígito verificador no corresponde
- **THEN** el sistema rechaza la cédula e informa que el dígito verificador es inválido

#### Scenario: Longitud o caracteres inválidos

- **WHEN** el operador escribe `001-123456-3` con 10 dígitos o `001-1234567-A` con una letra
- **THEN** el sistema rechaza la cédula e informa que debe tener 11 dígitos numéricos

### Requirement: Alta de cliente

El sistema SHALL permitir registrar un cliente con cédula, nombre, apellido, teléfono y dirección. La cédula SHALL ser obligatoria, única en el padrón y cumplir el formato dominicano, y los demás campos no pueden quedar vacíos.

#### Scenario: Alta exitosa

- **WHEN** el operador registra un cliente con cédula `001-1234567-3`, nombre `Juan`, apellido `Perez`, teléfono `809-555-1234` y dirección `Calle 1 #2`
- **THEN** el sistema guarda el cliente, le asigna un identificador y confirma el alta

#### Scenario: Cédula duplicada

- **WHEN** el operador intenta registrar un cliente con una cédula que ya existe, escrita con o sin guiones
- **THEN** el sistema rechaza el alta e indica que la cédula ya está registrada, sin crear un segundo registro

#### Scenario: Campo obligatorio vacío

- **WHEN** el operador deja vacío la cédula, el nombre, el apellido, el teléfono o la dirección
- **THEN** el sistema rechaza el alta y señala el campo que falta

### Requirement: Consulta y búsqueda de clientes

El sistema SHALL listar los clientes registrados y SHALL permitir buscarlos por cédula exacta o por coincidencia parcial de nombre o apellido, sin distinguir mayúsculas de minúsculas.

#### Scenario: Búsqueda por cédula

- **WHEN** el operador busca la cédula `001-1234567-3`
- **THEN** el sistema muestra únicamente el cliente con esa cédula

#### Scenario: Búsqueda por cédula sin guiones

- **WHEN** el operador busca `00112345673` sin guiones
- **THEN** el sistema normaliza el texto de búsqueda y muestra el mismo cliente de la cédula `001-1234567-3`

#### Scenario: Búsqueda parcial por nombre

- **WHEN** el operador busca el texto `per`
- **THEN** el sistema muestra todos los clientes cuyo nombre o apellido contiene `per`, sin importar mayúsculas

#### Scenario: Búsqueda sin resultados

- **WHEN** el operador busca un texto que no coincide con ningún cliente
- **THEN** el sistema informa que no hay resultados y vuelve al menú sin modificaciones

#### Scenario: Padrón vacío

- **WHEN** el operador pide el listado y no hay clientes registrados
- **THEN** el sistema muestra un mensaje indicando que el padrón está vacío

### Requirement: Modificación de cliente

El sistema SHALL permitir modificar cédula, nombre, apellido, teléfono y dirección de un cliente existente. La cédula nueva SHALL cumplir el formato dominicano y no SHALL repetir la de otro cliente. Los préstamos del cliente SHALL seguir asociados tras la corrección.

#### Scenario: Modificación de datos de contacto

- **WHEN** el operador cambia el teléfono de un cliente existente a `809-555-9999`
- **THEN** el sistema guarda el nuevo teléfono y confirma el cambio

#### Scenario: Corrección de una cédula mal escrita

- **WHEN** el operador corrige la cédula de un cliente de `001-1234567-3` a `402-1234567-8`
- **THEN** el sistema guarda la cédula nueva y los préstamos de ese cliente siguen asociados, mostrando la cédula corregida en los reportes

#### Scenario: Cédula nueva ya registrada

- **WHEN** el operador intenta cambiar la cédula de un cliente a una que pertenece a otro cliente
- **THEN** el sistema rechaza el cambio e informa que la cédula ya está registrada, sin modificar ningún dato

#### Scenario: Cédula nueva con formato inválido

- **WHEN** el operador intenta cambiar la cédula a `001-1234567-8`
- **THEN** el sistema rechaza el cambio e informa que el dígito verificador es inválido

#### Scenario: Cliente inexistente

- **WHEN** el operador intenta modificar un cliente cuyo identificador o cédula no existe
- **THEN** el sistema informa que el cliente no existe y no modifica ningún registro

### Requirement: Eliminación de cliente

El sistema SHALL eliminar un cliente solo cuando no tenga préstamos registrados, previa confirmación. Si el cliente tiene al menos un préstamo, el sistema SHALL rechazar la eliminación e informar cuántos préstamos lo impiden.

#### Scenario: Cliente sin préstamos

- **WHEN** el operador confirma la eliminación de un cliente que no tiene préstamos
- **THEN** el sistema elimina el cliente y lo quita del padrón

#### Scenario: Cliente con préstamos

- **WHEN** el operador intenta eliminar un cliente que tiene dos préstamos
- **THEN** el sistema rechaza la eliminación, informa que existen dos préstamos asociados y conserva el cliente intacto
