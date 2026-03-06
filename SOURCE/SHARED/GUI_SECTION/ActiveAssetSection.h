/**
 * Made by Ryan Devens on 2026-03-06
 */

#pragma once
#include "GUISection.h"
#include <map>
#include <string>

/**
 * A GUISection that renders the currently selected asset.
 *
 * Call setLevelAssets() whenever the active level changes to supply the
 * path→friendlyName map for that level. Call setActiveAsset() with a
 * friendly name to select which asset to display; the section resolves
 * the corresponding file path internally and dispatches draw() to the
 * appropriate GraphicsRenderer method based on the file extension
 * (.png → drawImage, .svg → drawSVG, everything else → drawText).
 */
class ActiveAssetSection : public GUISection
{
public:
    using GUISection::GUISection;

    /** Replace the asset catalogue for the current level and clear the active path. */
    void setLevelAssets(const std::map<std::string, std::string>& assets);

    /**
     * Find the file path whose friendly name matches and store it as the
     * active path. Clears the active path if friendlyName is empty or not found.
     */
    void setActiveAsset(const std::string& friendlyName);

    void draw(GraphicsRenderer& renderer, bool showLabel = false) const override;

    /** The resolved file path for the currently active asset, or empty if none. */
    std::string getActivePath() const { return mActivePath; }

private:
    std::map<std::string, std::string> mLevelAssets; // path → friendlyName
    std::string                        mActivePath;
};
