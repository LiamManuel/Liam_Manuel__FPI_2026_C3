#include <stdio.h>

/* Funciones para el manejo de caracteres de la biblioteca ssttddiioo..hh */

void main(void)
{
   char p1, p2, p3 = '$';
   /* Declaracion de las variables tipo caracter p1, p2, y p3. Observe que a p3 se le asigna el simbolo $. */
   printf("\nIngrese un caracter: ");
   p1 = getchar();    /* Se utiliza la funcion getchar para leer un caracter. */
   putchar(p1);        /* Se utiliza la funcion putchar para escribir un caracter. */
   printf("\n");
   fflush(stdin);

   printf("\nEl caracter p3 es: ");
   putchar(p3);
   /* Se utiliza la funcion putchar para escribir el caracter almacenado en p3. */
   printf("\n");
   printf("Ingrese otro caracter: ");
   fflush(stdin);
   scanf("%c", &p2);
   /* Se puede utilizar scanf con el formato de variable %c para leer un caracter. */
   printf("%c",&p2);
   /* Se puede utilizar scanf con el formato de variable %c para escribir un caracter. */
}



