#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"

typedef union Rand64 Rand64;

union Rand64 {
    uint64_t value;
    uint8_t values[8];
};

typedef struct BiomeSequenceData BiomeSequenceData;

struct BiomeSequenceData {
    Generator* g;
    int x;
    int z;
    int radius;
    int biome;
    int biomeCalculated;
};

enum SetRelation {
    EQUAL, SUBSET, SUPERSET, INTERSECT
};

uint64_t rand64(void);

int biomeSequence(int, void*);

int setBySequenceIndicatorFunction(int, int(*)(int, void*), void*, int, int[], int*[], int);

int indexOf(int, int, int[]);

char* getLine(char[], int, FILE*);

int main(int argc, char* argv[]) {
    printf("SpawnSeedSearcher\n");
    int n = 1024;
    char line1[n];
    char line2[n];
    char* mcString = line1;

    if (argc >= 2) {
        mcString = argv[1];
    } else {
        printf("Enter Minecraft version id or press enter to use the latest Minecraft version id: ");
        getLine(mcString, n, stdin);
    }

    int mc = MC_NEWEST;

    if (mcString[0] != '\0' && (sscanf(mcString, "%d", &mc) != 1 || mc < 0 || mc > MC_NEWEST)) {
        printf("Invalid Minecraft version id '%s'.\n", mcString);
        getchar();
        return 1;
    }

    char* xString = line1;

    if (argc >= 4) {
        xString = argv[2];
    } else {
        printf("Enter x coordinate or press enter to use the spawn x coordinate: ");
        getLine(xString, n, stdin);
    }

    int xSpawn = xString[0] == '\0';
    int x;

    if (!xSpawn && sscanf(xString, "%d", &x) != 1) {
        printf("Invalid x coordinate '%s'.\n", xString);
        getchar();
        return 1;
    }

    char* zString = line1;

    if (argc >= 4) {
        zString = argv[3];
    } else {
        printf("Enter z coordinate or press enter to use the spawn z coordinate: ");
        getLine(zString, n, stdin);
    }

    int zSpawn = zString[0] == '\0';
    int z;

    if (!zSpawn && sscanf(zString, "%d", &z) != 1) {
        printf("Invalid z coordinate '%s'.\n", zString);
        getchar();
        return 1;
    }

    char* radiusString = line1;

    if (argc >= 5) {
        radiusString = argv[4];
    } else {
        printf("Enter radius or press enter to use no radius: ");
        getLine(radiusString, n, stdin);
    }

    int radius = 0;

    if (radiusString[0] != '\0' && (sscanf(radiusString, "%d", &radius) != 1 || radius < 0)) {
        printf("Invalid radius '%s'.\n", radiusString);
        getchar();
        return 1;
    }

    char* setRelationString = line1;

    if (argc >= 6) {
        setRelationString = argv[5];
    } else {
        printf("Enter set relation id or press enter to use equal: ");
        getLine(setRelationString, n, stdin);
    }

    int setRelation = EQUAL;

    if (setRelationString[0] != '\0' && (sscanf(setRelationString, "%d", &setRelation) != 1 || setRelation < 0 || setRelation > 3)) {
        printf("Invalid set relation id '%s'.\n", setRelationString);
        getchar();
        return 1;
    }

    char* biomesString = line1;

    if (argc >= 7) {
        biomesString = argv[6];
    } else {
        printf("Enter biome id sets separated by '|' with biome ids separated by ',' or press enter to use no biome id sets: ");
        getLine(biomesString, n, stdin);
    }

    int biomeSetCount = 0;

    if (biomesString[0] != '\0') {
        biomeSetCount = 1;

        for (int i = 0; biomesString[i] != '\0'; i++) {
            if (biomesString[i] == '|') {
                biomeSetCount++;
            }
        }
    }

    int biomeCounts[biomeSetCount];
    int* biomeSets[biomeSetCount];

    for (int i = 0, j = 0; i < biomeSetCount; i++) {
        int biomeCount = 0;

        if (biomesString[j] != '|' && biomesString[j] != '\0') {
            biomeCount = 1;

            for (int k = j; biomesString[k] != '|' && biomesString[k] != '\0'; k++) {
                if (biomesString[k] == ',') {
                    biomeCount++;
                }
            }
        }

        int* biomes = malloc(biomeCount * sizeof(int));

        for (int k = 0; k < biomeCount; k++) {
            int l = 0;

            for (; biomesString[j] != '|' && biomesString[j] != '\0' && biomesString[j] != ','; j++, l++) {
                line2[l] = biomesString[j];
            }

            line2[l] = '\0';
            j++;
            int biome;

            if (sscanf(line2, "%d", &biome) != 1) {
                printf("Invalid biome id '%s'.\n", line2);
                getchar();
                return 1;
            }

            if (indexOf(biome, k, biomes) != -1) {
                printf("Duplicate biome id '%s'.\n", line2);
                getchar();
                return 1;
            }

            biomes[k] = biome;
        }

        if (biomeCount == 0) {
            j++;
        }

        biomeCounts[i] = biomeCount;
        biomeSets[i] = biomes;
    }

    char* spawnString = line1;

    if (argc >= 8) {
        spawnString = argv[7];
    } else {
        printf("Enter 'y' to check if the spawn is at the given coordinates or press enter: ");
        getLine(spawnString, n, stdin);
    }

    int spawn = 0;

    if (spawnString[0] != '\0' && !(spawn = spawnString[0] == 'y' && spawnString[1] == '\0')) {
        printf("Invalid option '%s'.\n", spawnString);
        getchar();
        return 1;
    }

    char* dimString = line1;

    if (argc >= 9) {
        dimString = argv[8];
    } else {
        printf("Enter dimension id or press enter to use the default dimension id: ");
        getLine(dimString, n, stdin);
    }

    int dim = 0;

    if (dimString[0] != '\0' && (sscanf(dimString, "%d", &dim) != 1 || dim < -1 || dim > 1)) {
        printf("Invalid dimension id '%s'.\n", dimString);
        getchar();
        return 1;
    }

    char* flagsString = line1;

    if (argc >= 10) {
        flagsString = argv[9];
    } else {
        printf("Enter flags or press enter to use the default flags: ");
        getLine(flagsString, n, stdin);
    }

    uint32_t flags = 0;

    if (flagsString[0] != '\0' && sscanf(flagsString, "%" SCNu32, &flags) != 1) {
        printf("Invalid flags '%s'.\n", flagsString);
        getchar();
        return 1;
    }

    srand(time(NULL));
    Generator g;
    setupGenerator(&g, mc, flags);
    long long counter = 0LL;

    while (1) {
        uint64_t seed = rand64();
        applySeed(&g, dim, seed);
        Pos pos;
        int posCalculated = 0;
        int biome = -1;
        int biomeCalculated = 0;
        int biomesCheck = biomeSetCount == 0;

        if (!biomesCheck) {
            if (xSpawn || zSpawn) {
                pos = getSpawn(&g);
                posCalculated = 1;

                if (xSpawn) {
                    x = pos.x;
                }

                if (zSpawn) {
                    z = pos.z;
                }
            }

            BiomeSequenceData biomeSequenceData;
            biomeSequenceData.g = &g;
            biomeSequenceData.x = x;
            biomeSequenceData.z = z;
            biomeSequenceData.radius = radius;
            biomeSequenceData.biome = biome;
            biomeSequenceData.biomeCalculated = biomeCalculated;
            biomesCheck = setBySequenceIndicatorFunction((2 * radius + 1) * (2 * radius + 1), biomeSequence, &biomeSequenceData, biomeSetCount, biomeCounts, biomeSets, setRelation);
            biome = biomeSequenceData.biome;
            biomeCalculated = biomeSequenceData.biomeCalculated;
        }

        if (biomesCheck) {
            int spawnCheck = !spawn;

            if (!spawnCheck) {
                if (!posCalculated) {
                    pos = getSpawn(&g);
                    posCalculated = 1;

                    if (xSpawn) {
                        x = pos.x;
                    }

                    if (zSpawn) {
                        z = pos.z;
                    }
                }

                spawnCheck = pos.x == x && pos.z == z;
            }

            if (spawnCheck) {
                if (!posCalculated) {
                    pos = getSpawn(&g);

                    if (xSpawn) {
                        x = pos.x;
                    }

                    if (zSpawn) {
                        z = pos.z;
                    }
                }

                if (!biomeCalculated) {
                    biome = getBiomeAt(&g, 1, x, 64, z);
                }

                int64_t result = seed;
                printf("%" PRId64 " (biome id: %d, spawn x: %d, spawn z: %d, rejected: %lld)\n", result, biome, pos.x, pos.z, counter);
                counter = 0LL;
            } else {
                counter++;
            }
        } else {
            counter++;
        }
    }

    getchar();
    return 0;
}

uint64_t rand64(void) {
    Rand64 result;
    result.values[0] = rand();
    result.values[1] = rand();
    result.values[2] = rand();
    result.values[3] = rand();
    result.values[4] = rand();
    result.values[5] = rand();
    result.values[6] = rand();
    result.values[7] = rand();
    return result.value;
}

int biomeSequence(int index, void* currying) {
    BiomeSequenceData* biomeSequenceData = currying;

    if (biomeSequenceData->radius < 0) {
        return -1;
    }

    if (index == 0) {
        int biome = getBiomeAt(biomeSequenceData->g, 1, biomeSequenceData->x, 64, biomeSequenceData->z);
        biomeSequenceData->biome = biome;
        biomeSequenceData->biomeCalculated = 1;
        return biome;
    }

    if (biomeSequenceData->radius == 0) {
        return -1;
    }

    if (index == 1) {
        return getBiomeAt(biomeSequenceData->g, 1, biomeSequenceData->x - biomeSequenceData->radius, 64, biomeSequenceData->z - biomeSequenceData->radius);
    }

    if (index == 2) {
        return getBiomeAt(biomeSequenceData->g, 1, biomeSequenceData->x - biomeSequenceData->radius, 64, biomeSequenceData->z + biomeSequenceData->radius);
    }

    if (index == 3) {
        return getBiomeAt(biomeSequenceData->g, 1, biomeSequenceData->x + biomeSequenceData->radius, 64, biomeSequenceData->z - biomeSequenceData->radius);
    }

    if (index == 4) {
        return getBiomeAt(biomeSequenceData->g, 1, biomeSequenceData->x + biomeSequenceData->radius, 64, biomeSequenceData->z + biomeSequenceData->radius);
    }

    index -= 5;

    if (index >= 0) {
        index++;

        if (index >= 2 * biomeSequenceData->radius) {
            index++;

            if (index >= 2 * (biomeSequenceData->radius * biomeSequenceData->radius + biomeSequenceData->radius)) {
                index++;

                if (index >= 2 * (2 * biomeSequenceData->radius * biomeSequenceData->radius + biomeSequenceData->radius)) {
                    index++;

                    if (index >= 4 * (biomeSequenceData->radius * biomeSequenceData->radius + biomeSequenceData->radius)) {
                        return -1;
                    }
                }
            }
        }
    } else {
        return -1;
    }

    int i = index / (2 * biomeSequenceData->radius + 1);
    int j = index - i * (2 * biomeSequenceData->radius + 1);
    return getBiomeAt(biomeSequenceData->g, 1, biomeSequenceData->x - biomeSequenceData->radius + i, 64, biomeSequenceData->z - biomeSequenceData->radius + j);
}

int setBySequenceIndicatorFunction(int sequenceLength, int(* sequence)(int, void*), void* currying, int setCount, int elementCounts[], int* sets[], int setRelation) {
    if (setRelation == EQUAL) {
        if (sequenceLength <= 0) {
            // Start optimization.
            if (setCount <= 0) {
                return 0;
            }
            // End optimization.

            // Start optimization.
            for (int i = 0; i < setCount; i++) {
                if (elementCounts[i] <= 0) {
                    return 1;
                }
            }
            // End optimization.
        } else {
            // Start optimization.
            int empty = 1;

            for (int i = 0; i < setCount; i++) {
                if (elementCounts[i] > 0) {
                    empty = 0;
                    break;
                }
            }

            if (empty) {
                return 0;
            }
            // End optimization.
        }

        int subset[setCount];

        for (int i = 0; i < setCount; i++) {
            subset[i] = 1;
        }

        int* superset[setCount];

        for (int i = 0; i < setCount; i++) {
            superset[i] = calloc(elementCounts[i], sizeof(int));
        }

        for (int i = 0; i < sequenceLength; i++) {
            int element = sequence(i, currying);

            for (int j = 0; j < setCount; j++) {
                if (subset[j]) {
                    int index = indexOf(element, elementCounts[j], sets[j]);

                    if (index == -1) {
                        subset[j] = 0;

                        if (indexOf(1, setCount, subset) == -1) {
                            for (int k = 0; k < setCount; k++) {
                                free(superset[k]);
                            }

                            return 0;
                        }
                    }

                    superset[j][index] = 1;
                }
            }
        }

        for (int i = 0; i < setCount; i++) {
            if (subset[i] && indexOf(0, elementCounts[i], superset[i]) == -1) {
                for (int j = 0; j < setCount; j++) {
                    free(superset[j]);
                }

                return 1;
            }
        }

        for (int i = 0; i < setCount; i++) {
            free(superset[i]);
        }

        return 0;
    } else if (setRelation == SUBSET) {
        if (setCount <= 0) {
            return 0;
        }

        // Start optimization.
        if (sequenceLength <= 0) {
            return 1;
        }

        // Start optimization.
        int empty = 1;

        for (int i = 0; i < setCount; i++) {
            if (elementCounts[i] > 0) {
                empty = 0;
                break;
            }
        }

        if (empty) {
            return 0;
        }
        // End optimization.
        // End optimization.

        int subset[setCount];

        for (int i = 0; i < setCount; i++) {
            subset[i] = 1;
        }

        for (int i = 0; i < sequenceLength; i++) {
            int element = sequence(i, currying);

            for (int j = 0; j < setCount; j++) {
                if (subset[j] && indexOf(element, elementCounts[j], sets[j]) == -1) {
                    subset[j] = 0;

                    if (indexOf(1, setCount, subset) == -1) {
                        return 0;
                    }
                }
            }
        }

        return 1;
    } else if (setRelation == SUPERSET) {
        // Start optimization.
        if (setCount <= 0) {
            return 0;
        }
        // End optimization.

        for (int i = 0; i < setCount; i++) {
            if (elementCounts[i] <= 0) {
                return 1;
            }
        }

        // Start optimization.
        if (sequenceLength <= 0) {
            return 0;
        }
        // End optimization.

        int* superset[setCount];

        for (int i = 0; i < setCount; i++) {
            superset[i] = calloc(elementCounts[i], sizeof(int));
        }

        for (int i = 0; i < sequenceLength; i++) {
            int element = sequence(i, currying);

            for (int j = 0; j < setCount; j++) {
                int index = indexOf(element, elementCounts[j], sets[j]);

                if (index != -1 && !superset[j][index]) {
                    superset[j][index] = 1;

                    if (indexOf(0, elementCounts[j], superset[j]) == -1) {
                        for (int k = 0; k < setCount; k++) {
                            free(superset[k]);
                        }

                        return 1;
                    }
                }
            }
        }

        for (int i = 0; i < setCount; i++) {
            free(superset[i]);
        }

        return 0;
    } else if (setRelation == INTERSECT) {
        // Start optimization.
        // Start optimization.
        if (sequenceLength <= 0) {
            return 0;
        }
        // End optimization.

        int empty = 1;

        for (int i = 0; i < setCount; i++) {
            if (elementCounts[i] > 0) {
                empty = 0;
                break;
            }
        }

        if (empty) {
            return 0;
        }
        // End optimization.

        for (int i = 0; i < sequenceLength; i++) {
            int element = sequence(i, currying);

            for (int j = 0; j < setCount; j++) {
                if (indexOf(element, elementCounts[j], sets[j]) != -1) {
                    return 1;
                }
            }
        }

        return 0;
    }

    return -1;
}

int indexOf(int value, int arrayLength, int array[]) {
    for (int i = 0; i < arrayLength; i++) {
        if (array[i] == value) {
            return i;
        }
    }

    return -1;
}

char* getLine(char line[], int lineLength, FILE* stream) {
    line[0] = '\0'; // The contents of the array are not altered on failure.
    char* result = fgets(line, lineLength, stream);
    line[lineLength - 1] = '\0'; // To be on the safe side. Actually this is handled by fgets.
    char* newLine = strchr(line, '\n');

    if (newLine == NULL) {
        int c;

        while ((c = fgetc(stream)) != '\n' && c != EOF) { // https://c-faq.com/stdio/stdinflush.html

        }

        return result;
    }

    *newLine = '\0';
    return result;
}
