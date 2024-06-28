#ifndef VAR_H
#define VAR_H
#include <string>
#include <vector>
#include <map>

enum class Biomes {
TropicalRainforest,
TemperateDeciduousForest,
Taiga,
Savanna,
Prairie,
HotDesert,
ColdDesert,
ArcticTundra,
AlpineTundra,
Mediterranean,
Freshwater,
Rivers,
Wetlands,
Oceans,
CoralReefs,
Estuaries,
Mangroves,
};

struct Block {
    long x;
    long y;
    Biomes biomes;
};

// Struct to hold player information
struct Player {
    std::string username;
    std::string uuid;
    uint_fast8_t health;
    uint_fast8_t strength;
    uint_fast32_t money;
    std::map<std::string, uint_fast16_t> inventory;
    Block location;
};
#endif