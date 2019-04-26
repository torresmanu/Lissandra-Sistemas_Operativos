# tp-2019-1c-creativOS

## Pasos previos a levantar los proyectos en Eclipse:
1. Si ya estaban instaladas algunas commons en el sistema, pararse en la carpeta de esas commons y ejecutar `sudo make uninstall`

## Pasos para levantar los proyectos en Eclipse:
1. Clonarse el repo
2. Hacer `cd commons` y ejecutar `sudo make install`
3. En el Eclipse ir a `New -> Makefile Project with Existing Code -> Importar Kernel y elegir Linux GCC`
4. En el Eclipse ir a `New -> Makefile Project with Existing Code -> Importar PoolMemorias y elegir Linux GCC`
5. En el Eclipse ir a `New -> Makefile Project with Existing Code -> Importar LFS y elegir Linux GCC`

## Pasos para modificar funciones de las commons:
1. En el Eclipse ir a `New -> Makefile Project with Existing Code -> Importar commons y elegir Linux GCC`
2. Agregar el *archivo.c* y *archivo.h* nuevo o editar los ya existentes
3. Abrir una terminal, ir hasta la carpeta `commons` y ejecutar:
    1. `sudo make uninstall`
    2. `sudo make install`
