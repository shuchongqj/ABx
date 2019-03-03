#pragma once

namespace Game {

class Actor;

namespace Components {

class AttackComp
{
private:
    Actor& owner_;
    bool attacking_;
    int64_t lastAttackTime_;
public:
    explicit AttackComp(Actor& owner) :
        owner_(owner),
        attacking_(false),
        lastAttackTime_(0)
    { }
    ~AttackComp() = default;
    void Update(uint32_t timeElapsed);
    bool IsAttacking() const { return attacking_; }
    void Cancel();
    int64_t GetLastAttackTime() const { return lastAttackTime_; }
};

}
}
