/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the PDC control
 */

#include <chrono>
#include <cstdlib>
#include <ctime>

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <surveillance/FlightRegistry.h>
#include <surveillance/PdcControl.h>

using namespace std::chrono;
using namespace topskytower;
using namespace topskytower::surveillance;

static std::string __datalink = "http://www.hoppie.nl/acars/system/connect.html?logon=%LOGON%&from=%SENDER%";
static std::string __receivedData;

PdcControl::MessageQueue::MessageQueue() :
        inbounds(),
        outbounds(),
        expectedAnswerId(UINT32_MAX),
        expectedAnswer(PdcControl::CpdlcMessage::AnswerDefinition::Undefined),
        positiveAnswer(false),
        lastReadMessage() { }

void PdcControl::MessageQueue::clear() {
    this->inbounds.clear();
    this->outbounds.clear();
    this->expectedAnswerId = UINT32_MAX;
    this->expectedAnswer = PdcControl::CpdlcMessage::AnswerDefinition::Undefined;
    this->positiveAnswer = false;
    this->lastReadMessage = nullptr;
}

void PdcControl::MessageQueue::enqueue(const PdcControl::MessagePtr& message, bool inbound) {
    if (true == inbound)
        this->inbounds.push_back(message);
    else
        this->outbounds.push_back(message);
}

bool PdcControl::MessageQueue::dequeue(PdcControl::MessagePtr& message, bool inbound) {
    bool retval = false;

    if (true == inbound && 0 != this->inbounds.size()) {
        message = this->inbounds.front();
        this->inbounds.pop_front();
        retval = true;
    }
    else if (0 != this->outbounds.size()) {
        message = this->outbounds.front();
        this->outbounds.pop_front();
        retval = true;
    }

    return retval;
}

bool PdcControl::MessageQueue::read(PdcControl::MessagePtr& message) {
    bool retval = 0 != this->inbounds.size();

    if (true == retval) {
        this->lastReadMessage = this->inbounds.front();
        this->inbounds.pop_front();
        message = this->lastReadMessage;
    }

    return retval;
}

bool PdcControl::MessageQueue::answerRequested() const {
    bool retval = PdcControl::CpdlcMessage::AnswerDefinition::No != this->expectedAnswer;
    retval &= PdcControl::CpdlcMessage::AnswerDefinition::Undefined != this->expectedAnswer;
    retval &= PdcControl::CpdlcMessage::AnswerDefinition::NotRequired != this->expectedAnswer;
    return retval;
}

PdcControl::PdcControl() :
        m_airportsLock(),
        m_airports(),
        m_cpdlcCounter(0),
        m_stopHoppiesThread(false),
        m_hoppiesThread(&PdcControl::run, this),
        m_comChannelsLock(),
        m_comChannels() {
    /* TODO get Hoppies ID from configuration */
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    this->m_cpdlcCounter = std::rand() % 10000 + 1789;
}

PdcControl::~PdcControl() {
    this->m_stopHoppiesThread = true;
}

void PdcControl::configure(const types::SystemConfiguration& config) {
    this->m_systemConfig = config;
}

static std::size_t receiveCurl(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::string serverResult = static_cast<char*>(ptr);
    __receivedData += serverResult;
    return size * nmemb;
}

static __inline std::uint8_t __toHex(std::uint8_t x) {
    return x + (x > 9 ? ('A' - 10) : '0');
}

void PdcControl::sendMessage(std::string& message) {
    __receivedData.clear();

    helper::String::stringReplace(message, "%LOGON%", this->m_systemConfig.hoppiesCode);
    /* replace invalid characters by URI markers */
    std::ostringstream os;
    for (std::string::const_iterator it = message.begin(); it != message.end(); ++it) {
        /* valid characters */
        if (0 != isalnum(*it) || '?' == *it || '&' == *it || '/' == *it || ':' == *it || '=' == *it || '.' == *it || '-' == *it)
            os << *it;
        else if (' ' == *it)
            os << '+';
        else
            os << '%' << __toHex(*it >> 4) << __toHex(*it % 16);
    }

    CURLcode result;
    CURL* curl;

    curl = curl_easy_init();
    if (nullptr != curl) {
        /* configure the connection */
        curl_easy_setopt(curl, CURLOPT_URL, os.str().c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receiveCurl);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        /* request the NOTAMs */
        result = curl_easy_perform(curl);

        /* handle the return code of the receive-function */
        if (CURLE_OK != result)
            __receivedData.clear();
        curl_easy_cleanup(curl);
    }
}

bool PdcControl::translateToCpdlc(const PdcControl::Message& message, PdcControl::CpdlcMessage& cpdlcMsg) {
    auto split = helper::String::splitString(message.message, "/");
    if (6 <= split.size()) {
        /* create the CPDLC message block */
        cpdlcMsg.receiver = message.receiver;
        cpdlcMsg.sender = message.sender;
        cpdlcMsg.incomeMessageId = static_cast<std::uint32_t>(std::atoi(split[2].c_str()));
        if (0 != split[3].size())
            cpdlcMsg.repliedToMessageId = static_cast<std::uint32_t>(std::atoi(split[3].c_str()));

        if ("Y" == split[4])
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::Yes;
        else if ("N" == split[4])
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::No;
        else if ("WU" == split[4])
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::WilcoUnable;
        else if ("AN" == split[4])
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::AffirmNegative;
        else if ("R" == split[4])
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::Roger;
        else if ("NE" == split[4])
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::NotRequired;
        else
            cpdlcMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::Undefined;

        /* create the message */
        for (std::size_t c = 5; c < split.size() - 1; ++c)
            cpdlcMsg.message += split[c] + "/";
        cpdlcMsg.message += split.back();
    }

    return 6 <= split.size();
}

void PdcControl::handleMessage(PdcControl::Message& message) {
    auto& channel = this->m_comChannels[message.sender];

    /* register the simple telex message */
    if (PdcControl::MessageType::Telex == message.type || std::string::npos == message.message.find("/data2/", 0)) {
        message.type = PdcControl::MessageType::Telex;
        channel.enqueue(std::make_shared<PdcControl::Message>(message), true);
    }
    /* handle a CPDLC message */
    else if (PdcControl::MessageType::CPDLC == message.type) {
        PdcControl::CpdlcMessage cpdlcMsg;
        if (false == PdcControl::translateToCpdlc(message, cpdlcMsg))
            return;

        /* received a logon message -> reject it */
        if (std::string::npos != cpdlcMsg.message.find("LOGON")) {
            PdcControl::CpdlcMessage rejectMsg;
            rejectMsg.sender = cpdlcMsg.receiver;
            rejectMsg.receiver = cpdlcMsg.sender;
            rejectMsg.answerType = PdcControl::CpdlcMessage::AnswerDefinition::NotRequired;

            bool answerRequired = PdcControl::CpdlcMessage::AnswerDefinition::Undefined != cpdlcMsg.answerType;
            answerRequired &= PdcControl::CpdlcMessage::AnswerDefinition::No != cpdlcMsg.answerType;
            answerRequired &= PdcControl::CpdlcMessage::AnswerDefinition::NotRequired != cpdlcMsg.answerType;
            if (true == answerRequired)
                rejectMsg.incomeMessageId = cpdlcMsg.incomeMessageId;

            rejectMsg.message = "UNABLE";
            channel.enqueue(std::make_shared<PdcControl::CpdlcMessage>(rejectMsg), false);
            return;
        }
        /* check if we received an answer */
        else if (true == channel.answerRequested() && cpdlcMsg.repliedToMessageId == channel.expectedAnswerId) {
            bool positive = std::string::npos != cpdlcMsg.message.find("WILCO");
            positive |= std::string::npos != cpdlcMsg.message.find("AFFIRM");
            positive |= std::string::npos != cpdlcMsg.message.find("ROGER");

            channel.expectedAnswerId = UINT32_MAX;
            channel.expectedAnswer = PdcControl::CpdlcMessage::AnswerDefinition::Undefined;
            channel.positiveAnswer = positive;
        }

        /* enqueue the message */
        channel.enqueue(std::make_shared<PdcControl::CpdlcMessage>(cpdlcMsg), true);
    }
}

void PdcControl::receiveMessages() {
    if (0 == this->m_airports.size())
        return;

    /* get a copy to avoid long-term locks */
    this->m_airportsLock.lock();
    std::list<std::string> airports(this->m_airports);
    this->m_airportsLock.unlock();

    /* iterate over the airports */
    for (const auto& airport : std::as_const(airports)) {
        /* create the URL to receive the data and get it */
        auto message = __datalink + "&to=SERVER&type=POLL";
        helper::String::stringReplace(message, "%SENDER%", airport);
        this->sendMessage(message);

        /* check if the answer is valid */
        if (0 != std::strncmp("ok", __receivedData.c_str(), 2) || __receivedData.size() <= 3)
            return;
        __receivedData.erase(0, 3);

        /* parse the poll answer */
        static std::string delimiter = "}} ";
        std::size_t pos = 0, oldPos = 0;

        while (std::string::npos != (pos = __receivedData.find(delimiter, oldPos))) {
            auto token = __receivedData.substr(oldPos, pos);
            oldPos = pos + 3;

            std::stringstream stream(token);
            PdcControl::Message pdcMsg;
            std::string parsed;
            int i = 0;

            /* create the message */
            pdcMsg.receiver = airport;
            while (std::getline(stream, parsed, ' ')) {
                if (0 == parsed.length())
                    continue;

                /* erase start and end block of message */
                if ('{' == parsed[0])
                    parsed.erase(0, 1);

                if (0 == i) {
                    pdcMsg.sender = parsed;
                }
                else if (1 == i) {
                    if ("telex" == parsed)
                        pdcMsg.type = PdcControl::MessageType::Telex;
                    else if ("cpdlc" == parsed)
                        pdcMsg.type = PdcControl::MessageType::CPDLC;
                }
                else {
                    std::size_t endPos;

                    if (std::string::npos != (endPos = parsed.find('}')))
                        parsed[endPos] = '\0';

                    pdcMsg.message += (0 == pdcMsg.message.length() ? "" : " ") + parsed;
                }

                i += 1;
            }

            /* validate that the flight exists */
            if (false == surveillance::FlightRegistry::instance().flightExists(pdcMsg.sender))
                continue;

            this->m_comChannelsLock.lock();
            this->handleMessage(pdcMsg);
            this->m_comChannelsLock.unlock();
        }
    }
}

bool PdcControl::prepareCpdlc(std::string& url, const PdcControl::MessagePtr& message) {
    /* convert the message and check if it was successful */
    auto cpdlcMsg = std::dynamic_pointer_cast<PdcControl::CpdlcMessage>(message);
    if (nullptr == cpdlcMsg)
        return false;

    /* generate the message ID */
    this->m_cpdlcCounter += 1;
    this->m_cpdlcCounter %= 10000;
    std::uint32_t messageId = this->m_cpdlcCounter;

    /* generate the IDs */
    std::string strMessageId = std::to_string(messageId);
    strMessageId = std::string(4 - strMessageId.length(), '0') + strMessageId;

    /* generate the message */
    url += "&packet=/data2/" + std::to_string(messageId) + "/";
    if (UINT32_MAX != cpdlcMsg->incomeMessageId) {
        std::string strIncomeId = std::to_string(cpdlcMsg->incomeMessageId);
        strIncomeId = std::string(4 - strIncomeId.length(), '0') + strIncomeId;
        url += strIncomeId;
    }
    url += "/";
    if (PdcControl::CpdlcMessage::AnswerDefinition::No == cpdlcMsg->answerType)
        url += "N";
    else if (PdcControl::CpdlcMessage::AnswerDefinition::Yes == cpdlcMsg->answerType)
        url += "Y";
    else if (PdcControl::CpdlcMessage::AnswerDefinition::WilcoUnable == cpdlcMsg->answerType)
        url += "WU";
    else if (PdcControl::CpdlcMessage::AnswerDefinition::AffirmNegative == cpdlcMsg->answerType)
        url += "AN";
    else if (PdcControl::CpdlcMessage::AnswerDefinition::Roger == cpdlcMsg->answerType)
        url += "R";
    else if (PdcControl::CpdlcMessage::AnswerDefinition::NotRequired == cpdlcMsg->answerType)
        url += "NE";
    url += "/";
    url += cpdlcMsg->message;

    std::lock_guard(this->m_comChannelsLock);
    this->m_comChannels[cpdlcMsg->receiver].expectedAnswer = cpdlcMsg->answerType;
    this->m_comChannels[cpdlcMsg->receiver].expectedAnswerId = messageId;

    return true;
}

bool PdcControl::prepareTelex(std::string& url, const PdcControl::MessagePtr& message) {
    if (0 == message->message.length())
        return false;

    url += "&packet=" + message->message;

    return true;
}

static __inline std::string __translateMessageType(PdcControl::MessageType type) {
    switch (type) {
    case PdcControl::MessageType::Telex:
        return "telex";
    case PdcControl::MessageType::CPDLC:
        return "cpdlc";
    default:
        return "";
    }
}

bool PdcControl::sendMessage(const PdcControl::MessagePtr& message) {
    if (0 == message->receiver.length() || PdcControl::MessageType::Unknown == message->type)
        return false;
    if (this->m_airports.cend() == std::find(this->m_airports.cbegin(), this->m_airports.cend(), message->sender))
        return false;

    /* create the message */
    auto url = __datalink;
    url += "&to=" + message->receiver + "&type=" + __translateMessageType(message->type);

    /* finalize the URL */
    bool retval = false;
    if (PdcControl::MessageType::Telex == message->type)
        retval = PdcControl::prepareTelex(url, message);
    else if (PdcControl::MessageType::CPDLC == message->type)
        retval = this->prepareCpdlc(url, message);

    /* something went wrong */
    if (false == retval)
        return false;

    this->sendMessage(url);

    /* check if the answer is valid */
    return 0 == std::strncmp("ok", __receivedData.c_str(), 2);
}

void PdcControl::run() {
    while (false == this->m_stopHoppiesThread) {
        /* unable to communicate with Hoppies */
        if (0 == this->m_systemConfig.hoppiesCode.length()) {
            std::this_thread::sleep_for(1s);
            continue;
        }

        this->receiveMessages();

        this->m_comChannelsLock.lock();

        for (auto it = this->m_comChannels.begin(); this->m_comChannels.end() != it; ++it) {
            PdcControl::MessagePtr message;
            while (true == it->second.dequeue(message, false)) {
                if (false == this->sendMessage(message)) {
                    /* try it multiple times */
                    if (5 > message->failedTransmit) {
                        message->failedTransmit += 1;
                        it->second.enqueue(message, false);
                    }
                }
            }
        }

        this->m_comChannelsLock.unlock();

        std::this_thread::sleep_for(1s);
    }
}

void PdcControl::addAirport(const std::string& icao) {
    std::lock_guard guard(this->m_airportsLock);

    auto it = std::find(this->m_airports.cbegin(), this->m_airports.cend(), icao);
    if (this->m_airports.cend() == it)
        this->m_airports.push_back(icao);
}

void PdcControl::removeAirport(const std::string& icao) {
    std::lock_guard guard(this->m_airportsLock);

    auto it = std::find(this->m_airports.begin(), this->m_airports.end(), icao);
    if (this->m_airports.end() != it)
        this->m_airports.erase(it);
}

PdcControl& PdcControl::instance() {
    static PdcControl __instance;
    return __instance;
}
