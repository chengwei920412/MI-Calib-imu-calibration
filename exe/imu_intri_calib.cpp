// Copyright (c) 2023-2024. Created on 4/19/24 12:27 PM by shlchen@whu.edu.cn (Shuolong Chen), who received the B.S. degree in
// geodesy and geomatics engineering from Wuhan University, Wuhan China, in 2023. He is currently a master candidate at
// the school of Geodesy and Geomatics, Wuhan University. His area of research currently focuses on integrated navigation
// systems and multi-sensor fusion.

#include "ros/ros.h"
#include "spdlog/spdlog.h"
#include "calib/status.hpp"
#include "spdlog/fmt/bundled/color.h"
#include "nofree/imu_intri_calib.hpp"
#include "nofree/imu_intri_calib.hpp"

// config the 'spdlog' log pattern
void ConfigSpdlog() {
    // [log type]-[thread]-[time] message
    spdlog::set_pattern("%^[%L]%$-[%t]-[%H:%M:%S.%e] %v");

    // set log level
    spdlog::set_level(spdlog::level::debug);
}

void PrintLibInfo() {
    // ns_pretab::PrettyTable tab;
    // tab.addRowGrids(0, 1, 0, 2, ns_pretab::Align::CENTER, "");
    // tab.addGrid(1, 0, "MI-Calib");
    // tab.addGrid(1, 1, "https://github.com/Unsigned-Long/MI-Calib.git");
    // tab.addGrid(2, 0, "Author");
    // tab.addGrid(2, 1, "Shuolong Chen");
    // tab.addGrid(3, 0, "E-Mail");
    // tab.addGrid(3, 1, "shlchen@whu.edu.cn");
    // std::cout << tab << std::endl;
    std::cout << "+--------------------------------------------------------------------+\n"
                 "| _|      _|  _|_|_|              _|_|_|            _|  _|  _|       |\n"
                 "| _|_|  _|_|    _|              _|          _|_|_|  _|      _|_|_|   |\n"
                 "| _|  _|  _|    _|  _|_|_|_|_|  _|        _|    _|  _|  _|  _|    _| |\n"
                 "| _|      _|    _|              _|        _|    _|  _|  _|  _|    _| |\n"
                 "| _|      _|  _|_|_|              _|_|_|    _|_|_|  _|  _|  _|_|_|   |\n"
                 "+-----------+--------------------------------------------------------+\n"
                 "| RIs-Calib | https://github.com/Unsigned-Long/MI-Calib.git          |\n"
                 "+-----------+--------------------------------------------------------+\n"
                 "| Author    | Shuolong Chen                                          |\n"
                 "+-----------+--------------------------------------------------------+\n"
                 "| E-Mail    | shlchen@whu.edu.cn                                     |\n"
                 "+-----------+--------------------------------------------------------+" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "mi_calib_imu_intri_calib");
    try {
        ConfigSpdlog();

        PrintLibInfo();

        // load settings
        std::string configPath;
        if (!ros::param::get("/mi_calib_imu_intri_calib/config_path", configPath)) {
            throw ns_mi::Status(
                    ns_mi::Status::Flag::CRITICAL,
                    "the configure path couldn't obtained from ros param '/mi_calib_imu_intri_calib/config_path'."
            );
        }
        spdlog::info("loading configure from yaml file '{}'...", configPath);

        auto configor = ns_mi::IMUIntriCalibSolver::Configor::LoadConfigure(configPath);

        if (!std::filesystem::exists(configor.outputPath) &&
            !std::filesystem::create_directories(configor.outputPath)) {
            throw ns_mi::Status(
                    ns_mi::Status::Flag::CRITICAL,
                    "the output path dose not exist! output path: " + configor.outputPath
            );
        }

        auto solver = ns_mi::IMUIntriCalibSolver::Create(configor);

        solver->Process();

        std::string filename = configor.IMUTopic + "-intri.yaml";
        for (int i = 1; i < static_cast<int>(filename.size()); ++i) {
            auto &c = filename.at(i);
            if (c == '/') { c = '-'; }
        }

        solver->GetIntrinsics().Save(configor.outputPath + '/' + filename);

    } catch (const ns_mi::Status &status) {
        // if error happened, print it
        static const auto FStyle = fmt::emphasis::italic | fmt::fg(fmt::color::green);
        static const auto WECStyle = fmt::emphasis::italic | fmt::fg(fmt::color::red);
        switch (status.flag) {
            case ns_mi::Status::Flag::FINE:
                // this case usually won't happen
                spdlog::info(fmt::format(FStyle, "{}", status.what));
                break;
            case ns_mi::Status::Flag::WARNING:
                spdlog::warn(fmt::format(WECStyle, "{}", status.what));
                break;
            case ns_mi::Status::Flag::ERROR:
                spdlog::error(fmt::format(WECStyle, "{}", status.what));
                break;
            case ns_mi::Status::Flag::CRITICAL:
                spdlog::critical(fmt::format(WECStyle, "{}", status.what));
                break;
        }
    } catch (const std::exception &e) {
        // an unknown exception not thrown by this program
        static const auto WECStyle = fmt::emphasis::italic | fmt::fg(fmt::color::red);
        spdlog::critical(fmt::format(WECStyle, "unknown error happened: '{}'", e.what()));
    }

    ros::shutdown();
    return 0;
}
