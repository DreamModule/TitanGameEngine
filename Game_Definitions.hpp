#ifndef GAME_DEFINITIONS_HPP
#define GAME_DEFINITIONS_HPP
#include "Titan_Core.hpp"
namespace Game::Events {
    static const Titan::u32 START_MATCH = Titan::Data::Hash::FNV1a_32("START");
    static const Titan::u32 EXIT_GAME = Titan::Data::Hash::FNV1a_32("EXIT");
}
#endif
