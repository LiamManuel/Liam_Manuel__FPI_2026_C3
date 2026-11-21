#include <stdio.h>

/* Paso de una función como parámetro por referencia. */

int Suma(int x, int y)
/* La función Suma regresa la suma de los parámetros de tipo entero
X y Y. */
{
return (x + y);
}

int Resta(int x, int y)
/* Esta función regresa la resta de los parámetros de tipo entero
X y Y. */
{
    return (x - y);
}

int Control(int (*apf)(int, int), int x, int y)
/* Esta función recibe como parámetro otra función -la dirección- y
dependiendo de cuál sea ésta, llama a la función Suma o Resta. */
{
    int RES;
    RES = (*apf) (x,y);    /* Se llama a la función Suma o Resta. */
    return (RES);
}

void main(void)
{
    int R1, R2;
    R1 = Control(Suma,  15,5);  /* Se pasa como parámetro para la función Suma. */
    R2 = Control (Resta,10,4); /* Se pasa como parámetro para la función Resta. */
    printf("\nEl resultado 1 es: %d", R1);
    printf("\nEl resultado 2 es: %d", R2);
}
