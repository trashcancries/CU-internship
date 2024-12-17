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

void getMatrixDimensionsFromCSV(const char* filename, int* n) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int rowCount = 0, columnCount = 0;

    if (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }
    rowCount++;

    while (fgets(line, sizeof(line), file)) {
        rowCount++;
    }

    fclose(file);

    *n = columnCount;
}

int*** allocateMatrix(int n) {
    int numMatrices = 1;
    int*** filters = (int***)malloc(numMatrices * sizeof(int**));
    if (!filters) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numMatrices; i++) {
        filters[i] = (int**)malloc(n * sizeof(int*));
        for (int j = 0; j < n; j++) {
            filters[i][j] = (int*)malloc(n * sizeof(int));
        }
    }
    return filters;
}

void readFiltersFromCSV(const char* filename, int*** filters, int n) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int matrixIndex = 0, rowIndex = 0;

    while (fgets(line, sizeof(line), file)) {
        if (matrixIndex >= 1) break;

        char* token = strtok(line, ",");
        for (int colIndex = 0; colIndex < n && token; colIndex++) {
            filters[matrixIndex][rowIndex][colIndex] = atoi(token);
            token = strtok(NULL, ",");
        }

        rowIndex++;
        if (rowIndex == n) {
            rowIndex = 0;
            matrixIndex++;
        }
    }

    fclose(file);
}

void getFilterMatrixDimensionsFromCSV(const char* filename, int* numMatrices, int* n) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int rowCount = 0, columnCount = 0;

    if (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ",");
        while (token) {
            columnCount++;
            token = strtok(NULL, ",");
        }
    }
    rowCount++;

    while (fgets(line, sizeof(line), file)) {
        rowCount++;
    }

    fclose(file);

    *n = columnCount;
    *numMatrices = rowCount / (*n);
}

int*** allocateFilters(int numMatrices, int n) {
    int*** filters = (int***)malloc(numMatrices * sizeof(int**));
    if (!filters) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numMatrices; i++) {
        filters[i] = (int**)malloc(n * sizeof(int*));
        for (int j = 0; j < n; j++) {
            filters[i][j] = (int*)malloc(n * sizeof(int));
        }
    }
    return filters;
}

void readFiltersFromCSV(const char* filename, int*** filters, int numMatrices, int n) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int matrixIndex = 0, rowIndex = 0;

    while (fgets(line, sizeof(line), file)) {
        if (matrixIndex >= numMatrices) break;

        char* token = strtok(line, ",");
        for (int colIndex = 0; colIndex < n && token; colIndex++) {
            filters[matrixIndex][rowIndex][colIndex] = atoi(token);
            token = strtok(NULL, ",");
        }

        rowIndex++;
        if (rowIndex == n) {
            rowIndex = 0;
            matrixIndex++;
        }
    }

    fclose(file);
}

void freeFilters(int*** filters, int numMatrices, int n) {
    for (int i = 0; i < numMatrices; i++) {
        for (int j = 0; j < n; j++) {
            free(filters[i][j]);
        }
        free(filters[i]);
    }
    free(filters);
}

void freeMatrix(int*** filters, int n) {
    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < n; j++) {
            free(filters[i][j]);
        }
        free(filters[i]);
    }
    free(filters);
}

int main() {
    const char* filtersFile = "filters.csv";
    int numMatrices, n;

    getFilterMatrixDimensionsFromCSV(filtersFile, &numMatrices, &n);
    int*** filters = allocateFilters(numMatrices, n);
    readFiltersFromCSV(filtersFile, filters, numMatrices, n);

    const char* inputFile = "input.csv";
    int x;

    getMatrixDimensionsFromCSV(inputFile, &x);
    int*** input = allocateMatrix(x);
    readMatrixFromCSV(inputFile, input, x);

    // TODO: implement convolution b/w filters and input
    int stride;
    printf("Enter stride value: ");
    scanf("%d", &stride);

    int a = ((x - n) / stride) + 1;

    if (a <= 0) {
        printf("Error: Kernel B size must be smaller than input matrix A dimensions.\n");
        return 1;
    }

    int*** res = allocateMatrix(a);

    for (int i = 0; i < numMatrices; i++) {
        convolve(x, x, input, n, n, filters, stride, a, a, res);

        for (int j = 0; j < a; j++) {
        for (int k = 0; k < a; k++) {
            printf("%d ", res[j][k]);
        }
        printf("\n");
    }

    freeMatrix(input, x);
    }

    freeFilters(filters, numMatrices, n);
    freeMatrix(input, x);

    return 0;
}