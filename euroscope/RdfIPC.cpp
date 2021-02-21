/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the version checker
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include "Plugin.h"
#include "RdfIPC.h"

using namespace topskytower::euroscope;

static const char* __pipeSyncName = "\\\\.\\pipe\\tstSyncPipe";

void RdfIPC::syncThread() {
    char buffer[513];

    using namespace std::chrono_literals;

    while (false == this->m_stopThread) {
        DWORD readBytes = 0;

        HANDLE relevantPipe;
        if (false == this->m_slave) {
            relevantPipe = this->m_syncHandle;
        }
        else {
            if (NULL == this->m_recvHandle) {
                std::this_thread::sleep_for(50ms);
                continue;
            }

            relevantPipe = this->m_recvHandle;
        }

        /* test if data is available */
        PeekNamedPipe(relevantPipe, buffer, 512, &readBytes, NULL, NULL);
        if (0 == readBytes) {
            std::this_thread::sleep_for(50ms);
            continue;
        }

        if (FALSE != ReadFile(relevantPipe, buffer, 512, &readBytes, NULL)) {
            buffer[readBytes] = '\0';
            bool found = false;

            if (false == this->m_slave) {
                std::lock_guard guard(this->m_slavesLock);

                for (auto it = this->m_slaves.begin(); this->m_slaves.end() != it;) {
                    if (it->name == buffer) {
                        CloseHandle(it->receiver);
                        this->m_slaves.erase(it);
                        found = true;
                        break;
                    }
                }

                if (false == found) {
                    Receiver receiver = {
                        buffer,
                        CreateFileA(buffer, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)
                    };
                    this->m_slaves.push_back(std::move(receiver));
                }
            }
            else {
                this->m_plugin->afvMessage(buffer);
            }
        }
    }
}

RdfIPC::RdfIPC(PlugIn* plugin) :
        m_plugin(plugin),
        m_syncHandle(CreateFileA(__pipeSyncName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)),
        m_slave(INVALID_HANDLE_VALUE != this->m_syncHandle),
        m_slavesLock(),
        m_slaves(),
        m_slavePipeName(),
        m_recvHandle(NULL),
        m_stopThread(false),
        m_syncThread(&RdfIPC::syncThread, this) {
    if (true == this->m_slave) {
        auto now = std::chrono::system_clock::now();
        this->m_slavePipeName = std::string("\\\\.\\pipe\\tst");
        this->m_slavePipeName += std::to_string(now.time_since_epoch().count());

        this->m_recvHandle = CreateNamedPipeA(this->m_slavePipeName.c_str(), PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE,
                                              PIPE_UNLIMITED_INSTANCES, 0, 512, 0, NULL);

        WriteFile(this->m_syncHandle, this->m_slavePipeName.c_str(), this->m_slavePipeName.length(), NULL, NULL);
    }
    else {
        this->m_syncHandle = CreateNamedPipeA(__pipeSyncName, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE,
                                              PIPE_UNLIMITED_INSTANCES, 0, 512, 0, NULL);
    }
}

RdfIPC::~RdfIPC() {
    this->m_stopThread = true;
    this->m_syncThread.join();

    if (true == this->m_slave) {
        WriteFile(this->m_syncHandle, this->m_slavePipeName.c_str(), this->m_slavePipeName.length(), NULL, NULL);
        CloseHandle(this->m_recvHandle);
    }
    else {
        for (auto& slave : this->m_slaves)
            CloseHandle(slave.receiver);
        this->m_slaves.clear();
    }

    CloseHandle(this->m_syncHandle);
}

bool RdfIPC::isSlave() const {
    return this->m_slave;
}

void RdfIPC::sendAfvMessage(const std::string& message) {
    std::lock_guard guard(this->m_slavesLock);

    for (auto it = this->m_slaves.begin(); this->m_slaves.end() != it;) {
        if (FALSE == WriteFile(it->receiver, message.c_str(), message.length() + 1, NULL, NULL)) {
            switch (GetLastError()) {
            case ERROR_INVALID_USER_BUFFER:
            case ERROR_OPERATION_ABORTED:
            case ERROR_NOT_ENOUGH_QUOTA:
            case ERROR_BROKEN_PIPE:
                CloseHandle(it->receiver);
                it = this->m_slaves.erase(it);
                break;
            default:
                ++it;
                break;
            }
        }
        else {
            ++it;
        }
    }
}
