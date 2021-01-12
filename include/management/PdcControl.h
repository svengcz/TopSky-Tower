/*
 * @brief Defines a PDC control system
 * @file surveillance/PdcControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <functional>
#include <thread>
#include <list>
#include <map>
#include <mutex>

#include <types/Flight.h>
#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace management {
        /**
         * @brief The PDC control is responsible to handle the communication with the pilots via PDC.
         * @ingroup management
         *
         * It uses <a href="https://www.hoppie.nl/acars/">Hoppies system</a> as the backend for the PDC-communication with the pilots.
         * The PDC control rejects logon-calls, etc. and is only used for clearances and normal text messages.
         *
         * The control handles all incoming and outgoing messages.
         * If it receives an incoming message it performs a message validation and checks if automated response is required.
         * The system automatically rejects LOGON messages, because ground stations do not allow logons for clearances.
         *
         * The PDC system supports the following outgoing messages:
         * - Standby: Sends the AC a stand-by message that does not require an answer
         * - Clearance: Sends the clearance with the initial climb, etc.
         *
         * It is designed in that way that it can handle CPDLC and TELEX messages.
         * It provides an indicator and a sound-ping as soon as new messages are received.
         * 
         * ![PDC indicator](doc/imgs/PdcIndicator.png)
         * 
         * A reader is available that shows the oldest, received message.
         * If multiple messages are received is it required to open multiple viewer.
         * 
         * ![PDC reader](doc/imgs/PdcReader.png)
         * 
         * It is possible to send CPDLC stand-by messages by one click or send the departure clearance.
         * The clearance opens a window that allows a review of the clearance and add a custom message.
         * In the end does it send a CPDLC-message to the aircraft.
         * 
         * ![PDC departure clearance](doc/imgs/PdcClearance.png)
         */
        class PdcControl {
        public:
#ifndef DOXYGEN_IGNORE
            /**
             * @brief Defines the different message types of the system
             */
            enum class MessageType {
                Unknown = 0, /**< Defines an unknown and invalid message type */
                CPDLC = 1, /**< Defines a CPDLC message */
                Telex = 2  /**< Defines a generic telex message */
            };

            /**
             * @brief Defines a generic message which can be used as a base for other messages and also directly for telex messages
             */
            struct Message {
                std::uint32_t failedTransmit; /**< Counter to track failed transmissions */
                std::string sender;           /**< Defines the sender's callsign */
                std::string receiver;         /**< Defines the receiver's callsign */
                MessageType type;             /**< Defines the message type */
                std::string message;          /**< Defines the content of the message */

                /**
                 * @brief Creates an empty Telex message
                 */
                Message() :
                        failedTransmit(0),
                        sender(),
                        receiver(),
                        type(PdcControl::MessageType::Unknown),
                        message() { }
                /**
                 * @brief Destructor to enable polymorphism
                 */
                virtual ~Message() { }
            };

            typedef std::shared_ptr<PdcControl::Message> MessagePtr; /**< Defines a shorter name for a message */

            /**
             * @brief Defines the CPDLC message
             */
            struct CpdlcMessage : Message {
                /**
                 * @brief Defines the different response types
                 */
                enum class AnswerDefinition {
                    Undefined = 0,      /**! No response defined */
                    No = 1,             /**! No response expected */
                    Yes = 2,            /**! An answer is required */
                    WilcoUnable = 3,    /**! Expecting Wilco/Unable as answer */
                    AffirmNegative = 4, /**! Expecting Affirm/Negative as answer */
                    Roger = 5,          /**! Expecting Roger as answer */
                    NotRequired = 6     /**! No answer required */
                };

                std::uint32_t    incomeMessageId;    /**< Defines the message ID of the sender */
                std::uint32_t    repliedToMessageId; /**< Defines the ID which was answered to */
                AnswerDefinition answerType;         /**< Defines the answer type */

                CpdlcMessage() :
                        Message(),
                        incomeMessageId(UINT32_MAX),
                        repliedToMessageId(UINT32_MAX),
                        answerType(CpdlcMessage::AnswerDefinition::Undefined) {
                    this->type = PdcControl::MessageType::CPDLC;
                }
            };
            typedef std::shared_ptr<CpdlcMessage> CpdlcMessagePtr; /**< Defines a more readable type */

            /**
             * @brief Defines a PDC-based clearance message
             */
            struct ClearanceMessage : CpdlcMessage {
                std::string destination;           /**< Defines the destination airport */
                std::string sid;                   /**< Defines the standard instrument departure */
                std::string runway;                /**< Defines the departure runway */
                std::string frequency;             /**< Defines the next frequency */
                std::string targetStartupTime;     /**< Defines the target start-up time (TSAT) */
                std::string calculatedTakeOffTime; /**< Defines the calculated take-off time (CTOT) */
                std::string clearanceLimit;        /**< Defines the climb clearance limit */
                std::string squawk;                /**< Defines the assigned squawk */

                /**
                 * @brief Creates an empty PDC-clearance message
                 */
                ClearanceMessage() :
                        CpdlcMessage(),
                        destination(),
                        sid(),
                        runway(),
                        frequency(),
                        targetStartupTime(),
                        calculatedTakeOffTime(),
                        clearanceLimit(),
                        squawk() {
                    this->answerType = CpdlcMessage::AnswerDefinition::WilcoUnable;
                }
            };
            typedef std::shared_ptr<ClearanceMessage> ClearanceMessagePtr; /**< Defines a more readable type */

        private:
            struct MessageQueue {
                std::list<PdcControl::MessagePtr>          inbounds;
                std::list<PdcControl::MessagePtr>          outbounds;
                std::uint32_t                              expectedAnswerId;
                PdcControl::CpdlcMessage::AnswerDefinition expectedAnswer;
                bool                                       positiveAnswer;
                PdcControl::MessagePtr                     lastReadMessage;

                MessageQueue();
                void clear();
                void enqueue(const PdcControl::MessagePtr& message, bool inbound);
                bool dequeue(PdcControl::MessagePtr& message, bool inbound);
                bool read(PdcControl::MessagePtr& message);
                bool answerRequested() const;
            };

            std::mutex                          m_airportsLock;
            std::list<std::string>              m_airports;
            std::int32_t                        m_cpdlcCounter;
            volatile bool                       m_stopHoppiesThread;
            std::thread                         m_hoppiesThread;
            std::mutex                          m_comChannelsLock;
            std::map<std::string, MessageQueue> m_comChannels;
            std::function<void()>               m_notification;

            PdcControl();
            void receiveMessages();
            void sendMessage(std::string& message);
            static bool translateToCpdlc(const Message& message, CpdlcMessage& cpdlcMsg);
            void handleMessage(Message& message);
            bool prepareCpdlc(std::string& url, const MessagePtr& message);
            bool prepareTelex(std::string& url, const MessagePtr& message);
            static CpdlcMessagePtr prepareClearance(const ClearanceMessagePtr& message);
            bool sendMessage(const MessagePtr& message);
            void run();

        public:
            /**
             * @brief Destroys all internal structures
             */
            ~PdcControl();

            PdcControl(const PdcControl& other) = delete;
            PdcControl(PdcControl&& other) = delete;

            PdcControl& operator=(const PdcControl& other) = delete;
            PdcControl& operator=(PdcControl&& other) = delete;

            /**
             * @brief Adds an airport to the PDC communication
             * @param[in] icao The ICAO of the new airport
             */
            void addAirport(const std::string& icao);
            /**
             * @brief Removes an airport out of the PDC communication
             * @param[in] icao The ICAO of the removable airport
             */
            void removeAirport(const std::string& icao);
            /**
             * @brief Checks if an airport is logged into the system
             * @param[in] icao The airport's ICAO code
             * @return True if it is online, else false
             */
            bool airportLoggedIn(const std::string& icao) const;
            /**
             * @brief Checks if an airport is active in Hoppies
             * @param[in] icao The ICAO of the airport
             * @return True if the airport is online, else false
             */
            bool airportOnline(const std::string& icao) const;
            /**
             * @brief Checks if new messages are available
             * @param[in] flight The flight structure
             * @return True if new messages are available, else false
             */
            bool messagesAvailable(const types::Flight& flight) const;
            /**
             * @brief Returns the next message in the queue
             * @param[in] flight The flight structure
             * @return The first message in the queue
             */
            MessagePtr nextMessage(const types::Flight& flight);
            /**
             * @brief Sends a stand-by message
             * @param[in] flight The flight that needs to stand-by
             */
            void sendStandbyMessage(const types::Flight& flight);
            /**
             * @brief Sends the clearance message
             * @param[in] message The clearance message
             */
            void sendClearanceMessage(const ClearanceMessagePtr& message);
            /**
             * @brief Registers a callback that is triggered as soon as a new PDC message received
             * @tparam T The element which registers the callback
             * @tparam F The callback function
             * @param[in] instance The instance which registers the callback
             * @param[in] cbFunction The callback function
             */
            template <typename T, typename F>
            void registerNotificationCallback(T* instance, F cbFunction) {
                std::function<void()> func = std::bind(cbFunction, instance);
                this->m_notification = func;
            }
            /**
             * @brief Returns the PDC control singleton
             * @return The PDC control system
             */
            static PdcControl& instance();
#endif
        };
    }
}
