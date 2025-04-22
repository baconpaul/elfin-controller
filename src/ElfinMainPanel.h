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

#ifndef ELFIN_CONTROLLER_ELFINMAINPANEL_H
#define ELFIN_CONTROLLER_ELFINMAINPANEL_H

#include "sst/jucegui/components/WindowPanel.h"
#include "sst/jucegui/components/Knob.h"
#include "sst/jucegui/data/Continuous.h"
#include "ElfinProcessor.h"

namespace baconpaul::elfin_controller
{
struct FilterPanel;
struct OscPanel;

struct ElfinMainPanel : sst::jucegui::components::WindowPanel
{
    ElfinMainPanel(ElfinControllerAudioProcessor &);
    ~ElfinMainPanel();

    std::unique_ptr<FilterPanel> filterPanel;
    std::unique_ptr<OscPanel> oscPanel;

    void resized() override;
};
} // namespace baconpaul::elfin_controller
#endif // ELFINMAINPANEL_H
