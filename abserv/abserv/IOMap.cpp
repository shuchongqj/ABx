#include "stdafx.h"
#include "IOMap.h"
#include "DataProvider.h"
#include <pugixml.hpp>
#include "Logger.h"
#include "CollisionShape.h"
#include "GameObject.h"
#include "TerrainPatch.h"
#include "Profiler.h"
#include "Subsystems.h"
#include "Model.h"
#include "Game.h"
#include "FileUtils.h"

namespace IO {
namespace IOMap {

static bool LoadSceneNode(Game::Map& map, const pugi::xml_node& node)
{
    auto* dataProvider = GetSubsystem<IO::DataProvider>();
    // Game load thread
    if (auto game = map.GetGame())
    {
        std::mutex loadNodeLock;

        Math::Vector3 pos;
        Math::Vector3 scale = Math::Vector3::One;
        Math::Quaternion rot = Math::Quaternion::Identity;
        std::string name;
        std::string group;
        bool isSpawnPoint = false;
        for (const auto& attr : node.children("attribute"))
        {
            const pugi::xml_attribute& name_attr = attr.attribute("name");
            const pugi::xml_attribute& value_attr = attr.attribute("value");
            const size_t name_hash = sa::StringHashRt(name_attr.as_string());
            switch (name_hash)
            {
            case AttrPosition:
                pos = Math::Vector3(value_attr.as_string());
                break;
            case AttrRotation:
                rot = Math::Quaternion(value_attr.as_string()).Normal();
                break;
            case AttrScale:
                scale = Math::Vector3(value_attr.as_string());
                break;
            case AttrName:
                isSpawnPoint = strcmp(value_attr.as_string(), "SpawnPoint") == 0;
                name = value_attr.as_string();
                break;
            case AttrVariables:
            {
                for (const auto& var : attr)
                {
                    if (var.attribute("hash").as_ullong() == VarGroup)
                    {
                        group = var.attribute("value").as_string();
                        break;
                    }
                }
                break;
            }
            default:
                continue;
            }
        }

        if (isSpawnPoint)
        {
#ifdef DEBUG_GAME
            //            LOG_DEBUG << "Spawn point: " << group << "; Pos: " << pos.ToString() << std::endl;
#endif
            // This node is a spawn point. No need to do anything more.
            map.spawnPoints_.push_back({ pos, rot, group });
            return true;
        }

        std::shared_ptr<Game::Model> model;
        std::shared_ptr<Game::GameObject> object;
        Math::Vector3 size = Math::Vector3::One;
        Math::Vector3 offset = Math::Vector3::Zero;
        Math::Quaternion offsetRot = Math::Quaternion::Identity;
        // If we have a rigid body collide by default with everything. That's also Urho3Ds default.
        uint32_t colisionMask = 0xFFFFFFFF;
        for (const auto& comp : node.children("component"))
        {
            const pugi::xml_attribute& type_attr = comp.attribute("type");
            const size_t type_hash = sa::StringHashRt(type_attr.as_string());
            // StaticModel must be first component
            switch (type_hash)
            {
            case AttrStaticModel:
            {
                object = std::make_shared<Game::GameObject>();
                object->SetName(name);
                object->collisionMask_ = colisionMask;
                object->transformation_ = Math::Transformation(pos, scale, rot);
                {
                    std::lock_guard<std::mutex> lock(loadNodeLock);
                    game->AddObjectInternal(object);
                }
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = sa::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case AttrModel:
                    {
                        std::string modelValue = value_attr.as_string();
                        std::vector<std::string> modelFile = Utils::Split(modelValue, ";");
                        if (modelFile.size() == 2)
                        {
                            if (dataProvider->Exists<Game::Model>(modelFile[1]))
                                // Model is optional
                                model = dataProvider->GetAsset<Game::Model>(modelFile[1]);
#ifdef DEBUG_COLLISION
                            if (model)
                                LOG_DEBUG << model->fileName_ << ": " << model->GetBoundingBox().ToString() << std::endl;
#endif
                        }
                        break;
                    }
                    case AttrIsOccluder:
                        object->occluder_ = value_attr.as_bool();
                        break;
                    case AttrIsOccludee:
                        object->occludee_ = value_attr.as_bool();
                        break;
                    }
                }
                break;
            }
            case AttrCollisionShape:
            {
                size_t coll_shape = AttrCollisionShapeTypeBox;
                if (object)
                {
                    for (const auto& attr : comp.children())
                    {
                        const pugi::xml_attribute& name_attr = attr.attribute("name");
                        const size_t name_hash = sa::StringHashRt(name_attr.as_string());
                        const pugi::xml_attribute& value_attr = attr.attribute("value");
                        const size_t value_hash = sa::StringHashRt(value_attr.as_string());
                        switch (name_hash)
                        {
                        case AttrSize:
                            size = Math::Vector3(value_attr.as_string());
                            break;
                        case AttrOffsetPos:
                            offset = Math::Vector3(value_attr.as_string());
                            break;
                        case AttrOffsetRot:
                            offsetRot = Math::Quaternion(value_attr.as_string()).Normal();
                            break;
                        case AttrShapeType:
                        {
                            coll_shape = value_hash;
                        }
                        }
                    }

                    if (object && !object->GetCollisionShape())
                    {
                        if (coll_shape == AttrCollisionShapeTypeTriangleMesh ||
                            coll_shape == AttrCollisionShapeTypeConvexHull ||
                            coll_shape == AttrCollisionShapeTypeCapsule)
                        {
                            if (model)
                            {
#ifdef DEBUG_COLLISION
                                LOG_DEBUG << "Setting ConvexHull collision shape for " << object->GetName() << std::endl;
#endif
                                object->SetCollisionShape(
                                    std::make_unique<Math::CollisionShape<Math::ConvexHull>>(
                                        Math::ShapeType::ConvexHull, model->shape_->vertexData_)
                                );
                            }
                        }
                        else if (coll_shape == AttrCollisionShapeTypeBox && size != Math::Vector3::Zero)
                        {
                            // The object has the scaling.
                            const Math::Vector3 halfSize = (size * 0.5f);
                            Math::BoundingBox bb(-halfSize + offset, halfSize + offset);
                            // Add Node and Offset rotation -> absolute orientation
                            bb.orientation_ = rot * offsetRot;
                            // Object has then no rotation
                            object->transformation_.oriention_ = Math::Quaternion::Identity;
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting BB collision shape for " << object->GetName() <<
                                " min/max " << bb.ToString() <<
                                ", size +/- " << halfSize.ToString() << ", orientation " << bb.orientation_.ToString() << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShape<Math::BoundingBox>>(
                                    Math::ShapeType::BoundingBox, bb)
                            );
                        }
                        else if ((coll_shape == AttrCollisionShapeTypeSphere || coll_shape == AttrCollisionShapeTypeCylinder) &&
                            size != Math::Vector3::Zero)
                        {
                            // The object has the scaling.
                            float radius = size.x_ * 0.5f;
                            Math::Sphere sphere(offset, radius);
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting Sphere collision shape for " << object->GetName() <<
                                " Center " << offset.ToString() <<
                                ", Radius " << radius << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShape<Math::Sphere>>(
                                    Math::ShapeType::Sphere, sphere)
                            );
                        }
                        else if (model)
                        {
                            // If none of the above set to model bounding box
                            auto bb = model->GetBoundingBox();
                            bb.orientation_ = rot * offsetRot;
                            // Object has then no rotation
                            object->transformation_.oriention_ = Math::Quaternion::Identity;
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting BB collision shape for " << object->GetName() <<
                                " to model BB " << model->GetBoundingBox().ToString() << " orientation " << bb.orientation_.ToString() << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShape<Math::BoundingBox>>(
                                    Math::ShapeType::BoundingBox, bb)
                            );
                        }
                    }
                }
                break;
            }
            case AttrRigidBody:
            {
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = sa::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case AttrCollisionMask:
                        colisionMask = value_attr.as_uint();
                        if (object)
                            object->collisionMask_ = colisionMask;
                        break;
                    }
                }
                break;
            }
            case AttrTerrain:
            {
                assert(map.terrain_);
                map.terrain_->transformation_ = Math::Transformation(pos, scale, rot);
                map.terrain_->GetHeightMap()->matrix_ = map.terrain_->transformation_.GetMatrix();
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = sa::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case AttrVertexSpacing:
                        map.terrain_->GetHeightMap()->spacing_ = Math::Vector3(value_attr.as_string());
                        break;
                    }
                }
            }
            }
        }
        return true;
    }
    LOG_ERROR << "The map does not have a game object" << std::endl;
    return false;
}

static bool LoadScene(Game::Map& map, const std::string& name)
{
    // Game load thread
    const std::string file = GetSubsystem<IO::DataProvider>()->GetDataFile(Utils::AddSlash(map.data_.directory) + name);
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
    {
        LOG_ERROR << "Error loading file " << file << ": " << result.description() << std::endl;
        return false;
    }
    const pugi::xml_node& scene_node = doc.child("scene");
    if (!scene_node)
    {
        LOG_ERROR << "File " << file << " does not have a scene node" << std::endl;
        return false;
    }

    for (pugi::xml_node_iterator it = scene_node.begin(); it != scene_node.end(); ++it)
    {
        if (strcmp((*it).name(), "node") == 0)
        {
            if (!LoadSceneNode(map, *it))
            {
                LOG_ERROR << "Error loading scene node" << std::endl;
                // Can't continue
                return false;
            }
        }
    }

    // Update spawn points
    for (auto& p : map.spawnPoints_)
    {
        p.position.y_ = map.terrain_->GetHeight(p.position);
    }

#ifdef DEBUG_GAME
//    LOG_DEBUG << "Spawn points: " << map.spawnPoints_.size() << std::endl;
#endif
    return true;
}

bool Load(Game::Map& map)
{
    AB_PROFILE;
    // Game load thread
    std::string file = GetSubsystem<IO::DataProvider>()->GetDataFile(Utils::AddSlash(map.data_.directory) + "index.xml");
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
    {
        LOG_ERROR << "Error loading file " << file << ": " << result.description() << std::endl;
        return false;
    }
    const pugi::xml_node& index_node = doc.child("index");
    if (!index_node)
    {
        LOG_ERROR << "File " << file << " does not have an index node" << std::endl;
        return false;
    }

    auto dataProv = GetSubsystem<IO::DataProvider>();
    std::string sceneFile;
    std::string navMeshFile;
    std::string terrainFile;

    for (const auto& file_node : index_node.children("file"))
    {
        const pugi::xml_attribute& type_attr = file_node.attribute("type");
        const size_t type_hash = sa::StringHashRt(type_attr.as_string());
        const pugi::xml_attribute& src_attr = file_node.attribute("src");
        switch (type_hash)
        {
        case FileTypeScene:
            sceneFile = src_attr.as_string();
            break;
        case FileTypeNavmesh:
        {
            navMeshFile = src_attr.as_string();
            break;
        }
        case FileTypeTerrain:
            terrainFile = src_attr.as_string();
            break;
        }
    }

    if (sceneFile.empty())
    {
        LOG_ERROR << "Map file " << file << " does not contain a scene" << std::endl;
        return false;
    }
    if (navMeshFile.empty())
    {
        LOG_ERROR << "Map file " << file << " does not contain a nav mesh" << std::endl;
        return false;
    }
    if (terrainFile.empty())
    {
        LOG_ERROR << "Map file " << file << " does not contain a terrain" << std::endl;
        return false;
    }

    // Before scene
    map.terrain_ = dataProv->GetAsset<Game::Terrain>(Utils::AddSlash(map.data_.directory) + terrainFile);
    if (!map.terrain_)
    {
        LOG_ERROR << "Error loading terrain " << terrainFile << std::endl;
        return false;
    }

    map.navMesh_ = dataProv->GetAsset<Navigation::NavigationMesh>(Utils::AddSlash(map.data_.directory) + navMeshFile);
    if (!map.navMesh_)
    {
        LOG_ERROR << "Error loading nav mesh " << navMeshFile << std::endl;
        return false;
    }
    if (!LoadScene(map, sceneFile))
    {
        LOG_ERROR << "Error loading scene " << navMeshFile << std::endl;
        return false;
    }

    map.CreatePatches();
    // TODO: Make TerrainPatches part of Game::Map (Not Terrain)
    // After loading the Scene add terrain patches as game objects
    for (size_t i = 0; i < map.GetPatchesCount(); ++i)
    {
        // We need a copy of that
        Game::TerrainPatch* patch = map.GetPatch(static_cast<unsigned>(i));
        map.AddGameObject(patch->GetThis<Game::TerrainPatch>());
    }

    return true;
}

}
}
