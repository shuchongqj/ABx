#pragma once

namespace Game {

class Actor;

namespace Components {

class CollisionComp
{
private:
    Actor& owner_;
public:
    explicit CollisionComp(Actor& owner) :
        owner_(owner)
    { }
    ~CollisionComp() = default;

    void DoCollisions();
};

}
}
