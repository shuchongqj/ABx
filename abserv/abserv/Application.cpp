#include "stdafx.h"
#include "Application.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ConfigManager.h"
#include "Task.h"
#include "Logger.h"
#include "StringUtils.h"
#include "GameManager.h"
#include <functional>
#include "Random.h"
#include "Connection.h"
#include "SkillManager.h"
#include "Skill.h"
#include "EffectManager.h"
#include "DataProvider.h"
#include "Maintenance.h"
#include "Utils.h"
#include <AB/ProtocolCodes.h>
#include <base64.h>
#include "Profiler.h"
#include "PlayerManager.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Connection.h"
#include "Bans.h"
#include "CpuUsage.h"

#include "DebugNew.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    lastLoadCalc_(0),
    ioService_(),
    gamePort_(0)
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
    serviceManager_ = std::make_unique<Net::ServiceManager>(ioService_);
}

Application::~Application()
{
    serviceManager_->Stop();
    Game::GameManager::Instance.Stop();
    Asynch::Scheduler::Instance.Stop();
    Asynch::Dispatcher::Instance.Stop();
}

bool Application::ParseCommandLine()
{
    for (int i = 0; i != arguments_.size(); i++)
    {
        const std::string& a = arguments_[i];
        if (a.compare("-conf") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                configFile_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -conf" << std::endl;
        }
        else if (a.compare("-log") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                logDir_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
        else if (a.compare("-id") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                serverId_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -id" << std::endl;
        }
        else if (a.compare("-ip") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                gameIp_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -ip" << std::endl;
        }
        else if (a.compare("-host") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                gameHost_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -host" << std::endl;
        }
        else if (a.compare("-port") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                gamePort_ = static_cast<uint16_t>(atoi(arguments_[i].c_str()));
            }
            else
                LOG_WARNING << "Missing argument for -port" << std::endl;
        }
        else if (a.compare("-h") == 0 || a.compare("-help") == 0)
        {
            return false;
        }
    }
    return true;
}

void Application::ShowHelp()
{
    std::cout << "abserv [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  id <id>: Server ID" << std::endl;
    std::cout << "  ip <ip>: Game ip" << std::endl;
    std::cout << "  host <host>: Game host" << std::endl;
    std::cout << "  port <port>: Game port" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
        return false;

    if (!ParseCommandLine())
    {
        ShowHelp();
        return false;
    }
    if (!logDir_.empty())
    {
        // From the command line
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    Asynch::Dispatcher::Instance.Start();
    Asynch::Scheduler::Instance.Start();

    if (!LoadMain())
        return false;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    if (!serviceManager_->IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_->IsRunning();
}

void Application::HandleMessage(const Net::MessageMsg& msg)
{
    if (msg.type_ == Net::MessageType::Shutdown)
    {
        std::string serverId = msg.GetBodyString();
        if (serverId.compare(serverId_) == 0)
            Asynch::Dispatcher::Instance.Add(Asynch::CreateTask(std::bind(&Application::Stop, this)));
        return;
    }
    msgDispatcher_->Dispatch(msg);
}

bool Application::LoadMain()
{
    int64_t startLoading = Utils::AbTick();

    LOG_INFO << "Loading..." << std::endl;

    if (configFile_.empty())
        configFile_ = path_ + "/" + CONFIG_FILE;
    LOG_INFO << "Loading configuration: " << configFile_ << "...";
    if (!ConfigManager::Instance.Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to load configuration file" << std::endl;
        return false;
    }
    if (serverId_.empty())
        serverId_ = ConfigManager::Instance[ConfigManager::Key::ServerID].GetString();
    Net::ConnectionManager::maxPacketsPerSec = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::MaxPacketsPerSecond].GetInt64());
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Initializing RNG...";
    Utils::Random::Instance.Initialize();
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to data server...";
    const std::string& dataHost = ConfigManager::Instance[ConfigManager::Key::DataServerHost].GetString();
    uint16_t dataPort = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::DataServerPort].GetInt());
    dataClient_->Connect(dataHost, dataPort);
    if (!dataClient_->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to message server...";
    const std::string& msgHost = ConfigManager::Instance[ConfigManager::Key::MessageServerHost].GetString();
    uint16_t msgPort = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::MessageServerPort].GetInt());

    msgClient_ = std::make_unique<Net::MessageClient>(ioService_);
    msgClient_->Connect(msgHost, msgPort, std::bind(&Application::HandleMessage, this, std::placeholders::_1));
    msgDispatcher_ = std::make_unique<MessageDispatcher>();
    if (msgClient_->IsConnected())
        LOG_INFO << "[done]" << std::endl;
    else
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to message server" << std::endl;
    }

    if (gameHost_.empty())
        gameHost_ = ConfigManager::Instance[ConfigManager::Key::GameHost].GetString();
    uint32_t ip;
    if (!gameIp_.empty())
        ip = Utils::ConvertStringToIP(gameIp_);
    else
        ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::GameIP].GetInt());
    if (gamePort_ == 0)
        gamePort_ = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::GamePort].GetInt());
    if (gamePort_ != 0)
        serviceManager_->Add<Net::ProtocolGame>(ip, gamePort_, [](uint32_t remoteIp) -> bool
    {
        return Auth::BanManager::Instance.AcceptConnection(remoteIp);
    });

    int64_t loadingTime = (Utils::AbTick() - startLoading);

    PrintServerInfo();

    LOG_INFO << "Loading done in ";
    if (loadingTime < 1000)
        LOG_INFO << loadingTime << " ms";
    else
        LOG_INFO << (loadingTime / 1000) << " s";
    LOG_INFO << std::endl;

    Maintenance::Instance.Run();
    Game::GameManager::Instance.Start();

    return true;
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Server name: " << ConfigManager::Instance[ConfigManager::Key::ServerName].GetString() << std::endl;
    LOG_INFO << "  Location: " << ConfigManager::Instance[ConfigManager::Key::Location].GetString() << std::endl;
    LOG_INFO << "  Protocol version: " << AB::PROTOCOL_VERSION << std::endl;

    std::list<std::pair<uint32_t, uint16_t>> ports = serviceManager_->GetPorts();
    LOG_INFO << "  Listening: ";
    while (ports.size())
    {
        LOG_INFO << Utils::ConvertIPToString(ports.front().first) << ":" << ports.front().second << " ";
        ports.pop_front();
    }
    LOG_INFO << std::endl;

    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
    LOG_INFO << "  Message Server: " << msgClient_->GetHost() << ":" << msgClient_->GetPort() << std::endl;
}

void Application::Run()
{
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient_->Read(serv);
    serv.host = gameHost_;
    serv.location = ConfigManager::Instance[ConfigManager::Key::Location].GetString();
    serv.port = gamePort_;
    serv.name = ConfigManager::Instance[ConfigManager::Key::ServerName].GetString();
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeGameServer;
    serv.startTime = Utils::AbTick();
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    LOG_INFO << "Server is running" << std::endl;
    // If we use a log file close current and reopen as file logger
    if (logDir_.empty())
        logDir_ = ConfigManager::Instance[ConfigManager::Key::LogDir].GetString();
    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::ServerId;
    msg.SetBodyString(GetServerId());
    msgClient_->Write(msg);

    running_ = true;
    serviceManager_->Run();
    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown...";

    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (dataClient_->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::AbTick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        dataClient_->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient_->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;

    Game::PlayerManager::Instance.KickAllPlayers();
    // Before serviceManager_.Stop()
    Maintenance::Instance.Stop();

    msgClient_->Close();
    ioService_.stop();
}

uint8_t Application::GetLoad()
{
    static System::CpuUsage usage;

    if ((Utils::AbTick() - lastLoadCalc_) > 1000 || loads_.empty())
    {
        lastLoadCalc_ = Utils::AbTick();
        size_t playerCount = Game::PlayerManager::Instance.GetPlayerCount();
        float ld = ((float)playerCount / (float)SERVER_MAX_CONNECTIONS) * 100.0f;
        uint8_t load = static_cast<uint8_t>(ld);
        uint8_t utilization = static_cast<uint8_t>(Asynch::Dispatcher::Instance.GetUtilization());
        if (utilization > load)
            load = utilization;
        short l = usage.GetUsage();
        if (l > load)
            // Use the higher value
            load = static_cast<uint8_t>(l);
        if (load > 100)
            load = 100;

        while (loads_.size() > 9)
            loads_.erase(loads_.begin());
        loads_.push_back(static_cast<int>(load));
    }
    return GetAvgLoad();
}
