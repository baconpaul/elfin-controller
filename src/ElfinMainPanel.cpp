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
#include "sst/jucegui/data/Continuous.h"

struct ParamSource : sst::jucegui::data::Continuous
{
    juce::AudioParameterFloat *par{nullptr};
    ParamSource(juce::AudioParameterFloat *v) { par = v; }

    std::string getLabel() const override { return "L"; }
    float getValue() const override { return par->get(); }
    void setValueFromGUI(const float &f) override { par->setValueNotifyingHost(f); }
    void setValueFromModel(const float &f) override { }
    float getDefaultValue() const override { return 0.5; }
    float getMin() const override { return 0; }
    float getMax() const override { return 1; }
};
struct FilterPanel : sst::jucegui::components::NamedPanel
{
    std::unique_ptr<ParamSource> filterCutoffSource;
    std::unique_ptr<sst::jucegui::components::Knob> filterCutoff;
    FilterPanel(ElfinControllerAudioProcessor &p) : NamedPanel("Filter")
    {
        auto *par = p.params[0];
        filterCutoffSource = std::make_unique<ParamSource>(par);
        filterCutoff = std::make_unique<sst::jucegui::components::Knob>();
        filterCutoff->setSource(filterCutoffSource.get());
        addAndMakeVisible(*filterCutoff);
    }
    void resized() override
    {
        auto c = getContentArea();
        filterCutoff->setBounds(c.withWidth(c.getHeight()));
    }
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
