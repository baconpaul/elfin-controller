/*
 * Elfin Controller
 *
 * A small controller plugin for the Elfin 04 Polysynth
 *
 * Copyright 2024-2025, Paul Walker and Various authors, as described in the github
 * transaction log.
 *
 * This source repo is released under the MIT license
 *
 * The source code and license are at https://github.com/baconpaul/elfin-controller
 */

#ifndef ELFIN_CONTROLLER_CONFIGURATION_H
#define ELFIN_CONTROLLER_CONFIGURATION_H

#include <array>
namespace baconpaul::elfin_controller
{
struct ElfinDescription
{
    std::string streaming_name{};
    std::string name{};
    int32_t midiCC{-1};

    ElfinDescription() = default;
    ElfinDescription(const std::string &s, const std::string &n, int32_t m)
        : name(n), midiCC(m), streaming_name(s)
    {
    }
};

// This enum value doesn't stream. Its just for code readabiligy
enum ElfinControl
{
    OSC12_TYPE,
    OSC12_MIX,
    OSC2_COARSE,
    OSC2_FINE,
    FILT_CUTOFF,
    FILT_RESONANCE,
    numElfinControlTypes
};

static constexpr int nElfinParams = (int)ElfinControl::numElfinControlTypes;

static inline std::map<ElfinControl, ElfinDescription> elfinConfig{
    {OSC12_TYPE, ElfinDescription{"osc12_type", "OSC 1/2 Type", 24}},
    {OSC12_MIX, {"osc12_mix", "OSC 1/2 Mix", 25}},
    {OSC2_COARSE, {"osc2_coarse", "OSC2 Coarse", 20}},
    {OSC2_FINE, {"osc2_fine", "OSC2 Fine", 21}},

    {FILT_CUTOFF, {"filt_cutoff", "Filter Cutoff", 16}},
    {FILT_RESONANCE, {"filt_resonance", "Filter Resonance", 17}}};

#define ELFLOG(...) std::cout << __FILE__ << ":" << __LINE__ << " " << __VA_ARGS__ << std::endl;
};     // namespace baconpaul::elfin_controller
#endif // CONFIGURATION_H
