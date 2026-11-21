#include <stdio.h>
/* Traspuesta.
El programa calcula la traspuesta de una matriz. */

const int MAX = 50;

void lectura(int [][MAX], int, int);
void Traspuesta(int [][MAX], int[][MAX], int, int); /* Prototipos de funciones. */
void imprime(int [][MAX], int, int);

void main(void)
{
    int MAT[MAX][MAX], TRA[MAX][MAX];
    int FIL, COL;
    do
    {
        printf("Ingrese el numero de filas de la mariz: ");
        scanf("%d", &FIL);
    }
    while(COL > MAX || COL < 1);
    /* Se verifica que el numero de columnas sea el correcto. */
    lectura(MAT, FIL, COL);
    Traspuesta(MAT, TRA, FIL, COL);
    imprime(TRA, COL, FIL);
}

void lectura(int A[][MAX], int F, int C)
/* Esta funcion se utiliza para una matriz de tipo entero de F filas y C columnas. */
{
    int I,J;
    for(I=0; I<F; I++)
        for(J=0; J<C; J++)
          {
              printf("Ingrese el elemento %d %d: ", I+1, J+1 );
              scanf("%d", &A[I][J]);
          }
}

void Traspuesta(int M1[][MAX], int M2[][MAX], int F, int C)
/* Esta función se utiliza para calcular la traspuesta. */
{
   int I, J;
      for (I=0; I< F; I++)
       for (J=0; J<C; J++)
      M2[J][I] = M1[I][J];
}

void imprime(int A[][MAX], int F, int C)
/*Esta función se utiliza para escribir una matriz de tipo entero de F filas
➥y C columnas —en este caso la traspuesta. */
{
    int I, J;
    for(I=0; I<F; I++)
        for(J=0; J<C; J++)
        printf("\nElemento %d %d: %d", I+1, J+1, A[I][J]);
}
