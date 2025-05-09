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

#ifndef ELFIN_CONTROLLER_CONFIGURATION_H
#define ELFIN_CONTROLLER_CONFIGURATION_H

#include <map>
#include <array>
#include <vector>
#include <iostream>

namespace baconpaul::elfin_controller
{
struct ElfinDescription
{
    std::string streaming_name{};
    std::string name{}, label{};
    int32_t midiCC{-1}, midiCCDefault{0};
    bool isBipolar{false};

    ElfinDescription() = default;
    ElfinDescription(const std::string &s, const std::string &n, const std::string &l, int32_t m,
                     int32_t ccDef, bool isBip = false)
        : name(n), label(l), midiCC(m), midiCCDefault(ccDef), streaming_name(s), isBipolar(isBip)
    {
    }

    struct LabeledMidiRange
    {
        LabeledMidiRange() {}
        LabeledMidiRange(int f, int t, const std::string &l) : from(f), to(t), label(l) {}
        int16_t from{-1}, to{-1};
        std::string label{"err"};
    };
    std::vector<LabeledMidiRange> discreteRanges;
    bool hasDiscreteRanges() const { return !discreteRanges.empty(); }

    void setAsTwoStage(const std::string &lo, const std::string &hi)
    {
        discreteRanges.clear();
        discreteRanges.emplace_back(0, 63, lo);
        discreteRanges.emplace_back(64, 127, hi);
    }
};

// This enum value doesn't stream. Its just for code readabiligy
enum ElfinControl
{
    OSC12_TYPE,
    OSC12_MIX,
    OSC2_COARSE,
    OSC2_FINE,

    SUB_TYPE,
    SUB_LEVEL,

    EG_TO_PITCH,
    EG_TO_PITCH_TARGET,

    FILT_CUTOFF,
    FILT_RESONANCE,
    FILT_EG,

    EG_ON_OFF,
    EG_A,
    EG_D,
    EG_S,
    EG_R,

    LFO_TYPE,
    LFO_RATE,
    LFO_TO_PITCH,
    LFO_TO_CUTOFF,

    LFO_DEPTH,
    EG_TO_LFORATE,
    LFO_TO_PITCH_TARGET,
    LFO_FADE_TIME,

    PBEND_RANGE,
    PITCH_TO_CUTOFF,
    EXP_TO_CUTOFF,
    EXP_TO_AMP_LEVEL,

    PORTA,
    LEGATO,
    KEY_ASSIGN_MODE,
    EXP_BY_VEL,

    OSC_LEVEL,
    UNI_DETUNE,
    POLY_UNI_MODE,
    DAMP_AND_ATTACK,

    numElfinControlTypes
};

static constexpr int nElfinParams = (int)ElfinControl::numElfinControlTypes;

extern std::map<ElfinControl, ElfinDescription> elfinConfig;

void setupConfiguration();

#define ELFLOG(...) std::cout << __FILE__ << ":" << __LINE__ << " " << __VA_ARGS__ << std::endl

};     // namespace baconpaul::elfin_controller
#endif // CONFIGURATION_H
