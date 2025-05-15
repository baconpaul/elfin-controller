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

#ifndef ELFIN_CONTROLLER_ELFINKNOB_H
#define ELFIN_CONTROLLER_ELFINKNOB_H

#include <sst/jucegui/components/Knob.h>

namespace baconpaul::elfin_controller
{

struct ElfinKnob : sst::jucegui::components::Knob
{
    void paint(juce::Graphics &g);
};

} // namespace baconpaul::elfin_controller
#endif // ELFINKNOB_H
