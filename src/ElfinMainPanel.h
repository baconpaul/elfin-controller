//
// Created by Paul Walker on 3/28/25.
//

#ifndef ELFINMAINPANEL_H
#define ELFINMAINPANEL_H

#include "sst/jucegui/components/WindowPanel.h"
#include "ElfinProcessor.h"
struct ElfinMainPanel : sst::jucegui::components::WindowPanel
{
    ElfinMainPanel(ElfinControllerAudioProcessor &);
};
#endif // ELFINMAINPANEL_H
