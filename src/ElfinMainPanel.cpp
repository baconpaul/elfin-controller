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

#include "ParamSources.h"
#include "CustomWidgets.h"
#include "SubPanels.h"

#include "UIConstants.h"

namespace baconpaul::elfin_controller
{

namespace jcmp = sst::jucegui::components;
namespace jlo = sst::jucegui::layouts;

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
        [w = juce::Component::SafePointer(this)](int style, int idx, const fs::path &p)
    {
        if (!w)
            return;

        switch (style)
        {
        case 0:
            w->initPatch();
            break;
        case 1:
        {
            const std::string s = w->presetManager->factoryXMLFor(idx);
            if (!s.empty())
            {
                w->processor.fromXML(s);
            }
        }
        break;
        case 2:
            w->loadFromFile(p);
            break;
        }
        w->repaint();
    };

    presetButton = std::make_unique<PresetButton>();
    presetButton->setCustomClass(PatchMenu);
    presetButton->setSource(presetDataBinding.get());
    presetButton->onPopupMenu = [w = juce::Component::SafePointer(this)]()
    {
        if (w)
            w->showElfinMainMenu();
    };
    presetButton->onDice = [w = juce::Component::SafePointer(this)]()
    {
        if (w)
        {
            w->processor.randomizePatch();
            w->repaint();
        };
    };
    presetButton->arrowPosition = jcmp::JogUpDownButton::RIGHT_SIDE;
    addAndMakeVisible(*presetButton);

    sheet_t::addClass(PatchMenu).withBaseClass(jcmp::JogUpDownButton::Styles::styleClass);

    setStyle(sst::jucegui::style::StyleSheet::getBuiltInStyleSheet(
        sst::jucegui::style::StyleSheet::DARK));

    setupStyle();

    lnf = std::make_unique<sst::jucegui::style::LookAndFeelManager>(this);
    lnf->setStyle(style());

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

    setTransform(juce::AffineTransform().scaled(uiScale));
}

ElfinMainPanel::~ElfinMainPanel()
{
    if (timer)
        timer->stopTimer();
    setLookAndFeel(nullptr);
}

void ElfinMainPanel::resized()
{
    auto b = getLocalBounds().reduced(outerMargin);

    auto l1 = b.withHeight(labelHeight);
    elfinLogo->setBounds(l1.withTrimmedRight(400));
    hideawayLogo->setBounds(l1.withTrimmedLeft(400));

    presetButton->setBounds(l1.withHeight(labelHeight).withTrimmedLeft(margin).reduced(185, 0));

    auto listArea = b.withTrimmedTop(labelHeight);

    auto lo = jlo::VList()
                  .at(listArea.getX(), listArea.getY())
                  .withWidth(listArea.getWidth())
                  .withAutoGap(0);

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
                                 w->loadFromFile(result[0]);
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
                                     jf.replaceWithText(w->processor.toXML());
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
    if (bl.x + toolTip->getWidth() > getWidth() * 0.95)
    {
        bl.x -= toolTip->getWidth() - oc->getWidth();
    }
    toolTip->setTopLeftPosition(bl);
}

void ElfinMainPanel::updateToolTip(ElfinControllerAudioProcessor::float_param_t *p)
{
    assert(toolTip);

    using row_t = jcmp::ToolTip::Row;
    std::vector<row_t> rows;

    std::string title = p->desc.name;
    title += " (cc=" + std::to_string(p->desc.midiCC) + ")";
    row_t val;
    val.centerAlignText = "value=" + std::to_string(p->getCC());

    if (p->getCC() == p->desc.midiCCStart && !p->desc.midiCCStartLabel.empty())
        val.centerAlignText += " (" + p->desc.midiCCStartLabel + ")";
    if (p->getCC() == p->desc.midiCCEnd && !p->desc.midiCCEndLabel.empty())
        val.centerAlignText += " (" + p->desc.midiCCEndLabel + ")";

    val.centerIsMonospace = true;
    rows.push_back(val);
    toolTip->setTooltipTitleAndData(title, rows);
    toolTip->resetSizeFromData();
    toolTip->titleAlignment = juce::Justification::centredTop;
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
    if (files[0].endsWith(".syx"))
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
        loadFromFile(jf);
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

    st->setFont(
        jcmp::ToolTip::Styles::styleClass, jcmp::ToolTip::Styles::labelfont,
        st->getFont(jcmp::Label::Styles::styleClass, jcmp::Label::Styles::labelfont).boldened());
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

void ElfinMainPanel::initPatch()
{
    for (auto p : processor.params)
    {
        auto f = p->getFloatForCC(p->desc.midiCCDefault);
        p->setValueNotifyingHost(f);
    }
}

void ElfinMainPanel::loadFromFile(const juce::File &jf)
{
    if (jf.getFileExtension() == ".elfin")
    {
        auto s = jf.loadFileAsString().toStdString();
        processor.fromXML(s);
    }
    else if (jf.getFileExtension() == ".syx")
    {
        if (jf.getSize() != 108)
        {
            ELFLOG("Sysex file with wrong size");
            return;
        }
        juce::MemoryBlock mb;
        jf.loadFileAsData(mb);
        std::vector<uint8_t> data((uint8_t *)mb.getData(), (uint8_t *)mb.getData() + mb.getSize());
        processor.fromSYX(data);
    }
}

void ElfinMainPanel::loadFromFile(const fs::path &p)
{

    if (p.extension() == ".elfin")
    {
        std::ifstream file(p, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            ELFLOG("ERROR");
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        processor.fromXML(buffer.str());
    }
    if (p.extension() == ".syx")
    {
        try
        {
            auto sz = fs::file_size(p);
            ELFLOG("Size is " << sz);
            if (sz != 108)
                return;
        }
        catch (fs::filesystem_error &e)
        {
            ELFLOG("Cant stat syx file");
            return;
        }
        std::ifstream file(p, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            ELFLOG("ERROR");
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto s = buffer.str();
        std::vector<uint8_t> data(s.begin(), s.end());
        processor.fromSYX(data);
    }
}

void ElfinMainPanel::showElfinMainMenu()
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
    m.addItem("Init Patch",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->presetDataBinding->setValueFromGUI(0);
              });
    m.addSeparator();

    m.addSeparator();
    m.addItem("Resend Patch to MIDI",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  for (auto p : w->processor.params)
                      p->invalid = true;
              });
    m.addItem("Send All Notes Off",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->processor.sendAllNotesOff = true;
              });

    m.addSeparator();
    m.addItem("About",
              [w = juce::Component::SafePointer(this)]()
              {
                  if (!w)
                      return;
                  w->aboutScreen->showOver(w.getComponent());
              });
    m.addItem(
        "Source Code", []()
        { juce::URL("https://github.com/baconpaul/elfin-controller").launchInDefaultBrowser(); });

    m.addSeparator();
    auto vi = std::string() + sst::plugininfra::VersionInformation::git_implied_display_version;
    m.addItem("Version:", false, false, []() {});
    m.addItem(vi, false, false, []() {});
    m.addItem(sst::plugininfra::VersionInformation::git_commit_hash, false, false, []() {});
    m.addColumnBreak();
    m.addSectionHeader("Factory Presets");
    m.addSeparator();

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

    for (auto &[k, v] : presetManager->factoryPatchTree)
    {
        auto sub = juce::PopupMenu();
        for (auto &c : v)
        {
            auto lab = c.first;
            auto ps = lab.find(".elfin");
            if (ps != std::string::npos)
                lab = lab.substr(0, ps);
            sub.addItem(lab, mk(c.second));
        }
        m.addSubMenu(k, sub);
    }
    if (!presetManager->userPatches.empty())
    {
        m.addColumnBreak();
        m.addSectionHeader("User Presets");
        m.addSeparator();
    }

    for (auto &[k, v] : presetManager->userPatchTree)
    {
        if (k.empty())
        {
            for (auto &c : v)
            {
                m.addItem(c.first.replace_extension().u8string(), mk(c.second));
            }
        }
        else
        {
            auto sub = juce::PopupMenu();
            for (auto &c : v)
            {
                sub.addItem(c.first.filename().replace_extension().u8string(), mk(c.second));
            }
            m.addSubMenu(k.u8string(), sub);
        }
    }

    auto bd = presetButton->getBounds();
    auto where = bd.getBottomLeft();
    auto rec =
        juce::Rectangle<int>().withWidth(1).withHeight(1).withPosition(localPointToGlobal(where));
    m.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetScreenArea(rec));
}

} // namespace baconpaul::elfin_controller
