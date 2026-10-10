#include <stdio.h>

/* Prueba de parámetros. */

int f1 (int *);
/* Prototipo de función. El parámetro es de tipo entero y por referencia
- observa el uso del operador de indirección. */

int main(void)
{
    int I, K = 4;
    for(I = 1; I <= 3; I++)
    {
        printf("\n\nValor de K antes de llamar a la funcion: %d", ++K);
        printf("\n\nValor de K despues de llamar a la funcion: %d", f1(&K));
        /* Llamada a la función f1. Se pasa a la dirección de la variable K,
        por medio del operador de dirección: &. */
    }
}

int f1(int * R)
/* La función f1 recibe un parametro por referencia. Cada vez que el parametro
se utiliza en la función debe ir precedido por el operador de indirección. */
{
    *R += *R;
}
