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

#include "ElfinMainPanel.h"
#include "sst/jucegui/components/NamedPanel.h"
#include "sst/jucegui/data/Continuous.h"
#include "sst/jucegui/components/Label.h"
#include "sst/jucegui/components/Knob.h"
#include "sst/jucegui/components/MultiSwitch.h"

namespace baconpaul::elfin_controller
{
struct ParamSource : sst::jucegui::data::Continuous
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    ParamSource(ElfinControllerAudioProcessor::float_param_t *v) { par = v; }

    std::string getLabel() const override { return par->desc.label; }
    float getValue() const override { return par->get(); }
    void setValueFromGUI(const float &f) override
    {
        if (par->getCC() != par->getCCForFloat(f))
        {
            par->setValueNotifyingHost(f);
        }
    }
    void setValueFromModel(const float &f) override {}
    float getDefaultValue() const override { return 0.5; }
    float getMin() const override { return 0; }
    float getMax() const override { return 1; }
};

struct DiscreteParamSource : sst::jucegui::data::Discrete
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    DiscreteParamSource(ElfinControllerAudioProcessor::float_param_t *v) { par = v; }

    std::string getLabel() const override { return par->desc.label; }
    int cacheCC{-1}, cacheLookup{-1};
    int getValue() const override
    {
        auto v = par->getCC();
        int idx{0};
        for (auto &d : par->desc.discreteRanges)
        {
            if (v >= d.from && v <= d.to)
                return idx;
            idx++;
        }
        return 0;
    }
    void setValueFromGUI(const int &i) override
    {
        auto rng = par->desc.discreteRanges[i];
        auto mid = (rng.from + rng.to) / 2;
        auto f = par->getFloatForCC(mid);
        par->setValueNotifyingHost(f);
    }
    void setValueFromModel(const int &f) override {}
    int getDefaultValue() const override { return 2; }
    int getMin() const override { return 0; }
    int getMax() const override { return par->desc.discreteRanges.size() - 1; }
    std::string getValueAsStringFor(int i) const override
    {
        return par->desc.discreteRanges[i].label;
    }
};

struct BasePanel : sst::jucegui::components::NamedPanel
{
    BasePanel(const std::string &s) : NamedPanel(s) {}

    std::map<ElfinControl, std::unique_ptr<ParamSource>> sources;
    std::map<ElfinControl, std::unique_ptr<DiscreteParamSource>> discreteSources;
    std::map<ElfinControl, std::unique_ptr<juce::Component>> widgets;

    template <typename W = sst::jucegui::components::Knob>
    W *attach(ElfinControllerAudioProcessor &p, ElfinControl c)
    {
        std::unique_ptr<W> wid;
        std::unique_ptr<ParamSource> ps;
        bindAndAdd(ps, wid, p.params[c]);
        auto res = wid.get();
        sources[c] = std::move(ps);
        widgets[c] = std::move(wid);
        return res;
    }

    template <typename W = sst::jucegui::components::MultiSwitch>
    W *attachDiscrete(ElfinControllerAudioProcessor &p, ElfinControl c)
    {
        std::unique_ptr<W> wid;
        std::unique_ptr<DiscreteParamSource> ps;
        bindAndAdd(ps, wid, p.params[c]);
        auto res = wid.get();
        discreteSources[c] = std::move(ps);
        widgets[c] = std::move(wid);
        return res;
    }

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

    void resizeInOrder(const std::vector<ElfinControl> &order)
    {
        auto c = getContentArea();
        auto kHeight = c.getHeight();
        auto bx = c.withWidth(kHeight - 18).withHeight(kHeight).translated(2, 0);
        for (auto &c : order)
        {
            widgets[c]->setBounds(bx);
            bx = bx.translated(kHeight, 0);
        }
    }

    void createFrom(ElfinControllerAudioProcessor &p, const std::vector<ElfinControl> &contents)
    {
        for (auto &c : contents)
        {
            auto par = p.params[c];
            if (!par)
                continue;
            if (par->desc.hasDiscreteRanges())
            {
                ELFLOG("Discrete control at " << par->desc.name);
                attachDiscrete(p, c);
            }
            else
            {
                attach(p, c);
            }
        }
    }
};

struct FilterPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::FILT_CUTOFF, ElfinControl::FILT_RESONANCE,
                                       ElfinControl::FILT_EG};
    FilterPanel(ElfinControllerAudioProcessor &p) : BasePanel("Filter") { createFrom(p, contents); }
    void resized() override { resizeInOrder(contents); }
};

struct OscPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::OSC12_TYPE,  ElfinControl::OSC12_MIX,
                                       ElfinControl::OSC2_COARSE, ElfinControl::OSC2_FINE,
                                       ElfinControl::SUB_TYPE,    ElfinControl::SUB_LEVEL};
    OscPanel(ElfinControllerAudioProcessor &p) : BasePanel("Oscillator")
    {
        createFrom(p, contents);
    }
    void resized() override { resizeInOrder(contents); }
};

struct EGPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::EG_ON_OFF, ElfinControl::EG_A,
                                       ElfinControl::EG_D, ElfinControl::EG_S, ElfinControl::EG_R};
    EGPanel(ElfinControllerAudioProcessor &p) : BasePanel("EG") { createFrom(p, contents); }
    void resized() override { resizeInOrder(contents); }
};


struct LFOPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::LFO_TYPE, ElfinControl::LFO_RATE, ElfinControl::LFO_DEPTH, ElfinControl::LFO_FADE_TIME,
        ElfinControl::LFO_TO_CUTOFF, ElfinControl::LFO_TO_PITCH, ElfinControl::LFO_TO_PITCH_TARGET,
    ElfinControl::EG_TO_LFORATE};
    LFOPanel(ElfinControllerAudioProcessor &p) : BasePanel("LFO") { createFrom(p, contents); }
    void resized() override { resizeInOrder(contents); }
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

    egPanel = std::make_unique<EGPanel>(p);
    addAndMakeVisible(*egPanel);

    lfoPanel = std::make_unique<LFOPanel>(p);
    addAndMakeVisible(*lfoPanel);

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
    auto fpB = b.withWidth(210).withHeight(100);
    filterPanel->setBounds(fpB);
    oscPanel->setBounds(fpB.translated(0, fpB.getHeight() + 10).withWidth(450));
    egPanel->setBounds(oscPanel->getBounds().translated(0, fpB.getHeight() + 10).withWidth(450));
    lfoPanel->setBounds(egPanel->getBounds().translated(0, fpB.getHeight() + 10).withWidth(600));
}

void ElfinMainPanel::paint(juce::Graphics &g) { WindowPanel::paint(g); }

void ElfinMainPanel::onIdle()
{
    bool doRepaint = false;
    if (processor.refreshUI)
    {
        doRepaint = true;
        processor.refreshUI = false;
    }

    if (doRepaint)
        repaint();
}

} // namespace baconpaul::elfin_controller
