/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the separation constraints
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <system/Separation.h>

using namespace topskytower;
using namespace topskytower::system;

std::map<std::pair<types::Aircraft::WTC, types::Aircraft::WTC>, types::Length> Separation::EuclideanDistance = {
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Unknown), 3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Light),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Medium),  3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Heavy),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Super),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Unknown), 3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Light),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Medium),  3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Heavy),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Super),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Unknown), 3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Light),   5.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Medium),  3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Heavy),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Super),   3.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Unknown), 4.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Light),   6.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Medium),  5.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Heavy),   4.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Super),   4.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Unknown), 6.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Light),   8.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Medium),  7.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Heavy),   6.0f * types::nauticmile },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Super),   6.0f * types::nauticmile }
};

std::map<std::pair<types::Aircraft::WTC, types::Aircraft::WTC>, types::Time> Separation::TimeDistance = {
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Unknown), 0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Light),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Medium),  0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Heavy),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Super),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Unknown), 0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Light),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Medium),  0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Heavy),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Super),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Unknown), 0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Light),   2.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Medium),  0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Heavy),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Super),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Unknown), 0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Light),   2.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Medium),  2.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Heavy),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Super),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Unknown), 0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Light),   3.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Medium),  3.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Heavy),   0.0f * types::minute },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Super),   0.0f * types::minute }
};
