#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"

typedef union Random64 Random64;

union Random64 {
    uint64_t value;
    uint8_t values[8];
};

typedef struct BiomeSequenceData BiomeSequenceData;

struct BiomeSequenceData {
    Generator* generator;
    int x;
    int y;
    int z;
    int radius;
    int biomeAtXYZ;
    int biomeAtXYZCalculated;
};

enum SetRelation {
    EQUAL, SUBSET, SUPERSET, INTERSECT
};

uint64_t random64(void);

float getApproximateHeight(Generator*, int, int);

int floorToInt(float);

int biomeSequence(int, void*);

int setBySequenceIndicatorFunction(int, int(*)(int, void*), void*, int, int[], int*[], int);

int indexOf(int, int, int[]);

char* getLine(char[], int, FILE*);

int main(int argc, char* argv[]) {
    printf("SpawnSeedSearcher\n");
    int lineLength = 1024;
    char line1[lineLength];
    char line2[lineLength];
    char* mcString = line1;

    if (argc >= 2) {
        mcString = argv[1];
    } else {
        printf("Enter Minecraft version ID or press enter to use the latest Minecraft version ID: ");
        getLine(mcString, lineLength, stdin);
    }

    int mc = MC_NEWEST;

    if (mcString[0] != '\0' && (sscanf(mcString, "%d", &mc) != 1 || mc < 0 || mc > MC_NEWEST)) {
        printf("Invalid Minecraft version ID '%s'.\n", mcString);
        getchar();
        return 1;
    }

    char* flagsString = line1;

    if (argc >= 3) {
        flagsString = argv[2];
    } else {
        printf("Enter flags or press enter to use the default flags: ");
        getLine(flagsString, lineLength, stdin);
    }

    uint32_t flags = 0;

    if (flagsString[0] != '\0' && sscanf(flagsString, "%" SCNu32, &flags) != 1) {
        printf("Invalid flags '%s'.\n", flagsString);
        getchar();
        return 1;
    }

    char* dimString = line1;

    if (argc >= 4) {
        dimString = argv[3];
    } else {
        printf("Enter dimension ID or press enter to use the default dimension ID: ");
        getLine(dimString, lineLength, stdin);
    }

    int dim = 0;

    if (dimString[0] != '\0' && (sscanf(dimString, "%d", &dim) != 1 || dim < -1 || dim > 1)) {
        printf("Invalid dimension ID '%s'.\n", dimString);
        getchar();
        return 1;
    }

    char* seedsFileString = line1;

    if (argc >= 5) {
        seedsFileString = argv[4];
    } else {
        printf("Enter seeds file or press enter to use random seeds: ");
        getLine(seedsFileString, lineLength, stdin);
    }

    FILE* seedsFile = NULL;

    if (seedsFileString[0] != '\0' && (seedsFile = fopen(seedsFileString, "r")) == NULL) {
        printf("Invalid seeds file '%s'.\n", seedsFileString);
        getchar();
        return 1;
    }

    char* xString = line1;

    if (argc >= 6) {
        xString = argv[5];
    } else {
        printf("Enter x coordinate or press enter to use the spawn x coordinate: ");
        getLine(xString, lineLength, stdin);
    }

    int useSpawnX = xString[0] == '\0';
    int x;

    if (!useSpawnX && sscanf(xString, "%d", &x) != 1) {
        printf("Invalid x coordinate '%s'.\n", xString);
        getchar();
        return 1;
    }

    char* zString = line1;

    if (argc >= 7) {
        zString = argv[6];
    } else {
        printf("Enter z coordinate or press enter to use the spawn z coordinate: ");
        getLine(zString, lineLength, stdin);
    }

    int useSpawnZ = zString[0] == '\0';
    int z;

    if (!useSpawnZ && sscanf(zString, "%d", &z) != 1) {
        printf("Invalid z coordinate '%s'.\n", zString);
        getchar();
        return 1;
    }

    char* yString = line1;

    if (argc >= 8) {
        yString = argv[7];
    } else {
        printf("Enter y coordinate or press enter to use the approximate height at the given x and z coordinates: ");
        getLine(yString, lineLength, stdin);
    }

    int useHeightAtXZ = yString[0] == '\0';
    int y;

    if (!useHeightAtXZ && sscanf(yString, "%d", &y) != 1) {
        printf("Invalid y coordinate '%s'.\n", yString);
        getchar();
        return 1;
    }

    char* radiusString = line1;

    if (argc >= 9) {
        radiusString = argv[8];
    } else {
        printf("Enter radius or press enter to use no radius: ");
        getLine(radiusString, lineLength, stdin);
    }

    int radius = 0;

    if (radiusString[0] != '\0' && (sscanf(radiusString, "%d", &radius) != 1 || radius < 0)) {
        printf("Invalid radius '%s'.\n", radiusString);
        getchar();
        return 1;
    }

    char* setRelationString = line1;

    if (argc >= 10) {
        setRelationString = argv[9];
    } else {
        printf("Enter set relation ID or press enter to use equal: ");
        getLine(setRelationString, lineLength, stdin);
    }

    int setRelation = EQUAL;

    if (setRelationString[0] != '\0' && (sscanf(setRelationString, "%d", &setRelation) != 1 || setRelation < 0 || setRelation > 3)) {
        printf("Invalid set relation ID '%s'.\n", setRelationString);
        getchar();
        return 1;
    }

    char* biomesString = line1;

    if (argc >= 11) {
        biomesString = argv[10];
    } else {
        printf("Enter biome ID sets separated by '|' with biome IDs separated by ',' or press enter to use no biome ID sets: ");
        getLine(biomesString, lineLength, stdin);
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
                printf("Invalid biome ID '%s'.\n", line2);
                getchar();
                return 1;
            }

            if (indexOf(biome, k, biomes) != -1) {
                printf("Duplicate biome ID '%s'.\n", line2);
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

    char* checkSpawnXString = line1;

    if (argc >= 12) {
        checkSpawnXString = argv[11];
    } else {
        printf("Enter 'y' to check if the spawn x coordinate matches the given x coordinate or press enter: ");
        getLine(checkSpawnXString, lineLength, stdin);
    }

    int checkSpawnX = 0;

    if (checkSpawnXString[0] != '\0' && !(checkSpawnX = checkSpawnXString[0] == 'y' && checkSpawnXString[1] == '\0')) {
        printf("Invalid option '%s'.\n", checkSpawnXString);
        getchar();
        return 1;
    }

    char* checkSpawnZString = line1;

    if (argc >= 13) {
        checkSpawnZString = argv[12];
    } else {
        printf("Enter 'y' to check if the spawn z coordinate matches the given z coordinate or press enter: ");
        getLine(checkSpawnZString, lineLength, stdin);
    }

    int checkSpawnZ = 0;

    if (checkSpawnZString[0] != '\0' && !(checkSpawnZ = checkSpawnZString[0] == 'y' && checkSpawnZString[1] == '\0')) {
        printf("Invalid option '%s'.\n", checkSpawnZString);
        getchar();
        return 1;
    }

    char* checkHeightAtXZString = line1;

    if (argc >= 14) {
        checkHeightAtXZString = argv[13];
    } else {
        printf("Enter 'y' to check if the approximate height at the given x and z coordinates matches the given y coordinate or press enter: ");
        getLine(checkHeightAtXZString, lineLength, stdin);
    }

    int checkHeightAtXZ = 0;

    if (checkHeightAtXZString[0] != '\0' && !(checkHeightAtXZ = checkHeightAtXZString[0] == 'y' && checkHeightAtXZString[1] == '\0')) {
        printf("Invalid option '%s'.\n", checkHeightAtXZString);
        getchar();
        return 1;
    }

    srand(time(NULL));
    Generator generator;
    setupGenerator(&generator, mc, flags);
    long long counter = 0LL;

    while (seedsFile == NULL || getLine(line1, lineLength, seedsFile) != NULL) {
        uint64_t seed;

        if (seedsFile == NULL) {
            seed = random64();
        } else {
            int64_t result;

            if (sscanf(line1, "%" PRId64, &result) != 1) {
                printf("Invalid seed '%s' in seeds file '%s'.\n", line1, seedsFileString);
                fclose(seedsFile);
                getchar();
                return 1;
            }

            seed = result;
        }

        applySeed(&generator, dim, seed);
        Pos spawn;
        int spawnCalculated = 0;
        int heightAtXZ;
        int heightAtXZCalculated = 0;
        int biomeAtXYZ;
        int biomeAtXYZCalculated = 0;
        int biomesCheckResult;

        if (biomeSetCount != 0) {
            if (useSpawnX || useSpawnZ) {
                spawn = getSpawn(&generator);
                spawnCalculated = 1;

                if (useSpawnX) {
                    x = spawn.x;
                }

                if (useSpawnZ) {
                    z = spawn.z;
                }
            }

            if (useHeightAtXZ) {
                heightAtXZ = floorToInt(getApproximateHeight(&generator, x, z));
                heightAtXZCalculated = 1;
                y = heightAtXZ;
            }

            BiomeSequenceData biomeSequenceData;
            biomeSequenceData.generator = &generator;
            biomeSequenceData.x = x;
            biomeSequenceData.y = y;
            biomeSequenceData.z = z;
            biomeSequenceData.radius = radius;
            biomeSequenceData.biomeAtXYZCalculated = biomeAtXYZCalculated;
            biomesCheckResult = setBySequenceIndicatorFunction((2 * radius + 1) * (2 * radius + 1), biomeSequence, &biomeSequenceData, biomeSetCount, biomeCounts, biomeSets, setRelation);
            biomeAtXYZ = biomeSequenceData.biomeAtXYZ;
            biomeAtXYZCalculated = biomeSequenceData.biomeAtXYZCalculated;
        } else {
            biomesCheckResult = 1;
        }

        if (biomesCheckResult) {
            int spawnCheckResult;

            if (checkSpawnX || checkSpawnZ) {
                if (!spawnCalculated) {
                    spawn = getSpawn(&generator);
                    spawnCalculated = 1;

                    if (useSpawnX) {
                        x = spawn.x;
                    }

                    if (useSpawnZ) {
                        z = spawn.z;
                    }
                }

                spawnCheckResult = (!checkSpawnX || spawn.x == x) && (!checkSpawnZ || spawn.z == z);
            } else {
                spawnCheckResult = 1;
            }

            if (spawnCheckResult) {
                int heightCheckResult;

                if (checkHeightAtXZ) {
                    if ((useSpawnX || useSpawnZ) && !spawnCalculated) {
                        spawn = getSpawn(&generator);
                        spawnCalculated = 1;

                        if (useSpawnX) {
                            x = spawn.x;
                        }

                        if (useSpawnZ) {
                            z = spawn.z;
                        }
                    }

                    if (!heightAtXZCalculated) {
                        heightAtXZ = floorToInt(getApproximateHeight(&generator, x, z));
                        heightAtXZCalculated = 1;

                        if (useHeightAtXZ) {
                            y = heightAtXZ;
                        }
                    }

                    heightCheckResult = heightAtXZ == y;
                } else {
                    heightCheckResult = 1;
                }

                if (heightCheckResult) {
                    if ((dim == 0 || useSpawnX || useSpawnZ) && !spawnCalculated) {
                        spawn = getSpawn(&generator);
                        spawnCalculated = 1;

                        if (useSpawnX) {
                            x = spawn.x;
                        }

                        if (useSpawnZ) {
                            z = spawn.z;
                        }
                    }

                    if (useHeightAtXZ && !heightAtXZCalculated) {
                        heightAtXZ = floorToInt(getApproximateHeight(&generator, x, z));
                        heightAtXZCalculated = 1;
                        y = heightAtXZ;
                    }

                    if (!biomeAtXYZCalculated) {
                        biomeAtXYZ = getBiomeAt(&generator, 1, x, y, z);
                        biomeAtXYZCalculated = 1;
                    }

                    int heightAtSpawn;
                    int biomeAtSpawn;

                    if ((useSpawnX || checkSpawnX) && (useSpawnZ || checkSpawnZ) && heightAtXZCalculated) {
                        heightAtSpawn = heightAtXZ;

                        if (useHeightAtXZ || checkHeightAtXZ) {
                            biomeAtSpawn = biomeAtXYZ;
                        } else {
                            biomeAtSpawn = getBiomeAt(&generator, 1, spawn.x, heightAtSpawn, spawn.z);
                        }
                    } else if (spawnCalculated) {
                        heightAtSpawn = floorToInt(getApproximateHeight(&generator, spawn.x, spawn.z));
                        biomeAtSpawn = getBiomeAt(&generator, 1, spawn.x, heightAtSpawn, spawn.z);
                    }

                    int64_t result = seed;
                    printf("%" PRId64 " (x: %d, y: %d, z: %d, biome ID: %d, ", result, x, y, z, biomeAtXYZ);

                    if (spawnCalculated) {
                        printf("spawn x: %d, spawn z: %d, height at spawn: %d, biome ID at spawn: %d, ", spawn.x, spawn.z, heightAtSpawn, biomeAtSpawn);
                    } else {
                        printf("spawn x: -, spawn z: -, height at spawn: -, biome ID at spawn: -, ");
                    }

                    printf("rejected: %lld)\n", counter);
                    counter = 0LL;
                } else {
                    counter++;
                }
            } else {
                counter++;
            }
        } else {
            counter++;
        }
    }

    if (seedsFile != NULL) {
        fclose(seedsFile);
    }

    getchar();
    return 0;
}

uint64_t random64(void) {
    Random64 result;
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

float getApproximateHeight(Generator* generator, int x, int z) {
    float h;
    SurfaceNoise surfaceNoise;
    initSurfaceNoise(&surfaceNoise, generator->dim, generator->seed);
    mapApproxHeight(&h, NULL, generator, &surfaceNoise, x >> 2, z >> 2, 1, 1);
    return h;
}

int floorToInt(float value) {
    int i = value;
    return value < i ? i - 1 : i;
}

int biomeSequence(int index, void* currying) {
    BiomeSequenceData* biomeSequenceData = currying;

    if (biomeSequenceData->radius < 0) {
        return -1;
    }

    if (index == 0) {
        int biomeAtXYZ = getBiomeAt(biomeSequenceData->generator, 1, biomeSequenceData->x, biomeSequenceData->y, biomeSequenceData->z);
        biomeSequenceData->biomeAtXYZ = biomeAtXYZ;
        biomeSequenceData->biomeAtXYZCalculated = 1;
        return biomeAtXYZ;
    }

    if (biomeSequenceData->radius == 0) {
        return -1;
    }

    if (index == 1) {
        return getBiomeAt(biomeSequenceData->generator, 1, biomeSequenceData->x - biomeSequenceData->radius, biomeSequenceData->y, biomeSequenceData->z - biomeSequenceData->radius);
    }

    if (index == 2) {
        return getBiomeAt(biomeSequenceData->generator, 1, biomeSequenceData->x - biomeSequenceData->radius, biomeSequenceData->y, biomeSequenceData->z + biomeSequenceData->radius);
    }

    if (index == 3) {
        return getBiomeAt(biomeSequenceData->generator, 1, biomeSequenceData->x + biomeSequenceData->radius, biomeSequenceData->y, biomeSequenceData->z - biomeSequenceData->radius);
    }

    if (index == 4) {
        return getBiomeAt(biomeSequenceData->generator, 1, biomeSequenceData->x + biomeSequenceData->radius, biomeSequenceData->y, biomeSequenceData->z + biomeSequenceData->radius);
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
    return getBiomeAt(biomeSequenceData->generator, 1, biomeSequenceData->x - biomeSequenceData->radius + i, biomeSequenceData->y, biomeSequenceData->z - biomeSequenceData->radius + j);
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
