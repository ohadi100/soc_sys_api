/* Copyright (c) 2023 Volkswagen Group */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#ifndef UNIT_TESTS
#include "ara/log/logging.h" 
#else
#include <time.h>
#include <string.h>
#include <iostream>
#endif
namespace sok
{
namespace common
{

#ifndef UNIT_TESTS

inline ara::log::Logger& getLogger() noexcept 
{
    static ara::log::Logger& logger = ara::log::CreateLogger("FVM", "Freshness Value Manager");
    return logger;
}

#define LOGD(...) sok::common::getLogger().LogDebug() << __VA_ARGS__;
#define LOGI(...) sok::common::getLogger().LogInfo() << __VA_ARGS__;
#define LOGW(...) sok::common::getLogger().LogWarn() << __VA_ARGS__;
#define LOGE(...) sok::common::getLogger().LogError() << __VA_ARGS__;

#else

static inline char *timenow();

#define LOGD(...) std::cout << sok::common::timenow() << "  Debug: " << __VA_ARGS__  << std::endl;
#define LOGI(...) std::cout << sok::common::timenow() << "  Info: " << __VA_ARGS__  << std::endl;
#define LOGW(...) std::cout << sok::common::timenow() << "  * Warning: * " << __VA_ARGS__ << std::endl;
#define LOGE(...) std::cout << sok::common::timenow() << "  *** Error: *** " << __VA_ARGS__ << std::endl;

static inline char *timenow() {
    static char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}

#endif

} // namespace common
} // namespace sok

#endif // LOGGER_HPP