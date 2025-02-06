#!/bin/bash
cd "${0%/*}"
gcc -Wall -o SpawnSeedSearcher main.c cubiomes/biomenoise.c cubiomes/biomes.c cubiomes/finders.c cubiomes/generator.c cubiomes/layers.c cubiomes/noise.c randombytes/randombytes.c -fwrapv -lm
