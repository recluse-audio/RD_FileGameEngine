#include "Zone.h"
#include "../SCENE/Scene.h"

Zone::Zone(Scene& scene, Bounds bounds, std::string zoneID, std::string target, std::string noteTarget, std::string label)
: mSceneID(scene.getSceneID())
, mBounds(bounds)
, mZoneID(zoneID)
, mTarget(target)
, mNoteTarget(noteTarget)
, mLabel(label)
{
}

bool Zone::containsPoint(int x, int y) const
{
    // Coarse AABB cull first.
    if (x < mBounds.mX || x > mBounds.mX + mBounds.mW ||
        y < mBounds.mY || y > mBounds.mY + mBounds.mH)
        return false;

    if (mPolygon.empty())
        return true; // rect-only zone

    // Ray-cast point-in-polygon test.
    bool inside = false;
    int  n      = (int)mPolygon.size();
    for (int i = 0, j = n - 1; i < n; j = i++)
    {
        int xi = mPolygon[i].first,  yi = mPolygon[i].second;
        int xj = mPolygon[j].first,  yj = mPolygon[j].second;
        if ((yi > y) != (yj > y) &&
            x < (xj - xi) * (float)(y - yi) / (yj - yi) + xi)
            inside = !inside;
    }
    return inside;
}
