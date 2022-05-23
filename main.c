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

uint64_t rand64(void);

int checkBiome(const Generator*, int, int, int, int, int, int[], int*);

char* getLine(char*, int);

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
        getLine(mcString, n);
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
        getLine(xString, n);
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
        getLine(zString, n);
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
        getLine(radiusString, n);
    }

    int radius = 0;

    if (radiusString[0] != '\0' && (sscanf(radiusString, "%d", &radius) != 1 || radius < 0)) {
        printf("Invalid radius '%s'.\n", radiusString);
        getchar();
        return 1;
    }

    char* biomesString = line1;

    if (argc >= 6) {
        biomesString = argv[5];
    } else {
        printf("Enter biome ids separated by ',' or press enter to use all biome ids: ");
        getLine(biomesString, n);
    }

    int biomeCount = 0;

    if (biomesString[0] != '\0') {
        biomeCount = 1;

        for (int i = 0; biomesString[i] != '\0'; i++) {
            if (biomesString[i] == ',') {
                biomeCount++;
            }
        }
    }

    int biomes[biomeCount];

    for (int i = 0, j = 0; i < biomeCount; i++) {
        int k = 0;

        for (; biomesString[j] != '\0' && biomesString[j] != ','; j++, k++) {
            line2[k] = biomesString[j];
        }

        line2[k] = '\0';
        j++;
        int biome;

        if (sscanf(line2, "%d", &biome) != 1) {
            printf("Invalid biome id '%s'.\n", line2);
            getchar();
            return 1;
        }

        biomes[i] = biome;
    }

    char* spawnString = line1;

    if (argc >= 7) {
        spawnString = argv[6];
    } else {
        printf("Enter 'y' to check if the spawn is at the given coordinates or press enter: ");
        getLine(spawnString, n);
    }

    int spawn = 0;

    if (spawnString[0] != '\0' && !(spawn = spawnString[0] == 'y' && spawnString[1] == '\0')) {
        printf("Invalid option '%s'.\n", spawnString);
        getchar();
        return 1;
    }

    char* dimString = line1;

    if (argc >= 8) {
        dimString = argv[7];
    } else {
        printf("Enter dimension id or press enter to use the default dimension id: ");
        getLine(dimString, n);
    }

    int dim = 0;

    if (dimString[0] != '\0' && (sscanf(dimString, "%d", &dim) != 1 || dim < -1 || dim > 1)) {
        printf("Invalid dimension id '%s'.\n", dimString);
        getchar();
        return 1;
    }

    char* flagsString = line1;

    if (argc >= 9) {
        flagsString = argv[8];
    } else {
        printf("Enter flags or press enter to use the default flags: ");
        getLine(flagsString, n);
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
        int biome;
        int biomeCalculated = 0;
        int biomesCheck = biomeCount == 0;

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

            if (checkBiome(&g, 1, x, 64, z, biomeCount, biomes, &biome)) {
                if (radius == 0) {
                    biomesCheck = 1;
                } else if (checkBiome(&g, 1, x - radius, 64, z - radius, biomeCount, biomes, NULL) && checkBiome(&g, 1, x - radius, 64, z + radius, biomeCount, biomes, NULL) && checkBiome(&g, 1, x + radius, 64, z - radius, biomeCount, biomes, NULL) && checkBiome(&g, 1, x + radius, 64, z + radius, biomeCount, biomes, NULL)) {
                    biomesCheck = 1;

                    for (int i = -radius; i <= radius; i++) {
                        for (int j = -radius; j <= radius; j++) {
                            if (i == 0 && j == 0 || (i == -radius || i == radius) && (j == -radius || j == radius)) {
                                continue;
                            }

                            if (!checkBiome(&g, 1, x + i, 64, z + j, biomeCount, biomes, NULL)) {
                                biomesCheck = 0;
                                break;
                            }
                        }

                        if (!biomesCheck) {
                            break;
                        }
                    }
                }
            }

            biomeCalculated = 1;
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

int checkBiome(const Generator* g, int scale, int x, int y, int z, int biomeCount, int biomes[], int* biome) {
    int b = getBiomeAt(g, scale, x, y, z);

    if (biome != NULL) {
        *biome = b;
    }

    for (int i = 0; i < biomeCount; i++) {
        if (biomes[i] == b) {
            return 1;
        }
    }

    return 0;
}

char* getLine(char* line, int n) {
    char* result = fgets(line, n, stdin);
    char* offset = strstr(line, "\n");

    if (offset != NULL) {
        *offset = '\0';
    }

    fflush(stdin);
    return result;
}
