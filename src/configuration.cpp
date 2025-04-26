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

#include "configuration.h"

namespace baconpaul::elfin_controller
{
std::map<ElfinControl, ElfinDescription> elfinConfig;
void setupConfiguration()
{
    elfinConfig = std::map<ElfinControl, ElfinDescription>{
        {OSC12_TYPE, ElfinDescription{"osc12_type", "OSC 1/2 Type", "Type", 24}},
        {OSC12_MIX, {"osc12_mix", "OSC 1/2 Mix", "1/2 Mix", 25}},
        {OSC2_COARSE, {"osc2_coarse", "OSC2 Coarse", "2 Coarse", 20}},
        {OSC2_FINE, {"osc2_fine", "OSC2 Fine", "2 Fine", 21}},

        {FILT_CUTOFF, {"filt_cutoff", "Filter Cutoff", "Cut", 16}},
        {FILT_RESONANCE, {"filt_resonance", "Filter Resonance", "Res", 17}},
        {FILT_EG, {"filt_eg", "Filter EG to Cutoff", "EG>CO", 18}},

        {SUB_TYPE, {"sub_type", "Sub Type", "SubType", 29}},
        {SUB_LEVEL, {"sub_level", "Sub Level", "SubLev", 26}},

        {EG_ON_OFF, {"eg_onoff", "AEG Active", "OnOff", 31}},
        {EG_A, {"eg_a", "EG Attack", "A", 23}},
        {EG_D, {"eg_d", "EG Decay", "D", 19}},
        {EG_S, {"eg_s", "EG Sustain", "S", 27}},
        {EG_R, {"eg_r", "EG Release", "R", 28}},

    };
}
} // namespace baconpaul::elfin_controller