#include <stdio.h>

/* Cubo-3.
El programa calcula el cubo de los 10 primeros números naturales con la ayuda de una función
y utilizando parámetros por valor. */

int cubo(int);      /* Prototipo de función y utilizando parámetros es de tipo entero. */

void main(void)
{
    int I;
    for(I = 1; I <= 10; I++)
        printf("\nEl cubo de I es: %d", cubo(I));
    /* LLamada a la función cubo. El paso del parámetro es por valor. */
}

int cubo( int K)     /* K es un parámetro por valor de tipo entero. */
/* La función calcula el cubo del parametro K. */
{
    return(K*K*K);
}
