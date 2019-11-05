#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsAllyHealthLow : public AI::Condition
{
public:
    CONDITON_FACTORY(IsAllyHealthLow)

    explicit IsAllyHealthLow(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
