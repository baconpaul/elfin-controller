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

#include "PresetManager.h"
#include "sst/plugininfra/strnatcmp.h"

namespace baconpaul::elfin_controller
{
void PresetManager::recurseUserPresetFrom(const fs::path &p)
{
    if (fs::is_directory(p))
    {
        for (auto &el : fs::directory_iterator(p))
        {
            auto elp = el.path();
            if (elp.filename() == "." || elp.filename() == "..")
            {
                continue;
            }
            if (fs::is_directory(elp))
            {
                recurseUserPresetFrom(elp);
            }
            else if (fs::is_regular_file(elp) &&
                     (elp.extension() == ".elfin" || elp.extension() == ".syx"))
            {
                auto pushP = elp.lexically_relative(userPatchesPath);
                userPatches.push_back(pushP);
            }
        }
    }
}

void PresetManager::rescanUserPresets()
{
    userPatches.clear();
    try
    {
        recurseUserPresetFrom(userPatchesPath);
        std::sort(userPatches.begin(), userPatches.end(),
                  [](const fs::path &a, const fs::path &b)
                  {
                      auto appe = a.parent_path().empty();
                      auto bppe = b.parent_path().empty();

                      if (appe && bppe)
                      {
                          return strnatcasecmp(a.filename().u8string().c_str(),
                                               b.filename().u8string().c_str()) < 0;
                      }
                      else if (appe)
                      {
                          return true;
                      }
                      else if (bppe)
                      {
                          return false;
                      }
                      else
                      {
                          return a < b;
                      }
                  });
    }
    catch (fs::filesystem_error &)
    {
    }

    int32_t pidx = (int32_t)(1 + factoryPatchVector.size());
    for (auto &p : userPatches)
    {
        userPatchTree[p.parent_path()].push_back({p, pidx});
        pidx++;
    }

#if 0
    for (const auto &[k, v] : userPatchTree)
    {
        ELFLOG("K/V = '" << k.u8string() << "'");
        for (const auto &p : v)
        {
            ELFLOG("    - " << p.first.u8string() << " / " << p.second);
        }
    }
#endif
}

} // namespace baconpaul::elfin_controller