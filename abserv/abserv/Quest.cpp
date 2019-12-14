#include "stdafx.h"
#include "Quest.h"
#include "ScriptManager.h"
#include "Script.h"
#include "Subsystems.h"
#include "DataProvider.h"
#include "Player.h"
#include "NetworkMessage.h"
#include "ItemFactory.h"
#include "PropStream.h"

namespace Game {

void Quest::RegisterLua(kaguya::State& state)
{
    state["Quest"].setClass(kaguya::UserdataMetatable<Quest>()
        .addFunction("GetOwner", &Quest::_LuaGetOwner)
        .addFunction("IsCompleted", &Quest::IsCompleted)
        .addFunction("IsRewarded", &Quest::IsRewarded)
        .addFunction("GetVarString", &Quest::_LuaGetVarString)
        .addFunction("SetVarString", &Quest::_LuaSetVarString)
        .addFunction("GetVarNumber", &Quest::_LuaGetVarNumber)
        .addFunction("SetVarNumber", &Quest::_LuaSetVarNumber)
    );
}

Quest::Quest(Player& owner, AB::Entities::PlayerQuest&& playerQuest) :
    owner_(owner),
    playerQuest_(std::move(playerQuest))
{
    InitializeLua();
    LoadProgress();
}

void Quest::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Quest::LoadScript(const std::string& fileName)
{
    if (fileName.empty())
        return false;

    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script)
        return false;
    if (!script->Execute(luaState_))
        return false;

    if (Lua::IsFunction(luaState_, "onUpdate"))
        functions_ |= FunctionUpdate;
    return true;
}

void Quest::LoadProgress()
{
    IO::PropReadStream stream;
    stream.Init(playerQuest_.progress.data(), playerQuest_.progress.length());
    if (!Utils::VariantMapRead(variables_, stream))
    {
        LOG_WARNING << "Error loading Quest progress" << std::endl;
    }
}

void Quest::SaveProgress()
{
    IO::PropWriteStream stream;
    Utils::VariantMapWrite(variables_, stream);
    size_t ssize = 0;
    const char* s = stream.GetStream(ssize);
    playerQuest_.progress = std::string(s, ssize);
}

bool Quest::CollectReward()
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Quest q;
    q.uuid = playerQuest_.questUuid;
    if (!client->Read(q))
        return false;
    owner_.progressComp_->AddXp(q.rewardXp);
    if (q.rewardMoney > 0)
    {
        if (!owner_.AddMoney(static_cast<uint32_t>(q.rewardMoney)))
            return false;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    for (const auto& item : q.rewardItems)
    {
        uint32_t id = factory->CreatePlayerItem(owner_, item, 1);
        if (id != 0)
            owner_.AddToInventory(id);
    }

    playerQuest_.rewarded = true;
    return true;
}

void Quest::Update(uint32_t timeElapsed)
{
    if (HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](timeElapsed);
}

void Quest::Write(Net::NetworkMessage& message)
{
    // TODO:
    (void)message;
}

Player* Quest::_LuaGetOwner()
{
    return &owner_;
}

std::string Quest::_LuaGetVarString(const std::string& name)
{
    return GetVar(name).GetString();
}

void Quest::_LuaSetVarString(const std::string& name, const std::string& value)
{
    SetVar(name, Utils::Variant(value));
}

float Quest::_LuaGetVarNumber(const std::string& name)
{
    return GetVar(name).GetFloat();
}

void Quest::_LuaSetVarNumber(const std::string& name, float value)
{
    SetVar(name, Utils::Variant(value));
}

const Utils::Variant& Quest::GetVar(const std::string& name) const
{
    auto it = variables_.find(sa::StringHashRt(name.c_str()));
    if (it != variables_.end())
        return (*it).second;
    return Utils::Variant::Empty;
}

void Quest::SetVar(const std::string& name, const Utils::Variant& val)
{
    variables_[sa::StringHashRt(name.c_str())] = val;
}

}
