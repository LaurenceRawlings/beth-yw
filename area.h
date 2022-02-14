#ifndef AREA_H_
#define AREA_H_

/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains the Area class declaration. Area objects contain all the
    Measure objects for a given local area, along with names for that area and a
    unique authority code.
 */

#include <string>
#include <map>

#include "measure.h"

/*
    An Area object consists of a unique authority code, a container for names
    for the area in any number of different languages, and a container for the
    Measures objects.
*/
class Area
{
private:
    std::string localAuthorityCode;
    std::map<std::string, std::string> names;
    std::map<std::string, Measure> measures;

public:
    Area(const std::string &localAuthorityCode);
    const std::string getLocalAuthorityCode() const noexcept;
    const std::string getName(std::string lang) const;
    const std::map<std::string, std::string> getNames() const noexcept;
    void setName(std::string lang, const std::string &name);
    Measure& getMeasure(std::string codename);
    const std::map<std::string, Measure> getMeasures() const noexcept;
    void setMeasure(std::string codename, const Measure &measure) noexcept;
    const size_t size() const noexcept;
    friend std::ostream& operator<<(std::ostream &os, const Area &area);
    friend bool operator==(const Area& lhs, const Area& rhs);
    friend void operator+=(Area& lhs, const Area& rhs);
};

#endif // AREA_H_