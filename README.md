# Cruz FileSystem

Grupo: `ThanOS`

Integrantes:

- Camila Chávez      - 16207041
- Cristóbal Ilabaca  - 16636902
- Daniel Leal        - 17637554
- Felipe Valenzuela  - 15636461

## Decisiones de diseño

Decidimos que el programa cargará el disco completo en la memoria y solamente escribirá en el disco real una vez que se desmonte el disco virtual debido a que esto facilita modelar el disco con `structs`. El disco almacena bloques "crudos" de información que son interpretados al momento de usarse y luego vuelven a ser almacenados como bloques de bytes.

## Supuestos

Suponemos que cuando las funciones de `loading` y `unloading` sean llamadas haciendo referencia a la carpeta raíz del repo como directorio real, ésta será llamada así:

```c
cr_unload("/virtual/path", "./");
```

y no así:

```c
cr_unload("/virtual/path", "");
```

Suponemos, además, que todas las rutas entregadas al sistema de archivos serán absolutas, es decir, partirán con un `/` y harán referencia siempre a una ruta que parte desde el bloque índice del disco.
