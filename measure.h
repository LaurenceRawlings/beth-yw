#ifndef MEASURE_H_
#define MEASURE_H_

/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains the decalaration of the Measure class.
 */

#include <string>
#include <map>

/*
    The Measure class contains a measure code, label, and a container for readings
    from across a number of years.
*/
class Measure
{
private:
    std::string codename;
    std::string label;
    std::map<unsigned int, double> values;
    void rightAlign(std::string &string1, std::string &string2) const noexcept;

public:
    Measure(const std::string &codename, const std::string &label);
    const std::string getCodename() const noexcept;
    const std::string getLabel() const noexcept;
    void setLabel(const std::string &label) noexcept;
    const double getValue(unsigned int year) const;
    const std::map<unsigned int, double> getValues() const noexcept;
    void setValue(unsigned int year, double value);
    const size_t size() const noexcept;
    const double getDifference() const noexcept;
    const double getDifferenceAsPercentage() const noexcept;
    const double getAverage() const noexcept;
    friend std::ostream& operator<<(std::ostream &os, const Measure &measure);
    friend bool operator==(const Measure& lhs, const Measure& rhs);
    friend void operator+=(Measure& lhs, const Measure& rhs);
};

#endif // MEASURE_H_
