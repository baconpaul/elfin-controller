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
        {OSC12_TYPE, ElfinDescription{"osc12_type", "OSC 1/2 Type", "Type", 24, 7}},
        {OSC12_MIX, {"osc12_mix", "OSC 1/2 Mix", "1/2 Mix", 25, 54}},
        {OSC2_COARSE, {"osc2_coarse", "OSC2 Coarse", "2 Coarse", 20, 64, true}},
        {OSC2_FINE, {"osc2_fine", "OSC2 Fine", "2 Fine", 21, 68, true}},

        {FILT_CUTOFF, {"filt_cutoff", "Filter Cutoff", "Cutoff", 16, 65}},
        {FILT_RESONANCE, {"filt_resonance", "Filter Resonance", "Res", 17, 82}},
        {FILT_EG, {"filt_eg", "Filter EG to Cutoff", "EG>CO", 18, 77, true}},

        {SUB_TYPE, {"sub_type", "Sub Type", "SubType", 29, 15}},
        {SUB_LEVEL, {"sub_level", "Sub Level", "SubLev", 26, 79}},

        {EG_TO_PITCH, {"eg_to_pitch", "EG to Pitch", "EG>Pitch", 104, 67, true}},
        {EG_TO_PITCH_TARGET, {"eg_to_pitch_target", "EG Pitch Target", "EGPTgt", 105, 95}},

        {EG_ON_OFF, {"eg_onoff", "AEG Active", "OnOff", 31, 31}},
        {EG_A, {"eg_a", "EG Attack", "A", 23, 27}},
        {EG_D, {"eg_d", "EG Decay", "D", 19, 47}},
        {EG_S, {"eg_s", "EG Sustain", "S", 27, 99}},
        {EG_R, {"eg_r", "EG Release", "R", 28, 102}},

        {LFO_TYPE, {"lfo_type", "LFO Type", "Type", 14, 31}},
        {LFO_RATE, {"lfo_rate", "LFO Rate", "Rate", 80, 15}},
        {LFO_DEPTH, {"lfo_depth", "LFO Depth", "Depth", 81, 30}},
        {LFO_TO_PITCH, {"lfo_to_pitch", "LFO To Pitch", "> Pitch", 82, 67, true}},
        {LFO_TO_CUTOFF, {"lfo_tu_cutoff", "LFO To Cutoff", "> Cutoff", 83, true}},
        {LFO_TO_PITCH_TARGET, {"lfo_to_pitch_tgt", "LFO To Pitch Target", "Target", 9, 95}},
        {LFO_FADE_TIME, {"lfo_fade_time", "LFO Fade Time", "Fade T", 15, 15}},
        {EG_TO_LFORATE, {"eg_to_rate", "EG to LFO Rate", "EG > Rate", 3, 78, true}},

        {PBEND_RANGE, {"pbend_range", "Pitch Bend Range", "PB", 85, 51}},
        {PITCH_TO_CUTOFF, {"pitch_to_cut", "Pitch to Cutoff", "Ptch>CO", 86, 64}},
        {EXP_TO_CUTOFF, {"exp_to_cut", "Exp to Cutoff", "Exp>CO", 106, 64}},
        {EXP_TO_AMP_LEVEL, {"exp_to_amp", "Exp to Amp", "Exp>Amp", 107, 64}},
        {EXP_BY_VEL, {"exp_by_vel", "Exp by Vel", "Exp/Vel", 89, 31}},

        {PORTA, {"portamento", "Portamento", "Porta", 22, 95}},
        {LEGATO, {"legato", "Legato", "Legato", 30, 95}},
        {KEY_ASSIGN_MODE, {"key_assign", "Key Assign", "KAsn", 87, 119}},
        {OSC_LEVEL, {"osc_level", "OSC Level", "OscLev", 108, 127}},
        {UNI_DETUNE, {"uni_detune", "Unison Detune", "UniDt", 109, 24}},
        {POLY_UNI_MODE, {"poly_uni", "Poly Unison Mode", "PUMode", 110, 31}},
        {DAMP_AND_ATTACK, {"damp_and_attack", "Damp and Attack", "Dmp/Atk", 111, 64}}};

    // Set up the discrete ranges
    auto &ot = elfinConfig[OSC12_TYPE];
    ot.discreteRanges.emplace_back(0, 15, "Saw/Saw");
    ot.discreteRanges.emplace_back(16, 39, "Saw/Sqr");
    ot.discreteRanges.emplace_back(40, 63, "Saw/Noise");
    ot.discreteRanges.emplace_back(64, 87, "Sqr/Noise");
    ot.discreteRanges.emplace_back(88, 111, "Sqr/Saw");
    ot.discreteRanges.emplace_back(112, 127, "Sqr/Sqr");

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

    elfinConfig[EG_ON_OFF].setAsTwoStage("Off", "On");
    elfinConfig[EG_R].setAsTwoStage("Off", "On");
    elfinConfig[LFO_TO_PITCH_TARGET].setAsTwoStage("1 and 2", "2");
    elfinConfig[EG_TO_PITCH_TARGET].setAsTwoStage("1 and 2", "2");
    elfinConfig[EXP_BY_VEL].setAsTwoStage("Off", "On");
    elfinConfig[LEGATO].setAsTwoStage("Off", "On");
    elfinConfig[POLY_UNI_MODE].setAsTwoStage("Poly", "Unison");

    auto &kasn = elfinConfig[KEY_ASSIGN_MODE];
    kasn.discreteRanges.emplace_back(0, 47, "Low ST");
    kasn.discreteRanges.emplace_back(48, 79, "Duo ST");
    kasn.discreteRanges.emplace_back(80, 111, "Highest ST");
    kasn.discreteRanges.emplace_back(112, 127, "Last MT");

    auto &ptc = elfinConfig[PITCH_TO_CUTOFF];
    ptc.discreteRanges.emplace_back(0, 32, "Off");
    ptc.discreteRanges.emplace_back(33, 96, "Half");
    ptc.discreteRanges.emplace_back(97, 127, "On");

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