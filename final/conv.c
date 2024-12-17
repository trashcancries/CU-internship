#include <stdio.h>
#include <stdlib.h>

void convolve(int rowsA, int colsA, int A[rowsA][colsA], 
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

void readMatrixFromFile(const char* filename, int rows, int cols, int matrix[rows][cols]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        exit(1);
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
    fclose(file);
}

void inferMatrixDimensions(const char* filename, int* rows, int* cols, int* numFilters) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        exit(1);
    }
    char ch;
    *cols = 0; *rows = 0; 
    int tempRows = 0;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == ',') {
            if (tempRows == 0) (*cols)++;
        } else if (ch == '\n') {
            (*rows)++;
            tempRows = 1;
        }
    }
    (*cols)++;
    *numFilters = *rows / *cols;
    fclose(file);
}

int main() {
    int rowsA, colsA, rowsB, colsB, rowsC, colsC, numFilters, stride;
    printf("Enter stride value: ");
    scanf("%d", &stride);

    inferMatrixDimensions("input_matrix.txt", &rowsA, &colsA, &numFilters);
    int A[rowsA][colsA];
    readMatrixFromFile("input_matrix.txt", rowsA, colsA, A);
    printf("Input Matrix A read from file.\n");

    rowsB = colsA;
    colsB = colsA;
    int B[rowsB][colsB];
    readMatrixFromFile("filter_matrix.txt", rowsB, colsB, B);
    printf("Filter Matrix B read from file.\n");

    rowsC = ((rowsA - rowsB) / stride) + 1;
    colsC = ((colsA - colsB) / stride) + 1;
    if (rowsC <= 0 || colsC <= 0) {
        printf("Error: Kernel B size must be smaller than input matrix A dimensions.\n");
        return 1;
    }

    int C[rowsC][colsC];
    convolve(rowsA, colsA, A, rowsB, colsB, B, stride, rowsC, colsC, C);

    printf("Resultant Matrix after Convolution with Matrix B:\n");
    for (int i = 0; i < rowsC; i++) {
        for (int j = 0; j < colsC; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
    return 0;
}
