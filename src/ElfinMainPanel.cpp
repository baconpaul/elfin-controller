//
// Created by Paul Walker on 3/28/25.
//

#include "ElfinMainPanel.h"

ElfinMainPanel::ElfinMainPanel(ElfinControllerAudioProcessor &p)
    : sst::jucegui::components::WindowPanel()
{
    sst::jucegui::style::StyleSheet::initializeStyleSheets([]() {});

    setStyle(sst::jucegui::style::StyleSheet::getBuiltInStyleSheet(
        sst::jucegui::style::StyleSheet::DARK));
}