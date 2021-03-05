/*
 * @brief Defines the airport file format
 * @file formats/AirportFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <formats/EseFileFormat.h>
#include <formats/FileFormat.h>
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
         * - [AIRPORT] Defines some airport specific parameters
         * - [DEPARTURES] Defines all SIDs and the destination constraints
         * - [STANDS] Defines all stands with generic constraint definitions
         * - [PRIORITIES] Defines the airline to stand assignment priorities
         * - [TAXIWAYS] Defines all taxiway configuration components
         *
         * The airport-block contains information about possible independent approaches or independent departures.
         * Two different types of independent approaches are possible, the normal IPA and the PRM approach.
         * The following block describes the syntax to define IPA or PRM runway combinations.
         * If the IPD is not set, the system assumes direct dependencies between two different runways
         * @code{.xml}
         * [IPA,PRM]:RUNWAY1:RUNWAY2
         * IPD:RUNWAY1:RUNWAY2
         * @endcode
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
         *
         * The taxiways block defines the taxiway structure of the airport.
         * It is used to configure the safety net functions for the ground.
         * @code{.xml}
         * HOLD:LVP:RUNWAY:NAME:WTC:HOLDINGPOINT_LAT:HOLDINGPOINT_LON:RUNWAYDIRECTION_LAT:RUNWAYDIRECTION_LON
         * @endcode
         *
         * <table>
         *   <caption id="multi_row">Setting entries</caption>
         *   <tr>
         *     <th>Name</th><th>Description</th><th>Default value</th><th>Unit</th>
         *   </tr>
         *   <tr>
         *     <td>LVP</td>
         *     <td>Indicates if the holding point is used during low visibility procedures are active</td>
         *     <td>N</td><td>L (for LVP),N (for Normal)</td>
         *   </tr>
         *   <tr>
         *     <td>RUNWAY</td>
         *     <td>The runway's name that activates the holding point</td>
         *     <td></td><td></td>
         *   </tr>
         *   <tr>
         *     <td>NAME</td>
         *     <td>The holding point's name</td>
         *     <td></td><td></td>
         *   </tr>
         *   <tr>
         *     <td>WTC</td>
         *     <td>The maximum allowed WTC for departures at this holding point</td>
         *     <td></td><td></td>
         *   </tr>
         *   <tr>
         *     <td>HOLDINGPOINT_LAT</td>
         *     <td>The latitude coordinate  of the holding point's center position</td>
         *     <td></td><td>ES-formatted coordinate string</td>
         *   </tr>
         *   <tr>
         *     <td>HOLDINGPOINT_LON</td>
         *     <td>The longitude coordinate of the holding point's center position</td>
         *     <td></td><td>ES-formatted coordinate string</td>
         *   </tr>
         *   <tr>
         *     <td>RUNWAYDIRECTION_LAT</td>
         *     <td>The latitude coordinate of the next point close to the runway to describe the heading of the taxiway</td>
         *     <td></td><td>ES-formatted coordinate string</td>
         *   </tr>
         *   <tr>
         *     <td>RUNWAYDIRECTION_LON</td>
         *     <td>The longitude coordinate of the next point close to the runway to describe the heading of the taxiway</td>
         *     <td></td><td>ES-formatted coordinate string</td>
         *   </tr>
         * </table>
         */
        class AirportFileFormat : public FileFormat {
        private:
#ifndef DOXYGEN_IGNORE
            std::string m_filename;

            static bool parseSid(const std::vector<std::string>& elements, types::StandardInstrumentDeparture& sid);
            static bool parseConstraint(const std::vector<std::string>& elements, types::DestinationConstraint& constraint);
            bool parseDepartures(types::AirportConfiguration& config, const std::vector<std::string>& lines, std::uint32_t lineOffset);
            static bool parseStandDefinition(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseWingspan(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseLength(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseHeight(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseWtc(const std::string& categories, std::list<types::Aircraft::WTC>& list);
            static bool parseEngineType(const std::string& types, std::list<types::Aircraft::EngineType>& list);
            bool parseStands(types::AirportConfiguration& config, const std::vector<std::string>& lines, std::uint32_t lineOffset);
            static bool parsePriorities(const std::vector<std::string>& elements, types::StandPriorities& priorities);
            bool parsePriorities(types::AirportConfiguration& config, const std::vector<std::string>& lines, std::uint32_t lineOffset);
            static bool parseHoldingPoint(const std::vector<std::string>& elements, types::HoldingPoint& holdingPoint);
            bool parseTaxiways(types::AirportConfiguration& config, const std::vector<std::string>& lines, std::uint32_t lineOffset);
            bool parseAirportData(types::AirportConfiguration& config, const std::vector<std::string>& lines, std::uint32_t lineOffset);

        public:
            /**
             * @brief Parses an airport configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            AirportFileFormat(const std::string& filename);

            /**
             * @brief Parses the set configuration file
             * @param[out] config The resulting configuration
             * @return True if the configuration file was valid, else false
             */
            bool parse(types::AirportConfiguration& config);
#endif
        };
    }
}
