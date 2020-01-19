#include "stdafx.h"
#include "Chat.h"
#include "Game.h"
#include "Player.h"
#include "Npc.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "Application.h"
#include "MessageMsg.h"
#include <AB/Entities/GuildMembers.h>
#include "DataClient.h"
#include "Profiler.h"
#include "PropStream.h"
#include "Subsystems.h"
#include "UuidUtils.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {

ChatChannel::~ChatChannel() = default;

Chat::Chat()
{
    // Keep a reference to this chat so it doesn't get deleted.
    tradeChat_ = std::dynamic_pointer_cast<ChatChannel>(std::make_shared<TradeChatChannel>());
    static const std::pair<ChatType, uint64_t> tradeChannelId = { ChatType::Trade, 0ULL };
    channels_.emplace(tradeChannelId, tradeChat_);
}

std::shared_ptr<ChatChannel> Chat::Get(ChatType type, uint64_t id)
{
    switch (type)
    {
    case ChatType::None:
        return std::shared_ptr<ChatChannel>();
    case ChatType::Whisper:
        return std::make_shared<WhisperChatChannel>(id);
    default:
        break;
    }

    const std::pair<ChatType, uint64_t> channelId = { type, id };
    const auto it = channels_.find(channelId);
    if (it != channels_.end())
        return (*it).second;

    std::shared_ptr<ChatChannel> c;
    switch (type)
    {
    case ChatType::Map:
        c = std::make_shared<GameChatChannel>(id);
        break;
    case ChatType::Party:
        c = std::make_shared<PartyChatChannel>(id);
        break;
    default:
        c = std::make_shared<ChatChannel>(id);
        break;
    }
    channels_.emplace(channelId, c);
    return c;
}

std::shared_ptr<ChatChannel> Chat::Get(ChatType type, const std::string& uuid)
{
    if (Utils::Uuid::IsEmpty(uuid) || type == ChatType::None)
        return std::shared_ptr<ChatChannel>();

    const std::pair<ChatType, uint64_t> channelId = { type, sa::StringHashRt(uuid.c_str()) };
    const auto it = channels_.find(channelId);
    if (it != channels_.end())
        return (*it).second;

    std::shared_ptr<ChatChannel> c;
    switch (type)
    {
    case ChatType::Guild:
        c = std::make_shared<GuildChatChannel>(uuid);
        channels_.emplace(channelId, c);
        break;
    case ChatType::Whisper:
        c = std::make_shared<WhisperChatChannel>(uuid);
        channels_.emplace(channelId, c);
        break;
    default:
        break;
    }
    return c;
}

void Chat::Remove(ChatType type, uint64_t id)
{
    const std::pair<ChatType, uint64_t> channelId = { type, id };
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        channels_.erase(it);
}

void Chat::CleanChats()
{
    if (channels_.size() == 0)
        return;

#ifdef _DEBUG
    LOG_DEBUG << "Cleaning chats" << std::endl;
#endif
    auto i = channels_.begin();
    while ((i = std::find_if(i, channels_.end(), [](const auto& current) -> bool
    {
        return current.second.use_count() == 1;
    })) != channels_.end())
        channels_.erase(i++);
}

GameChatChannel::GameChatChannel(uint64_t id) :
    ChatChannel(id)
{
    game_ = GetSubsystem<GameManager>()->Get(static_cast<uint32_t>(id));
}

bool GameChatChannel::Talk(Player& player, const std::string& text)
{
    if (auto g = game_.lock())
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
        AB::Packets::Server::ChatMessage packet = {
            AB::GameProtocol::ChatChannelGeneral,
            player.id_,
            player.GetName(),
            text
        };
        AB::Packets::Add(packet, *msg);
        g->VisitPlayers([&player, &msg](Player* current) {
            if (current->IsIgnored(player))
                return Iteration::Continue;
            current->WriteToOutput(*msg);
            return Iteration::Continue;
        });
        return true;
    }
    return false;
}

bool GameChatChannel::TalkNpc(Npc& npc, const std::string& text)
{
    if (auto g = game_.lock())
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
        AB::Packets::Server::ChatMessage packet = {
            AB::GameProtocol::ChatChannelGeneral,
            npc.id_,
            npc.GetName(),
            text
        };
        AB::Packets::Add(packet, *msg);
        g->VisitPlayers([&msg](Player* player) {
            player->WriteToOutput(*msg);
            return Iteration::Continue;
        });
        return true;
    }
    return false;
}

WhisperChatChannel::WhisperChatChannel(uint64_t id) :
    ChatChannel(id)
{
    player_ = GetSubsystem<PlayerManager>()->GetPlayerById(static_cast<uint32_t>(id));
}

WhisperChatChannel::WhisperChatChannel(const std::string& playerUuid) :
    ChatChannel(sa::StringHashRt(playerUuid.c_str())),
    playerUuid_(playerUuid)
{
}

bool WhisperChatChannel::Talk(Player& player, const std::string& text)
{
    if (auto p = player_.lock())
    {
        if (p->IsIgnored(player))
            return false;
        if (!p->IsOnline())
            return false;

        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
        AB::Packets::Server::ChatMessage packet = {
            AB::GameProtocol::ChatChannelWhisper,
            player.id_,
            player.GetName(),
            text
        };
        AB::Packets::Add(packet, *msg);
        p->WriteToOutput(*msg);
        return true;
    }

    // Maybe not on this server
    AB_PROFILE;
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::Whipser;
    Net::MessageClient* cli = GetSubsystem<Net::MessageClient>();
    IO::PropWriteStream stream;
    stream.WriteString(playerUuid_);
    stream.WriteString(player.GetName());
    stream.WriteString(text);
    msg.SetPropStream(stream);
    return cli->Write(msg);
}

bool WhisperChatChannel::Talk(const std::string& playerName, const std::string& text)
{
    if (auto p = player_.lock())
    {
        if (p->IsIgnored(playerName))
            return false;
        if (!p->IsOnline())
            return false;

        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
        AB::Packets::Server::ChatMessage packet = {
            AB::GameProtocol::ChatChannelWhisper,
            0,
            playerName,
            text
        };
        AB::Packets::Add(packet, *msg);
        p->WriteToOutput(*msg);
        return true;
    }
    return false;
}

bool GuildChatChannel::Talk(Player& player, const std::string& text)
{
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::GuildChat;
    Net::MessageClient* cli = GetSubsystem<Net::MessageClient>();
    IO::PropWriteStream stream;
    stream.WriteString(guildUuid_);
    stream.WriteString(player.GetName());
    stream.WriteString(text);
    msg.SetPropStream(stream);
    return cli->Write(msg);
}

void GuildChatChannel::Broadcast(const std::string& playerName, const std::string& text)
{
    AB_PROFILE;
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::GuildMembers gs;
    gs.uuid = guildUuid_;
    if (!cli->Read(gs))
    {
        LOG_WARNING << "Unable to read members for Guild " << guildUuid_ << std::endl;
        return;
    }

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
    AB::Packets::Server::ChatMessage packet = {
        AB::GameProtocol::ChatChannelGuild,
        0,
        playerName,
        text
    };
    AB::Packets::Add(packet, *msg);

    for (const auto& g : gs.members)
    {
        std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerByAccountUuid(g.accountUuid);
        if (!player)
            // This player not on this server.
            continue;
        if (player->IsIgnored(playerName))
            continue;

        player->WriteToOutput(*msg);
    }
}

bool TradeChatChannel::Talk(Player& player, const std::string& text)
{
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::TradeChat;
    Net::MessageClient* cli = GetSubsystem<Net::MessageClient>();
    IO::PropWriteStream stream;
    stream.WriteString(player.GetName());
    stream.WriteString(text);
    msg.SetPropStream(stream);
    return cli->Write(msg);
}

void TradeChatChannel::Broadcast(const std::string& playerName, const std::string& text)
{
    auto msg = Net::NetworkMessage::GetNew();
    auto* playerMngr = GetSubsystem<PlayerManager>();
    msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
    AB::Packets::Server::ChatMessage packet = {
        AB::GameProtocol::ChatChannelTrade,
        0,
        playerName,
        text
    };
    AB::Packets::Add(packet, *msg);
    playerMngr->VisitPlayers([&playerName, &msg](Player& player) {
        if (player.IsIgnored(playerName))
            return Iteration::Continue;
        player.WriteToOutput(*msg);
        return Iteration::Continue;
    });
}

bool PartyChatChannel::Talk(Player& player, const std::string& text)
{
    if (party_)
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
        AB::Packets::Server::ChatMessage packet = {
            AB::GameProtocol::ChatChannelParty,
            0,
            player.GetName(),
            text
        };
        AB::Packets::Add(packet, *msg);

        party_->VisitPlayers([&player, &msg](Player& _player) {
            if (_player.IsIgnored(player))
                return Iteration::Continue;
            _player.WriteToOutput(*msg);
            return Iteration::Continue;
        });
        return true;
    }
    return false;
}

bool PartyChatChannel::TalkNpc(Npc& npc, const std::string& text)
{
    if (party_)
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChatMessage);
        AB::Packets::Server::ChatMessage packet = {
            AB::GameProtocol::ChatChannelParty,
            npc.id_,
            npc.GetName(),
            text
        };
        AB::Packets::Add(packet, *msg);
        party_->VisitPlayers([&msg](auto& player) {
            player.WriteToOutput(*msg);
            return Iteration::Continue;
        });
        return true;
    }
    return false;
}

}
