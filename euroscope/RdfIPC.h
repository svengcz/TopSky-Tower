/*
 * @brief Defines the IPC blocks
 * @file euroscope/RdfIPC.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>
#include <Windows.h>

namespace topskytower {
    namespace euroscope {
        class PlugIn;

        /**
         * @brief Defines the ICP logic between TopSky-Tower instances
         * @ingroup euroscope
         */
        class RdfIPC {
        private:
            struct Receiver {
                std::string name;
                HANDLE      receiver;

                Receiver() :
                        name(),
                        receiver(NULL) { }
            };

            PlugIn*             m_plugin;
            HANDLE              m_syncHandle;
            bool                m_slave;
            std::mutex          m_slavesLock;
            std::list<Receiver> m_slaves;
            std::string         m_slavePipeName;
            HANDLE              m_recvHandle;
            volatile bool       m_stopThread;
            std::thread         m_syncThread;

            void syncThread();

        public:
            RdfIPC(PlugIn* plugin);
            ~RdfIPC();

            bool isSlave() const;
            void sendAfvMessage(const std::string& message);
        };
    }
}
