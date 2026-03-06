/**
 * Made by Ryan Devens on 2026-02-24
 */

#pragma once
#include <string>
#include <utility>
#include <vector>

class Scene; // forward declaration - only a reference is needed
/**
 * Hit area (bounds) within an associated scene
 */
class Zone
{
public:
    // int rectangle class essentially for this zone
    class Bounds
    {
    public:
        int mX = 0;
        int mY = 0;
        int mW = 0;
        int mH = 0;

        Bounds(int x, int y, int w, int h)
        : mX(x)
        , mY(y)
        , mW(w)
        , mH(h)
        {
        }

        Bounds(const Bounds& other)
        : mX(other.mX)
        , mY(other.mY)
        , mW(other.mW)
        , mH(other.mH)
        {
        }
    };

    using Point   = std::pair<int, int>;
    using Polygon = std::vector<Point>;

    Zone(Scene& scene, Bounds bounds, std::string zoneID = "Default Zone ID", std::string target = "", std::string noteTarget = "", std::string label = "");

    bool containsPoint(int x, int y) const;

    void           setPolygon(Polygon polygon) { mPolygon = std::move(polygon); }
    bool           hasPolygon()          const { return !mPolygon.empty(); }
    const Polygon& getPolygon()          const { return mPolygon; }

    const std::string getZoneID()     const { return mZoneID; }
    const std::string getSceneID()    const { return mSceneID; }
    const std::string getTarget()     const { return mTarget; }
    const std::string getNoteTarget() const { return mNoteTarget; }
    const std::string getLabel()      const { return mLabel; }
    Bounds            getBounds()     const { return mBounds; }
private:
    std::string mSceneID;    // ID of the scene this zone belongs to
    Bounds      mBounds;
    Polygon     mPolygon;    // optional; if non-empty, used for hit-testing instead of mBounds
    std::string mZoneID     = "";
    std::string mTarget     = ""; // data-root-relative path of the scene to navigate to on hit
    std::string mNoteTarget = ""; // data-root-relative path of the note to mark discovered on hit (no navigation)
    std::string mLabel      = ""; // display text for programmatically drawn menu buttons
};
