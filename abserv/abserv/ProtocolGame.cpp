#include "stdafx.h"
#include "ProtocolGame.h"
#include "Player.h"
#include "PlayerManager.h"
#include "OutputMessage.h"
#include "IOPlayer.h"
#include "IOAccount.h"
#include "Bans.h"
#include "StringUtils.h"
#include "GameManager.h"
#include "Logger.h"
#include "Game.h"
#include "Random.h"
#include "Variant.h"
#include "IOGame.h"
#include "stdafx.h"
#include "ConfigManager.h"
#include "Subsystems.h"
#include <AB/DHKeys.hpp>
#include "UuidUtils.h"

#include "DebugNew.h"

namespace Net {

std::string ProtocolGame::serverId_ = Utils::Uuid::EMPTY_UUID;

void ProtocolGame::Login(const std::string& playerUuid, const uuids::uuid& accountUuid,
    const std::string& mapUuid, const std::string& instanceUuid)
{
    auto gameMan = GetSubsystem<Game::PlayerManager>();
    std::shared_ptr<Game::Player> foundPlayer = gameMan->GetPlayerByUuid(playerUuid);
    if (foundPlayer)
    {
        // Maybe DC/crash, let player connect again
        if (foundPlayer->GetInactiveTime() > 2000)
        {
            foundPlayer->Logout();
            foundPlayer.reset();
        }
        else
        {
            DisconnectClient(AB::Errors::AlreadyLoggedIn);
            return;
        }
    }

    if (GetSubsystem<Auth::BanManager>()->IsAccountBanned(accountUuid))
    {
        DisconnectClient(AB::Errors::AccountBanned);
        return;
    }

    player_ = gameMan->CreatePlayer(playerUuid, GetThis());

    if (!IO::IOPlayer::LoadPlayerByUuid(player_.get(), playerUuid))
    {
        DisconnectClient(AB::Errors::ErrorLoadingCharacter);
        return;
    }

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    // Check if game exists.
    AB::Entities::Game g;
    g.uuid = mapUuid;
    if (!client->Read(g))
    {
        DisconnectClient(AB::Errors::InvalidGame);
        return;
    }

    player_->account_.onlineStatus = AB::Entities::OnlineStatus::OnlineStatusOnline;
    player_->account_.currentCharacterUuid = player_->data_.uuid;
    player_->account_.currentServerUuid = ProtocolGame::serverId_;
    client->Update(player_->account_);

    player_->Initialize();
    player_->data_.currentMapUuid = mapUuid;
    player_->data_.lastLogin = Utils::AbTick();
    if (!uuids::uuid(instanceUuid).nil())
        player_->data_.instanceUuid = instanceUuid;
    client->Update(player_->data_);
    Connect(player_->id_);
    OutputMessagePool::Instance()->AddToAutoSend(shared_from_this());
}

void ProtocolGame::Logout()
{
    if (!player_)
        return;

    player_->logoutTime_ = Utils::AbTick();
    IO::IOPlayer::SavePlayer(player_.get());
    IO::IOAccount::AccountLogout(player_->data_.accountUuid);
    GetSubsystem<Game::PlayerManager>()->RemovePlayer(player_->id_);
    Disconnect();
    OutputMessagePool::Instance()->RemoveFromAutoSend(shared_from_this());
    player_.reset();
}

void ProtocolGame::ParsePacket(NetworkMessage& message)
{
    if (!acceptPackets_ ||
        GetSubsystem<Game::GameManager>()->GetState() != Game::GameManager::ManagerStateRunning ||
        message.GetSize() == 0)
        return;

    const uint8_t recvByte = message.GetByte();

    switch (recvByte)
    {
    case AB::GameProtocol::PacketTypePing:
        AddPlayerTask(&Game::Player::Ping);
        break;
    case AB::GameProtocol::PacketTypeLogout:
        AddPlayerTask(&Game::Player::Logout);
        break;
    case AB::GameProtocol::PacketTypeChangeMap:
    {
        const std::string mapUuid = message.GetString();
        AddPlayerTask(&Game::Player::ChangeInstance, mapUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeSendMail:
    {
        const std::string recipient = message.GetString();
        const std::string subject = message.GetString();
        const std::string body = message.GetString();
        AddPlayerTask(&Game::Player::SendMail, recipient, subject, body);
        break;
    }
    case AB::GameProtocol::PacketTypeGetMailHeaders:
        AddPlayerTask(&Game::Player::GetMailHeaders);
        break;
    case AB::GameProtocol::PacketTypeGetMail:
    {
        const std::string mailUuid = message.GetString();
        AddPlayerTask(&Game::Player::GetMail, mailUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeDeleteMail:
    {
        const std::string mailUuid = message.GetString();
        AddPlayerTask(&Game::Player::DeleteMail, mailUuid);
        break;
    }
    case AB::GameProtocol::PacketTypePartyInvitePlayer:
    {
        uint32_t playerId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::PartyInvitePlayer, playerId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyKickPlayer:
    {
        uint32_t playerId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::PartyKickPlayer, playerId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyLeave:
    {
        AddPlayerTask(&Game::Player::PartyLeave);
        break;
    }
    case AB::GameProtocol::PacketTypePartyAcceptInvite:
    {
        uint32_t inviterId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::PartyAccept, inviterId);
        break;
    }
    case AB::GameProtocol::PacketTypeMove:
    {
        Utils::VariantMap data;
        data[Game::InputDataDirection] = message.Get<uint8_t>();
        player_->AddInput(Game::InputType::Move, data);
        break;
    }
    case AB::GameProtocol::PacketTypeTurn:
    {
        Utils::VariantMap data;
        data[Game::InputDataDirection] = message.Get<uint8_t>();   // None | Left | Right
        player_->AddInput(Game::InputType::Turn, data);
        break;
    }
    case AB::GameProtocol::PacketTypeSetDirection:
    {
        Utils::VariantMap data;
        data[Game::InputDataDirection] = message.Get<float>();   // World angle Rad
        player_->AddInput(Game::InputType::Direction, data);
        break;
    }
    case AB::GameProtocol::PacketTypeSetState:
    {
        Utils::VariantMap data;
        data[Game::InputDataState] = message.Get<uint8_t>();
        player_->AddInput(Game::InputType::SetState, data);
        break;
    }
    case AB::GameProtocol::PacketTypeGoto:
    {
        Utils::VariantMap data;
        data[Game::InputDataVertexX] = message.Get<float>();
        data[Game::InputDataVertexY] = message.Get<float>();
        data[Game::InputDataVertexZ] = message.Get<float>();
        player_->AddInput(Game::InputType::Goto, data);
        break;
    }
    case AB::GameProtocol::PacketTypeFollow:
    {
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = message.Get<uint32_t>();
        player_->AddInput(Game::InputType::Follow, data);
        break;
    }
    case AB::GameProtocol::PacketTypeUseSkill:
    {
        // 1 based -> convert to 0 based
        uint8_t index = message.Get<uint8_t>();
        if (index > 0 && index <= PLAYER_MAX_SKILLS)
        {
            Utils::VariantMap data;
            data[Game::InputDataSkillIndex] = index - 1;
            player_->AddInput(Game::InputType::UseSkill, data);
        }
        break;
    }
    case AB::GameProtocol::PacketTypeCancel:
    {
        player_->AddInput(Game::InputType::Cancel);
        break;
    }
    case AB::GameProtocol::PacketTypeAttack:
    {
        player_->AddInput(Game::InputType::Attack);
        break;
    }
    case AB::GameProtocol::PacketTypeSelect:
    {
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = message.Get<uint32_t>();    // Source
        data[Game::InputDataObjectId2] = message.Get<uint32_t>();   // Target
        player_->AddInput(Game::InputType::Select, data);
        break;
    }
    case AB::GameProtocol::PacketTypeClickObject:
    {
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = message.Get<uint32_t>();    // Source
        data[Game::InputDataObjectId2] = message.Get<uint32_t>();   // Target
        player_->AddInput(Game::InputType::ClickObject, data);
        break;
    }
    case AB::GameProtocol::PacketTypeCommand:
    {
        Utils::VariantMap data;
        data[Game::InputDataCommandType] = message.GetByte();
        data[Game::InputDataCommandData] = message.GetString();
        player_->AddInput(Game::InputType::Command, data);
        break;
    }
    default:
        LOG_ERROR << Utils::ConvertIPToString(GetIP()) << ": Player " << player_->GetName() <<
            " sent an unknown packet header: 0x" <<
            std::hex << static_cast<uint16_t>(recvByte) << std::dec << std::endl;
        break;
    }
}

void ProtocolGame::OnRecvFirstMessage(NetworkMessage& msg)
{
    if (encryptionEnabled_)
    {
        if (!XTEADecrypt(msg))
        {
            Disconnect();
            return;
        }
    }

    msg.Skip(2);    // Client OS
    uint16_t version = msg.Get<uint16_t>();
    if (version != AB::PROTOCOL_VERSION)
    {
        DisconnectClient(AB::Errors::WrongProtocolVersion);
        return;
    }
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        clientKey_[i] = msg.GetByte();
    auto keys = GetSubsystem<Crypto::DHKeys>();
    // Switch now to the shared key
    keys->GetSharedKey(clientKey_, encKey_);
#ifdef DEBUG_NET
    LOG_DEBUG << "Client key received" << std::endl;
#endif // DEBUG_NET

    const std::string accountUuid = msg.GetString();
    if (accountUuid.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string password = msg.GetString();
    const std::string characterUuid = msg.GetString();
    const std::string map = msg.GetString();
    const std::string instance = msg.GetString();

    if (GetSubsystem<Auth::BanManager>()->IsIpBanned(GetIP()))
    {
        DisconnectClient(AB::Errors::IPBanned);
        return;
    }

    if (!IO::IOAccount::GameWorldAuth(accountUuid, password, characterUuid))
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        return;
    }

    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::Login, GetThis(), characterUuid, uuids::uuid(accountUuid), map, instance)
        )
    );
}

void ProtocolGame::OnConnect()
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    output->AddByte(AB::GameProtocol::KeyExchange);
    auto keys = GetSubsystem<Crypto::DHKeys>();
    output->AddBytes((const char*)&keys->GetPublickKey(), DH_KEY_LENGTH);
    Send(output);
#ifdef DEBUG_NET
    LOG_DEBUG << "Server key sent" << std::endl;
#endif // DEBUG_NET
}

void ProtocolGame::DisconnectClient(uint8_t error)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    output->AddByte(AB::GameProtocol::Error);
    output->AddByte(error);
    Send(output);
    Disconnect();
}

void ProtocolGame::Connect(uint32_t playerId)
{
    if (IsConnectionExpired())
        // ProtocolGame::release() has been called at this point and the Connection object
        // no longer exists, so we return to prevent leakage of the Player.
        return;

    std::shared_ptr<Game::Player> foundPlayer = GetSubsystem<Game::PlayerManager>()->GetPlayerById(playerId);
    if (!foundPlayer)
    {
        DisconnectClient(AB::Errors::UnknownError);
        // Shouldn't happen, player is created in ProtocolGame::Login
        LOG_ERROR << "Player not found: " << playerId << std::endl;
        return;
    }

    player_ = foundPlayer;
    player_->loginTime_ = Utils::AbTick();

    acceptPackets_ = true;

    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::EnterGame, GetThis())
        )
    );
}

void ProtocolGame::WriteToOutput(const NetworkMessage& message)
{
    GetOutputBuffer(message.GetSize())->Append(message);
}

void ProtocolGame::EnterGame()
{
    auto gameMan = GetSubsystem<Game::GameManager>();
    bool success = false;
    if (!uuids::uuid(player_->data_.instanceUuid).nil())
    {
        auto game = gameMan->GetInstance(player_->data_.instanceUuid);
        if (game)
        {
            game->PlayerJoin(player_->id_);
            success = true;
        }
    }
    else if (gameMan->AddPlayer(player_->data_.currentMapUuid, player_))
        success = true;

    if (success)
    {
        std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
        output->AddByte(AB::GameProtocol::GameEnter);
        output->AddString(Application::Instance->GetServerId());
        output->AddString(player_->data_.currentMapUuid);
        output->AddString(player_->data_.instanceUuid);
        output->Add<uint32_t>(player_->id_);
        Send(output);
    }
    else
        DisconnectClient(AB::Errors::CannotEnterGame);
}

void ProtocolGame::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    output->AddByte(AB::GameProtocol::ChangeInstance);
    output->AddString(Application::Instance->GetServerId());
    output->AddString(mapUuid);
    output->AddString(instanceUuid);
    output->AddString(player_->data_.uuid);
    Send(output);
}

}
