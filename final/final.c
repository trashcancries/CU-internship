#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int rows;
    int cols;
    int** data;
} Matrix;

Matrix* createMatrix(int rows, int cols) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (!matrix) {
        fprintf(stderr, "Memory allocation failed for matrix structure\n");
        exit(EXIT_FAILURE);
    }

    matrix->rows = rows;
    matrix->cols = cols;
    
    matrix->data = (int**)malloc(rows * sizeof(int*));
    if (!matrix->data) {
        free(matrix);
        fprintf(stderr, "Memory allocation failed for matrix rows\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++) {
        matrix->data[i] = (int*)calloc(cols, sizeof(int));
        if (!matrix->data[i]) {
            for (int j = 0; j < i; j++) {
                free(matrix->data[j]);
            }
            free(matrix->data);
            free(matrix);
            fprintf(stderr, "Memory allocation failed for matrix columns\n");
            exit(EXIT_FAILURE);
        }
    }

    return matrix;
}

void freeMatrix(Matrix* matrix) {
    if (!matrix) return;
    
    for (int i = 0; i < matrix->rows; i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}

Matrix* readMatrixFromCSV(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    int rows = 0, cols = 0;
    char line[4096];
    
    while (fgets(line, sizeof(line), file)) {
        rows++;
        
        if (rows == 1) {
            char* token = strtok(line, ",");
            while (token) {
                cols++;
                token = strtok(NULL, ",");
            }
            rewind(file);
        }
    }

    Matrix* matrix = createMatrix(rows, cols);

    rewind(file);
    for (int i = 0; i < rows; i++) {
        if (!fgets(line, sizeof(line), file)) break;
        
        char* token = strtok(line, ",");
        for (int j = 0; j < cols && token; j++) {
            matrix->data[i][j] = atoi(token);
            token = strtok(NULL, ",");
        }
    }

    fclose(file);
    return matrix;
}

Matrix* convolve(Matrix* input, Matrix* filter, int stride) {
    int outputRows = ((input->rows - filter->rows) / stride) + 1;
    int outputCols = ((input->cols - filter->cols) / stride) + 1;

    if (outputRows <= 0 || outputCols <= 0) {
        fprintf(stderr, "Invalid convolution dimensions\n");
        return NULL;
    }

    Matrix* output = createMatrix(outputRows, outputCols);

    for (int i = 0; i < outputRows; i++) {
        for (int j = 0; j < outputCols; j++) {
            int sum = 0;
            for (int m = 0; m < filter->rows; m++) {
                for (int n = 0; n < filter->cols; n++) {
                    sum += input->data[i*stride + m][j*stride + n] * filter->data[m][n];
                }
            }
            output->data[i][j] = sum;
        }
    }

    return output;
}

void printMatrix(Matrix* matrix) {
    if (!matrix) {
        printf("NULL matrix\n");
        return;
    }

    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            printf("%d ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

int main() {
    const char* inputFile = "input.csv";
    const char* filtersFile = "filters.csv";

    Matrix* inputMatrix = readMatrixFromCSV(inputFile);
    if (!inputMatrix) {
        fprintf(stderr, "Failed to read input matrix\n");
        return EXIT_FAILURE;
    }

    Matrix* filtersMatrix = readMatrixFromCSV(filtersFile);
    if (!filtersMatrix) {
        fprintf(stderr, "Failed to read filters matrix\n");
        freeMatrix(inputMatrix);
        return EXIT_FAILURE;
    }

    int stride;
    printf("Enter stride value: ");
    if (scanf("%d", &stride) != 1 || stride <= 0) {
        fprintf(stderr, "Invalid stride value\n");
        freeMatrix(inputMatrix);
        freeMatrix(filtersMatrix);
        return EXIT_FAILURE;
    }

    int numFilters = filtersMatrix->rows / filtersMatrix->cols;

    for (int f = 0; f < numFilters; f++) {
        Matrix* currentFilter = createMatrix(filtersMatrix->cols, filtersMatrix->cols);
        for (int i = 0; i < currentFilter->rows; i++) {
            for (int j = 0; j < currentFilter->cols; j++) {
                currentFilter->data[i][j] = filtersMatrix->data[f * currentFilter->rows + i][j];
            }
        }

        printf("\nConvolution Result for Filter %d:\n", f + 1);
        Matrix* result = convolve(inputMatrix, currentFilter, stride);
        
        if (result) {
            printMatrix(result);
            freeMatrix(result);
        }

        freeMatrix(currentFilter);
    }

    freeMatrix(inputMatrix);
    freeMatrix(filtersMatrix);

    return EXIT_SUCCESS;
}