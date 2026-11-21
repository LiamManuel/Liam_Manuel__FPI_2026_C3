#include <stdio.h>

/* Fábrica de lácteos.
El programa, al recibir como datos las ventas mensuales de diferentes
➥productos, obtiene información estadística valiosa para la empresa. */

void lectura1(int [15][12]);
void lectura2(float *, int);
void funcion1(int [][12], int, int, float *, float *);  /* Prototipos de funciones. */
void funcion2(float *, int );
void funcion3(float *, int);

void main(void)
{
    int FAB[15][12] = {0};   /* Inicializacion en 0 del arreglo FAB. */
    float COS[15], VEN[15];
    lectura1(FAB);
    lectura2(COS, 15);
    funcion1(FAB, 15, 12, COS, VEN);
    funcion2(VEN, 15);
    funcion3(VEN, 15);
}

void lectura1(int A[][12])
/* Esta función se utiliza para almacenar en el arreglo bidimensional las
➥diferentes transacciones que representan las ventas de los diferentes
➥productos. El fin de datos está dado por –1. */
{
    int MES, PRO, CAN;
    printf("\nIngrese mes, tipo de producto y cantidad vendida: ");
    scanf("%d %d %d", &MES, &PRO, &CAN);
    while (MES!= -1 && PRO!= -1 && CAN!= -1)
    {
        A[MES-1][PRO-1] += CAN;
        printf("Ingrese mes, tipo de producto y cantidad vendida:   ");
        scanf("%d %d %d", &MES, &PRO, &CAN);
    }
}

void lectura2(float A[], int N)
/* Esta función se utiliza para leer los precios de venta de los diferentes
➥productos. */
{
    int I;
    for(I=0; I<N; I++)
    {
        printf("Ingrese el costo del producto %d: ", I+1);
        scanf("%f", &A[I]);
    }
}

void funcion1(int A[][12], int F, int C, float V1[], float V2[])
/* Esta función se utiliza para calcular el monto de venta anual de cada uno
➥de los productos. Observa que el resultado se almacena en un arreglo
➥unidimensional que se utilizará posteriormente. */
{
    int I, J, SUM;
    printf("\n");
    for(I=0; I<F; I++)
    {
        SUM = 0;
        for(J=0; J<C; J++)
            SUM += A[I][J];
        V2[I] = V1[I] * SUM;
        printf("\nTotal de ventas del producto %d: %8.2f", I+1, V2[I]);
    }
}

void funcion2(float A[], int C)
/*Esta función se utiliza para calcular el monto total de ventas de la fábrica. */
{
    int I;
    float SUM = 0.0;
    for(I=0; I<C; I++)
        SUM += A[I];
    printf("n\nEl total de ventas de la fabrica: %.2f", SUM);
}

void funcion3(float A[], int C)
/* Esta función se utiliza para obtener el tipo de producto que más se ha vendido
➥y el monto de las ventas de dicho producto. */
{
    int I, TPR = 0;
    float VEN = A[0];
    for(I=1; I<C; I++)
        if(VEN < A[I])
    {
        TPR = I;
        VEN = A[I];
    }
    printf("\n\nTipo de producto mas vendido: %d \t Ventas: %2.f", TPR + 1, VEN);
}
