/*
 * Elfin Controller
 *
 * A small controller plugin for the Elfin 04 Polysynth
 *
 * Copyright 2025, Paul Walker and Various authors, as described in the github
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

        {LFO_TYPE, {"lfo_type", "LFO Type", "Type", 14}},
        {LFO_RATE, {"lfo_type", "LFO Rate", "Rate", 80}},
        {LFO_DEPTH, {"lfo_depth", "LFO Depth", "Depth", 81}},
        {LFO_TO_PITCH, {"lfo_to_pitch", "LFO To Pitch", "> Pitch", 82}},
        {LFO_TO_CUTOFF, {"lfo_tu_cutoff", "LFO To Cutoff", "> Cutoff", 83}},
        {LFO_TO_PITCH_TARGET, {"lfo_to_pitch_tgt", "LFO To Pitch Target", "Target", 9}},
        {LFO_FADE_TIME, {"lfo_fade_time", "LFO Fade Time", "Fade T", 15}},
        {EG_TO_LFORATE, {"eg_to_rate", "EG to LFO Rate", "EG > Rate", 3}},
    };

    // Set up the discrete ranges
    auto &ot = elfinConfig[OSC12_TYPE];
    ot.discreteRanges.emplace_back(0, 15, "Saw/Saw");
    ot.discreteRanges.emplace_back(16, 39, "Saw/Sqr");
    ot.discreteRanges.emplace_back(40, 63, "Saw/Noise");
    ot.discreteRanges.emplace_back(64, 87, "Sqr/Noise");
    ot.discreteRanges.emplace_back(88, 111, "Sqr/Saw");
    ot.discreteRanges.emplace_back(112, 127, "Sqr/Sqr");

    auto &aeg = elfinConfig[EG_ON_OFF];
    aeg.discreteRanges.emplace_back(0, 63, "Off");
    aeg.discreteRanges.emplace_back(64, 127, "On");

    auto &sst = elfinConfig[SUB_TYPE];
    sst.discreteRanges.emplace_back(0, 31, "SIN");
    sst.discreteRanges.emplace_back(32, 95, "NOISE");
    sst.discreteRanges.emplace_back(96, 127, "SQUARE");

    auto &lfot = elfinConfig[LFO_TYPE];
    lfot.discreteRanges.emplace_back(0, 15, "Tri, No KT");
    lfot.discreteRanges.emplace_back(16, 47, "Tri");
    lfot.discreteRanges.emplace_back(48, 79, "SAW Dn");
    lfot.discreteRanges.emplace_back(80, 111, "Rand");
    lfot.discreteRanges.emplace_back(112, 127, "Sqr");

    auto &lfop = elfinConfig[LFO_TO_PITCH_TARGET];
    lfop.discreteRanges.emplace_back(0, 63, "1 and 2");
    lfop.discreteRanges.emplace_back(64, 127, "2");

    for (int i = 0; i < ElfinControl::numElfinControlTypes; ++i)
    {
        if (elfinConfig.find((ElfinControl)i) == elfinConfig.end())
        {
            ELFLOG("Unmapped control : (ElfinControl)" << i);
        }
    }
}
} // namespace baconpaul::elfin_controller