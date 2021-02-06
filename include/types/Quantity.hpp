/*
 * @brief Defines and implements classes to define physical SI units
 * @file types/Quantity.hpp
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <ratio>

#include <helper/Math.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Defines the base class to define different physical SI units
         * @ingroup types
         *
         * The template arguments are used to define the exponent of the SI unit.
         * These Quantity definitions are the base to describe all physical relations.
         *
         * By use of this implementation is it possible to ensure, that the SI units
         * are as expected after the calculations.
         *
         * This avoids logical errors and it is useful to avoid the auto-keyword otherwise
         * is it not possible to check the correctness of the SI units.
         *
         * @tparam M The exponent of the mass parts
         * @tparam L The exponent of the length parts
         * @tparam T The exponent of the time parts
         * @tparam A The exponent of the angular parts
         */
        template <typename M, typename L, typename T, typename A>
        class Quantity {
        private:
            float m_value;

        public:
            /**
             * @brief Initializes a quantity with the default value
             */
            constexpr Quantity() : m_value(0.0f) { }
            /**
             * @brief Initializes the quantity with a given value
             * @param[in] value The set value
             */
            constexpr Quantity(float value) : m_value(value) { }

            /**
             * @brief Adds rhs into this instance and returns the updated instance
             * @param[in] rhs The right-hand-side component
             * @return The resulting quantity with the updated value
             */
            constexpr Quantity const& operator+=(const Quantity& rhs) {
                this->m_value += rhs.m_value;
                return *this;
            }
            /**
             * @brief Substracts rhs into this instance and returns the updated instance
             * @param[in] rhs The right-hand-side component
             * @return The resulting quantity with the updated value
             */
            constexpr Quantity const& operator-=(const Quantity& rhs) {
                this->m_value -= rhs.m_value;
                return *this;
            }

            /**
             * @brief Returns the value in SI units
             * @return The contained value
             */
            constexpr float value() const {
                return this->m_value;
            }
            /**
             * @brief Sets the value in SI units
             * @param[in] value The new SI unit based value
             */
            constexpr void setValue(float value) {
                this->m_value = value;
            }
            /**
             * @brief Converts the value into a specific unit
             * @param[in] rhs The scaling factor to convert the value
             * @return The converted value
             */
            constexpr float convert(const Quantity& rhs) const {
                return this->m_value / rhs.m_value;
            }
            /**
             * @brief Calculates the square-root and updates the SI units as well
             * @return The resulting instance with updated template arguments
             */
            constexpr Quantity<std::ratio_divide<M, std::ratio<2>>, std::ratio_divide<L, std::ratio<2>>,
                               std::ratio_divide<T, std::ratio<2>>, std::ratio_divide<A, std::ratio<2>>> sqrt() const {
                return Quantity<std::ratio_divide<M, std::ratio<2>>, std::ratio_divide<L, std::ratio<2>>,
                                std::ratio_divide<T, std::ratio<2>>, std::ratio_divide<A, std::ratio<2>>>(std::sqrtf(this->m_value));
            }
            /**
             * @brief Calculates the absolute of the contained value and returns it as a new Quantity instance
             * @return The absolute value instance
             */
            constexpr Quantity<M, L, T, A> abs() const {
                return Quantity<M, L, T, A>(std::abs(this->m_value));
            }
        };

        /** The mass specialization [kg] */
        typedef Quantity<std::ratio<1>, std::ratio<0>, std::ratio<0>, std::ratio<0>>  Mass;
        /** The length specialization [m] */
        typedef Quantity<std::ratio<0>, std::ratio<1>, std::ratio<0>, std::ratio<0>>  Length;
        /** The time specialization [s] */
        typedef Quantity<std::ratio<0>, std::ratio<0>, std::ratio<1>, std::ratio<0>>  Time;
        /** The angle specialization [rad] */
        typedef Quantity<std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<1>>  Angle;
        /** The velocity specialization [m/s] */
        typedef Quantity<std::ratio<0>, std::ratio<1>, std::ratio<-1>, std::ratio<0>> Velocity;
        /** The acceleration specialization [m/(s*s)] */
        typedef Quantity<std::ratio<0>, std::ratio<1>, std::ratio<-2>, std::ratio<0>> Acceleration;
        /** The angular velocity specialization [1/s] */
        typedef Quantity<std::ratio<0>, std::ratio<0>, std::ratio<-1>, std::ratio<1>> AngularVelocity;
        /** The angular acceleration specialization [1/(s*s)] */
        typedef Quantity<std::ratio<0>, std::ratio<0>, std::ratio<-2>, std::ratio<1>> AngularAcceleration;

        /**
         * @brief Adds rhs to lhs and returns a new instance
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return The resulting quantity with the updated value
         */
        template <typename M, typename L, typename T, typename A>
        constexpr Quantity<M, L, T, A> operator+(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return Quantity<M, L, T, A>(lhs.value() + rhs.value());
        }
        /**
         * @brief Substracts rhs to lhs and returns a new instance
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return The resulting quantity with the updated value
         */
        template <typename M, typename L, typename T, typename A>
        constexpr Quantity<M, L, T, A> operator-(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return Quantity<M, L, T, A>(lhs.value() - rhs.value());
        }
        /**
         * @brief Multiplies rhs to lhs and returns a new instance
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return The resulting quantity with the updated value
         */
        template <typename M1, typename L1, typename T1, typename A1,
                  typename M2, typename L2, typename T2, typename A2>
        constexpr Quantity<std::ratio_add<M1, M2>, std::ratio_add<L1, L2>,
                           std::ratio_add<T1, T2>, std::ratio_add<A1, A2>>
                operator*(const Quantity<M1, L1, T1, A1>& lhs, const Quantity<M2, L2, T2, A2>& rhs) {
            return Quantity<std::ratio_add<M1, M2>, std::ratio_add<L1, L2>,
                            std::ratio_add<T1, T2>, std::ratio_add<A1, A2>>(lhs.value() * rhs.value());
        }
        /**
         * @brief Multiplies lhs to rhs and returns a new instance
         * @param[in] lhs The SI-unit free factor
         * @param[in] rhs The right-hand-side component
         * @return The resulting quantity with the updated value
         */
        template <typename M, typename L, typename T, typename A>
        constexpr Quantity<M, L, T, A> operator*(const float& lhs, const Quantity<M, L, T, A>& rhs) {
            return Quantity<M, L, T, A>(lhs * rhs.value());
        }
        /**
         * @brief Multiplies rhs to lhs and returns a new instance
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The SI-unit free factor
         * @return The resulting quantity with the updated value
         */
        template <typename M, typename L, typename T, typename A>
        constexpr Quantity<M, L, T, A> operator*(const Quantity<M, L, T, A>& lhs, const float& rhs) {
            return Quantity<M, L, T, A>(lhs.value() * rhs);
        }
        /**
         * @brief Divides rhs from lhs and returns a new instance
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return The resulting quantity with the updated value
         */
        template <typename M1, typename L1, typename T1, typename A1,
                  typename M2, typename L2, typename T2, typename A2>
        constexpr Quantity<std::ratio_subtract<M1, M2>, std::ratio_subtract<L1, L2>,
                           std::ratio_subtract<T1, T2>, std::ratio_subtract<A1, A2>>
                operator/(const Quantity<M1, L1, T1, A1>& lhs, const Quantity<M2, L2, T2, A2>& rhs) {
            return Quantity<std::ratio_subtract<M1, M2>, std::ratio_subtract<L1, L2>,
                            std::ratio_subtract<T1, T2>, std::ratio_subtract<A1, A2>>(lhs.value() / rhs.value());
        }
        /**
         * @brief Divides rhs from lhs and returns a new instance
         * @param[in] lhs The SI-unit free factor
         * @param[in] rhs The right-hand-side component
         * @return The resulting quantity with the updated value
         */
        template <typename M, typename L, typename T, typename A>
        constexpr Quantity<std::ratio_subtract<std::ratio<0>, M>, std::ratio_subtract<std::ratio<0>, L>,
                           std::ratio_subtract<std::ratio<0>, T>, std::ratio_subtract<std::ratio<0>, A>>
                operator/(const float& lhs, const Quantity<M, L, T, A>& rhs) {
            return Quantity<std::ratio_subtract<std::ratio<0>, M>, std::ratio_subtract<std::ratio<0>, L>,
                            std::ratio_subtract<std::ratio<0>, T>, std::ratio_subtract<std::ratio<0>, A>>(lhs / rhs.value());
        }
        /**
         * @brief Multiplies lhs from rhs and returns a new instance
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The SI-unit free factor
         * @return The resulting quantity with the updated value
         */
        template <typename M, typename L, typename T, typename A>
        constexpr Quantity<M, L, T, A> operator/(const Quantity<M, L, T, A>& lhs, const float& rhs) {
            return Quantity<M, L, T, A>(lhs.value() / rhs);
        }

        /**
         * @brief Compares if two instance are equal or almost equal
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return True if the instances are almost equal, else false
         */
        template <typename M, typename L, typename T, typename A>
        constexpr bool operator==(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return true == helper::Math::almostEqual(lhs.value(), rhs.value(), 1e-8f);
        }
        /**
         * @brief Compares if two instance are not equal
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return True if the instances are not equal
         */
        template <typename M, typename L, typename T, typename A>
        constexpr bool operator!=(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return false == helper::Math::almostEqual(lhs.value(), rhs.value(), 1e-8f);
        }
        /**
         * @brief Compares if lhs is smaller or equal compared to rhs
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return True if lhs is smaller or equal compared to rhs
         */
        template <typename M, typename L, typename T, typename A>
        constexpr bool operator<=(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return lhs.value() <= rhs.value();
        }
        /**
         * @brief Compares if lhs is smaller compared to rhs
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return True if lhs is smaller compared to rhs
         */
        template <typename M, typename L, typename T, typename A>
        constexpr bool operator<(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return lhs.value() < rhs.value();
        }
        /**
         * @brief Compares if lhs is greater or equal compared to rhs
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return True if lhs is greater or equal compared to rhs
         */
        template <typename M, typename L, typename T, typename A>
        constexpr bool operator>=(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return lhs.value() >= rhs.value();
        }
        /**
         * @brief Compares if lhs is greater compared to rhs
         * @param[in] lhs The left-hand-side component
         * @param[in] rhs The right-hand-side component
         * @return True if lhs is greater compared to rhs
         */
        template <typename M, typename L, typename T, typename A>
        constexpr bool operator>(const Quantity<M, L, T, A>& lhs, const Quantity<M, L, T, A>& rhs) {
            return lhs.value() > rhs.value();
        }

        /**< Defines a kilogram */
        constexpr Mass kilogram(1.0f);
        constexpr Mass pound = 0.453592f * kilogram;
        /**< Defines the literal of kilograms */
        constexpr Mass operator"" _kg(long double value) { return Mass(static_cast<float>(value)); }
        /**< Defines the literal of kilograms */
        constexpr Mass operator"" _kg(unsigned long long int value) { return Mass(static_cast<float>(value)); }
        /**< Defines the literal of pounds */
        constexpr Mass operator"" _lbs(long double value) { return static_cast<float>(value) * pound; }
        /**< Defines the literal of pounds */
        constexpr Mass operator"" _lbs(unsigned long long int value) { return static_cast<float>(value) * pound; }

        /**< Defines a metres */
        constexpr Length metre(1.0f);
        /**< Defines a feet */
        constexpr Length feet = 0.3048f * metre;
        /**< Defines a kilometres */
        constexpr Length kilometre = 1000.0f * metre;
        /**< Defines a nautical mile */
        constexpr Length nauticmile = 1852.0f * metre;
        /**< Defines the literal of metres */
        constexpr Length operator"" _m(long double value) { return Length(static_cast<float>(value)); }
        /**< Defines the literal of metres */
        constexpr Length operator"" _m(unsigned long long int value) { return Length(static_cast<float>(value)); }
        /**< Defines the literal of feets */
        constexpr Length operator"" _ft(long double value) { return static_cast<float>(value) * feet; }
        /**< Defines the literal of feets */
        constexpr Length operator"" _ft(unsigned long long int value) { return static_cast<float>(value) * feet; }
        /**< Defines the literal of kilometres */
        constexpr Length operator"" _km(long double value) { return static_cast<float>(value)* kilometre; }
        /**< Defines the literal of kilometres */
        constexpr Length operator"" _km(unsigned long long int value) { return static_cast<float>(value)* kilometre; }
        /**< Defines the literal of nautical miles */
        constexpr Length operator"" _nm(long double value) { return static_cast<float>(value)* nauticmile; }
        /**< Defines the literal of nautical miles */
        constexpr Length operator"" _nm(unsigned long long int value) { return static_cast<float>(value)* nauticmile; }

        /**< Defines a second */
        constexpr Time second(1.0f);
        /**< Defines a millisecond */
        constexpr Time millisecond = second / 1000.0f;
        /**< Defines a minute */
        constexpr Time minute = 60.0f * second;
        /**< Defines a hour */
        constexpr Time hour = 60.0f * minute;
        /**< Defines the literal of milliseconds */
        constexpr Time operator"" _ms(long double value) { return static_cast<float>(value) * millisecond; }
        /**< Defines the literal of milliseconds */
        constexpr Time operator"" _ms(unsigned long long int value) { return static_cast<float>(value) * millisecond; }
        /**< Defines the literal of seconds */
        constexpr Time operator"" _s(long double value) { return static_cast<float>(value) * second; }
        /**< Defines the literal of seconds */
        constexpr Time operator"" _s(unsigned long long int value) { return static_cast<float>(value) * second; }
        /**< Defines the literal of minutes */
        constexpr Time operator"" _min(long double value) { return static_cast<float>(value) * minute; }
        /**< Defines the literal of minutes */
        constexpr Time operator"" _min(unsigned long long int value) { return static_cast<float>(value) * minute; }
        /**< Defines the literal of hours */
        constexpr Time operator"" _h(long double value) { return static_cast<float>(value) * hour; }
        /**< Defines the literal of hours */
        constexpr Time operator"" _h(unsigned long long int value) { return static_cast<float>(value) * hour; }

        /**< Defines the literal of PI */
        constexpr float operator"" _pi(long double value) { return static_cast<float>(value) * 3.1415926535897932384626433832795f; }
        /**< Defines the literal of PI */
        constexpr float operator"" _pi(unsigned long long int value) { return static_cast<float>(value) * 3.1415926535897932384626433832795f; }
        /**< Defines a degree */
        constexpr Angle degree = 1.0f;
        /**< Defines a radian */
        constexpr Angle radian = 180.0f / 1_pi * degree;
        /**< Defines the literal of radians */
        constexpr Angle operator"" _rad(long double value) { return static_cast<float>(value) * radian; }
        /**< Defines the literal of radians */
        constexpr Angle operator"" _rad(unsigned long long int value) { return static_cast<float>(value) * radian; }
        /**< Defines the literal of degrees */
        constexpr Angle operator"" _deg(long double value) { return static_cast<float>(value) * degree; }
        /**< Defines the literal of degrees */
        constexpr Angle operator"" _deg(unsigned long long int value) { return static_cast<float>(value) * degree; }

        /**< Defines a knot */
        constexpr Velocity knot = 0.51444f * metre / second;
        /**< Defines the literal of metre per second */
        constexpr Velocity operator"" _mps(long double value) { return Velocity(static_cast<float>(value)); }
        /**< Defines the literal of metre per second */
        constexpr Velocity operator"" _mps(unsigned long long int value) { return Velocity(static_cast<float>(value)); }
        /**< Defines the literal of feet per minute */
        constexpr Velocity operator"" _ftpmin(long double value) { return static_cast<float>(value) * feet / minute; }
        /**< Defines the literal of feet per minute */
        constexpr Velocity operator"" _ftpmin(unsigned long long int value) { return static_cast<float>(value) * feet / minute; }
        /**< Defines the literal of kilometre per hour */
        constexpr Velocity operator"" _kmph(long double value) { return static_cast<float>(value) * kilometre / hour; }
        /**< Defines the literal of kilometre per hour */
        constexpr Velocity operator"" _kmph(unsigned long long int value) { return static_cast<float>(value) * kilometre / hour; }
        /**< Defines the literal of knots */
        constexpr Velocity operator"" _kn(long double value) { return static_cast<float>(value) * knot; }
        /**< Defines the literal of knots */
        constexpr Velocity operator"" _kn(unsigned long long int value) { return static_cast<float>(value) * knot; }

        /**< Defines the gravity */
        constexpr Acceleration G = 9.80665f * metre / (second* second);
        /**< Defines the literal of metre per square-second */
        constexpr Acceleration operator"" _mps2(long double value) { return Acceleration(static_cast<float>(value)); }
        /**< Defines the literal of metre per square-second */
        constexpr Acceleration operator"" _mps2(unsigned long long int value) { return Acceleration(static_cast<float>(value)); }
        /**< Defines the literal of gravities */
        constexpr Acceleration operator"" _g(long double value) { return static_cast<float>(value) * G; }
        /**< Defines the literal of gravities */
        constexpr Acceleration operator"" _g(unsigned long long int value) { return static_cast<float>(value) * G; }

        /**< Defines the literal of radian per second */
        constexpr AngularVelocity operator"" _radps(long double value) { return AngularVelocity(static_cast<float>(value)); }
        /**< Defines the literal of radian per second */
        constexpr AngularVelocity operator"" _radps(unsigned long long int value) { return AngularVelocity(static_cast<float>(value)); }
        /**< Defines the literal of degree per second */
        constexpr AngularVelocity operator"" _degps(long double value) { return static_cast<float>(value) * degree / second; }
        /**< Defines the literal of degree per second */
        constexpr AngularVelocity operator"" _degps(unsigned long long int value) { return static_cast<float>(value) * degree / second; }

        /**< Defines the literal of radian per square-second */
        constexpr AngularAcceleration operator"" _radps2(long double value) { return AngularAcceleration(static_cast<float>(value)); }
        /**< Defines the literal of radian per square-second */
        constexpr AngularAcceleration operator"" _radps2(unsigned long long int value) { return AngularAcceleration(static_cast<float>(value)); }
        /**< Defines the literal of degree per square-second */
        constexpr AngularAcceleration operator"" _degps2(long double value) { return static_cast<float>(value) * degree / (second * second); }
        /**< Defines the literal of degree per square-second */
        constexpr AngularAcceleration operator"" _degps2(unsigned long long int value) { return static_cast<float>(value) * degree / (second * second); }
    }
}
