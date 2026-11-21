#include <stdio.h>
/* incremento de precio.
El Programa, al recibir como dato el precio de un producto importado,
incrementa por 11% el mismo si este es inferior a 1,500.
PRE Y NPR: Variable de tipo real */

int main (void)
{
    float PRE,NPR;
    printf("Ingrese el precio del producto: ");
    scanf("%f", &PRE);

    if(PRE > 1500);
    {
        NPR = PRE * 1.11;
        printf("\nNuevo Precio: %7.2f",NPR);
    }

}

