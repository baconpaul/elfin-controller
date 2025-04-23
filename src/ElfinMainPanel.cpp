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

namespace baconpaul::elfin_controller
{
struct ParamSource : sst::jucegui::data::Continuous
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    ParamSource(ElfinControllerAudioProcessor::float_param_t *v) { par = v; }

    std::string getLabel() const override { return "L"; }
    float getValue() const override { return par->get(); }
    void setValueFromGUI(const float &f) override
    {
        if (par->getCC() != par->getCCFor(f))
        {
            par->setValueNotifyingHost(f);
        }
    }
    void setValueFromModel(const float &f) override {}
    float getDefaultValue() const override { return 0.5; }
    float getMin() const override { return 0; }
    float getMax() const override { return 1; }
};
struct BasePanel : sst::jucegui::components::NamedPanel
{
    BasePanel(const std::string &s) : NamedPanel(s) {}
    template <typename W = sst::jucegui::components::Knob, typename D = ParamSource>
    void bindAndAdd(std::unique_ptr<D> &d, std::unique_ptr<W> &w,
                    ElfinControllerAudioProcessor::float_param_t *p)
    {
        d = std::make_unique<D>(p);
        w = std::make_unique<W>();
        w->setSource(d.get());
        addAndMakeVisible(*w);
    }
};

struct FilterPanel : BasePanel
{
    std::unique_ptr<ParamSource> filterCutoffSource, resonanceSource;
    std::unique_ptr<sst::jucegui::components::Knob> filterCutoff, resonance;
    FilterPanel(ElfinControllerAudioProcessor &p) : BasePanel("Filter")
    {
        bindAndAdd(filterCutoffSource, filterCutoff, p.params[ElfinControl::FILT_CUTOFF]);
        bindAndAdd(resonanceSource, resonance, p.params[ElfinControl::FILT_RESONANCE]);
    }
    void resized() override
    {
        auto c = getContentArea();
        filterCutoff->setBounds(c.withWidth(c.getHeight() - 3));
        resonance->setBounds(filterCutoff->getBounds().translated(c.getHeight(), 0));
    }
};

struct OscPanel : BasePanel
{
    std::unique_ptr<ParamSource> osc12TSource, osc12MSource, osc2Csource, osc2Fsource;
    std::unique_ptr<sst::jucegui::components::Knob> osc12T, osc12M, osc2C, osc2F;
    OscPanel(ElfinControllerAudioProcessor &p) : BasePanel("Osc")
    {
        bindAndAdd(osc12TSource, osc12T, p.params[OSC12_TYPE]);
        bindAndAdd(osc12MSource, osc12M, p.params[OSC12_MIX]);
        bindAndAdd(osc2Csource, osc2C, p.params[OSC2_COARSE]);
        bindAndAdd(osc2Fsource, osc2F, p.params[OSC2_FINE]);
    }
    void resized() override
    {
        auto c = getContentArea();
        osc12T->setBounds(c.withWidth(c.getHeight() - 3));
        osc12M->setBounds(osc12T->getBounds().translated(c.getHeight(), 0));
        osc2C->setBounds(osc12M->getBounds().translated(c.getHeight(), 0));
        osc2F->setBounds(osc2C->getBounds().translated(c.getHeight(), 0));
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

    oscPanel = std::make_unique<OscPanel>(p);
    addAndMakeVisible(*oscPanel);
}

ElfinMainPanel::~ElfinMainPanel() {}

void ElfinMainPanel::resized()
{
    auto b = getLocalBounds().reduced(5);
    auto fpB = b.withWidth(200).withHeight(100);
    filterPanel->setBounds(fpB);
    oscPanel->setBounds(fpB.translated(0, fpB.getHeight() + 10).withWidth(400));
}
} // namespace baconpaul::elfin_controller
