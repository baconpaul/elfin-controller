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
#include "sst/jucegui/components/Label.h"
#include "sst/jucegui/components/Knob.h"

namespace baconpaul::elfin_controller
{

#if LOGSCREEN
std::vector<std::string> logMessages;
std::mutex logLock;
#endif

struct ParamSource : sst::jucegui::data::Continuous
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    ParamSource(ElfinControllerAudioProcessor::float_param_t *v) { par = v; }

    std::string getLabel() const override { return par->desc.label; }
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

        w->onBeginEdit = [p]()
        {
            ELFLOG("Begin Edit " << p->desc.name);
            p->beginChangeGesture();
        };
        w->onEndEdit = [p]()
        {
            ELFLOG("End Edit " << p->desc.name);
            p->endChangeGesture();
        };
    }

    template <typename A, typename B> void placeBelow(const A &a, const B &b)
    {
        auto ab = a->getBounds();
        auto bb = ab.translated(0, ab.getHeight()).withHeight(14);
        b->setBounds(bb);
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
        auto kHeight = c.getHeight();
        filterCutoff->setBounds(c.withWidth(kHeight - 18).withHeight(kHeight));
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
        auto kHeight = c.getHeight();

        osc12T->setBounds(c.withWidth(kHeight - 18));
        osc12M->setBounds(osc12T->getBounds().translated(kHeight, 0));
        osc2C->setBounds(osc12M->getBounds().translated(kHeight, 0));
        osc2F->setBounds(osc2C->getBounds().translated(kHeight, 0));
    }
};

struct IdleTimer : juce::Timer
{
    ElfinMainPanel *parent{nullptr};
    IdleTimer(ElfinMainPanel *p) : parent(p){};
    void timerCallback() override { parent->onIdle(); }
};

ElfinMainPanel::ElfinMainPanel(ElfinControllerAudioProcessor &p)
    : sst::jucegui::components::WindowPanel(), processor(p)
{
    sst::jucegui::style::StyleSheet::initializeStyleSheets([]() {});

    setStyle(sst::jucegui::style::StyleSheet::getBuiltInStyleSheet(
        sst::jucegui::style::StyleSheet::DARK));

    filterPanel = std::make_unique<FilterPanel>(p);
    addAndMakeVisible(*filterPanel);

    oscPanel = std::make_unique<OscPanel>(p);
    addAndMakeVisible(*oscPanel);
    timer = std::make_unique<IdleTimer>(this);
    timer->startTimer(50);
}

ElfinMainPanel::~ElfinMainPanel()
{
    if (timer)
        timer->stopTimer();
}

void ElfinMainPanel::resized()
{
    auto b = getLocalBounds().reduced(5);
    auto fpB = b.withWidth(200).withHeight(100);
    filterPanel->setBounds(fpB);
    oscPanel->setBounds(fpB.translated(0, fpB.getHeight() + 10).withWidth(400));
}

void ElfinMainPanel::paint(juce::Graphics &g)
{
    WindowPanel::paint(g);
#if LOGSCREEN
    std::lock_guard<std::mutex> loggg(logLock);
    auto lms = logMessages.size() - 1;
    auto fs = 11;
    auto nr = 15;

    g.setFont(fs);
    g.setColour(juce::Colours::yellow);
    auto h = getHeight() - nr * fs;
    for (int i = 0; i < nr; ++i)
    {
        auto ll = lms - i;
        if (ll < 0)
            break;
        g.drawText(logMessages[ll], 5, h, getWidth() - 10, 10, juce::Justification::centredLeft);
        h += fs + 2;
    }
#endif
}

void ElfinMainPanel::onIdle()
{
    bool doRepaint = false;
    if (processor.refreshUI)
    {
        doRepaint = true;
        processor.refreshUI = false;
    }

#if LOGSCREEN
    {
        std::lock_guard<std::mutex> loggg(logLock);
        if (logMessages.size() != lastLogSize)
        {
            doRepaint = true;
        }
    }
#endif
    if (doRepaint)
        repaint();
}

} // namespace baconpaul::elfin_controller
