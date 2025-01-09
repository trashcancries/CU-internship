#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int rows;
    int cols;
    float** data;
} Matrix;

Matrix* createMatrix(int rows, int cols) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (!matrix) {
        fprintf(stderr, "Memory allocation failed for matrix structure\n");
        exit(EXIT_FAILURE);
    }

    matrix->rows = rows;
    matrix->cols = cols;

    matrix->data = (float**)malloc(rows * sizeof(float*));
    if (!matrix->data) {
        free(matrix);
        fprintf(stderr, "Memory allocation failed for matrix rows\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++) {
        matrix->data[i] = (float*)calloc(cols, sizeof(float));
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

    const int maxValues = 2000; 
    float* tempData = (float*)malloc(maxValues * sizeof(float));
    if (!tempData) {
        fprintf(stderr, "Memory allocation failed for temporary storage\n");
        fclose(file);
        return NULL;
    }

    int totalValues = 0;
    char line[8192]; 

    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ", \n");
        while (token) {
            if (totalValues < maxValues) {
                tempData[totalValues] = atof(token);
                totalValues++;
            } else {
                fprintf(stderr, "Exceeded maximum expected values\n");
                break;
            }
            token = strtok(NULL, ", \n");
        }
    }

    fclose(file);

    Matrix* matrix = createMatrix(1, totalValues); 
    for (int i = 0; i < totalValues; i++) {
        matrix->data[0][i] = tempData[i];
    }

    free(tempData);

    return matrix;
}

Matrix* readFiltersFromCSV(const char* filename) {
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

Matrix* readBiasesFromCSV(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    int rows = 0;
    char line[4096];
    
    while (fgets(line, sizeof(line), file)) {
        rows++;
    }
    rewind(file);

    Matrix* matrix = createMatrix(rows, 1);  
    for (int i = 0; i < rows; i++) {
        if (!fgets(line, sizeof(line), file)) break;
        matrix->data[i][0] = atof(line);  
    }

    fclose(file);
    return matrix;
}

float relu(float x) {
    return x > 0 ? x : 0;
}

float leakyRelu(float x) {
    float alpha = 0.1;
    return x > 0 ? x : alpha * x;
}

float selu(float x, float alpha, float scale) {
    return x > 0 ? scale * x : scale * alpha * (exp(x) - 1);
}

float elu(float x, float alpha) {
    return x > 0 ? x : alpha * (exp(x) - 1);
}

Matrix* convolve(Matrix* input, Matrix* filter, Matrix* bias, int stride) {
    int outputRows = ((input->rows - filter->rows) / stride) + 1;
    int outputCols = ((input->cols - filter->cols) / stride) + 1;

    if (outputRows <= 0 || outputCols <= 0) {
        fprintf(stderr, "Invalid convolution dimensions\n");
        return NULL;
    }

    Matrix* output = createMatrix(outputRows, outputCols);

    for (int i = 0; i < outputRows; i++) {
        for (int j = 0; j < outputCols; j++) {
            float sum = 0;
            for (int m = 0; m < filter->rows; m++) {
                for (int n = 0; n < filter->cols; n++) {
                    sum += input->data[i * stride + m][j * stride + n] * filter->data[m][n];
                }
            }
            sum += bias->data[0][0];
            sum = leakyRelu(sum);  
            output->data[i][j] = sum;
        }
    }

    return output;
}

Matrix* maxPool(Matrix* input, int poolRows, int poolCols, int stride) {
    int outputRows = ((input->rows - poolRows) / stride) + 1;
    int outputCols = ((input->cols - poolCols) / stride) + 1;

    if (outputRows <= 0 || outputCols <= 0) {
        fprintf(stderr, "Invalid pooling dimensions\n");
        return NULL;
    }

    Matrix* output = createMatrix(outputRows, outputCols);

    for (int i = 0; i < outputRows; i++) {
        for (int j = 0; j < outputCols; j++) {
            float maxVal = -INFINITY;
            for (int m = 0; m < poolRows; m++) {
                for (int n = 0; n < poolCols; n++) {
                    int rowIndex = i * stride + m;
                    int colIndex = j * stride + n;
                    if (rowIndex < input->rows && colIndex < input->cols) {
                        if (input->data[rowIndex][colIndex] > maxVal) {
                            maxVal = input->data[rowIndex][colIndex];
                        }
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
            printf("%f ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

Matrix* secondLayerConvolutionAndPooling(Matrix* input, Matrix* filtersMatrix, Matrix* biasesMatrix, int stride, int poolRows, int poolCols, int poolStride) {
    int numFilters = filtersMatrix->rows;
    Matrix* finalResult = NULL;
    
    printf("\n=== Starting Second Layer Processing ===\n");
    printf("Number of filters: %d\n", numFilters);
    printf("Input matrix dimensions: %dx%d\n", input->rows, input->cols);
    
    // Process each filter
    for (int f = 0; f < numFilters; f++) {
        printf("\nProcessing filter %d...\n", f + 1);
        
        // Extract current filter
        Matrix* currentFilter = createMatrix(1, filtersMatrix->cols);
        printf("Filter dimensions: %dx%d\n", currentFilter->rows, currentFilter->cols);
        
        for (int j = 0; j < filtersMatrix->cols; j++) {
            currentFilter->data[0][j] = filtersMatrix->data[f][j];
        }

        // Create bias matrix
        Matrix* currentBias = createMatrix(1, 1);
        currentBias->data[0][0] = biasesMatrix->data[f][0];
        printf("Using bias value: %f\n", currentBias->data[0][0]);

        // Apply convolution
        Matrix* convResult = convolve(input, currentFilter, currentBias, stride);
        if (convResult) {
            printf("\nConvolution Result for Second Layer Filter %d:\n", f + 1);
            printf("Convolution output dimensions: %dx%d\n", convResult->rows, convResult->cols);
            printMatrix(convResult);

            // Apply max pooling
            Matrix* poolResult = maxPool(convResult, poolRows, poolCols, poolStride);
            if (poolResult) {
                printf("\nMax Pooling Result for Second Layer Filter %d:\n", f + 1);
                printf("Pooling output dimensions: %dx%d\n", poolResult->rows, poolResult->cols);
                printMatrix(poolResult);
                
                // Store all results instead of just the first one
                if (f == 0) {
                    finalResult = poolResult;
                } else {
                    // Here we might want to combine results - for now just keep the last one
                    if (finalResult) {
                        freeMatrix(finalResult);
                    }
                    finalResult = poolResult;
                }
            } else {
                printf("Max pooling failed for filter %d\n", f + 1);
            }
            freeMatrix(convResult);
        } else {
            printf("Convolution failed for filter %d\n", f + 1);
        }

        freeMatrix(currentBias);
        freeMatrix(currentFilter);
    }

    printf("\n=== Second Layer Processing Complete ===\n");
    return finalResult;
}

Matrix* thirdLayerConvolutionAndPooling(Matrix* input, Matrix* filtersMatrix, Matrix* biasesMatrix, int stride, int poolRows, int poolCols, int poolStride) {
    int numFilters = filtersMatrix->rows;
    Matrix* finalResult = NULL;
    
    printf("\n=== Starting Third Layer Processing ===\n");
    printf("Number of filters: %d\n", numFilters);
    printf("Input matrix dimensions: %dx%d\n", input->rows, input->cols);
    
    // Process each filter
    for (int f = 0; f < numFilters; f++) {
        printf("\nProcessing filter %d...\n", f + 1);
        
        // Extract current filter
        Matrix* currentFilter = createMatrix(1, filtersMatrix->cols);
        printf("Filter dimensions: %dx%d\n", currentFilter->rows, currentFilter->cols);
        
        for (int j = 0; j < filtersMatrix->cols; j++) {
            currentFilter->data[0][j] = filtersMatrix->data[f][j];
        }

        // Create bias matrix
        Matrix* currentBias = createMatrix(1, 1);
        currentBias->data[0][0] = biasesMatrix->data[f][0];
        printf("Using bias value: %f\n", currentBias->data[0][0]);

        // Apply convolution
        Matrix* convResult = convolve(input, currentFilter, currentBias, stride);
        if (convResult) {
            printf("\nConvolution Result for Third Layer Filter %d:\n", f + 1);
            printf("Convolution output dimensions: %dx%d\n", convResult->rows, convResult->cols);
            printMatrix(convResult);

            // Apply max pooling
            Matrix* poolResult = maxPool(convResult, poolRows, poolCols, poolStride);
            if (poolResult) {
                printf("\nMax Pooling Result for Third Layer Filter %d:\n", f + 1);
                printf("Pooling output dimensions: %dx%d\n", poolResult->rows, poolResult->cols);
                printMatrix(poolResult);
                
                if (f == 0) {
                    finalResult = poolResult;
                } else {
                    if (finalResult) {
                        freeMatrix(finalResult);
                    }
                    finalResult = poolResult;
                }
            } else {
                printf("Max pooling failed for filter %d\n", f + 1);
            }
            freeMatrix(convResult);
        } else {
            printf("Convolution failed for filter %d\n", f + 1);
        }

        freeMatrix(currentBias);
        freeMatrix(currentFilter);
    }

    printf("\n=== Third Layer Processing Complete ===\n");
    return finalResult;
}

// ... [All previous functions remain the same until main()]

int main() {
    const char* inputFile = "test.csv";
    const char* filtersFile = "CNN_layer_1_filter_weights.csv";
    const char* biasesFile = "CNN_layer_1_filter_bias.csv";  
    const char* secondLayerFiltersFile = "CNN_layer_2_filter_weights.csv";
    const char* secondLayerBiasesFile = "CNN_layer_2_filter_bias.csv";
    const char* thirdLayerFiltersFile = "CNN_layer_3_filter_weights.csv";
    const char* thirdLayerBiasesFile = "CNN_layer_3_filter_bias.csv";

    Matrix* inputMatrix = readMatrixFromCSV(inputFile);
    if (!inputMatrix) {
        fprintf(stderr, "Failed to read input matrix\n");
        return EXIT_FAILURE;
    }

    printf("\nInput Matrix:\n");
    printMatrix(inputMatrix);

    // Read all layer filters and biases
    Matrix* filtersMatrix = readFiltersFromCSV(filtersFile);
    Matrix* biasesMatrix = readBiasesFromCSV(biasesFile);
    Matrix* secondLayerFiltersMatrix = readFiltersFromCSV(secondLayerFiltersFile);
    Matrix* secondLayerBiasesMatrix = readBiasesFromCSV(secondLayerBiasesFile);
    Matrix* thirdLayerFiltersMatrix = readFiltersFromCSV(thirdLayerFiltersFile);
    Matrix* thirdLayerBiasesMatrix = readBiasesFromCSV(thirdLayerBiasesFile);

    // Check all matrices were loaded successfully
    if (!filtersMatrix || !biasesMatrix || !secondLayerFiltersMatrix || 
        !secondLayerBiasesMatrix || !thirdLayerFiltersMatrix || !thirdLayerBiasesMatrix) {
        fprintf(stderr, "Failed to read one or more filter or bias matrices\n");
        if (inputMatrix) freeMatrix(inputMatrix);
        if (filtersMatrix) freeMatrix(filtersMatrix);
        if (biasesMatrix) freeMatrix(biasesMatrix);
        if (secondLayerFiltersMatrix) freeMatrix(secondLayerFiltersMatrix);
        if (secondLayerBiasesMatrix) freeMatrix(secondLayerBiasesMatrix);
        if (thirdLayerFiltersMatrix) freeMatrix(thirdLayerFiltersMatrix);
        if (thirdLayerBiasesMatrix) freeMatrix(thirdLayerBiasesMatrix);
        return EXIT_FAILURE;
    }

    // Print all matrices
    printf("\nFilters Matrix (First Layer):\n");
    printMatrix(filtersMatrix);
    printf("\nBiases Matrix (First Layer):\n");
    printMatrix(biasesMatrix);
    printf("\nSecond Layer Filters Matrix:\n");
    printMatrix(secondLayerFiltersMatrix);
    printf("\nSecond Layer Biases Matrix:\n");
    printMatrix(secondLayerBiasesMatrix);
    printf("\nThird Layer Filters Matrix:\n");
    printMatrix(thirdLayerFiltersMatrix);
    printf("\nThird Layer Biases Matrix:\n");
    printMatrix(thirdLayerBiasesMatrix);

    // Configuration parameters
    int stride = 2;
    int filterRows = 1;
    int filterCols = 10;
    int numFilters = filtersMatrix->rows / filterRows;
    int poolRows = 5;
    int poolCols = 1;
    int poolStride = 5;

    // Process first layer
    printf("\n=== Processing First Layer ===\n");
    for (int f = 0; f < numFilters; f++) {
        // Create current filter for first layer
        Matrix* currentFilter = createMatrix(filterRows, filterCols);
        for (int i = 0; i < filterRows; i++) {
            for (int j = 0; j < filterCols; j++) {
                currentFilter->data[i][j] = filtersMatrix->data[f * filterRows + i][j];
            }
        }

        // Create current bias for first layer
        Matrix* currentBias = createMatrix(1, 1);
        currentBias->data[0][0] = biasesMatrix->data[f][0];

        // First layer convolution
        printf("\nFirst Layer - Processing Filter %d:\n", f + 1);
        Matrix* firstLayerResult = convolve(inputMatrix, currentFilter, currentBias, stride);
        if (firstLayerResult) {
            printf("First Layer Convolution Output:\n");
            printMatrix(firstLayerResult);

            // First layer pooling
            Matrix* firstLayerPooled = maxPool(firstLayerResult, poolRows, poolCols, poolStride);
            if (firstLayerPooled) {
                printf("First Layer Pooling Output:\n");
                printMatrix(firstLayerPooled);

                // Second layer processing
                int numSecondLayerFilters = secondLayerFiltersMatrix->rows;
                for (int sf = 0; sf < numSecondLayerFilters; sf++) {
                    // Create second layer filter and bias
                    Matrix* secondFilter = createMatrix(1, secondLayerFiltersMatrix->cols);
                    for (int j = 0; j < secondLayerFiltersMatrix->cols; j++) {
                        secondFilter->data[0][j] = secondLayerFiltersMatrix->data[sf][j];
                    }
                    
                    Matrix* secondBias = createMatrix(1, 1);
                    secondBias->data[0][0] = secondLayerBiasesMatrix->data[sf][0];

                    // Second layer convolution
                    printf("\nSecond Layer - Processing Filter Chain %d-%d:\n", f + 1, sf + 1);
                    Matrix* secondLayerResult = convolve(firstLayerPooled, secondFilter, secondBias, stride);
                    if (secondLayerResult) {
                        printf("Second Layer Convolution Output:\n");
                        printMatrix(secondLayerResult);

                        // Second layer pooling
                        Matrix* secondLayerPooled = maxPool(secondLayerResult, poolRows, poolCols, poolStride);
                        if (secondLayerPooled) {
                            printf("Second Layer Pooling Output:\n");
                            printMatrix(secondLayerPooled);

                            // Third layer processing
                            int numThirdLayerFilters = thirdLayerFiltersMatrix->rows;
                            for (int tf = 0; tf < numThirdLayerFilters; tf++) {
                                // Create third layer filter and bias
                                Matrix* thirdFilter = createMatrix(1, thirdLayerFiltersMatrix->cols);
                                for (int j = 0; j < thirdLayerFiltersMatrix->cols; j++) {
                                    thirdFilter->data[0][j] = thirdLayerFiltersMatrix->data[tf][j];
                                }
                                
                                Matrix* thirdBias = createMatrix(1, 1);
                                thirdBias->data[0][0] = thirdLayerBiasesMatrix->data[tf][0];

                                // Third layer convolution
                                printf("\nThird Layer - Processing Filter Chain %d-%d-%d:\n", f + 1, sf + 1, tf + 1);
                                Matrix* thirdLayerResult = convolve(secondLayerPooled, thirdFilter, thirdBias, stride);
                                if (thirdLayerResult) {
                                    printf("Third Layer Convolution Output:\n");
                                    printMatrix(thirdLayerResult);

                                    // Third layer pooling
                                    Matrix* thirdLayerPooled = maxPool(thirdLayerResult, poolRows, poolCols, poolStride);
                                    if (thirdLayerPooled) {
                                        printf("Third Layer Final Output (Filter Chain %d-%d-%d):\n", f + 1, sf + 1, tf + 1);
                                        printMatrix(thirdLayerPooled);
                                        freeMatrix(thirdLayerPooled);
                                    }
                                    freeMatrix(thirdLayerResult);
                                }
                                freeMatrix(thirdFilter);
                                freeMatrix(thirdBias);
                            }
                            freeMatrix(secondLayerPooled);
                        }
                        freeMatrix(secondLayerResult);
                    }
                    freeMatrix(secondFilter);
                    freeMatrix(secondBias);
                }
                freeMatrix(firstLayerPooled);
            }
            freeMatrix(firstLayerResult);
        }
        freeMatrix(currentFilter);
        freeMatrix(currentBias);
    }

    // Free all matrices
    freeMatrix(inputMatrix);
    freeMatrix(filtersMatrix);
    freeMatrix(biasesMatrix);
    freeMatrix(secondLayerFiltersMatrix);
    freeMatrix(secondLayerBiasesMatrix);
    freeMatrix(thirdLayerFiltersMatrix);
    freeMatrix(thirdLayerBiasesMatrix);

    return EXIT_SUCCESS;
}