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
#include <set>

namespace baconpaul::elfin_controller
{
std::map<ElfinControl, ElfinDescription> elfinConfig;
void setupConfiguration()
{
    elfinConfig = std::map<ElfinControl, ElfinDescription>{
        {OSC12_TYPE, ElfinDescription{"osc12_type", "Osc 1/2 Wave", "Wave", 24, 7}},
        {OSC12_MIX, {"osc12_mix", "Osc 1/2 Mix", "Mix", 25, 64, true}},
        {OSC2_COARSE, {"osc2_coarse", "Osc 2 Coarse", "Coarse", 20, 64, true}},
        {OSC2_FINE, {"osc2_fine", "Osc 2 Fine", "Fine", 21, 64, true}},

        {FILT_CUTOFF, {"filt_cutoff", "Filter Cutoff", "Cutoff", 16, 127}},
        {FILT_RESONANCE, {"filt_resonance", "Filter Resonance", "Resonance", 17, 0}},
        {EG_TO_CUTOFF, {"eg_to_cutoff", "Envelope -> Cutoff", "> Cutoff", 18, 64, true}},

        {SUB_TYPE, {"sub_type", "Sub Wave", "Sub", 29, 15}},
        {SUB_LEVEL, {"sub_level", "Sub Level", "Level", 26, 0}},

        {EG_TO_PITCH, {"eg_to_pitch", "EG -> Pitch", "> Pitch", 104, 64, true}},
        {EG_TO_PITCH_TARGET,
         {"eg_to_pitch_target", "Osc Pitch Target", "Osc Pitch Target", 105, 31}},

        {EG_ON_OFF, {"eg_onoff", "EG -> VCA", "> VCA", 31, 31}},
        {EG_A, {"eg_a", "EG Attack", "Attack", 23, 0}},
        {EG_D, {"eg_d", "EG Decay", "Decay", 19, 0}},
        {EG_S, {"eg_s", "EG Sustain", "Sustain", 27, 127}},
        {EG_R, {"eg_r", "EG Release", "Release", 28, 31}},

        {LFO_TYPE, {"lfo_type", "LFO Wave", "Wave", 14, 7}},
        {LFO_RATE, {"lfo_rate", "LFO Rate", "Rate", 80, 64}},
        {LFO_DEPTH, {"lfo_depth", "LFO Depth", "Depth", 81, 127}},
        {LFO_TO_PITCH, {"lfo_to_pitch", "LFO -> Pitch", "> Pitch", 82, 64, true}},
        {LFO_TO_CUTOFF, {"lfo_to_cutoff", "LFO -> Cutoff", "> Cutoff", 83, 64, true}},
        {LFO_TO_PITCH_TARGET,
         {"lfo_to_pitch_tgt", "LFO Osc Pitch Target", "Osc Pitch Target", 9, 31}},
        {LFO_FADE_TIME, {"lfo_fade_time", "LFO Fade In", "Fade In", 15, 0}},
        {EG_TO_LFORATE, {"eg_to_rate", "EG -> LFO Rate", "> LFO Rate", 3, 64, true}},

        {PBEND_RANGE, {"pbend_range", "Pitch Bend Range", "Bend Range", 85, 2}},
        {PITCH_TO_CUTOFF, {"pitch_to_cut", "Filter Keytrack", "Keytrack", 86, 16}},
        {EXP_TO_CUTOFF, {"exp_to_cut", "Velocity -> Cutoff", "> Cutoff", 106, 0}},
        {EXP_TO_AMP_LEVEL, {"exp_to_amp", "Velocity -> VCA", "> VCA", 107, 0}},

        {PORTA, {"portamento", "Portamento", "Portamento", 22, 1}},
        {LEGATO, {"legato", "Legato", "Legato", 30, 31}},
        {KEY_ASSIGN_MODE, {"key_assign", "Key Assign", "Key Assign", 87, 119}},
        {OSC_LEVEL, {"osc_level", "Osc Level", "Osc Level", 108, 127}},
        {UNI_DETUNE, {"uni_detune", "Unison Detune", "Detune", 109, 0}},
        {POLY_UNI_MODE, {"poly_uni", "Poly Unison Mode", "Mode", 110, 31}},
        {DAMP_AND_ATTACK, {"damp_and_attack", "EG Damping", "EG Damping", 111, 63}}};

    // Set up the discrete ranges
    auto &ot = elfinConfig[OSC12_TYPE];
    ot.discreteRanges.emplace_back(0, 15, "Saw/Saw");
    ot.discreteRanges.emplace_back(16, 39, "Saw/Square");
    ot.discreteRanges.emplace_back(40, 63, "Saw/Noise");
    ot.discreteRanges.emplace_back(64, 87, "Square/Noise");
    ot.discreteRanges.emplace_back(88, 111, "Square/Saw");
    ot.discreteRanges.emplace_back(112, 127, "Square/Square");

    auto &sst = elfinConfig[SUB_TYPE];
    sst.discreteRanges.emplace_back(0, 31, "Sine");
    sst.discreteRanges.emplace_back(32, 95, "Noise");
    sst.discreteRanges.emplace_back(96, 127, "Square");

    auto &lfot = elfinConfig[LFO_TYPE];
    lfot.discreteRanges.emplace_back(0, 15, "Tri No KT");
    lfot.discreteRanges.emplace_back(16, 47, "Tri");
    lfot.discreteRanges.emplace_back(48, 79, "Saw");
    lfot.discreteRanges.emplace_back(80, 111, "Random");
    lfot.discreteRanges.emplace_back(112, 127, "Square");

    elfinConfig[EG_ON_OFF].setAsTwoStage("Off", "On");
    elfinConfig[EG_R].setAsTwoStage("Off", "On");
    elfinConfig[LFO_TO_PITCH_TARGET].setAsTwoStage("1+2", "2");
    elfinConfig[EG_TO_PITCH_TARGET].setAsTwoStage("1+2", "2");
    elfinConfig[LEGATO].setAsTwoStage("Off", "On");
    elfinConfig[POLY_UNI_MODE].setAsTwoStage("Poly", "Unison");

    elfinConfig[PBEND_RANGE].midiCCEnd = 30;
    elfinConfig[DAMP_AND_ATTACK].midiCCStart = 63;
    elfinConfig[DAMP_AND_ATTACK].midiCCStartLabel = "Off";

    auto &kasn = elfinConfig[KEY_ASSIGN_MODE];
    kasn.discreteRanges.emplace_back(0, 47, "Low ST");
    kasn.discreteRanges.emplace_back(48, 79, "Duo ST");
    kasn.discreteRanges.emplace_back(80, 111, "Highest ST");
    kasn.discreteRanges.emplace_back(112, 127, "Last MT");

    auto &ptc = elfinConfig[PITCH_TO_CUTOFF];
    ptc.discreteRanges.emplace_back(0, 32, "None");
    ptc.discreteRanges.emplace_back(33, 96, "Half");
    ptc.discreteRanges.emplace_back(97, 127, "Full");

    std::set<std::string> mappedStreaming;
    for (int i = 0; i < ElfinControl::numElfinControlTypes; ++i)
    {
        if (elfinConfig.find((ElfinControl)i) == elfinConfig.end())
        {
            ELFLOG("Unmapped control : (ElfinControl)" << i);
        }
        auto &ec = elfinConfig[(ElfinControl)i];
        if (mappedStreaming.find(ec.streaming_name) != mappedStreaming.end())
        {
            ELFLOG("Double key " << ec.streaming_name);
            std::terminate();
        }
        mappedStreaming.insert(ec.streaming_name);
    }
}
} // namespace baconpaul::elfin_controller