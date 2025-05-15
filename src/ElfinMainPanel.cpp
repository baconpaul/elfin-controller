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

#include <fstream>

#include "sst/plugininfra/paths.h"
#include "sst/plugininfra/version_information.h"

#include "sst/jucegui/components/NamedPanel.h"
#include "sst/jucegui/data/Continuous.h"
#include "sst/jucegui/components/Label.h"
#include "sst/jucegui/components/Knob.h"
#include "sst/jucegui/components/MultiSwitch.h"
#include "sst/jucegui/components/VSlider.h"
#include "sst/jucegui/components/ToggleButton.h"
#include "sst/jucegui/layouts/ListLayout.h"

// This is super gross
#include "../libs/sst/sst-jucegui/src/sst/jucegui/components/KnobPainter.hxx"

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(elfin_content);

namespace baconpaul::elfin_controller
{

namespace jcmp = sst::jucegui::components;
namespace jlo = sst::jucegui::layouts;

static constexpr int outerMargin{5}, margin{5}, interControlMargin{15};
static constexpr int labelHeight{25}, subLabelHeight{25}, sectionHeight{104};
static constexpr int widgetHeight{53}, widgetLabelHeight{17};

struct LogoBase : juce::Component
{
    std::unique_ptr<juce::Drawable> logoSVG;

    LogoBase(const std::string &ln)
    {
        try
        {
            auto fs = cmrc::elfin_content::get_filesystem();
            auto f = fs.open("resources/content/logos/" + ln);
            std::string s(f.begin(), f.size());
            auto xml = juce::XmlDocument::parse(s);
            logoSVG = juce::Drawable::createFromSVG(*xml);
        }
        catch (const std::exception &e)
        {
            ELFLOG(e.what());
        }
    }
};
struct ElfinLogo : LogoBase
{
    ElfinLogo() : LogoBase("The Elfin Logo.svg") { assert(logoSVG); }

    void paint(juce::Graphics &g) override
    {
        if (!logoSVG)
            return;

        auto bd = logoSVG->getBounds();
        auto t = juce::AffineTransform();
        t = t.translated(0, -bd.getY());
        t = t.scaled(2.0, 2.0);
        t = t.translated((getWidth() - 2 * bd.getWidth()) / 2 - 10, 0);
        logoSVG->draw(g, 1.0, t);
    }
};

struct HideawayLogo : LogoBase
{
    HideawayLogo() : LogoBase("Hideaway logo.svg") { assert(logoSVG); }

    void paint(juce::Graphics &g) override
    {
        if (!logoSVG)
            return;
        auto scale = 0.45;
        auto bd = logoSVG->getBounds();
        auto t = juce::AffineTransform();
        t = t.translated(0, -bd.getY());
        t = t.scaled(scale, scale);
        t = t.translated((getWidth() - scale * bd.getWidth()) / 2, 0);
        logoSVG->draw(g, 1.0, t);
    }
};

struct CustomKnob : jcmp::Knob
{
    void paint(juce::Graphics &g);
};

struct ParamSource : sst::jucegui::data::Continuous
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    ElfinMainPanel &panel;

    ParamSource(ElfinControllerAudioProcessor::float_param_t *v, ElfinMainPanel &m) : panel(m)
    {
        par = v;
    }

    std::string getLabel() const override { return par->desc.label; }
    float getValue() const override { return par->get(); }
    bool isBipolar() const override { return par->desc.isBipolar; }
    void setValueFromGUI(const float &f) override
    {
        if (par->getCC() != par->getCCForFloat(f))
        {
            par->setValueNotifyingHost(f);
            panel.updateToolTip(par);
        }
    }
    void setValueFromModel(const float &f) override {}
    float getDefaultValue() const override { return par->getFloatForCC(par->desc.midiCCDefault); }
    float getMin() const override { return 0; }
    float getMax() const override { return 1; }
};

struct DiscreteParamSource : sst::jucegui::data::Discrete
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    DiscreteParamSource(ElfinControllerAudioProcessor::float_param_t *v, ElfinMainPanel &)
    {
        par = v;
    }

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

struct BasePanel : jcmp::NamedPanel
{
    ElfinMainPanel &main;
    BasePanel(ElfinMainPanel &m, const std::string &s) : main(m), NamedPanel(s) {}

    template <typename W = CustomKnob> W *attach(ElfinControllerAudioProcessor &p, ElfinControl c)
    {
        std::unique_ptr<W> wid;
        std::unique_ptr<ParamSource> ps;
        bindAndAdd(ps, wid, p.params[c]);
        auto res = wid.get();
        if constexpr (std::is_same_v<W, CustomKnob>)
        {
            res->setDrawLabel(false);
        }
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
        main.discreteSources[c] = std::move(ps);
        main.widgets[c] = std::move(wid);
        return res;
    }

    template <typename W = CustomKnob, typename D = ParamSource>
    void bindAndAdd(std::unique_ptr<D> &d, std::unique_ptr<W> &w,
                    ElfinControllerAudioProcessor::float_param_t *p)
    {
        d = std::make_unique<D>(p, main);
        w = std::make_unique<W>();
        w->setSource(d.get());
        addAndMakeVisible(*w);

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
            res.add(jlo::Component(*lit->second).withHeight(widgetLabelHeight).withWidth(width));

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
                    return "Sqr";
            }
            else
            {
                switch (i)
                {
                case 0:
                    return "Saw";
                case 1:
                    return "Sqr";
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
    OscPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Oscillator")
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
        lo.addGap(n);

        lo.add(controlLayoutComponent(OSC2_COARSE));
        lo.add(controlLayoutComponent(OSC2_FINE));
        lo.add(controlLayoutComponent(OSC_LEVEL));

        lo.addGap(n);
        lo.add(controlLayoutComponent(SUB_LEVEL));
        lo.add(controlLayoutComponent(SUB_TYPE));

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
        addLabel(EG_A, "A");
        attach(p, EG_D);
        addLabel(EG_D, "D");
        attach(p, EG_S);
        addLabel(EG_S, "S");

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
        auto pwid = widgetHeight + tgtWidth + margin;
        auto pl = jlo::VList().withWidth(tgtWidth);
        auto ptl = jlo::HList().withHeight(widgetHeight);
        ptl.add(jlo::Component(*main.widgets[EG_TO_PITCH]).withWidth(widgetHeight));
        ptl.addGap(margin);
        ptl.add(jlo::Component(*main.widgets[EG_TO_PITCH_TARGET]).withWidth(tgtWidth));
        pl.add(ptl);
        pl.add(jlo::Component(*main.widgetLabels[EG_TO_PITCH])
                   .withWidth(pwid)
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
        auto pwid = widgetHeight + tgtWidth + margin;
        auto pl = jlo::VList().withWidth(tgtWidth);
        auto ptl = jlo::HList().withHeight(widgetHeight);
        ptl.add(jlo::Component(*main.widgets[LFO_TO_PITCH]).withWidth(widgetHeight));
        ptl.addGap(margin);
        ptl.add(jlo::Component(*main.widgets[LFO_TO_PITCH_TARGET]).withWidth(tgtWidth));
        pl.add(ptl);
        pl.add(jlo::Component(*main.widgetLabels[LFO_TO_PITCH])
                   .withWidth(pwid)
                   .withHeight(widgetLabelHeight));
        lo.add(pl);

        lo.doLayout();
    }
};

struct ModPanel : BasePanel
{
    ModPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Expression")
    {
        attach(p, EXP_TO_CUTOFF);
        addLabel(EXP_TO_CUTOFF, rightArrow + "Cutoff");

        attach(p, EXP_TO_AMP_LEVEL);
        addLabel(EXP_TO_AMP_LEVEL, rightArrow + "Amp");
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
    std::vector<ElfinControl> contents{ElfinControl::POLY_UNI_MODE,  ElfinControl::UNI_DETUNE,
                                       ElfinControl::PORTA,          ElfinControl::LEGATO,
                                       ElfinControl::PBEND_RANGE,    ElfinControl::DAMP_AND_ATTACK,
                                       ElfinControl::KEY_ASSIGN_MODE};
    SettingsPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p)
        : BasePanel(m, "Voice Manager")
    {
        createFrom(p, contents);
    }
    void resized() override
    {
        auto lo = getLayoutHList();
        lo.addGap(15);
        int n = 20;
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
            if (c == UNI_DETUNE)
                lo.addGap(widgetHeight + 2 * margin);
            if (c == PBEND_RANGE)
                lo.addGap(widgetHeight);
            if (c == DAMP_AND_ATTACK)
                lo.addGap(widgetHeight - 20);
        }
        lo.doLayout();
    }
};

struct IdleTimer : juce::Timer
{
    ElfinMainPanel *parent{nullptr};
    IdleTimer(ElfinMainPanel *p) : parent(p){};
    void timerCallback() override { parent->onIdle(); }
};

namespace jstl = sst::jucegui::style;
using sheet_t = jstl::StyleSheet;
static constexpr sheet_t::Class PatchMenu("elfin-controller.patch-menu");

ElfinMainPanel::ElfinMainPanel(ElfinControllerAudioProcessor &p) : jcmp::WindowPanel(), processor(p)
{
    sst::jucegui::style::StyleSheet::initializeStyleSheets([]() {});

    userPath = sst::plugininfra::paths::bestDocumentsFolderPathFor("ElfinController");
    presetManager = std::make_unique<PresetManager>(userPath);

    presetDataBinding = std::make_unique<PresetDataBinding>(*presetManager);
    presetDataBinding->onLoad =
        [w = juce::Component::SafePointer(this)](int style, const fs::path &p)
    {
        if (!w)
            return;

        switch (style)
        {
        case 0:
            w->initPatch();
            break;
        case 1:
            ELFLOG("No factory patches yet");
            break;
        case 2:
            w->loadFromFile(p);
            break;
        }
        w->repaint();
    };

    presetButton = std::make_unique<jcmp::JogUpDownButton>();
    presetButton->setCustomClass(PatchMenu);
    presetButton->setSource(presetDataBinding.get());
    presetButton->onPopupMenu = [this]() { showPresetsMenu(); };
    addAndMakeVisible(*presetButton);

    sheet_t::addClass(PatchMenu).withBaseClass(jcmp::JogUpDownButton::Styles::styleClass);

    setStyle(sst::jucegui::style::StyleSheet::getBuiltInStyleSheet(
        sst::jucegui::style::StyleSheet::DARK));

    setupStyle();

    lnf = std::make_unique<sst::jucegui::style::LookAndFeelManager>(this);
    lnf->setStyle(style());

    mainMenu = std::make_unique<jcmp::GlyphButton>(jcmp::GlyphPainter::GlyphType::SETTINGS);
    mainMenu->setOnCallback(
        [w = juce::Component::SafePointer(this)]()
        {
            if (!w)
                return;
            w->showMainMenu();
        });
    addAndMakeVisible(*mainMenu);

    filterPanel = std::make_unique<FilterPanel>(*this, p);
    addAndMakeVisible(*filterPanel);

    oscPanel = std::make_unique<OscPanel>(*this, p);
    addAndMakeVisible(*oscPanel);

    egPanel = std::make_unique<EGPanel>(*this, p);
    addAndMakeVisible(*egPanel);

    lfoPanel = std::make_unique<LFOPanel>(*this, p);
    addAndMakeVisible(*lfoPanel);

    modPanel = std::make_unique<ModPanel>(*this, p);
    addAndMakeVisible(*modPanel);

    settingsPanel = std::make_unique<SettingsPanel>(*this, p);
    addAndMakeVisible(*settingsPanel);

    elfinLogo = std::make_unique<ElfinLogo>();
    addAndMakeVisible(*elfinLogo);

    hideawayLogo = std::make_unique<HideawayLogo>();
    addAndMakeVisible(*hideawayLogo);

    // hideawayLabel = std::make_unique<jcmp::Label>();
    // hideawayLabel->setText("Hideaway Studios");
    // hideawayLabel->setFontHeightOverride(15);
    // addAndMakeVisible(*hideawayLabel);

    aboutScreen = std::make_unique<ElfinAbout>();
    addChildComponent(*aboutScreen);

    timer = std::make_unique<IdleTimer>(this);
    timer->startTimer(50);

    // Debug check
    for (int i = 0; i < ElfinControl::numElfinControlTypes; ++i)
    {
        // This is in a custom split widget
        if (i == OSC12_TYPE)
        {
            continue;
        }
        if (widgets.find((ElfinControl)i) == widgets.end())
        {
            ELFLOG("Undisplayed control : (ElfinControl)" << i);
        }
    }
}

ElfinMainPanel::~ElfinMainPanel()
{
    if (timer)
        timer->stopTimer();
    setLookAndFeel(nullptr);
}

void ElfinMainPanel::showMainMenu()
{
    auto p = juce::PopupMenu();
    p.addSectionHeader("Elfin Controller");
    p.addSeparator();
    p.addItem("Save Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (w)
                      w->savePatch();
              });
    p.addItem("Load Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (w)
                      w->loadPatch();
              });
    p.addItem("Randomize Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (w)
                      w->processor.randomizePatch();
              });
    p.addSeparator();
    p.addItem("Resend Patch to MIDI",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  for (auto p : w->processor.params)
                      p->invalid = true;
              });
    p.addItem("Reset to Default",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->initPatch();
              });
    p.addSeparator();
    p.addItem("About",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->aboutScreen->showOver(w.getComponent());
              });
    p.addItem(
        "Source Code", []()
        { juce::URL("https://github.com/baconpaul/elfin-controller").launchInDefaultBrowser(); });

    auto vi = std::string() + sst::plugininfra::VersionInformation::git_implied_display_version +
              " " + sst::plugininfra::VersionInformation::git_commit_hash;
    p.addItem(vi, false, false, []() {});
    p.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this));
}

void ElfinMainPanel::resized()
{
    auto b = getLocalBounds().reduced(outerMargin);

    auto l1 = b.withHeight(labelHeight);
    elfinLogo->setBounds(l1.reduced(labelHeight + subLabelHeight + margin, 0));
    hideawayLogo->setBounds(b.withTrimmedTop(b.getHeight() - 20));

    presetButton->setBounds(l1.translated(0, labelHeight)
                                .withHeight(subLabelHeight)
                                .reduced(labelHeight + subLabelHeight + margin + 100, 0));

    mainMenu->setBounds(
        l1.withHeight(labelHeight + subLabelHeight).withWidth(labelHeight + subLabelHeight));

    auto listArea = b.withTrimmedTop(labelHeight + subLabelHeight + margin);

    auto lo = jlo::VList()
                  .at(listArea.getX(), listArea.getY())
                  .withWidth(listArea.getWidth())
                  .withAutoGap(margin);

    auto rwid = 472 + 203 + margin;

    auto row1 = jlo::HList().withAutoGap(margin).withHeight(sectionHeight);
    row1.add(jlo::Component(*oscPanel).withWidth(rwid));

    lo.add(row1);

    auto row2 = jlo::HList().withAutoGap(margin).withHeight(sectionHeight);
    row2.add(jlo::Component(*egPanel).withWidth(540));
    row2.add(jlo::Component(*modPanel).withWidth(rwid - 540 - margin));
    lo.add(row2);

    auto row3 = jlo::HList().withAutoGap(margin).withHeight(sectionHeight);
    row3.add(jlo::Component(*lfoPanel).withWidth(472));
    row3.add(jlo::Component(*filterPanel).withWidth(rwid - 472 - margin));
    lo.add(row3);

    auto row4 = jlo::HList().withAutoGap(margin).withHeight(sectionHeight);
    row4.add(jlo::Component(*settingsPanel).withWidth(rwid));
    lo.add(row4);
    lo.doLayout();
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

void ElfinMainPanel::loadPatch()
{
    setupUserPath();
    fileChooser = std::make_unique<juce::FileChooser>("Load Patch", juce::File(userPath.u8string()),
                                                      "*.elfin");
    fileChooser->launchAsync(
        juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::openMode,
        [w = juce::Component::SafePointer(this)](const juce::FileChooser &c)
        {
            if (!w)
                return;
            auto result = c.getResults();
            if (result.isEmpty() || result.size() > 1)
            {
                return;
            }
            auto jf = result[0];
            if (jf.getFileExtension() == ".elfin")
            {
                auto s = jf.loadFileAsString().toStdString();
                w->processor.fromXML(s);
            }
            else if (jf.getFileExtension() == ".elfsyx")
            {
                juce::MemoryBlock mb;
                jf.loadFileAsData(mb);
                std::vector<uint8_t> data((uint8_t *)mb.getData(),
                                          (uint8_t *)mb.getData() + mb.getSize());
                w->processor.fromSYX(data);
            }
        });
}

void ElfinMainPanel::savePatch()
{
    setupUserPath();
    fileChooser = std::make_unique<juce::FileChooser>("Save Patch", juce::File(userPath.u8string()),
                                                      "*.elfin");
    fileChooser->launchAsync(juce::FileBrowserComponent::canSelectFiles |
                                 juce::FileBrowserComponent::saveMode |
                                 juce::FileBrowserComponent::warnAboutOverwriting,
                             [w = juce::Component::SafePointer(this)](const juce::FileChooser &c)
                             {
                                 if (!w)
                                     return;
                                 auto result = c.getResults();
                                 if (result.isEmpty() || result.size() > 1)
                                 {
                                     return;
                                 }
                                 auto jf = result[0];
                                 if (jf.create() == juce::Result::ok())
                                 {
                                     jf.appendText(w->processor.toXML());
                                     w->presetManager->rescanUserPresets();
                                 }
                                 else
                                 {
                                     ELFLOG("Error saving");
                                 }
                             });
}

void ElfinMainPanel::setupUserPath()
{
    try
    {
        if (!fs::is_directory(userPath))
        {
            fs::create_directories(userPath);
        }
    }
    catch (fs::filesystem_error &e)
    {
    }
}

void ElfinMainPanel::showToolTip(ElfinControllerAudioProcessor::float_param_t *p,
                                 juce::Component *c)
{
    if (!toolTip)
    {
        toolTip = std::make_unique<jcmp::ToolTip>();
        addChildComponent(*toolTip);
    }
    auto oc = c;
    toolTip->setVisible(true);
    updateToolTip(p);
    auto bl = c->getBounds().getBottomLeft();
    while (c != this && c->getParentComponent())
    {
        c = c->getParentComponent();
        bl += c->getBounds().getTopLeft();
    }
    if (bl.y > getHeight() * 0.85)
    {
        bl.y -= oc->getHeight() + toolTip->getHeight();
    }
    toolTip->setTopLeftPosition(bl);
}

void ElfinMainPanel::updateToolTip(ElfinControllerAudioProcessor::float_param_t *p)
{
    assert(toolTip);

    using row_t = jcmp::ToolTip::Row;
    std::vector<row_t> rows;

    row_t one;
    one.leftAlignText = "CC=" + std::to_string(p->desc.midiCC);
    one.leftIsMonospace = true;
    row_t two;
    two.leftAlignText = "VAL=" + std::to_string(p->getCC());
    two.leftIsMonospace = true;
    rows.push_back(two);
    rows.push_back(one);
    toolTip->setTooltipTitleAndData(p->desc.name, rows);
    toolTip->resetSizeFromData();
}

void ElfinMainPanel::hideToolTip()
{
    assert(toolTip);
    toolTip->setVisible(false);
}

bool ElfinMainPanel::isInterestedInFileDrag(const juce::StringArray &files)
{
    if (files.size() != 1)
        return false;
    if (files[0].endsWith(".elfin"))
        return true;
    if (files[0].endsWith(".elfsyx"))
        return true;
    return false;
}
void ElfinMainPanel::filesDropped(const juce::StringArray &files, int x, int y)
{
    if (files.size() != 1)
        return;
    for (auto &f : files)
    {
        auto jf = juce::File(f);
        if (jf.getFileExtension() == ".elfin")
        {
            auto s = jf.loadFileAsString().toStdString();
            processor.fromXML(s);
        }
        else if (jf.getFileExtension() == ".elfsyx")
        {
            juce::MemoryBlock mb;
            jf.loadFileAsData(mb);
            std::vector<uint8_t> data((uint8_t *)mb.getData(),
                                      (uint8_t *)mb.getData() + mb.getSize());
            processor.fromSYX(data);
        }
    }
}

void ElfinMainPanel::setupStyle()
{
    const auto &st = style();
    namespace jbs = jcmp::base_styles;

    st->setFont(
        PatchMenu, jcmp::MenuButton::Styles::labelfont,
        st->getFont(jcmp::MenuButton::Styles::styleClass, jcmp::MenuButton::Styles::labelfont)
            .withHeight(18));
    st->setColour(PatchMenu, jcmp::MenuButton::Styles::fill, juce::Colour(0x30, 0x30, 0x30));

    st->setColour(jbs::ValueGutter::styleClass, jbs::ValueGutter::gutter,
                  juce::Colour(0x30, 0x30, 0x30));
    st->setColour(jbs::ValueGutter::styleClass, jbs::ValueGutter::gutter_hover,
                  juce::Colour(0x40, 0x40, 0x40));

    st->setColour(jbs::Outlined::styleClass, jbs::Outlined::brightoutline,
                  juce::Colour(0xEE, 0xEE, 0xEE));
    st->setColour(jbs::BaseLabel::styleClass, jbs::BaseLabel::labelcolor,
                  juce::Colour(0xEE, 0xEE, 0xEE));
    st->setColour(jcmp::NamedPanel::Styles::styleClass, jcmp::NamedPanel::Styles::labelrule,
                  juce::Colour(0xEE, 0xEE, 0xEE));
    st->setColour(jcmp::NamedPanel::Styles::styleClass, jcmp::NamedPanel::Styles::background,
                  juce::Colour(0x15, 0x15, 0x15));

    st->setColour(jcmp::WindowPanel::Styles::styleClass, jcmp::WindowPanel::Styles::bgstart,
                  juce::Colour(0x00, 0x0, 0x0));
    st->setColour(jcmp::WindowPanel::Styles::styleClass, jcmp::WindowPanel::Styles::bgend,
                  juce::Colour(0x20, 0x20, 0x20));
}

void CustomKnob::paint(juce::Graphics &g)
{
    jcmp::Knob::paint(g);
    return;

    jcmp::knobPainterNoBody(g, this, continuous());

    auto b = getLocalBounds();
    auto knobarea = b.withHeight(b.getWidth());

    auto circle = [knobarea](float r) -> juce::Path
    {
        auto region = knobarea.toFloat().reduced(r);
        auto p = juce::Path();
        p.startNewSubPath(region.getCentreX(), region.getY());
        p.addArc(region.getX(), region.getY(), region.getWidth(), region.getHeight(), 0,
                 2 * juce::MathConstants<float>::pi);
        p.closeSubPath();
        return p;
    };

    auto c = juce::Colour(0xA0, 0xA0, 0x90);
    auto graded = juce::ColourGradient::vertical(c.brighter(0.2), knobarea.getY(), c.darker(0.3),
                                                 knobarea.getBottom());

    int r0 = 8;
    g.setColour(juce::Colours::black);
    g.fillPath(circle(r0));
    g.setGradientFill(graded);
    g.fillPath(circle(r0 + 1));
    auto ci = juce::Colour(0x20, 0x20, 0x20);
    auto gradedo = juce::ColourGradient::vertical(ci.darker(0.2), knobarea.getY(), ci.brighter(0.3),
                                                  knobarea.getBottom());

    g.setGradientFill(gradedo);
    g.fillPath(circle(r0 + 3));

    auto handleAngle = [knobarea](float v) -> float
    {
        float dPath = 0.2;
        float dAng = juce::MathConstants<float>::pi * (1 - dPath);
        float pt = dAng * (2 * v - 1);
        return pt;
    };

    g.saveState();
    g.addTransform(juce::AffineTransform()
                       .translated(-knobarea.getWidth() / 2, -knobarea.getHeight() / 2)
                       .rotated(handleAngle(continuous()->getValue01()))
                       .translated(knobarea.getWidth() / 2, knobarea.getHeight() / 2));

    auto hanWidth = 2.f;
    auto hanLen = 8.f;
    auto hanRect =
        juce::Rectangle<float>(knobarea.getWidth() / 2.f - hanWidth / 2.f, r0, hanWidth, hanLen);
    g.setColour(juce::Colour(0xE0, 0xE0, 0xA0));
    g.fillRect(hanRect);
    g.restoreState();
}

void ElfinMainPanel::initPatch()
{
    for (auto p : processor.params)
    {
        auto f = p->getFloatForCC(p->desc.midiCCDefault);
        p->setValueNotifyingHost(f);
    }
}

void ElfinMainPanel::loadFromFile(const fs::path &p)
{
    std::ifstream file(p, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        ELFLOG("ERROR");
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    if (p.extension() == ".elfin")
    {
        processor.fromXML(buffer.str());
    }
    if (p.extension() == ".elfsyx")
    {
        auto s = buffer.str();
        std::vector<uint8_t> data(s.begin(), s.end());
        processor.fromSYX(data);
    }
}

void ElfinMainPanel::showPresetsMenu()
{
    auto m = juce::PopupMenu();
    m.addSectionHeader("Presets");
    m.addSeparator();

    m.addItem("Save Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->savePatch();
              });
    m.addItem("Load Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->loadPatch();
              });
    m.addItem("Randomize Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->processor.randomizePatch();
              });
    m.addSeparator();
    m.addItem("Init",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->presetDataBinding->setValueFromGUI(0);
              });

    for (auto &[k, v] : presetManager->userPatchTree)
    {
        auto mk = [w = juce::Component::SafePointer(this)](const int &c)
        {
            return [q = w, idx = c]()
            {
                if (!q)
                    return;
                q->presetDataBinding->setValueFromGUI(idx);
                q->repaint();
            };
        };
        if (k.empty())
        {
            for (auto &c : v)
            {
                m.addItem(c.first.u8string(), mk(c.second));
            }
        }
        else
        {
            auto sub = juce::PopupMenu();
            for (auto &c : v)
            {
                sub.addItem(c.first.filename().u8string(), mk(c.second));
            }
            m.addSubMenu(k.u8string(), sub);
        }
    }

    m.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this));
}

} // namespace baconpaul::elfin_controller
