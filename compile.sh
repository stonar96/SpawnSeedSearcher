#!/bin/bash
cd "${0%/*}"
gcc -Wall -o SpawnSeedSearcher main.c cubiomes/biome_tree.c cubiomes/finders.c cubiomes/generator.c cubiomes/layers.c cubiomes/noise.c cubiomes/util.c -lm -pthread
