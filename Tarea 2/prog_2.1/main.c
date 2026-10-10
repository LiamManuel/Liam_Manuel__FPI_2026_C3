#include <stdio.h>

/* Promedio curso-
El programa, al recibir como dato el promedio de un alumno de un curso universitario, escribe
aprobado si su promedio es igual o mayor que 6.-

PRO: variable de tiempo real */

 int main(void)
{
    float PRO;
    printf("Ingrese el promedio del alumno: ");
    scanf("%f", &PRO);
    if( PRO >= 6)
      printf("\nAprobado");

      return 0;
}
