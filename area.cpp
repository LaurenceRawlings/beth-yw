/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains the implementation for the Area class. Area is a relatively
    simple class that contains a local authority code, a container of names in
    different languages and a series of Measure objects.
*/

#include <stdexcept>
#include <regex>
#include <utility>
#include <sstream>

#include "area.h"
#include "bethyw.h"

/*
    Construct an Area with a given local authority code.

    @param localAuthorityCode
        The local authority code of the Area
*/
Area::Area(const std::string &localAuthorityCode)
    : localAuthorityCode(localAuthorityCode)
{
}

/*
    Retrieve the local authority code for this Area.
    
    @return
        The Area's local authority code
*/
const std::string Area::getLocalAuthorityCode() const noexcept
{
    return localAuthorityCode;
}

/*
    Get a name for the Area in a specific language.

    @param lang
        A three-letter language code in ISO 639-3 format, e.g. cym or eng

    @return
        The name for the area in the given language

    @throws
        std::out_of_range if lang does not correspond to a language of a name stored
        inside the Area instance
*/
const std::string Area::getName(std::string lang) const
{
    BethYw::stringToLower(lang);
    auto it = names.find(lang);

    if (it == names.end())
    {
        throw std::out_of_range("Area name with the language " + lang + "could not be found!");
    }
    
    return it->second;
}

/*
    Get a map of all names for the Area.

    @return
        A map of all names
*/
const std::map<std::string, std::string> Area::getNames() const noexcept
{
    return names;
}

/*
    Set a name for the Area in a specific language.

    @param lang
        A three-letter (alphabetical) language code in ISO 639-3 format,
        e.g. cym or eng, which should be converted to lowercase

    @param name
        The name of the Area in `lang`

    @throws
        std::invalid_argument if lang is not a three letter alphabetic code
*/
void Area::setName(std::string lang, const std::string &name)
{
    // Use regex to match valid language code
    std::regex languageMatch("^[a-zA-Z]{3}$");

    if (!std::regex_match(lang.begin(), lang.end(), languageMatch))
    {
        throw std::invalid_argument("Area::setName: Language code must be three alphabetical letters only");
    }

    BethYw::stringToLower(lang);
    names[lang] = name;
}

/*
    Retrieve a Measure object, given its codename. This function is case
    insensitive when searching for a measure.

    @param key
        The codename for the measure you want to retrieve

    @return
        A Measure object

    @throws
        std::out_of_range if there is no measure with the given code.
*/
Measure& Area::getMeasure(std::string codename)
{
    BethYw::stringToLower(codename);
    auto it = measures.find(codename);

    if (it == measures.end())
    {
        throw std::out_of_range("No measure found matching " + codename);
    }

    return it->second;
}

/*
    Retrieve a map of all Measure objects.

    @return
        A map of all measures
*/
const std::map<std::string, Measure> Area::getMeasures() const noexcept
{
    return measures;
}

/*
    Add a particular Measure to this Area object. Note that the Measure's
    codename should be converted to lowercase.

    @param codename
        The codename for the Measure

    @param measure
        The Measure object

    @return
        void
*/
void Area::setMeasure(std::string codename, const Measure &measure) noexcept
{
    BethYw::stringToLower(codename);
    
    if (measures.count(codename))
    {
        getMeasure(codename) += measure;
    }
    else
    {
        measures.insert(std::make_pair(codename, measure));
    }
}

/*
    Retrieve the number of Measures we have for this Area.

    @return
        The size of the Area (i.e., the number of Measures)
*/
const size_t Area::size() const noexcept
{
    return measures.size();
}

/*
    Output the name of the Area in English and Welsh, followed by the local
    authority code. Then output all the measures for the area (see the coursework
    worksheet for specific formatting).

    If the Area only has only one name, output this. If the area has no names,
    output the name "Unnamed".

    Measures should be ordered by their Measure codename. If there are no measures
    output the line "<no measures>" after you have output the area names.

    @param os
        The output stream to write to

    @param area
        Area to write to the output stream

    @return
        Reference to the output stream
*/
std::ostream& operator<<(std::ostream &os, const Area &area)
{
    std::string details;
    std::string english;
    std::string welsh;

    // Check for present english and welsh names to format correctly
    if (area.names.count("eng"))
    {
        english = area.getName("eng");
    }

    if (area.names.count("cym"))
    {
        welsh = area.getName("cym");
    }

    if (english.length() > 0 && welsh.length() > 0)
    {
        details = english + " / " + welsh;
    }
    else if (english.length() > 0)
    {
        details = english;
    }
    else if (welsh.length() > 0)
    {
        details = welsh;
    }
    else
    {
        details = "Unnamed";
    }
    
    details += " (" + area.localAuthorityCode + ")";
    os << details << std::endl;

    if (area.size() > 0) 
    {
        for (auto it : area.measures)
        {
            os << it.second << std::endl;
        }
    }
    else {
        os << "<no measures>" << std::endl;
    }

    return os;
}

/*
    Two Area objects are only equal when their local authority code, 
    all names, and all data are equal.

    @param lhs
        An Area object

    @param rhs
        A second Area object

    @return
        true if both Area instances have the same local authority code, names
        and data; false otherwise.
*/
bool operator==(const Area& lhs, const Area& rhs) {
    return lhs.localAuthorityCode == rhs.localAuthorityCode && lhs.names == rhs.names && lhs.measures == rhs.measures;
}

/*
    Merge the rhs area into the lhs area with the rhs
    taking precedence.

    @param lhs
        An Area object

    @param rhs
        A second Area object
*/
void operator+=(Area& lhs, const Area& rhs)
{
    for (auto it : rhs.names)
    {
        lhs.setName(it.first, it.second);
    }

    for (auto it : rhs.measures)
    {
        lhs.setMeasure(it.first, it.second);
    }
}
