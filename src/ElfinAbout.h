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

#ifndef ELFIN_CONTROLLER_ELFINABOUT_H
#define ELFIN_CONTROLLER_ELFINABOUT_H

#include <juce_gui_basics/juce_gui_basics.h>

namespace baconpaul::elfin_controller
{
struct ElfinAbout : juce::Component
{
    void mouseUp(const juce::MouseEvent &event) override { setVisible(false); }
    void paint(juce::Graphics &g) override;

    void showOver(juce::Component *that)
    {
        setBounds(that->getBounds());
        setVisible(true);
        toFront(true);
    }
};
} // namespace baconpaul::elfin_controller
#endif // ELFINABOUT_H
