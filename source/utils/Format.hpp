//!
//! \file       Format.hpp
//! \author     Andriy Parfenyuk
//! \date       14.3.2017
//!
//! \brief      Format utils
//!
//! \license    GNU
//!
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
#ifndef FORMAT_HPP
#define FORMAT_HPP
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//!
//! \namespace utils
//!
//! \brief Namespace for utility function
//!
namespace utils
{

//---------------------------------------------------------------------------
//!
//! \brief Convert any type to string
//!
//! \tparam T - type of parameter
//! \param param [in] - parameter to convert
//!
//! \return string representation of parameter
//!
template <typename T>
std::string toString(const T& param)
{
    std::stringstream sstream;
    sstream << param;
    return sstream.str();
}

//---------------------------------------------------------------------------
//!
//! \brief Split string by delimiter
//!
//! \param str [in] - string to split
//! \param delim [in] - delimiter
//!
//! \return vector of strings
//!
std::vector<std::string> split(const std::string& str, char delim);

//---------------------------------------------------------------------------
//!
//! \brief Convert string to float
//!
//! \param str [in] - string to convert 
//!
//! \return float value
//!
float toFloat(const std::string& str);

//---------------------------------------------------------------------------
//!
//! \brief Convert string to int
//!
//! \param str [in] - string to convert
//!
//! \return int value
//!
int toInt(const std::string& str);

//---------------------------------------------------------------------------
//!
//! \brief Convert string to bool
//!
//! \param str [in] - string to convert
//!
//! \return bool value
//!
bool toBool(const std::string& str);

//---------------------------------------------------------------------------
//!
//! \brief Hash function for string literals in compile time
//!
//! \param str [in] - string to hash
//! \param seed [in] - seed value
//!
//! \return hash value
//!
constexpr uint32_t strHash(const char* str, std::uint32_t seed = 13)
{
    return (*str == '\0') ? seed
                          : strHash(str + 1, (seed ^ static_cast<std::uint32_t>(*str)) * 16777619u);
}

} // namespace utils

//---------------------------------------------------------------------------
#endif // !FORMAT_HPP

//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
