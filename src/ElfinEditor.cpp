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
#include <algorithm>
#include <cmath>

#include <juce_gui_basics/juce_gui_basics.h>
#include "sst/jucegui/style/StyleSheet.h"
#include "ElfinEditor.h"

namespace baconpaul::elfin_controller
{
//==============================================================================
ElfinControllerAudioProcessorEditor::ElfinControllerAudioProcessorEditor(
    ElfinControllerAudioProcessor &p)
    : AudioProcessorEditor(&p), processor(p)
{
    mainPanel = std::make_unique<ElfinMainPanel>(processor);
    addAndMakeVisible(*mainPanel);

    idleTimer = std::make_unique<IdleTimer>(this);
    idleTimer->startTimer(1000 / 60);

    setSize(690, 525);
}

ElfinControllerAudioProcessorEditor::~ElfinControllerAudioProcessorEditor() {}

void ElfinControllerAudioProcessorEditor::idle() {}

void ElfinControllerAudioProcessorEditor::handleAsyncUpdate() {}

void ElfinControllerAudioProcessorEditor::resized() { mainPanel->setBounds(getLocalBounds()); }
} // namespace baconpaul::elfin_controller