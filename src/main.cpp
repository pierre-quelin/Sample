/**
 * Copyright (c) 2021-2026 Pierre Quelin <pierre.quelin.1972@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * For the full license text, see:
 * https://opensource.org/license/mit/
 *
 * @file main.cpp
 * @brief Generic Sample shell — behavior comes entirely from main.ini + cfg JSON.
 */

#include "tools/design/config/Config.hpp"
#include "tools/design/factory/ApplicationServices.hpp"
#include "tools/design/factory/BootRoots.hpp"
#include "tools/design/factory/Factory.hpp"
#include "tools/design/factory/LinkAll.hpp"
#include "tools/design/ipc/EventBusBoot.hpp"
#include "tools/design/time/TimeManagerByTimer.hpp"
#include "tools/os/startup/LoggerBoot.hpp"
#include "tools/os/startup/MainIni.hpp"
#include "tools/os/timer/Timer.h"
#include "util/logger/Logger.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace tools::design::factory;
using namespace util::logger;

int main(int /*argc*/, char** /*argv*/)
{
    tools::os::startup::MainIni bootIni;
    try
    {
        bootIni = tools::os::startup::parseMainIni(tools::os::startup::resolveMainIniPath());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Bootstrap main.ini failed: " << e.what() << '\n';
        return 1;
    }

    tools::os::startup::LoggerBoot loggerBoot;
    try
    {
        loggerBoot = tools::os::startup::makeLoggerBoot(bootIni.loggerTokens);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Bootstrap logger sinks failed: " << e.what() << '\n';
        return 1;
    }

    auto logs = std::make_shared<LogService>(loggerBoot.sinks);
    for (const auto& hub : loggerBoot.tcpHubs)
    {
        hub->setLogService(logs);
        if (!hub->isRunning())
        {
            std::cerr << "Warning: TCP logger hub is not listening on port " << hub->port()
                      << '\n';
        }
    }

    foundationFactoryLinkAll();

    tools::design::ApplicationServices app;
    try
    {
        app.config       = tools::design::config::createLocal(bootIni.cfgPath);
        app.logs         = logs;
        app.platformName = bootIni.platformName;
        app.timer        = std::make_shared<tools::os::timer::Timer>();
        app.timer->start();
        app.timeManager = std::make_shared<tools::design::time::TimeManagerByTimer>(*app.timer);
        tools::design::install(app);

        if (!app.root().contains("EventBus"))
        {
            throw std::runtime_error("missing root 'EventBus' in config");
        }
        tools::design::ipc::wireEventBus(app, app.root()["EventBus"]);

        auto roots = createGlobalObjects(app);
        launchGlobalObjects(roots);

        auto logger = *app.loggerFor("Main");
        logger.log(LogService::LogLevel::INFO,
                   "Running as '{}' — press Enter to quit",
                   bootIni.platformName);

        std::string line;
        std::getline(std::cin, line);

        destroyGlobalObjects(app, roots);
        logger.log(LogService::LogLevel::INFO, "Shutdown complete");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Sample failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
