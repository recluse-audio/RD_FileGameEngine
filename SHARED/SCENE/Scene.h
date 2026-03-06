/**
 * Made by Ryan Devens on 2026-02-24
 */

#pragma once
#include <string>
#include <vector>
#include "../ZONE/Zone.h"

/**
 * This class defines the majority of the gameplay.
 *
 * Every scene carries two asset paths:
 *   primaryPath   - main content (PNG for locations, markdown for notes)
 *   secondaryPath - supplementary content (markdown summary for locations,
 *                   associated PNG for notes). May be empty.
 */
class Scene
{
public:
    Scene(std::string sceneID       = "",
          std::string parentID      = "Main",
          std::string name          = "",
          std::string primaryPath   = "",
          std::string secondaryPath = "");

    std::string getSceneID()        const;
    void setSceneID(std::string sceneID);
    std::string getParentSceneID()  const;
    std::string getName()           const;
    std::string getPrimaryPath()    const;
    std::string getSecondaryPath()  const;
    std::string getInterceptingZoneID(int x, int y)     const;
    std::string getInterceptingZoneTarget(int x, int y) const;
    bool        isRoot()                                const;
    void        setIsRoot(bool isRoot);
    bool        isDiscovered()                          const;
    void        setIsDiscovered(bool discovered);
    std::string getParentPath()                         const;
    void        setParentPath(std::string path);
    std::string getInterceptingZoneNoteTarget(int x, int y) const;
    std::string getNoteTarget() const;
    void        setNoteTarget(std::string path);

    void addChildScene(std::string childScene);
    void addZone(Zone zone);
    std::vector<std::string> getChildScenes() const;
    std::vector<Zone>        getZones()       const;
private:
    std::string mSceneID        = "";
    std::string mParentSceneID  = "";
    std::string mName           = "";
    std::string mPrimaryPath    = ""; // PNG for locations, markdown for notes
    std::string mSecondaryPath  = ""; // markdown for locations, PNG for notes (optional)
    std::string mNoteTarget     = ""; // data-root-relative path of the note .md to append to on discovery
    bool        mIsRoot         = false;
    bool        mIsDiscovered   = false;
    std::string mParentPath     = "";
    std::vector<std::string> mChildScenes;
    std::vector<Zone> mZones;
};
