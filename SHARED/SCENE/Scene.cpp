#include "Scene.h"

Scene::Scene(std::string sceneID, std::string parentID, std::string name,
             std::string primaryPath, std::string secondaryPath)
: mSceneID(sceneID)
, mParentSceneID(parentID)
, mName(name)
, mPrimaryPath(primaryPath)
, mSecondaryPath(secondaryPath)
{
}

std::string Scene::getSceneID() const        { return mSceneID; }
std::string Scene::getParentSceneID() const  { return mParentSceneID; }
std::string Scene::getName() const           { return mName; }
std::string Scene::getPrimaryPath() const    { return mPrimaryPath; }
std::string Scene::getSecondaryPath() const  { return mSecondaryPath; }

void Scene::setSceneID(std::string sceneID)  { mSceneID = sceneID; }

void Scene::addChildScene(std::string childScene)
{
    mChildScenes.push_back(childScene);
}

void Scene::addZone(Zone zone)
{
    mZones.push_back(zone);
}

std::vector<std::string> Scene::getChildScenes() const { return mChildScenes; }
std::vector<Zone>        Scene::getZones()        const { return mZones; }

std::string Scene::getInterceptingZoneID(int x, int y) const
{
    for (auto zone : mZones)
    {
        if (zone.containsPoint(x, y))
            return zone.getZoneID();
    }
    return "";
}

bool Scene::isRoot()                    const { return mIsRoot; }
void Scene::setIsRoot(bool isRoot)            { mIsRoot = isRoot; }
bool Scene::isDiscovered()              const { return mIsDiscovered; }
void Scene::setIsDiscovered(bool d)           { mIsDiscovered = d; }
std::string Scene::getParentPath()      const { return mParentPath; }
void Scene::setParentPath(std::string path)   { mParentPath = path; }
std::string Scene::getNoteTarget()      const { return mNoteTarget; }
void Scene::setNoteTarget(std::string path)   { mNoteTarget = path; }

std::string Scene::getInterceptingZoneNoteTarget(int x, int y) const
{
    for (auto zone : mZones)
        if (zone.containsPoint(x, y))
            return zone.getNoteTarget();
    return "";
}

std::string Scene::getInterceptingZoneTarget(int x, int y) const
{
    for (auto zone : mZones)
    {
        if (zone.containsPoint(x, y))
            return zone.getTarget();
    }
    return "";
}
