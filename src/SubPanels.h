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

#ifndef ELFIN_CONTROLLER_SUBPANELS_H
#define ELFIN_CONTROLLER_SUBPANELS_H

#include "sst/jucegui/components/NamedPanel.h"
#include "sst/jucegui/data/Continuous.h"
#include "sst/jucegui/components/Label.h"
#include "sst/jucegui/components/Knob.h"
#include "sst/jucegui/components/MultiSwitch.h"
#include "sst/jucegui/components/VSlider.h"
#include "sst/jucegui/components/ToggleButton.h"
#include "sst/jucegui/layouts/ListLayout.h"
#include "ElfinKnob.h"
#include "UIConstants.h"

namespace baconpaul::elfin_controller
{

namespace jcmp = sst::jucegui::components;
namespace jlo = sst::jucegui::layouts;

struct BasePanel : jcmp::NamedPanel
{
    ElfinMainPanel &main;
    BasePanel(ElfinMainPanel &m, const std::string &s) : main(m), NamedPanel(s)
    {
        labelPosition = IN_BORDER;
    }

    template <typename W = ElfinKnob> W *attach(ElfinControllerAudioProcessor &p, ElfinControl c)
    {
        std::unique_ptr<W> wid;
        std::unique_ptr<ParamSource> ps;
        bindAndAdd(ps, wid, p.params[c]);
        auto res = wid.get();
        if constexpr (std::is_same_v<W, ElfinKnob>)
        {
            res->setDrawLabel(false);
        }
        wid->delayUntilIdle = ElfinMainPanel::tooltipDelayInMS;

        main.sources[c] = std::move(ps);
        main.widgets[c] = std::move(wid);
        return res;
    }

    template <typename W = jcmp::MultiSwitch>
    W *attachDiscrete(ElfinControllerAudioProcessor &p, ElfinControl c)
    {
        std::unique_ptr<W> wid;
        std::unique_ptr<DiscreteParamSource> ps;
        bindAndAdd(ps, wid, p.params[c]);
        auto res = wid.get();
        wid->delayUntilIdle = ElfinMainPanel::tooltipDelayInMS;

        main.discreteSources[c] = std::move(ps);
        main.widgets[c] = std::move(wid);
        return res;
    }

    template <typename W = ElfinKnob, typename D = ParamSource>
    void bindAndAdd(std::unique_ptr<D> &d, std::unique_ptr<W> &w,
                    ElfinControllerAudioProcessor::float_param_t *p)
    {
        d = std::make_unique<D>(p, main);
        w = std::make_unique<W>();
        w->setSource(d.get());
        addAndMakeVisible(*w);

        w->delayUntilIdle = ElfinMainPanel::tooltipDelayInMS;

        w->onBeginEdit = [wv = w.get(), q = juce::Component::SafePointer(this), p]()
        {
            p->beginChangeGesture();
            if (q)
            {
                q->main.showToolTip(p, wv);
            }
        };
        w->onEndEdit = [w = juce::Component::SafePointer(this), p]()
        {
            p->endChangeGesture();
            if (w)
            {
                w->main.hideToolTip();
            }
        };
        w->onIdleHover = [wv = w.get(), q = juce::Component::SafePointer(this), p]()
        {
            if (q)
            {
                q->main.showToolTip(p, wv);
            }
        };
        w->onIdleHoverEnd = [w = juce::Component::SafePointer(this), p]()
        {
            if (w)
            {
                w->main.hideToolTip();
            }
        };
    }

    void resizeUsingLayout(const std::vector<ElfinControl> &contents)
    {
        auto lo = getLayoutHList();
        layoutInto(lo, contents);
        lo.doLayout();
    }

    jlo::LayoutComponent getLayoutHList()
    {
        auto c = getContentArea().reduced(2, 0);

        auto lo = jlo::HList()
                      .withHeight(c.getHeight())
                      .withAutoGap(interControlMargin)
                      .at(c.getX(), c.getY());
        return lo;
    }
    void layoutInto(jlo::LayoutComponent &lo, const std::vector<ElfinControl> &contents)
    {
        for (auto c : contents)
        {
            lo.add(controlLayoutComponent(c));
        }
    }

    jlo::LayoutComponent controlLayoutComponent(ElfinControl c, size_t width = widgetHeight)
    {
        auto res = jlo::VList().withWidth(width);

        auto wit = main.widgets.find(c);
        auto lit = main.widgetLabels.find(c);

        if (wit != main.widgets.end())
            res.add(jlo::Component(*wit->second).withHeight(widgetHeight).withWidth(width));
        else
            res.addGap(widgetHeight);

        if (lit != main.widgetLabels.end())
            res.add(jlo::Component(*lit->second)
                        .withHeight(widgetLabelHeight)
                        .withWidth(width + 20)
                        .centerInParent());

        return res;
    }

    jlo::LayoutComponent labeledItem(juce::Component &item, juce::Component &label,
                                     size_t width = widgetHeight)
    {
        auto res = jlo::VList().withWidth(width);

        res.add(jlo::Component(item).withHeight(widgetHeight).withWidth(width));
        res.add(jlo::Component(label).withHeight(widgetLabelHeight).withWidth(width));

        return res;
    }

    void addLabel(ElfinControl c, const std::string &l)
    {
        auto lab = std::make_unique<jcmp::Label>();
        lab->setText(l);
        lab->setJustification(juce::Justification::centred);
        addAndMakeVisible(*lab);
        main.widgetLabels[c] = std::move(lab);
    }

    void createFrom(ElfinControllerAudioProcessor &p, const std::vector<ElfinControl> &contents)
    {
        for (auto &c : contents)
        {
            auto par = p.params[c];
            if (!par)
                continue;
            bool makeLabel{true};
            if (par->desc.hasDiscreteRanges())
            {
                attachDiscrete(p, c);
            }
            else
            {
                attach(p, c);
            }

            if (makeLabel)
            {
                addLabel(c, par->desc.label);
            }
        }
    }
};

struct FilterPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::FILT_CUTOFF, ElfinControl::FILT_RESONANCE,
                                       ElfinControl::PITCH_TO_CUTOFF};
    FilterPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Filter")
    {
        createFrom(p, contents);
    }
    void resized() override { resizeUsingLayout(contents); }
};

struct OscPanel : BasePanel
{
    struct Osc12Selector : sst::jucegui::data::Discrete
    {
        ElfinControllerAudioProcessor::float_param_t *par{nullptr};
        int which{0};
        Osc12Selector *other{nullptr};
        Osc12Selector(ElfinControllerAudioProcessor::float_param_t *p, int w) : par(p), which(w) {}

        void resetFromBothParams(int iv1, int iv2)
        {
            assert(other);
            int cc{7};
            if (iv1 == 0) // saw gives us saw square noise as second
            {
                switch (iv2)
                {
                case 0: // saw saw is 7
                    cc = 7;
                    break;
                case 1: // saw square is midpoint of 16 and 39
                    cc = (39 + 16) / 2;
                    break;
                case 2: // sqw noise is midpoint of 40 and 63
                    cc = (40 + 63) / 2;
                    break;
                }
            }
            else // square
            {
                switch (iv2)
                {
                case 0: // square saw is 88 to 111
                    cc = (88 + 111) / 2;
                    break;
                case 1: // square square is 112 to 127
                    cc = (112 + 127) / 2;
                    break;
                case 2: // square noise is 64 to 87
                    cc = (64 + 87) / 2;
                    break;
                }
            }
            auto val = par->getFloatForCC(cc);
            par->beginChangeGesture();
            par->setValueNotifyingHost(val);
            par->endChangeGesture();
        }

        int getIValueFromPar() const
        {
            auto cc = par->getCC();

            if (which == 0) // osc1 is saw saw saw square square square
            {
                if (cc < 64)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else // oscillator 2 is saw square noise noise saw square
            {
                if (cc < 64)
                {
                    if (cc < 16)
                        return 0;
                    if (cc < 40)
                        return 1;
                    return 2;
                }
                else
                {
                    if (cc < 88)
                        return 2;
                    if (cc < 111)
                        return 0;
                    return 1;
                }
            }
            return 0;
        }
        std::string getLabel() const override { return "Osc " + std::to_string(which + 1); }
        int getValue() const override { return getIValueFromPar(); }
        int getDefaultValue() const override { return 0; }
        void setValueFromGUI(const int &v) override
        {
            resetFromBothParams(which == 0 ? v : other->getIValueFromPar(),
                                which == 0 ? other->getIValueFromPar() : v);
        }
        void setValueFromModel(const int &f) override {}
        std::string getValueAsStringFor(int i) const override
        {
            if (which == 0)
            {
                if (i == 0)
                    return "Saw";
                else
                    return "Square";
            }
            else
            {
                switch (i)
                {
                case 0:
                    return "Saw";
                case 1:
                    return "Square";
                case 2:
                    return "Noise";
                }
            }
            return "Err";
        }
        int getMin() const override { return 0; }
        int getMax() const override
        {
            if (which == 0)
                return 1;
            else
                return 2;
        }
    };

    std::unique_ptr<jcmp::MultiSwitch> o1t, o2t;
    std::unique_ptr<jcmp::Label> o1lab, o2lab;

    std::vector<ElfinControl> contents{ElfinControl::OSC12_MIX, ElfinControl::OSC2_COARSE,
                                       ElfinControl::OSC2_FINE, ElfinControl::SUB_TYPE,
                                       ElfinControl::SUB_LEVEL, ElfinControl::OSC_LEVEL};
    OscPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Oscillators")
    {
        auto typepar = p.params[OSC12_TYPE];
        assert(typepar);
        auto p1 = std::make_unique<Osc12Selector>(typepar, 0);
        auto p2 = std::make_unique<Osc12Selector>(typepar, 1);
        p1->other = p2.get();
        p2->other = p1.get();

        o1t = std::make_unique<jcmp::MultiSwitch>();
        o1t->setSource(p1.get());
        addAndMakeVisible(*o1t);

        o1lab = std::make_unique<jcmp::Label>();
        o1lab->setText("Osc 1");
        o1lab->setJustification(juce::Justification::centred);
        addAndMakeVisible(*o1lab);

        o2t = std::make_unique<jcmp::MultiSwitch>();
        o2t->setSource(p2.get());
        addAndMakeVisible(*o2t);

        o2lab = std::make_unique<jcmp::Label>();
        o2lab->setText("Osc 2");
        o2lab->setJustification(juce::Justification::centred);
        addAndMakeVisible(*o2lab);

        m.otherDiscrete.push_back(std::move(p1));
        m.otherDiscrete.push_back(std::move(p2));

        createFrom(p, contents);
    }
    void resized() override
    {
        auto lo = getLayoutHList();
        lo.add(labeledItem(*o1t, *o1lab));
        lo.add(controlLayoutComponent(OSC12_MIX));
        lo.add(labeledItem(*o2t, *o2lab));

        int n = 66;

        lo.add(controlLayoutComponent(OSC2_COARSE));
        lo.add(controlLayoutComponent(OSC2_FINE));

        lo.addGap(n);

        lo.add(controlLayoutComponent(OSC_LEVEL));

        lo.addGap(n);

        lo.add(controlLayoutComponent(SUB_TYPE));
        lo.add(controlLayoutComponent(SUB_LEVEL));

        auto bx = lo.doLayout();
    }
};

struct EGPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::EG_ON_OFF, ElfinControl::EG_A,
                                       ElfinControl::EG_D, ElfinControl::EG_S, ElfinControl::EG_R};
    EGPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "EG")
    {
        attach(p, EG_A);
        addLabel(EG_A, "Attack");
        attach(p, EG_D);
        addLabel(EG_D, "Decay");
        attach(p, EG_S);
        addLabel(EG_S, "Sustain");

        auto tb = attachDiscrete<jcmp::ToggleButton>(p, EG_ON_OFF);
        tb->setDrawMode(jcmp::ToggleButton::DrawMode::LABELED);
        tb->setLabel(rightArrow + "VCA");

        auto rb = attachDiscrete<jcmp::ToggleButton>(p, EG_R);
        rb->setDrawMode(jcmp::ToggleButton::DrawMode::LABELED);
        rb->setLabel("Release");

        attach(p, EG_TO_LFORATE);
        addLabel(EG_TO_LFORATE, rightArrow + "Rate");
        attach(p, EG_TO_CUTOFF);
        addLabel(EG_TO_CUTOFF, rightArrow + "Cutoff");
        attach(p, EG_TO_PITCH);
        addLabel(EG_TO_PITCH, rightArrow + "Pitch");

        attachDiscrete(p, EG_TO_PITCH_TARGET);
        // createFrom(p, contents);
    }
    void resized() override
    {
        auto lo = getLayoutHList();
        lo.add(controlLayoutComponent(EG_A));
        lo.add(controlLayoutComponent(EG_D));
        lo.add(controlLayoutComponent(EG_S));

        auto vl = jlo::VList().withWidth(60).withAutoGap(margin);
        vl.add(jlo::Component(*main.widgets[EG_ON_OFF]).withHeight(25));
        vl.add(jlo::Component(*main.widgets[EG_R]).withHeight(25));
        lo.add(vl);
        lo.addGap(20);

        lo.add(controlLayoutComponent(EG_TO_CUTOFF));
        lo.add(controlLayoutComponent(EG_TO_LFORATE));

        static constexpr int tgtWidth{30};
        auto pl = jlo::VList().withWidth(tgtWidth);
        auto ptl = jlo::HList().withHeight(widgetHeight);
        ptl.add(jlo::Component(*main.widgets[EG_TO_PITCH]).withWidth(widgetHeight));
        ptl.addGap(margin);
        ptl.add(jlo::Component(*main.widgets[EG_TO_PITCH_TARGET]).withWidth(tgtWidth));
        pl.add(ptl);
        pl.add(jlo::Component(*main.widgetLabels[EG_TO_PITCH])
                   .withWidth(widgetHeight)
                   .withHeight(widgetLabelHeight));
        lo.add(pl);

        lo.doLayout();
    }
};

struct LFOPanel : BasePanel
{
    std::vector<ElfinControl> contents{
        ElfinControl::LFO_TYPE,           ElfinControl::LFO_RATE,      ElfinControl::LFO_DEPTH,
        ElfinControl::LFO_FADE_TIME,      ElfinControl::LFO_TO_CUTOFF, ElfinControl::LFO_TO_PITCH,
        ElfinControl::LFO_TO_PITCH_TARGET};
    LFOPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "LFO")
    {
        attachDiscrete(p, LFO_TYPE);

        attach(p, LFO_RATE);
        addLabel(LFO_RATE, "Rate");
        attach(p, LFO_DEPTH);
        addLabel(LFO_DEPTH, "Depth");
        attach(p, LFO_FADE_TIME);
        addLabel(LFO_FADE_TIME, "Fade In");

        attach(p, LFO_TO_CUTOFF);
        addLabel(LFO_TO_CUTOFF, rightArrow + "Cutoff");

        attach(p, LFO_TO_PITCH);
        addLabel(LFO_TO_PITCH, rightArrow + "Pitch");
        attachDiscrete(p, LFO_TO_PITCH_TARGET);
    }
    void resized() override
    {
        auto lo = getLayoutHList();
        lo.add(jlo::Component(*main.widgets[LFO_TYPE])
                   .withHeight(widgetHeight + widgetLabelHeight)
                   .withWidth(widgetHeight * 1.5));
        lo.add(controlLayoutComponent(LFO_RATE));
        lo.add(controlLayoutComponent(LFO_DEPTH));
        lo.add(controlLayoutComponent(LFO_FADE_TIME));
        lo.add(controlLayoutComponent(LFO_TO_CUTOFF));

        static constexpr int tgtWidth{30};
        auto pl = jlo::VList().withWidth(tgtWidth);
        auto ptl = jlo::HList().withHeight(widgetHeight);
        ptl.add(jlo::Component(*main.widgets[LFO_TO_PITCH]).withWidth(widgetHeight));
        ptl.addGap(margin);
        ptl.add(jlo::Component(*main.widgets[LFO_TO_PITCH_TARGET]).withWidth(tgtWidth));
        pl.add(ptl);
        pl.add(jlo::Component(*main.widgetLabels[LFO_TO_PITCH])
                   .withWidth(widgetHeight)
                   .withHeight(widgetLabelHeight));
        lo.add(pl);

        lo.doLayout();
    }
};

struct ModPanel : BasePanel
{
    ModPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Velocity")
    {
        attach(p, EXP_TO_CUTOFF);
        addLabel(EXP_TO_CUTOFF, rightArrow + "Cutoff");

        attach(p, EXP_TO_AMP_LEVEL);
        addLabel(EXP_TO_AMP_LEVEL, rightArrow + "VCA");
    }
    void resized() override
    {
        auto lo = getLayoutHList();
        lo.add(controlLayoutComponent(EXP_TO_CUTOFF));
        lo.add(controlLayoutComponent(EXP_TO_AMP_LEVEL));

        lo.doLayout();
    }
};

struct SettingsPanel : BasePanel
{
    SettingsPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p)
        : BasePanel(m, "Voice Manager")
    {
        attach(p, UNI_DETUNE);
        addLabel(UNI_DETUNE, "Detune");

        attach(p, PORTA);
        addLabel(PORTA, "Portamento");

        attach(p, PBEND_RANGE);
        addLabel(PBEND_RANGE, "Bend Range");

        attach(p, DAMP_AND_ATTACK);
        addLabel(DAMP_AND_ATTACK, "EG Damping");

        attachDiscrete(p, KEY_ASSIGN_MODE);

        auto ub = attachDiscrete<jcmp::ToggleButton>(p, POLY_UNI_MODE);
        ub->setDrawMode(jcmp::ToggleButton::DrawMode::LABELED);
        ub->setLabel("Unison");
        main.discreteSources[POLY_UNI_MODE]->andThenOnGui = [this](int v) { resetUnison(); };

        auto lb = attachDiscrete<jcmp::ToggleButton>(p, LEGATO);
        lb->setDrawMode(jcmp::ToggleButton::DrawMode::LABELED);
        lb->setLabel("Legato");
    }

    void resetUnison()
    {
        auto uv = main.processor.params[POLY_UNI_MODE]->getCC() <= 64;
        main.widgets[UNI_DETUNE]->setEnabled(uv);
        main.widgets[KEY_ASSIGN_MODE]->setEnabled(!uv);
    }

    void resized() override
    {
        auto lo = getLayoutHList();
        int n = 20;

        auto vl = jlo::VList().withWidth(widgetHeight * 1.5).withAutoGap(margin);
        vl.add(jlo::Component(*main.widgets[POLY_UNI_MODE]).withHeight(25));
        vl.add(jlo::Component(*main.widgets[LEGATO]).withHeight(25));
        lo.add(vl);

        std::vector<ElfinControl> contents{ElfinControl::UNI_DETUNE, ElfinControl::PORTA,
                                           ElfinControl::PBEND_RANGE, ElfinControl::DAMP_AND_ATTACK,
                                           ElfinControl::KEY_ASSIGN_MODE};

        for (auto c : contents)
        {
            if (c != KEY_ASSIGN_MODE)
            {
                lo.add(controlLayoutComponent(c));
            }
            else
            {
                lo.add(jlo::Component(*main.widgets[KEY_ASSIGN_MODE])
                           .withWidth(widgetHeight * 1.5)
                           .withHeight(widgetHeight + widgetLabelHeight));
            }
            if (c == PORTA)
                lo.addGap(widgetHeight + 2 * margin);
            if (c == PBEND_RANGE)
                lo.addGap(2 * widgetHeight + 2 * margin);
            if (c == DAMP_AND_ATTACK)
                lo.addGap(widgetHeight * 0.5 + 3 * margin);
        }
        lo.doLayout();
    }
};

} // namespace baconpaul::elfin_controller

#endif // SUBPANELS_H
