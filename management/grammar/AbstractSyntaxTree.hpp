/*
 * @brief Defines the abstract syntax tree
 * @file grammar/AbstractSyntaxTree.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <vector>

#pragma warning(disable: 4459)
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#pragma warning(default: 4459)

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstNull;
            struct AstNodeVector;
            struct AstString;
            struct AstInteger;
            struct AstFloat;
            struct AstKeyValue;
            struct AstNotam;

            /**< Defines the generic node structure */
            typedef boost::variant<
                AstNull,
                AstString,
                AstInteger,
                AstFloat,
                boost::recursive_wrapper<AstNodeVector>,
                AstKeyValue,
                AstNotam
            > AstNode;

            struct AstNull { };

            struct AstNodeVector : public std::vector<AstNode> { };

            struct AstString : public std::string { };

            struct AstInteger {
                std::int64_t value;
                explicit inline AstInteger(const std::int64_t& v = 0) : value(v) { }
            };

            struct AstFloat {
                float value;
                explicit inline AstFloat(const float& v = 0.0f) : value(v) { }
            };

            struct AstKeyValue {
                std::string key;
                std::string value;
            };

            struct AstNotam {
                std::string title;
                std::string qCode;
                std::string icao;
                std::string startTime;
                std::string endTime;
                std::string content;
            };

            struct AstDebug : boost::static_visitor<> {
                std::ostringstream stream;
                int                depth;

                AstDebug(const AstNode& node) :
                        stream(), depth(0) {
                    boost::apply_visitor(*this, node);
                }

                inline std::string tab() {
                    return std::string(2 * this->depth, ' ');
                }

                void operator()(const AstNull&) {
                    this->stream << this->tab() << "NULL" << std::endl;
                }

                void operator()(const AstInteger& value) {
                    this->stream << this->tab() << "integer: " << value.value << std::endl;
                }

                void operator()(const AstFloat& value) {
                    this->stream << this->tab() << "float: " << value.value << std::endl;
                }

                void operator()(const AstString& value) {
                    this->stream << this->tab() << "string: " << value << std::endl;
                }

                void operator()(const std::string& text) {
                    this->stream << this->tab() << "text: \"" << text << "\"" << std::endl;
                }

                void recurse(const AstNode& node) {
                    this->depth += 1;
                    boost::apply_visitor(*this, node);
                    this->depth -= 1;
                }

                void operator()(const AstNodeVector& nodevector) {
                    this->stream << this->tab() << '{' << std::endl;
                    for (const AstNode& node : std::as_const(nodevector))
                        this->recurse(node);
                    this->stream << this->tab() << '}' << std::endl;
                }

                void operator()(const AstKeyValue& pair) {
                    this->stream << this->tab() << "[ " << pair.key << ", " << pair.value << " ]" << std::endl;
                }

                void operator()(const AstNotam& notam) {
                    this->stream << this->tab() << "Title: " << notam.title << std::endl;
                    this->stream << this->tab() << "qCode: " << notam.qCode << std::endl;
                    this->stream << this->tab() << "ICAO: " << notam.icao << std::endl;
                    this->stream << this->tab() << "Start: " << notam.startTime << std::endl;
                    this->stream << this->tab() << "End: " << notam.endTime << std::endl;
                    this->stream << this->tab() << "Content: " << notam.content << std::endl;
                }
            };
        }
    }
}

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstInteger,
    (std::int64_t, value)
);

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstFloat,
    (float, value)
);

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstKeyValue,
    (std::string, key)
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstNotam,
    (std::string, title)
    (std::string, qCode)
    (std::string, icao)
    (std::string, startTime)
    (std::string, endTime)
    (std::string, content)
)
