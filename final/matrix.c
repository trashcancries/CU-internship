#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getMatrixDimensionsFromCSV(const char* filename, int* numMatrices, int* n) {
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

void freeFilters(int*** filters, int numMatrices, int n) {
    for (int i = 0; i < numMatrices; i++) {
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

    getMatrixDimensionsFromCSV(filtersFile, &numMatrices, &n);

    int*** filters = allocateFilters(numMatrices, n);

    readFiltersFromCSV(filtersFile, filters, numMatrices, n);

    for (int i = 0; i < numMatrices; i++) {
        printf("Filter %d:\n", i + 1);
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                printf("%d ", filters[i][j][k]);
            }
            printf("\n");
        }
        printf("\n");
    }

    freeFilters(filters, numMatrices, n);

    return 0;
}
