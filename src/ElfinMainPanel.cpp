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

#include "ElfinMainPanel.h"
#include "sst/jucegui/components/NamedPanel.h"

struct FilterPanel : sst::jucegui::components::NamedPanel
{
    FilterPanel(ElfinControllerAudioProcessor &p) : NamedPanel("Filter") {}
};

ElfinMainPanel::ElfinMainPanel(ElfinControllerAudioProcessor &p)
    : sst::jucegui::components::WindowPanel()
{
    sst::jucegui::style::StyleSheet::initializeStyleSheets([]() {});

    setStyle(sst::jucegui::style::StyleSheet::getBuiltInStyleSheet(
        sst::jucegui::style::StyleSheet::DARK));

    filterPanel = std::make_unique<FilterPanel>(p);
    addAndMakeVisible(*filterPanel);
}

ElfinMainPanel::~ElfinMainPanel() {}

void ElfinMainPanel::resized()
{
    auto b = getLocalBounds().reduced(5);
    auto fpB = b.withWidth(200).withHeight(100);
    filterPanel->setBounds(fpB);
}
