#pragma once

#include "Client.h"
#include "Receiver.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Skill.h>
#include <AB/Entities/Effect.h>
#include <AB/Entities/Music.h>
#include <map>

struct EventItem
{
    StringHash eventId;
    VariantMap eventData;
};

class FwClient : public Object, public Client::Receiver
{
    URHO3D_OBJECT(FwClient, Object);
private:
    String currentLevel_;
    String currentMapUuid_;
    bool levelReady_;
    Vector<EventItem> queuedEvents_;
    uint32_t playerId_;
    Client::Client client_;
    Client::Client::ClientState lastState_;
    AB::Entities::CharList characters_;
    std::map<std::string, AB::Entities::Game> outposts_;
    std::map<std::string, AB::Entities::Game> games_;
    std::map<std::string, AB::Entities::Service> services_;
    std::vector<AB::Entities::MailHeader> mailHeaders_;
    std::map<std::string, AB::Entities::Profession> professions_;
    std::map<uint32_t, AB::Entities::Skill> skills_;
    std::map<uint32_t, AB::Entities::Effect> effects_;
    String currentServerId_;
    AB::Entities::Mail currentMail_;
    String currentCharacterUuid_;
    Mutex mutex_;
    HashMap<String, uint32_t> versions_;
    void LoadData();
    static bool IsOldData(uint32_t curVersion, XMLFile* file);
    void LoadGames(uint32_t curVersion);
    void LoadSkills(uint32_t curVersion);
    void LoadAttributes(uint32_t curVersion);
    void LoadProfessions(uint32_t curVersion);
    void LoadEffects(uint32_t curVersion);
    void LoadItems(uint32_t curVersion);
    void LoadMusic(uint32_t curVersion);
    bool MakeHttpRequest(const String& path, const String& outFile);
    bool loggedIn_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLevelReady(StringHash eventType, VariantMap& eventData);
    void QueueEvent(StringHash eventType, VariantMap& eventData);
public:
    static String GetProtocolErrorMessage(uint8_t err);
    static String GetSkillErrorMessage(AB::GameProtocol::SkillError err);

    FwClient(Context* context);
    ~FwClient();

    uint32_t GetIp() const
    {
        if (loggedIn_)
            return client_.GetIp();
        return 0;
    }
    /// Get difference of server and client clock
    int64_t GetClockDiff() const
    {
        if (loggedIn_)
            return client_.GetClockDiff();
        return 0;
    }
    void UpdateServers();
    bool Start();
    void Stop();
    void UnsubscribeUpdate();
    void SubscibeUpdate();
    void Update(float timeStep);
    void Login(const String& name, const String& pass);
    void CreateAccount(const String& name, const String& pass, const String& email, const String& accKey);
    void CreatePlayer(const String& name, const String& profUuid, uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp);
    void EnterWorld(const String& charUuid, const String& mapUuid);
    void ChangeWorld(const String& mapUuid);
    /// Causes the server to change the map for the whole party
    void ChangeMap(const String& mapUuid);
    void ChangeServer(const String& serverId);
    void Logout();
    void GetMailHeaders();
    void ReadMail(const std::string& uuid);
    void DeleteMail(const std::string& uuid);
    void SendMail(const std::string& recipient, const std::string& subject, const std::string& body);
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const String& data);
    void GotoPos(const Vector3& pos);
    void FollowObject(uint32_t objectId);
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void UseSkill(uint32_t index);
    void Cancel();
    void PartyInvitePlayer(uint32_t objectId);

    /// asio network error
    void OnNetworkError(const std::error_code& err) override;
    /// Protocol error, e.g. Login failed
    void OnProtocolError(uint8_t err) override;
    void OnPong(int) override { }
    void OnServerJoined(const AB::Entities::Service& service) override;
    void OnServerLeft(const AB::Entities::Service& service) override;

    void OnLoggedIn(const std::string&) override;
    void OnGetCharlist(const AB::Entities::CharList& chars) override;
    void OnGetOutposts(const std::vector<AB::Entities::Game>& games) override;
    void OnGetServices(const std::vector<AB::Entities::Service>& services) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) override;

    void OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers) override;
    void OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail) override;
    void OnEnterWorld(int64_t updateTick, const std::string& serverId,
        const std::string& mapUuid, const std::string& instanceUuid, uint32_t playerId,
        AB::Entities::GameType type, uint8_t partySize) override;
    void OnChangeInstance(int64_t updateTick, const std::string& serverId,
        const std::string& mapUuid, const std::string& instanceUuid, const std::string& charUuid) override;
    void OnSpawnObject(int64_t updateTick, uint32_t id, const Client::ObjectSpawn& objectSpawn,
        PropReadStream& data, bool existing) override;
    void OnDespawnObject(int64_t updateTick, uint32_t id) override;
    void OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos) override;
    void OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual) override;
    void OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state) override;
    void OnObjectSpeedChange(int64_t updateTick, uint32_t id, float speedFactor) override;
    void OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId) override;
    void OnObjectSkillFailure(int64_t updateTick, uint32_t id, int skillIndex, AB::GameProtocol::SkillError error) override;
    void OnObjectUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t energy, uint16_t adrenaline,
        uint16_t activation, uint16_t overcast) override;
    void OnObjectEndUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t recharge) override;
    void OnObjectEffectAdded(int64_t updateTick, uint32_t id, uint32_t effectIndex, uint32_t ticks) override;
    void OnObjectEffectRemoved(int64_t updateTick, uint32_t id, uint32_t effectIndex) override;
    void OnResourceChanged(int64_t updateTick, uint32_t id,
        AB::GameProtocol::ResourceType resType, int16_t value) override;
    void OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
        const std::string& senderName, const std::string& message) override;
    void OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
        uint32_t senderId, const std::string& senderName, const std::string& message) override;
    /// The player was invited into our party
    void OnPartyInvited(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId) override;
    /// Player was removed from our party
    void OnPartyRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId) override;
    /// Player was added to our party, i.e. the player accepted the invite
    void OnPartyAdded(int64_t updateTick, uint32_t acceptorId, uint32_t leaderId, uint32_t partyId) override;
    /// The invite to our party was removed
    void OnPartyInviteRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId) override;

    void SetState(Client::Client::ClientState state)
    {
        if (state == Client::Client::ClientState::Disconnected || state == Client::Client::ClientState::CreateAccount)
            loggedIn_ = false;
        client_.state_ = state;
    }
    const std::map<std::string, AB::Entities::Service>& GetServices() const
    {
        return services_;
    }
    const String& GetCurrentServerId() const
    {
        return currentServerId_;
    }
    const String& GetCurrentCharacterUuid() const
    {
        return currentCharacterUuid_;
    }
    const AB::Entities::CharList& GetCharacters() const
    {
        return characters_;
    }
    const AB::Entities::Game* GetGame(const String& uuid) const
    {
        auto it = games_.find(std::string(uuid.CString()));
        if (it == games_.end())
            return nullptr;
        return &(*it).second;

    }
    const std::map<std::string, AB::Entities::Game>& GetOutposts() const
    {
        return outposts_;
    }
    String GetGameName(const String& uuid) const
    {
        auto it = games_.find(std::string(uuid.CString()));
        if (it == games_.end())
            return "";
        return String((*it).second.name.c_str());
    }
    uint8_t GetPartySize(const String& uuid) const
    {
        auto it = games_.find(std::string(uuid.CString()));
        if (it == games_.end())
            return 1;
        return (*it).second.partySize;
    }
    const std::map<std::string, AB::Entities::Profession>& GetProfessions() const
    {
        return professions_;
    }
    AB::Entities::Profession* GetProfessionByIndex(uint32_t index)
    {
        auto it = std::find_if(professions_.begin(), professions_.end(), [&index](const auto& current) -> bool
        {
            return current.second.index == index;
        });
        if (it != professions_.end())
            return &(*it).second;
        static AB::Entities::Profession EMPTY;
        return &EMPTY;
    }

    const std::map<uint32_t, AB::Entities::Skill>& GetSkills() const { return skills_; }
    const AB::Entities::Skill* GetSkillByIndex(uint32_t index) const
    {
        auto it = skills_.find(index);
        if (it == skills_.end())
            return nullptr;
        return &(*it).second;
    }
    const AB::Entities::Effect* GetEffectByIndex(uint32_t index) const
    {
        auto it = effects_.find(index);
        if (it == effects_.end())
            return nullptr;
        return &(*it).second;
    }
    const std::vector<AB::Entities::MailHeader>& GetCurrentMailHeaders() const
    {
        return mailHeaders_;
    }
    const AB::Entities::Mail& GetCurrentMail() const
    {
        return currentMail_;
    }
    int GetAvgPing() const
    {
        return client_.GetAvgPing();
    }
    int GetLastPing() const
    {
        return client_.GetLastPing();
    }
    uint32_t GetPlayerId() const
    {
        return playerId_;
    }

    String accountName_;
    String accountPass_;
};

