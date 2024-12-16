#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to read two 2x2 matrices from a CSV file
void readFiltersFromCSV(const char* filename, int filters[2][2][2]) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int filterIndex = 0, rowIndex = 0;

    // Read the CSV line by line
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ",");
        for (int colIndex = 0; colIndex < 2 && token; colIndex++) {
            filters[filterIndex][rowIndex][colIndex] = atoi(token);
            token = strtok(NULL, ",");
        }
        rowIndex++;
        if (rowIndex == 2) {
            rowIndex = 0;
            filterIndex++;
        }
        if (filterIndex == 2) break; // Stop after two matrices
    }

    fclose(file);
}

// Main function
int main() {
    const char* filtersFile = "filters.csv";
    int filters[2][2][2]; // Fixed size for two 2x2 matrices

    readFiltersFromCSV(filtersFile, filters);

    // Print the matrices
    for (int i = 0; i < 2; i++) {
        printf("Filter %d:\n", i + 1);
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                printf("%d ", filters[i][j][k]);
            }
            printf("\n");
        }
        printf("\n");
    }

    return 0;
}
