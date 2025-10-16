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

#ifndef ELFIN_CONTROLLER_ELFINMAINPANEL_H
#define ELFIN_CONTROLLER_ELFINMAINPANEL_H

#include <map>

#include "filesystem/import.h"

#include "sst/jucegui/components/WindowPanel.h"
#include "sst/jucegui/components/Knob.h"
#include "sst/jucegui/components/Label.h"
#include "sst/jucegui/components/MenuButton.h"
#include "sst/jucegui/components/ToolTip.h"
#include "sst/jucegui/components/GlyphButton.h"
#include "sst/jucegui/components/JogUpDownButton.h"
#include "sst/jucegui/data/Continuous.h"
#include "sst/jucegui/style/JUCELookAndFeelAdapter.h"
#include "ElfinProcessor.h"
#include "ElfinAbout.h"
#include "PresetManager.h"

namespace baconpaul::elfin_controller
{
static constexpr float uiScale{1.2};

struct FilterPanel;
struct OscPanel;
struct EGPanel;
struct LFOPanel;
struct ModPanel;
struct SettingsPanel;

struct PresetButton;

struct ParamSource;
struct DiscreteParamSource;

struct ElfinMainPanel : sst::jucegui::components::WindowPanel, juce::FileDragAndDropTarget
{
    ElfinControllerAudioProcessor &processor;
    ElfinMainPanel(ElfinControllerAudioProcessor &);
    ~ElfinMainPanel();

    void showElfinMainMenu();

    std::vector<std::unique_ptr<sst::jucegui::data::Continuous>> otherSources;
    std::vector<std::unique_ptr<sst::jucegui::data::Discrete>> otherDiscrete;
    std::map<ElfinControl, std::unique_ptr<ParamSource>> sources;
    std::map<ElfinControl, std::unique_ptr<DiscreteParamSource>> discreteSources;
    std::map<ElfinControl, std::unique_ptr<juce::Component>> widgets;
    std::map<ElfinControl, std::unique_ptr<sst::jucegui::components::Label>> widgetLabels;

    std::unique_ptr<FilterPanel> filterPanel;
    std::unique_ptr<OscPanel> oscPanel;
    std::unique_ptr<EGPanel> egPanel;
    std::unique_ptr<LFOPanel> lfoPanel;
    std::unique_ptr<ModPanel> modPanel;
    std::unique_ptr<SettingsPanel> settingsPanel;

    std::unique_ptr<sst::jucegui::components::ToolTip> toolTip;
    void showToolTip(ElfinControllerAudioProcessor::float_param_t *, juce::Component *);
    void updateToolTip(ElfinControllerAudioProcessor::float_param_t *);
    void hideToolTip();
    int hideToolTipIn{-1};

    void diceMenu();

    std::unique_ptr<juce::Component> elfinLogo, hideawayLogo;

    std::unique_ptr<sst::jucegui::style::LookAndFeelManager> lnf;

    void onIdle();

    void resized() override;

    std::unique_ptr<juce::Timer> timer;
    int lastLogSize{0};

    static constexpr int tooltipDelayInMS{250};

    std::unique_ptr<juce::FileChooser> fileChooser;
    void savePatch(), loadPatch(), setupUserPath(), initPatch();
    void loadFromFile(const fs::path &p), loadFromFile(const juce::File &);
    fs::path userPath;
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<PresetDataBinding> presetDataBinding;
    std::unique_ptr<PresetButton> presetButton;

    std::unique_ptr<ElfinAbout> aboutScreen;

    bool isInterestedInFileDrag(const juce::StringArray &files) override;
    void filesDropped(const juce::StringArray &files, int x, int y) override;

    void setupStyle();
};
} // namespace baconpaul::elfin_controller
#endif // ELFINMAINPANEL_H
