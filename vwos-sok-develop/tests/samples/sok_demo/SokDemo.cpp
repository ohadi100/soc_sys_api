#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <csignal>
#include "ara/core/initialization.h"
#include "ara/exec/application_client.h"

#include "sok/fvm/FreshnessValueManagerDefinitions.hpp"
#include "sok/common/Logger.hpp"
#include "SokUserSample.hpp"


using namespace sok::fvm;

std::atomic<bool> g_running(true);

void signal_handler(int signal);

namespace {

class AraCoreScopedInitializer {
public:
    AraCoreScopedInitializer() {
        ara::core::Result<void> init_result = ara::core::Initialize();
        if (!init_result) {
            char const* msg{"ara::core::Initialize() failed for FVM sample!"};
            std::cerr << msg << "\nResult contains: " << init_result.Error().Message() << ", "
                    << init_result.Error().UserMessage() << std::endl;
            std::cerr.flush();
            ara::core::Abort(msg);
        }
    }
    
    AraCoreScopedInitializer(const AraCoreScopedInitializer&) = delete;
    AraCoreScopedInitializer& operator=(const AraCoreScopedInitializer&) = delete;

    ~AraCoreScopedInitializer() {
        auto onError = [](ara::core::ErrorCode const& error) {
            std::cerr << "ara::core::Deinitialize() failed for FVM sample!";
            std::cerr << "Result contains: " << error.Message() << ", " << error.UserMessage();
        };
        ara::core::Result<void> deinit_result{ara::core::Deinitialize()}; 
        deinit_result.InspectError(onError);
    }
};
} // anonymous namespace

int main(int argc, char* argv[])
{
    FrameConfig authPduFrameConfig;
    authPduFrameConfig.name = "SOK_DEMO_AUTH_BR_GW";
    authPduFrameConfig.maxPayloadSizeBytes = 32;  
    authPduFrameConfig.sourceIp = "fd53:7cb8:383:2::1";
    authPduFrameConfig.destinationIp = "ff14::75";
    authPduFrameConfig.sourcePort = 42994;
    authPduFrameConfig.destinationPort = 42995;

    PduConfig authPduConfig;
    authPduConfig.frameConfig = authPduFrameConfig;
    authPduConfig.name = "SOK_DEMO_AUTH_PDU";
    authPduConfig.id = 123456;
    authPduConfig.lengthBytes = 18;

    SignalConfig authPduSignalConfig;
    authPduSignalConfig.pduConfig = authPduConfig;
    authPduSignalConfig.name = "SOK_DEMO_AUTH_PDU_SIGNAL_DATA";
    authPduSignalConfig.startByte = 0;
    authPduSignalConfig.lengthInBits = 144;

    FrameConfig crDemoFrameConfig;
    crDemoFrameConfig.name = "SOK_DEMO_CR_GW";
    crDemoFrameConfig.maxPayloadSizeBytes = 32;  
    crDemoFrameConfig.sourceIp = "fd53:7cb8:383:2::1";
    crDemoFrameConfig.destinationIp = "ff14::76";
    crDemoFrameConfig.sourcePort = 42994;
    crDemoFrameConfig.destinationPort = 42995;

    PduConfig crDemoConfig;
    crDemoConfig.frameConfig = crDemoFrameConfig;
    crDemoConfig.name = "SOK_DEMO_CR_PDU";
    crDemoConfig.id = 234567;
    crDemoConfig.lengthBytes = 23;

    SignalConfig crDemoSignalConfig;
    crDemoSignalConfig.pduConfig = crDemoConfig;
    crDemoSignalConfig.name = "SOK_DEMO_CR_PDU_SIGNAL_DATA";
    crDemoSignalConfig.startByte = 0;
    crDemoSignalConfig.lengthInBits = 184;

    AraCoreScopedInitializer araInit{};
    if ((SIG_ERR == signal(SIGINT, signal_handler)) || (SIG_ERR == signal(SIGTERM, signal_handler))) {
        getLogger().LogError() << "Failed to subscribe for SIGINT and SIGTERM.";
        ara::core::Abort("Failed to subscribe for SIGINT and SIGTERM.");
    }

    if (argc != 2) {
        LOGE("Expected argument");
        return 1;
    }

    SokUser sokUser;
    if (!sokUser.Init()) {
        getLogger().LogError() << "Initialization has failed";
        ara::core::Abort("Initialization has failed");
    }

    ara::exec::ApplicationClient meterState;
    getLogger().LogInfo() << "ara::exec::ApplicationClient object created";
    meterState.ReportApplicationState(ara::exec::ApplicationState::kRunning);

    sokUser.StartFvmLoop();

    if (0 == strcmp(argv[1], "sink")) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        LOGI("Subscribing for auth pdu: " << 0);
        while (!sokUser.SubscribeToAuthPduService(0, authPduSignalConfig)) {
            LOGD("Failed subscribing to auth pdu, retrying");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        while (!sokUser.SubscribeToCrService(1, crDemoSignalConfig)) {
            LOGD("Failed subscribing to CR, retrying");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        while (g_running) {
            LOGI("Triggering challenge with pdu ID: " << 1);
            sokUser.TriggerCr(1);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    else if (0 == strcmp(argv[1], "source")) {
        std::vector<uint8_t> responseData({'S','O','K','_','C','R','_','R','E','S','P','O','N','S','E'});
        if (!sokUser.OfferCrService(1, responseData, crDemoSignalConfig)) {
            LOGE("Failed offering CR service");
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::vector<uint8_t> data({'S','O','K','_','A','U','T','H','_','P','D','U'});
        while (g_running) {
            LOGI("Sending auth pdu: " << 0 << ", with data: " << std::string(data.begin(), data.end()));
            sokUser.SendAuthPdu(0, data, authPduSignalConfig);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    sokUser.StopFvmLoop();
    sokUser.Stop();
    LOGD("Finished");
    meterState.ReportApplicationState(ara::exec::ApplicationState::kTerminating);
    return 0;
}

void signal_handler(int signal) {
    if (SIGINT == signal) {
        g_running = false;
    }
}
