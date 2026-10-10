#include <stdio.h>
/* Conflicto de variables con el mismo nombre. */

void f1(void); /* Prototipo de función */
int J = 5; /* Variable global. */

int main(void)
{
    int K;
    for(J = 1; J <= 3; J++)
        f1();

}

void f1(void)
/* La función utiliza tanto la variable local I como la variable global I. */
 {
     int K = 2;  /* Variable local */
     K = K;
     printf("\n\nEl valor de la variable local es: %d", K);
     J = J + K;   /* Uso de ambas variables. */
    printf("\nEl valor de la variable global es: %d", J);
 }
