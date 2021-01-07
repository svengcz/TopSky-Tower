/*
 * @brief Defines the airport file format
 * @file configuration/AirportFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <formats/EseFileFormat.h>
#include <types/AirportConfiguration.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the parser of airport configurations
         * @ingroup format
         *
         * The airport file format contains all airport specific information.
         * Every airport has it's own configuration file and the file's name needs the following format:
         * 'TopSkyTowerAirportICAO.txt' (i.e. 'TopSkyTowerAirportEDDB.txt')
         *
         * The airport configuration is defined into multiple sections:
         * - [DEPARTURES] Defines all SIDs and the destination constraints
         * - [STANDS] Defines all stands with generic constraint definitions
         * - [PRIORITIES] Defines the airline to stand assignment priorities
         *
         * The departures-block contains two different entries.
         * The first entry is the definition of the SID.
         * @code{.xml}
         * SID:SIDNAME:INITCLIMB:[STEPCLIMBS]:[ENGINETYPE]:[XDR]:[NAV]:[MINFL]:[MAXFL]
         * @endcode
         *
         * All entries in [] are optional.
         * <table>
         *   <caption id="multi_row">Setting entries</caption>
         *   <tr>
         *     <th>Name</th><th>Description</th><th>Default value</th><th>Unit</th>
         *   </tr>
         *   <tr>
         *     <td>SIDNAME</td>
         *     <td>Defines the SID's name</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>INITCLIMB</td>
         *     <td>Defines the initial climb clearance</td>
         *     <td></td><td>Feet</td>
         *   </tr>
         *   <tr>
         *     <td>STEPCLIMBS</td>
         *     <td>Defines if the SID has climb constraints.</td>
         *     <td>0</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>ENGINETYPE</td>
         *     <td>Defines the engine type restriction (T: Piston, Turboprop; J: Jet; E: Electrical)</td>
         *     <td>None</td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>XDR</td>
         *     <td>Defines if the SID requires a transponder.</td>
         *     <td>0</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>NAV</td>
         *     <td>Defines if the SID requires RNAV capabilities.</td>
         *     <td>0</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>MINFL</td>
         *     <td>Defines the minimum requested flight level</td>
         *     <td>0</td><td>Feet</td>
         *   </tr>
         *   <tr>
         *     <td>MAXFL</td>
         *     <td>Defines the maximum requested flight level</td>
         *     <td>90000</td><td>Feet</td>
         *   </tr>
         * </table>
         *
         * A destination constraint is defined by the following entry:
         * @code{.xml}
         * CSTR:ICAO:EVENODD:[MINFL]:[MAXFL]
         * @endcode
         *
         * All entries in [] are optional.
         * <table>
         *   <caption id="multi_row">Setting entries</caption>
         *   <tr>
         *     <th>Name</th><th>Description</th><th>Default value</th><th>Unit</th>
         *   </tr>
         *   <tr>
         *     <td>ICAO</td>
         *     <td>The ICAO code of the destination airport</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>EVENODD</td>
         *     <td>Defines if the requested flight level needs to be even or odd</td>
         *     <td></td><td>EVEN/ODD</td>
         *   </tr>
         *   <tr>
         *     <td>MINFL</td>
         *     <td>Defines the minimum requested flight level</td>
         *     <td>0</td><td>Feet</td>
         *   </tr>
         *   <tr>
         *     <td>MAXFL</td>
         *     <td>Defines the maximum requested flight level</td>
         *     <td>90000</td><td>Feet</td>
         *   </tr>
         * </table>
         *
         * The stands block defines all stands of the airport.
         * @code{.xml}
         * STAND:ICAO:NAME:LATITUDE:LONGITUDE:RADIUS
         * [WINGSPAN:MAXWINGSPAN]
         * [WINGSPAN:MINWINGSPAN:MAXWINGSPAN]
         * [LENGTH:MAXLENGTH]
         * [LENGTH:MINLENGTH:MAXLENGTH]
         * [HEIGHT:MAXHEIGHT]
         * [HEIGHT:MINHEIGHT:MAXHEIGHT]
         * [WTC:CODES]
         * [NOTWTC:CODES]
         * [ENGINETYPE:TYPE]
         * [NOTENGINETYPE:TYPE]
         * [BLOCKS:STANDS]
         * [PRIORITY:VALUE]
         * [MANUAL]
         * @endcode
         * 
         * All entries in [] are optional:
         * <table>
         *   <caption id="multi_row">Setting entries</caption>
         *   <tr>
         *     <th>Name</th><th>Description</th><th>Default value</th><th>Unit</th>
         *   </tr>
         *   <tr>
         *     <td>ICAO</td>
         *     <td>The ICAO code of the destination airport</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>WINGSPAN</td>
         *     <td>Two different configurations can be used. One defines only the maximum wingspan and the other the minimum and maximum</td>
         *     <td>Min: 0, Max: 1000</td><td>Meters</td>
         *   </tr>
         *   <tr>
         *     <td>LENGTH</td>
         *     <td>Two different configurations can be used. One defines only the maximum length and the other the minimum and maximum</td>
         *     <td>Min: 0, Max: 1000</td><td>Meters</td>
         *   </tr>
         *   <tr>
         *     <td>HEIGHT</td>
         *     <td>Two different configurations can be used. One defines only the maximum height and the other the minimum and maximum</td>
         *     <td>Min: 0, Max: 1000</td><td>Meters</td>
         *   </tr>
         *   <tr>
         *     <td>WTC</td>
         *     <td>Defines the allowed WTC categories. The categories are directly concatenated (i.e. WTC:HJ)</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>NOTWTC</td>
         *     <td>Defines the not allowed WTC categories. The categories are directly concatenated (i.e. NOTWTC:HJ)</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>ENGINETYPE</td>
         *     <td>Defines the allowed engine types. The categories are directly concatenated (i.e. ENGINETYPE:TJ or ENGINETYPE:T,J)</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>NOTENGINETYPE</td>
         *     <td>Defines the not allowed engine types. The categories are directly concatenated (i.e. NOTENGINETYPE:TJ or NOTENGINETYPE:T,J)</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>BLOCKS</td>
         *     <td>Defines the stands that are blocked if this stand is occupied (i.e. BLOCKS:B04,B05).</td>
         *     <td></td><td>Stand names</td>
         *   </tr>
         *   <tr>
         *     <td>PRIORITY</td>
         *     <td>Defines a single value that describes the overall stand's priority between -5 and 5.</td>
         *     <td>0</td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>MANUAL</td>
         *     <td>Defines if the stand is ignored in the automatic stand assignment</td>
         *     <td></td><td>None</td>
         *   </tr>
         * </table>
         *
         * The priorities block defines the airline specific priorities of stands.
         * It is used to optimize the stand assignments of arriving aircrafts.
         * The concept is that flexible that an airline can define multiple priorities for different stands.
         * @code{.xml}
         * AIRLINES:ICAOCODES
         * STANDS:PRIORITY:STANDS
         * @endcode
         *
         * <table>
         *   <caption id="multi_row">Setting entries</caption>
         *   <tr>
         *     <th>Name</th><th>Description</th><th>Default value</th><th>Unit</th>
         *   </tr>
         *   <tr>
         *     <td>AIRLINES</td>
         *     <td>The ICAO codes of the airlines. It is possible to share priorities between airlines (i.e. AIRLINES:DLH:SWR)</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>STANDS</td>
         *     <td>The stands block defines stands for a specific priority. The priority needs to be between -5 and 5. (i.e. STANDS:0:B02:B03)</td>
         *     <td></td><td>Stand names</td>
         *   </tr>
         * </table>
         */
        class AirportFileFormat {
        private:
            types::AirportConfiguration m_configuration;

            static bool parseSid(const std::vector<std::string>& elements, types::StandardInstrumentDeparture& sid);
            static bool parseConstraint(const std::vector<std::string>& elements, types::DestinationConstraint& constraint);
            bool parseDepartures(const std::vector<std::string>& lines);
            static bool parseStandDefinition(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseWingspan(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseLength(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseHeight(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseWtc(const std::string& categories, std::list<types::Aircraft::WTC>& list);
            static bool parseEngineType(const std::string& types, std::list<types::Aircraft::EngineType>& list);
            bool parseStands(const std::vector<std::string>& lines);
            static bool parsePriorities(const std::vector<std::string>& elements, types::StandPriorities& priorities);
            bool parsePriorities(const std::vector<std::string>& lines);

        public:
            /**
             * @brief Parses an airport configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            AirportFileFormat(const std::string& filename);

            /**
             * @brief Returns the airport configuration
             * @return The airport configuration
             */
            const types::AirportConfiguration& configuration() const;
        };
    }
}
