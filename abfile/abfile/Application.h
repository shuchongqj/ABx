#pragma once

#include "ServerApp.h"
#include <boost/filesystem.hpp>
#include <AB/Entities/Account.h>

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

namespace IO {
class DataClient;
}

template<typename T>
inline size_t stream_size(T& s)
{
    s.seekg(0, std::ios::end);
    size_t size = s.tellg();
    s.seekg(0, std::ios::beg);
    return size;
}

class Application : public ServerApp, public std::enable_shared_from_this<Application>
{
private:
    std::unique_ptr<HttpsServer> server_;
    std::string root_;
    std::string configFile_;
    std::string logDir_;
    std::unique_ptr<IO::DataClient> dataClient_;
    asio::io_service ioService_;
    std::string dataHost_;
    uint16_t dataPort_;
    std::string adminPassword_;
    int64_t startTime_;
    uint64_t bytesSent_;
    uint64_t maxThroughput_;
    uint32_t uptimeRound_;
    int64_t statusMeasureTime_;
    int64_t lastLoadCalc_;
    std::string serverId_;
    std::vector<int> loads_;
    std::mutex mutex_;
    bool requireAuth_;
    bool running_;
    bool ParseCommandLine();
    void ShowHelp();
    void UpdateBytesSent(size_t bytes);
    bool IsAllowed(std::shared_ptr<HttpsServer::Request> request);
    bool IsAdmin(std::shared_ptr<HttpsServer::Request> request);
    bool IsAccountBanned(const AB::Entities::Account& acc);
    static bool IsHiddenFile(const boost::filesystem::path& path);
    static SimpleWeb::CaseInsensitiveMultimap GetDefaultHeader();
    void GetHandlerDefault(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerGames(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerSkills(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerProfessions(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerAttributes(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerEffects(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerVersion(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerStatus(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void HandleError(std::shared_ptr<HttpsServer::Request> /*request*/,
        const SimpleWeb::error_code& ec);

    uint8_t GetAvgLoad() const
    {
        if (loads_.empty())
            return 0;

        float loads = 0.0f;
        for (int p : loads_)
            loads += static_cast<float>(p);
        return static_cast<uint8_t>(loads / loads_.size());
    }
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

