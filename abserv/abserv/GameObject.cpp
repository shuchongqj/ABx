#include "stdafx.h"
#include "GameObject.h"
#include "Game.h"
#include "Logger.h"
#include "ConvexHull.h"

#include "DebugNew.h"

namespace Game {

uint32_t GameObject::objectIds_ = 0;

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetId", &GameObject::GetId)
        .addFunction("GetGame", &GameObject::GetGame)
        .addFunction("GetName", &GameObject::GetName)
        .addFunction("GetCollisionMask", &GameObject::GetCollisionMask)
        .addFunction("SetCollisionMask", &GameObject::SetCollisionMask)
    );
}

GameObject::GameObject() :
    octant_(nullptr),
    sortValue_(0.0f),
    occludee_(true),
    occluder_(false),
    collisionMask_(0xFFFFFFFF)    // Collides with all by default
{
    id_ = GetNewId();
}

GameObject::~GameObject()
{
    RemoveFromOctree();
}

bool GameObject::Collides(GameObject* other, Math::Vector3& move) const
{
    if (!collisionShape_ || !other->GetCollisionShape())
        return false;

    switch (other->GetCollisionShape()->shapeType_)
    {
    case Math::ShapeTypeBoundingBox:
    {
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = (BBoxShape*)other->GetCollisionShape();
        const Math::BoundingBox bbox = shape->shape_.Transformed(other->transformation_.GetMatrix());
        return collisionShape_->Collides(transformation_.GetMatrix(), bbox, move);
    }
    case Math::ShapeTypeSphere:
    {
        using SphereShape = Math::CollisionShapeImpl<Math::Sphere>;
        SphereShape* shape = (SphereShape*)other->GetCollisionShape();
        const Math::Sphere sphere = shape->shape_.Transformed(other->transformation_.GetMatrix());
        return collisionShape_->Collides(transformation_.GetMatrix(), sphere, move);
    }
    case Math::ShapeTypeConvexHull:
    {
        using HullShape = Math::CollisionShapeImpl<Math::ConvexHull>;
        HullShape* shape = (HullShape*)other->GetCollisionShape();
        const Math::ConvexHull hull = shape->shape_.Transformed(other->transformation_.GetMatrix());
        return collisionShape_->Collides(transformation_.GetMatrix(), hull, move);
    }
    }
    return false;
}

void GameObject::ProcessRayQuery(const Math::RayOctreeQuery& query, std::vector<Math::RayQueryResult>& results)
{
    float distance = query.ray_.HitDistance(GetWorldBoundingBox());
    if (distance < query.maxDistance_)
    {
        Math::RayQueryResult result;
        result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
        result.normal_ = -query.ray_.direction_;
        result.distance_ = distance;
        result.object_ = this;
        results.push_back(result);
    }
}

bool GameObject::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(GetType());
    stream.WriteString(GetName());
    return true;
}

bool GameObject::QueryObjects(std::vector<GameObject*>& result, float radius)
{
    if (!octant_)
        return false;

    Math::Sphere sphere(transformation_.position_, radius);
    Math::SphereOctreeQuery query(result, sphere);
    Math::Octree* octree = octant_->GetRoot();
    octree->GetObjects(query);
    return true;
}

bool GameObject::QueryObjects(std::vector<GameObject*>& result, const Math::BoundingBox& box)
{
    if (!octant_)
        return false;

    Math::BoxOctreeQuery query(result, box);
    Math::Octree* octree = octant_->GetRoot();
    octree->GetObjects(query);
    return true;
}

void GameObject::AddToOctree()
{
    if (auto g = game_.lock())
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Adding " << GetName() << " to Octree" << std::endl;
#endif
        g->map_->octree_->InsertObject(this);
    }
}

void GameObject::RemoveFromOctree()
{
    if (octant_)
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Removing " << GetName() << " from Octree" << std::endl;
#endif
        octant_->RemoveObject(this);
    }
}

}
