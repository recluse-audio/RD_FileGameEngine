#include "SceneFactory.h"
#include "../ZONE/Zone.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

SceneFactory::SceneFactory(bool useHires)
: mUseHires(useHires)
{
}

std::unique_ptr<Scene> SceneFactory::build(const std::string& jsonString)
{
    json j = json::parse(jsonString, nullptr, false); // false = no exceptions
    if (j.is_discarded())
        return std::make_unique<Scene>();

    std::string sceneID       = j.value("id",             "");
    std::string parentID      = j.value("parent",         "Main");
    std::string name          = j.value("name",           "");
    std::string primaryPath   = mUseHires ? j.value("hires_image_path", "")
                                          : j.value("lores_image_path",  "");
    if (primaryPath.empty())
        primaryPath = j.value("lores_image_path", "");
    std::string secondaryPath = j.value("secondary_path", "");

    auto scene = std::make_unique<Scene>(sceneID, parentID, name, primaryPath, secondaryPath);
    scene->setIsRoot(j.value("isRoot", false));
    scene->setIsDiscovered(j.value("isDiscovered", false));
    scene->setParentPath(j.value("parent_path", ""));
    scene->setNoteTarget(j.value("notePath", ""));

    float hiresScaleX = 1.0f, hiresScaleY = 1.0f;
    if (mUseHires && j.contains("hires_canvas") && j["hires_canvas"].is_object())
    {
        float cw = j["hires_canvas"].value("width",  320.0f);
        float ch = j["hires_canvas"].value("height", 240.0f);
        hiresScaleX = 320.0f / cw;
        hiresScaleY = 240.0f / ch;
    }

    if (j.contains("zones") && j["zones"].is_array())
    {
        for (auto& z : j["zones"])
        {
            const char* pointsKey = (mUseHires && z.contains("hires_points")) ? "hires_points" : "points";
            float scaleX = (pointsKey[0] == 'h') ? hiresScaleX : 1.0f;
            float scaleY = (pointsKey[0] == 'h') ? hiresScaleY : 1.0f;
            if (z.contains(pointsKey) && z[pointsKey].is_array() && !z[pointsKey].empty())
            {
                Zone::Polygon poly;
                int minX = 320, minY = 240, maxX = 0, maxY = 0;
                for (auto& pt : z[pointsKey])
                {
                    int px = (int)(pt[0].get<float>() * scaleX);
                    int py = (int)(pt[1].get<float>() * scaleY);
                    poly.push_back({px, py});
                    if (px < minX) minX = px;
                    if (py < minY) minY = py;
                    if (px > maxX) maxX = px;
                    if (py > maxY) maxY = py;
                }
                Zone::Bounds bounds(minX, minY, maxX - minX, maxY - minY);
                Zone zone(*scene, bounds,
                          z.value("id", ""),
                          z.value("target", ""),
                          z.value("noteTarget", ""),
                          z.value("label", ""));
                zone.setPolygon(std::move(poly));
                scene->addZone(std::move(zone));
            }
            else
            {
                scene->addZone(Zone(*scene,
                                   Zone::Bounds(z.value("x",      0),
                                                z.value("y",      0),
                                                z.value("width",  0),
                                                z.value("height", 0)),
                                   z.value("id", ""),
                                   z.value("target", ""),
                                   z.value("noteTarget", ""),
                                   z.value("label", "")));
            }
        }
    }

    return scene;
}
