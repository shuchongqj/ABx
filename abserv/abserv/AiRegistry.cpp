/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "AiRegistry.h"
#include "actions/AiAttackSelection.h"
#include "actions/AiDie.h"
#include "actions/AiFlee.h"
#include "actions/AiFollow.h"
#include "actions/AiGainEnergy.h"
#include "actions/AiGoHome.h"
#include "actions/AiHealOther.h"
#include "actions/AiHealSelf.h"
#include "actions/AiIdle.h"
#include "actions/AiInterrupt.h"
#include "actions/AiMoveOutAOE.h"
#include "actions/AiMoveTo.h"
#include "actions/AiResurrectSelection.h"
#include "actions/AiSay.h"
#include "actions/AiUseDamageSkill.h"
#include "actions/AiWander.h"
#include "actions/AiMoveIntoSkillRange.h"
#include "conditions/AiHaveHome.h"
#include "conditions/AiHaveWanderRoute.h"
#include "conditions/AiHaveWeapon.h"
#include "conditions/AiIsAllyHealthCritical.h"
#include "conditions/AiIsAllyHealthLow.h"
#include "conditions/AiIsAttacked.h"
#include "conditions/AiIsCloseToSelection.h"
#include "conditions/AiIsEnergyLow.h"
#include "conditions/AiIsFighting.h"
#include "conditions/AiIsInAOE.h"
#include "conditions/AiIsInSkillRange.h"
#include "conditions/AiIsInWeaponRange.h"
#include "conditions/AiIsMeleeTarget.h"
#include "conditions/AiIsSelectionAlive.h"
#include "conditions/AiIsSelfHealthCritical.h"
#include "conditions/AiIsSelfHealthLow.h"
#include "conditions/AiRandomCondition.h"
#include "conditions/AiHaveResurrection.h"
#include "conditions/AiHaveSkill.h"
#include "filters/AiSelectAggro.h"
#include "filters/AiSelectAttackTarget.h"
#include "filters/AiSelectAttackers.h"
#include "filters/AiSelectDeadAllies.h"
#include "filters/AiSelectGettingDamage.h"
#include "filters/AiSelectLowHealth.h"
#include "filters/AiSelectMob.h"
#include "filters/AiSelectRandom.h"
#include "filters/AiSelectTargetAttacking.h"
#include "filters/AiSelectTargetUsingSkill.h"
#include "filters/AiSelectVisible.h"
#include "filters/AiSelectWithEffect.h"
#include "filters/AiSortByDistance.h"

namespace AI {

AiRegistry::AiRegistry() :
    Registry()
{ }

void AiRegistry::Initialize()
{
    Registry::Initialize();

    // We don't use Zones
    UnregisterFilterFactory("Zone");

    RegisterNodeFactory("GoHome", Actions::GoHome::GetFactory());
    RegisterNodeFactory("Die", Actions::Die::GetFactory());
    RegisterNodeFactory("Idle", Actions::Idle::GetFactory());
    RegisterNodeFactory("AttackSelection", Actions::AttackSelection::GetFactory());
    RegisterNodeFactory("HealOther", Actions::HealOther::GetFactory());
    RegisterNodeFactory("HealSelf", Actions::HealSelf::GetFactory());
    RegisterNodeFactory("Say", Actions::Say::GetFactory());
    RegisterNodeFactory("MoveTo", Actions::MoveTo::GetFactory());
    RegisterNodeFactory("ResurrectSelection", Actions::ResurrectSelection::GetFactory());
    RegisterNodeFactory("GainEnergy", Actions::GainEnergy::GetFactory());
    RegisterNodeFactory("Flee", Actions::Flee::GetFactory());
    RegisterNodeFactory("MoveOutAOE", Actions::MoveOutAOE::GetFactory());
    RegisterNodeFactory("Wander", Actions::Wander::GetFactory());
    RegisterNodeFactory("UseDamageSkill", Actions::UseDamageSkill::GetFactory());
    RegisterNodeFactory("Interrupt", Actions::Interrupt::GetFactory());
    RegisterNodeFactory("Follow", Actions::Follow::GetFactory());
    RegisterNodeFactory("MoveIntoSkillRange", Actions::MoveIntoSkillRange::GetFactory());

    RegisterConditionFactory("IsSelectionAlive", Conditions::IsSelectionAlive::GetFactory());
    RegisterConditionFactory("IsCloseToSelection", Conditions::IsCloseToSelection::GetFactory());
    RegisterConditionFactory("IsSelfHealthLow", Conditions::IsSelfHealthLow::GetFactory());
    RegisterConditionFactory("SelfHealthCritical", Conditions::IsSelfHealthCritical::GetFactory());
    RegisterConditionFactory("IsAllyHealthLow", Conditions::IsAllyHealthLow::GetFactory());
    RegisterConditionFactory("AllyHealthCritical", Conditions::IsAllyHealthCritical::GetFactory());
    RegisterConditionFactory("IsAttacked", Conditions::IsAttacked::GetFactory());
    RegisterConditionFactory("IsEnergyLow", Conditions::IsEnergyLow::GetFactory());
    RegisterConditionFactory("IsInAOE", Conditions::IsInAOE::GetFactory());
    RegisterConditionFactory("IsMeleeTarget", Conditions::IsMeleeTarget::GetFactory());
    RegisterConditionFactory("IsInSkillRange", Conditions::IsInSkillRange::GetFactory());
    RegisterConditionFactory("HaveHome", Conditions::HaveHome::GetFactory());
    RegisterConditionFactory("HaveWanderRoute", Conditions::HaveWanderRoute::GetFactory());
    RegisterConditionFactory("Random", Conditions::RandomCondition::GetFactory());
    RegisterConditionFactory("IsInWeaponRange", Conditions::IsInWeaponRange::GetFactory());
    RegisterConditionFactory("HaveWeapon", Conditions::HaveWeapon::GetFactory());
    RegisterConditionFactory("IsFighting", Conditions::IsFighting::GetFactory());
    RegisterConditionFactory("HaveResurrection", Conditions::HaveResurrection::GetFactory());
    RegisterConditionFactory("HaveSkill", Conditions::HaveSkill::GetFactory());

    RegisterFilterFactory("SelectVisible", Filters::SelectVisible::GetFactory());
    RegisterFilterFactory("SelectAggro", Filters::SelectAggro::GetFactory());
    RegisterFilterFactory("SelectLowHealth", Filters::SelectLowHealth::GetFactory());
    RegisterFilterFactory("SelectAttackers", Filters::SelectAttackers::GetFactory());
    RegisterFilterFactory("SelectDeadAllies", Filters::SelectDeadAllies::GetFactory());
    RegisterFilterFactory("SelectRandom", Filters::SelectRandom::GetFactory());
    RegisterFilterFactory("SelectAttackTarget", Filters::SelectAttackTarget::GetFactory());
    RegisterFilterFactory("SelectWithEffect", Filters::SelectWithEffect::GetFactory());
    RegisterFilterFactory("SelectMob", Filters::SelectMob::GetFactory());
    RegisterFilterFactory("SelectTargetUsingSkill", Filters::SelectTargetUsingSkill::GetFactory());
    RegisterFilterFactory("SelectTargetAttacking", Filters::SelectTargetAttacking::GetFactory());
    RegisterFilterFactory("SortByDistance", Filters::SortByDistance::GetFactory());
    RegisterFilterFactory("SelectGettingDamage", Filters::SelectGettingDamage::GetFactory());
}

}
