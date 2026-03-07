/**
 * Made by Ryan Devens on 2026-03-07
 */

#pragma once
#include "GUISection.h"
#include <string>
#include <vector>

/**
 * A GUISection that renders a vertical list of scene friendly names.
 * Call setScenes() whenever the active level changes.
 */
class SceneListSection : public GUISection
{
public:
    using GUISection::GUISection;

    void setScenes(const std::vector<std::string>& names);
    void setSelectedIndex(int index);
    bool registerHit(int x, int y);

    void draw(GraphicsRenderer& renderer, bool showLabel = false) const override;

    int                             getSelectedIndex() const { return mSelectedIndex; }
    std::string                     getSelectedName()  const;
    const std::vector<std::string>& getScenes()        const { return mSceneNames; }

private:
    std::vector<std::string> mSceneNames;
    int                      mSelectedIndex = -1;

    static constexpr int k_RowHeight = 14;
    static constexpr int k_Padding   = 4;
};
