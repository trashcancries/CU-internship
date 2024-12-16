#include <stdio.h>


void convol(int rowsA, int colsA, int A[rowsA][colsA], 
              int rowsB, int colsB, int B[rowsB][colsB], 
              int stride, int rowsC, int colsC, int C[rowsC][colsC]) {
    for (int i = 0; i < rowsC; i++) {
        for (int j = 0; j < colsC; j++) {
            int sum = 0;
            for (int m = 0; m < rowsB; m++) {
                for (int n = 0; n < colsB; n++) {
                    sum += A[i * stride + m][j * stride + n] * B[m][n];
                }
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    int rowsA, colsA, rowsB, colsB, rowsD, colsD;

    
    printf("Enter dimensions of Matrix A (rows and columns): ");
    scanf("%d %d", &rowsA, &colsA);

    
    int A[rowsA][colsA];
    printf("Enter elements of Matrix A:\n");
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsA; j++) {
            scanf("%d", &A[i][j]);
        }
    }

    
    printf("Enter dimensions of Matrix B (rows and columns): ");
    scanf("%d %d", &rowsB, &colsB);

    
    int B[rowsB][colsB];
    printf("Enter elements of Matrix B (kernel):\n");
    for (int i = 0; i < rowsB; i++) {
        for (int j = 0; j < colsB; j++) {
            scanf("%d", &B[i][j]);
        }
    }


    int rowsC = rowsA - rowsB + 1;
    int colsC = colsA - colsB + 1;
    if (rowsC <= 0 || colsC <= 0) {
        printf("Error: Kernel B size must be smaller than input matrix A dimensions.\n");
        return 1;
    }

    
    printf("Enter dimensions of Matrix D (rows and columns): ");
    scanf("%d %d", &rowsD, &colsD);

    
    int D[rowsD][colsD];
    printf("Enter elements of Matrix D (kernel):\n");
    for (int i = 0; i < rowsD; i++) {
        for (int j = 0; j < colsD; j++) {
            scanf("%d", &D[i][j]);
        }
    }

    
    int rowsE = rowsA - rowsD + 1;
    int colsE = colsA - colsD + 1;
    if (rowsE <= 0 || colsE <= 0) {
        printf("Error: Kernel D size must be smaller than input matrix A dimensions.\n");
        return 1;
    }

    
    int C[rowsC][colsC];
    int E[rowsE][colsE];

    
    convol(rowsA, colsA, A, rowsB, colsB, B, rowsC, colsC, C);

    
    convol(rowsA, colsA, A, rowsD, colsD, D, rowsE, colsE, E);

    
    printf("Resultant Matrix after Convolution with Matrix B:\n");
    for (int i = 0; i < rowsC; i++) {
        for (int j = 0; j < colsC; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }

    printf("Resultant Matrix after Convolution with Matrix D:\n");
    for (int i = 0; i < rowsE; i++) {
        for (int j = 0; j < colsE; j++) {
            printf("%d ", E[i][j]);
        }
        printf("\n");
    }

    return 0;
}