/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <helper/Exception.h>

using namespace topskytower::helper;

Exception::Exception(const std::string& sender, const std::string& message) :
        m_sender(sender),
        m_message(message) { }

const std::string& Exception::sender() const {
    return this->m_sender;
}

const std::string& Exception::message() const {
    return this->m_message;
}
