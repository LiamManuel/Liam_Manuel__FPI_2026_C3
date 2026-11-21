#include <stdio.h>
#include <string.h>

/* Vendedores.
El programa maneja información sobre las ventas que realizan los vendedores de
➥artículos domésticos de una importante empresa de la Ciudad de México. */

typedef struct                 /* Declaración de la estructura base. */
{
    char noba[10];               /* Nombre del banco. */
    char nocu[10];
} banco;

typedef union                  /* Declaración de la union fpago. */
{
    banco che;             /* Cheque. Campo de tiro estructura banco. */
    banco nomi;           /* Cómina. Campo de tiro estructura banco. */
    char venta;           /* Vetanilla. */
} fpago;

typedef struct                   /* Declaración de la estructura domicilio. */
{
    char cnu[20];           /* Calle y número. */
    char col[20];             /* Colonia. */
    char cp[5];             /* Código Postal. */
    char ciu[15];             /* Ciudad. */
} domicilio;

typedef struct              /* Declaración de la estructura vendedor. */
{
    int num;               /* Número de vendedor. */
    char nom[20];           /* Nombre del vendedor. */
    float ven[12];          /* Ventas del año. Arreglo unidimensional`de tipo real. */
    domicilio domi;        /* domi es de tipo estructura domicilio. */
    float sal;            /* Salario mensual. */
    fpago pago;            /* pago es de tipo unión fpago. */
    int cla;              /* Clave forma de pago. */
} vendedor;

void Lectura(vendedor *, int);
void F1(vendedor *, int);
void F2(vendedor *, int);     /* Prototipos de funciones. */
void F3(vendedor *, int);
void F4(vendedor *, int);

void main(void)
{
    vendedor VENDEDORES[100];
    /* Declaración del arreglo unidimensional de tipo estructura vendedor. */
    int TAM;
    do
    {
        printf("Ingrese el número de vendedores: ");
        scanf("%d", &TAM);
    }
    while (TAM > 100 || TAM < 1);
    /* Se verifica que el número de elementos del arreglo sea correcto. */
    Lectura(VENDEDORES, TAM);
    F1(VENDEDORES, TAM);
    F2(VENDEDORES, TAM);
    F3(VENDEDORES, TAM);
    F4(VENDEDORES, TAM);
    printf("\n\tFIN DEL PROGRAMA");
}

void Lectura(vendedor A[], int T)
/* Esta función se utiliza para leer un arreglo unidimensional de tipo
➥estructura vendedor de T elementos. */
{
    int I, J;
    for(I=0; I<T; I++)
    {
        printf("\n\tIngrese datos del vendedor %d", I+1);
        printf("\nNúmero de vendedor: ");
        scanf("%d", &A[I].num);
        printf("Nombre del vendedor: ");
        fflush(stdin);
        gets(A[I].nom);
        printf("Ventas del año: \n");
        for(J=0; J<12; J++)
        {
            printf("\tMes %d: ", J+1);
            scanf("%f", &A[I].ven[J]);
        }
        printf("Domicilio del vendedor: \n");
        printf("\tCalle y número: ");
        fflush(stdin);
        gets(A[I].domi.cnu);
        printf("\tColonia: ");
        fflush(stdin);
        gets(A[I].domi.col);
        printf("\tCódigo postal: ");
        fflush(stdin);
        gets(A[I].domi.cp);
        printf("\tCiudad: ");
        fflush(stdin);
        gets(A[I].domi.ciu);
        printf("Salario del vendedor: ");
        scanf("%f", &A[I].sal);
        printf("Forma de pago (Banco-1 Nómina-2 Ventanilla-3): ");
        scanf("%d", &A[I].cla);
        switch (A[I].cla)
        {
            case 1:{
                printf("\tNombre del banco: ");
                fflush(stdin);
                gets(A[I].pago.che.noba);
                printf("\tNúmero de cuenta: ");
                fflush(stdin);
                gets(A[I].pago.che.nocu);
            }
            break;
            case 2:{
                printf("\tNombre del banco: ");
                fflush(stdin);
                gets(A[I].pago.nomi.noba);
                printf("\tNúmero de cuenta: ");
                fflush(stdin);
                gets(A[I].pago.nomi.nocu);
            }
            break;
            case 3: A[I].pago.venta = 'S';
            break;
        }
    }
}

void F1(vendedor A[], int T)
/* Esta función se utiliza para generar las ventas totales anuales de cada uno
➥de los vendedores de la empresa. */
{
    int I, J;
    float SUM;
    printf("\n\t\tVentas Totales de los Vendedores");
    for(I=0; I<T; I++)
    {
        printf("\nVendedor: %d", A[I].num);
        SUM = 0.0;
        for(J=0; J<12; J++)
            SUM += A[I].ven[J];
        printf("\nVentas: %.2f\n", SUM);
    }
}

void F2(vendedor A[], int T)
/* Esta función se utiliza para incrementar 5% el salario de todos aquellos
➥vendedores cuyas ventas anuales superaron $1,500,000. */
{
    int I, J;
    float SUM;
    printf("\n\t\tIncremento a los Vendedores con Ventas > 1,500,000$");
    for(I=0; I<T; I++)
    {
        SUM = 0.0;
        for(J=0; J<12; J++)
            SUM += A[I].ven[J];
        if(SUM > 1500000.00)
        {
            A[I].sal = A[I].sal * 1.05;
            printf("Número de empleado: %d\nVentas: %.2f\nNuevo salario: %.2", A[I].num, SUM, A[I].sal);
        }
    }
}

void F3(vendedor A[], int T)
/* Esta función se utiliza para generar un listado de todos aquellos
➥vendedores que en el año vendieron menos de $300,000. */
{
    int  I, J;
    float SUM;
    printf("\n\t\tVendedores con ventas < 300,000");
    for(I=0; I<T; I++)
    {
        SUM = 0.0;
        for(J=0; J<12; J++)
            SUM += A[I].ven[J];
        if (SUM < 300000.00)
            printf("\nNúmero de empleado: %d\nNombre: %s\nVentas: %.2f", A[I].num, A[I].nom, SUM);
    }
}

void F4(vendedor A[], int T)
/* Esta función se usa para imprimir el número de empleado, el nombre del
➥banco y el número de cuenta de todos aquellos empleados a quienes se les
➥deposita su sueldo en cuenta de cheques. */
{
    int I;
    float SUM;
    printf("\n\t\tVendedores con Cuenta en el Banco");
    for(I=0; I<T; I++)
        if(A[I].cla == 1)
        printf("\nNúmero de vendedor: %d\n Banco: %s\nCuenta: %s", A[I].num, A[I].pago.che.noba, A[I].pago.che.nocu);
}
