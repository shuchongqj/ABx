#include "stdafx.h"
#include "GameManager.h"
#include "Utils.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "Player.h"
#include "Npc.h"

#include "DebugNew.h"

namespace Game {

GameManager GameManager::Instance;

void GameManager::Start()
{
    // Main Thread
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
    state_ = State::ManagerStateRunning;
}

void GameManager::Stop()
{
    // Main Thread
    if (state_ == State::ManagerStateRunning)
    {
        {
            std::lock_guard<std::recursive_mutex> lockClass(lock_);
            state_ = State::ManagerStateTerminated;
        }
        for (const auto& g : games_)
        {
            g.second->SetState(Game::ExecutionState::Terminated);
        }
    }
}

std::shared_ptr<Game> GameManager::CreateGame(const std::string& mapUuid)
{
    assert(state_ == State::ManagerStateRunning);
#ifdef DEBUG_GAME
    LOG_DEBUG << "Creating game " << mapUuid << std::endl;
#endif
    std::shared_ptr<Game> game = std::make_shared<Game>();
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game->id_ = GetNewGameId();
        games_[game->id_] = game;
        maps_[mapUuid].push_back(game.get());
        game->instanceData_.number = static_cast<uint16_t>(maps_[mapUuid].size());
    }
    game->SetState(Game::ExecutionState::Startup);
    game->Load(mapUuid);
    return game;
}

void GameManager::DeleteGameTask(uint32_t gameId)
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Deleting game " << gameId << std::endl;
#endif
    // Dispatcher Thread
    auto it = games_.find(gameId);
    if (it != games_.end())
    {
        maps_.erase((*it).second->data_.uuid);
        games_.erase(it);
    }
}

std::shared_ptr<Game> GameManager::GetInstance(const std::string& instanceUuid)
{
    auto it = std::find_if(games_.begin(), games_.end(), [&](auto const& current)
    {
        return current.second->instanceData_.uuid.compare(instanceUuid) == 0;
    });
    if (it != games_.end())
        return (*it).second;

    return std::shared_ptr<Game>();
}

std::shared_ptr<Game> GameManager::GetGame(const std::string& mapUuid, bool canCreate /* = false */)
{
    const auto it = maps_.find(mapUuid);
    if (it == maps_.end())
    {
        // Map does not exist
        if (!canCreate)
            return std::shared_ptr<Game>();
        return CreateGame(mapUuid);
    }
    // There are already some games with this map
    for (const auto& g : it->second)
    {
        if (g->GetPlayerCount() < GAME_MAX_PLAYER &&
            (g->GetState() == Game::ExecutionState::Running || g->GetState() == Game::ExecutionState::Startup))
        {
            const auto git = games_.find(g->id_);
            return (*git).second;
        }
    }
    if (canCreate)
        return CreateGame(mapUuid);
    return std::shared_ptr<Game>();
}

std::shared_ptr<Game> GameManager::Get(uint32_t gameId)
{
    auto it = games_.find(gameId);
    if (it != games_.end())
    {
        return (*it).second;
    }
    return std::shared_ptr<Game>();
}

bool GameManager::AddPlayer(const std::string& mapUuid, std::shared_ptr<Player> player)
{
    std::shared_ptr<Game> game;
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game = GetGame(mapUuid, true);
    }

    if (!game)
        return false;

    // No need to wait until assets loaded
    game->PlayerJoin(player->id_);
    return true;
}

void GameManager::CleanGames()
{
    for (const auto& g : games_)
    {
        if (g.second->IsInactive())
            g.second->SetState(Game::ExecutionState::Terminated);
    }
}

}
