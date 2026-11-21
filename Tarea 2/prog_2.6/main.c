#include <stdio.h>

/* Incremento de salario.
El programa, al recibir como dato el nivel de un profesor, incrementa su
➥salario en función de la tabla 2.3

Nivel 1 - Incremento 3.5%
Nivel 2 - Incremento 4.1%
Nivel 3 - Incremento 4.8%
Nivel 4 - Incremento 5.3%

NIV: Variable de tipo entero
SAL: Variable de tipo real. */

int main (void)
{
    float SAL;
    int NIV;

    printf("Ingrese el nivel academico del profesor: ");
    scanf("%d", &NIV);
    printf("Ingrese el salario: ");
    scanf("%f", &SAL);
    switch(NIV)
    printf("Ingrese el salario: ");
    scanf("%f", &SAL);

    switch(NIV)
    {
        case 1: SAL = SAL * 1.0035; break;
        case 2: SAL = SAL * 1.0041; break;
        case 3: SAL = SAL * 1.0048; break;
        case 4: SAL = SAL * 1.0053; break;
    }
    printf("\n\nNivel: %d \tNuevo Salario: %8.2f", NIV, SAL);

}
