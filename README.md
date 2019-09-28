# Cruz FileSystem

Grupo: `ThanOS`

Integrantes:

- Camila Chávez
- Cristóbal Ilabaca
- Daniel Leal
- Felipe Valenzuela

## Decisiones de diseño

Decidimos que el programa cargará el disco completo en la memoria y solamente escribirá en el disco real una vez que se desmonte el disco virtual debido a que esto facilita modelar el disco con `structs`.

## Supuestos

Suponemos que nos funciona.
