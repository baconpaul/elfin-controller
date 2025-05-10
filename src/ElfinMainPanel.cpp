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

#include "sst/plugininfra/paths.h"
#include "sst/plugininfra/version_information.h"

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

struct BasePanel : sst::jucegui::components::NamedPanel
{
    ElfinMainPanel &main;
    BasePanel(ElfinMainPanel &m, const std::string &s) : main(m), NamedPanel(s) {}

    template <typename W = sst::jucegui::components::Knob>
    W *attach(ElfinControllerAudioProcessor &p, ElfinControl c)
    {
        std::unique_ptr<W> wid;
        std::unique_ptr<ParamSource> ps;
        bindAndAdd(ps, wid, p.params[c]);
        auto res = wid.get();
        main.sources[c] = std::move(ps);
        main.widgets[c] = std::move(wid);
        return res;
    }

    template <typename W = sst::jucegui::components::MultiSwitch>
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

    template <typename W = sst::jucegui::components::Knob, typename D = ParamSource>
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

    template <typename A, typename B> void placeBelow(const A &a, const B &b)
    {
        auto ab = a->getBounds();
        auto bb = ab.translated(0, ab.getHeight()).withHeight(14);
        b->setBounds(bb);
    }

    void resizeInOrder(const std::vector<ElfinControl> &order, int xTrim = 0)
    {
        auto c = getContentArea();
        c = c.withTrimmedLeft(xTrim);
        auto kHeight = c.getHeight();
        auto bx = c.withWidth(kHeight - 18).withHeight(kHeight).translated(2, 0);
        for (auto &c : order)
        {
            main.widgets[c]->setBounds(bx);
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
    FilterPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Filter")
    {
        createFrom(p, contents);
    }
    void resized() override { resizeInOrder(contents); }
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

    std::unique_ptr<sst::jucegui::components::MultiSwitch> o1t, o2t;

    std::vector<ElfinControl> contents{ElfinControl::OSC12_MIX,         ElfinControl::OSC2_COARSE,
                                       ElfinControl::OSC2_FINE,         ElfinControl::SUB_TYPE,
                                       ElfinControl::SUB_LEVEL,         ElfinControl::EG_TO_PITCH,
                                       ElfinControl::EG_TO_PITCH_TARGET};
    OscPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "Oscillator")
    {
        auto typepar = p.params[OSC12_TYPE];
        assert(typepar);
        auto p1 = std::make_unique<Osc12Selector>(typepar, 0);
        auto p2 = std::make_unique<Osc12Selector>(typepar, 1);
        p1->other = p2.get();
        p2->other = p1.get();

        o1t = std::make_unique<sst::jucegui::components::MultiSwitch>();
        o1t->setSource(p1.get());
        addAndMakeVisible(*o1t);

        o2t = std::make_unique<sst::jucegui::components::MultiSwitch>();
        o2t->setSource(p2.get());
        addAndMakeVisible(*o2t);

        m.otherDiscrete.push_back(std::move(p1));
        m.otherDiscrete.push_back(std::move(p2));

        createFrom(p, contents);
    }
    void resized() override
    {
        auto c = getContentArea();
        auto kHeight = c.getHeight();
        auto bx = c.withWidth(kHeight - 24).withHeight(kHeight).translated(2, 0);
        o1t->setBounds(bx);
        bx = bx.translated(kHeight - 24, 0);
        o2t->setBounds(bx);

        resizeInOrder(contents, 2 * kHeight - 48 + 4);
    }
};

struct EGPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::EG_ON_OFF, ElfinControl::EG_A,
                                       ElfinControl::EG_D, ElfinControl::EG_S, ElfinControl::EG_R};
    EGPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "EG")
    {
        createFrom(p, contents);
    }
    void resized() override { resizeInOrder(contents); }
};

struct LFOPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::LFO_TYPE,
                                       ElfinControl::LFO_RATE,
                                       ElfinControl::LFO_DEPTH,
                                       ElfinControl::LFO_FADE_TIME,
                                       ElfinControl::LFO_TO_CUTOFF,
                                       ElfinControl::LFO_TO_PITCH,
                                       ElfinControl::LFO_TO_PITCH_TARGET,
                                       ElfinControl::EG_TO_LFORATE};
    LFOPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "LFO")
    {
        createFrom(p, contents);
    }
    void resized() override { resizeInOrder(contents); }
};

struct ModPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::PBEND_RANGE, ElfinControl::PITCH_TO_CUTOFF,
                                       ElfinControl::EXP_TO_CUTOFF, ElfinControl::EXP_TO_AMP_LEVEL,
                                       ElfinControl::EXP_BY_VEL};
    ModPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "MOD")
    {
        createFrom(p, contents);
    }
    void resized() override { resizeInOrder(contents); }
};

struct SettingsPanel : BasePanel
{
    std::vector<ElfinControl> contents{ElfinControl::PORTA,           ElfinControl::LEGATO,
                                       ElfinControl::KEY_ASSIGN_MODE, ElfinControl::OSC_LEVEL,
                                       ElfinControl::UNI_DETUNE,      ElfinControl::POLY_UNI_MODE,
                                       ElfinControl::DAMP_AND_ATTACK};
    SettingsPanel(ElfinMainPanel &m, ElfinControllerAudioProcessor &p) : BasePanel(m, "SETTINGS")
    {
        createFrom(p, contents);
    }
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

    userPath = sst::plugininfra::paths::bestDocumentsFolderPathFor("ElfinController");

    setStyle(sst::jucegui::style::StyleSheet::getBuiltInStyleSheet(
        sst::jucegui::style::StyleSheet::DARK));

    lnf = std::make_unique<sst::jucegui::style::LookAndFeelManager>(this);
    lnf->setStyle(style());

    mainMenu = std::make_unique<sst::jucegui::components::GlyphButton>(
        sst::jucegui::components::GlyphPainter::GlyphType::SETTINGS);
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

    titleLabel = std::make_unique<sst::jucegui::components::Label>();
    titleLabel->setText("Elfin-04 Polysynth Controller");
    titleLabel->setFontHeightOverride(28);
    addAndMakeVisible(*titleLabel);

    hideawayLabel = std::make_unique<sst::jucegui::components::Label>();
    hideawayLabel->setText("Hideaway Studios");
    hideawayLabel->setFontHeightOverride(15);
    addAndMakeVisible(*hideawayLabel);

    timer = std::make_unique<IdleTimer>(this);
    timer->startTimer(50);

    // Debug check
    for (int i = 0; i < ElfinControl::numElfinControlTypes; ++i)
    {
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
    p.addItem("Resend Patch to MIDI",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  for (auto p : w->processor.params)
                      p->invalid = true;
              });
    p.addSeparator();
    p.addItem("About", []() { ELFLOG("TODO: A reasonable about screen"); });
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
    static constexpr int margin{4};
    auto b = getLocalBounds().reduced(5);

    auto l1 = b.withHeight(30);
    titleLabel->setBounds(l1.reduced(60, 0));
    hideawayLabel->setBounds(l1.translated(0, 30).withHeight(20).reduced(60, 0));

    mainMenu->setBounds(l1.withHeight(50).withWidth(50));

    b = b.withTrimmedTop(55);
    auto fpB = b.withWidth(210).withHeight(100);
    filterPanel->setBounds(fpB);
    oscPanel->setBounds(fpB.translated(fpB.getWidth() + margin, 0).withWidth(580));
    egPanel->setBounds(
        filterPanel->getBounds().translated(0, fpB.getHeight() + margin).withWidth(360));
    modPanel->setBounds(
        egPanel->getBounds().translated(egPanel->getWidth() + margin, 0).withWidth(360));

    lfoPanel->setBounds(
        egPanel->getBounds().translated(0, fpB.getHeight() + margin).withWidth(600));
    settingsPanel->setBounds(
        lfoPanel->getBounds().translated(0, fpB.getHeight() + margin).withWidth(600));
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
    fileChooser->launchAsync(juce::FileBrowserComponent::canSelectFiles |
                                 juce::FileBrowserComponent::openMode,
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
                                 auto s = jf.loadFileAsString().toStdString();
                                 w->processor.fromXML(s);
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
                                 jf.create();
                                 jf.appendText(w->processor.toXML());
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
        toolTip = std::make_unique<sst::jucegui::components::ToolTip>();
        addChildComponent(*toolTip);
    }
    toolTip->setVisible(true);
    auto bl = c->getBounds().getBottomLeft();
    while (c != this && c->getParentComponent())
    {
        c = c->getParentComponent();
        bl += c->getBounds().getTopLeft();
    }
    toolTip->setTopLeftPosition(bl);
    updateToolTip(p);
}

void ElfinMainPanel::updateToolTip(ElfinControllerAudioProcessor::float_param_t *p)
{
    assert(toolTip);

    using row_t = sst::jucegui::components::ToolTip::Row;
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
    return false;
}
void ElfinMainPanel::filesDropped(const juce::StringArray &files, int x, int y)
{
    if (files.size() != 1)
        return;
    for (auto &f : files)
    {
        auto jf = juce::File(f);
        auto s = jf.loadFileAsString().toStdString();
        processor.fromXML(s);
    }
}
} // namespace baconpaul::elfin_controller
