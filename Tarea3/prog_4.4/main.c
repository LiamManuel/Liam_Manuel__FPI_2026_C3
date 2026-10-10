#include <stdio.h>
/* Prueba de variables globales, locales y estáticas.
El programa utiliza funciones en las que se usan diferentes tipos de
➥ variables. */

int f1(void);
int f2(void);
int f3(void);  /* Prototipo de funciones */
int f4(void);

int J = 3;  /* Variante global */

void main(void)
{
    int K;
   for (K = 1; K <= 3; K++)
   {
       printf("\nEl resultado de la función f1 es %d", f1());
       printf("\nEl resultado de la función f2 es %d", f2());
       printf("\nEl resultado de la función f3 es %d", f3());
       printf("\nEl resultado de la función f4 es %d", f4());

   }
}

int f1(void)
/* La función f1 utiliza la variante global. */
{
  J = J+J;
  return (J);
}

int f2(void)
/* La función f2 utiliza la variante local. */
{
    int K = 1;
    K++;
    return(K);
}

int f3(void)
/* La función f3 utiliza la variable estática. */
{
    static int K = 8;
    K += 2;
    return(K);
}

int f4(void)
/* La función f4 utiliza dos variables con el mismo nombre: local y global. */
{
    int K = 5;
    K = K + J; /* Uso de la variable local (K) y global (J) */
    return(K);
}


