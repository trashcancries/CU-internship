#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int rows;
    int cols;
    double** data;
} Matrix;

Matrix* createMatrix(int rows, int cols) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (!matrix) {
        fprintf(stderr, "Memory allocation failed for matrix structure\n");
        exit(EXIT_FAILURE);
    }

    matrix->rows = rows;
    matrix->cols = cols;

    matrix->data = (double**)malloc(rows * sizeof(double*));
    if (!matrix->data) {
        free(matrix);
        fprintf(stderr, "Memory allocation failed for matrix rows\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++) {
        matrix->data[i] = (double*)calloc(cols, sizeof(double));
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
            matrix->data[i][j] = atof(token);
            token = strtok(NULL, ",");
        }
    }

    fclose(file);
    return matrix;
}
double relu(double x) {
    return (x > 0) ? x : 0;
}

double leakyRelu(double x) {
    return (x > 0) ? x : 0.01 * x;
}

double elu(double x, double alpha) {
    return (x > 0) ? x : alpha * (exp(x) - 1);
}

double selu(double x) {
    const double lambda = 1.0507;
    const double alpha = 1.67326;
    return lambda * ((x > 0) ? x : alpha * (exp(x) - 1));
}

double lu(double x) {
    return (x > 0) ? x : 0.05 * x;  
}

typedef double (*ActivationFunction)(double);

Matrix* convolve(Matrix* input, Matrix* filter, int stride, ActivationFunction activation) {
    int outputRows = ((input->rows - filter->rows) / stride) + 1;
    int outputCols = ((input->cols - filter->cols) / stride) + 1;

    if (outputRows <= 0 || outputCols <= 0) {
        fprintf(stderr, "Invalid convolution dimensions\n");
        return NULL;
    }

    Matrix* output = createMatrix(outputRows, outputCols);

    for (int i = 0; i < outputRows; i++) {
        for (int j = 0; j < outputCols; j++) {
            double sum = 0;
            for (int m = 0; m < filter->rows; m++) {
                for (int n = 0; n < filter->cols; n++) {
                    sum += input->data[i * stride + m][j * stride + n] * filter->data[m][n];
                }
            }

            output->data[i][j] = (sum > 0) ? sum : 0;
        }
    }

    return output;
}

Matrix* maxPooling(Matrix* input, int poolSize) {
    int outputRows = input->rows / poolSize;
    int outputCols = input->cols / poolSize;

    if (outputRows <= 0 || outputCols <= 0) {
        fprintf(stderr, "Invalid pooling dimensions\n");
        return NULL;
    }

    Matrix* output = createMatrix(outputRows, outputCols);

    for (int i = 0; i < outputRows; i++) {
        for (int j = 0; j < outputCols; j++) {
            double maxVal = input->data[i * poolSize][j * poolSize];
            for (int m = 0; m < poolSize; m++) {
                for (int n = 0; n < poolSize; n++) {
                    if (input->data[i * poolSize + m][j * poolSize + n] > maxVal) {
                        maxVal = input->data[i * poolSize + m][j * poolSize + n];
                    }
                }
            }
            output->data[i][j] = maxVal;
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
            printf("%.2f ", matrix->data[i][j]);
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
        fprintf(stderr, "Failed to read filters\n");
        freeMatrix(inputMatrix);
        return EXIT_FAILURE;
    }

    int stride = 1;
    int poolSize = 1;

    Matrix* currentInput = inputMatrix;
    int filterOffset = 0;

    for (int layer = 0; layer < 2; layer++) {
        int filterSize = 2;  
        int numFilters = 2; 

        Matrix* layerOutput = NULL;

        for (int f = 0; f < numFilters; f++) {
            Matrix* currentFilter = createMatrix(filterSize, filterSize);
            for (int i = 0; i < filterSize; i++) {
                for (int j = 0; j < filterSize; j++) {
                    currentFilter->data[i][j] = filtersMatrix->data[filterOffset + f * filterSize + i][j];
                }
            }

            printf("\nConvolution Result for Layer %d Filter %d:\n", layer + 1, f + 1);
            Matrix* result = convolve(currentInput, currentFilter, stride, relu);

            if (result) {
                printMatrix(result);

                printf("\nMax Pooling Result for Layer %d Filter %d:\n", layer + 1, f + 1);
                Matrix* pooledResult = maxPooling(result, poolSize);
                if (pooledResult) {
                    printMatrix(pooledResult);
                    if (!layerOutput) {
                        layerOutput = pooledResult;
                    } else {
                        freeMatrix(pooledResult);
                    }
                }

                freeMatrix(result);
            }

            freeMatrix(currentFilter);
        }

        freeMatrix(currentInput);
        currentInput = layerOutput;
        filterOffset += numFilters * filterSize;
    }

     freeMatrix(currentInput);
    freeMatrix(filtersMatrix);

    return EXIT_SUCCESS;
}
