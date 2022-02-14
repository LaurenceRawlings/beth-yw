/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains the implementation of the Measure class. Measure is a
    very simple class that needs to contain a few member variables for its name,
    codename, and a Standard Library container for data.
*/

#include <stdexcept>

#include "measure.h"
#include "bethyw.h"

/*
    Construct a single Measure, that has values across many years.

    All StatsWales JSON files have a codename for measures.

    @param codename
        The codename for the measure

    @param label
        Human-readable (i.e. nice/explanatory) label for the measure
*/
Measure::Measure(const std::string &codename, const std::string &label)
    : codename(codename), label(label)
{
    BethYw::stringToLower(this->codename);
}

/*
    Retrieve the code for the Measure.

    @return
        The codename for the Measure
*/
const std::string Measure::getCodename() const noexcept
{
    return codename;
}

/*
    Retrieve the human-friendly label for the Measure.

    @return
        The human-friendly label for the Measure
*/
const std::string Measure::getLabel() const noexcept
{
    return label;
}

/*
    Change the label for the Measure.

    @param label
        The new label for the Measure
*/
void Measure::setLabel(const std::string &label) noexcept
{
    this->label = label;
}

/*
    Retrieve a Measure's value for a given year.

    @param key
        The year to find the value for

    @return
        The value stored for the given year

    @throws
        std::out_of_range if year does not exist in Measure
*/
const double Measure::getValue(unsigned int year) const
{
    auto it = values.find(year);

    if (it == values.end())
    {
        throw std::out_of_range("No value found for year " + std::to_string(year));
    }
    
    return it->second;
}

/*
    Retrieve a map of all a Measure's values.

    @return
        A map of all values
*/
const std::map<unsigned int, double> Measure::getValues() const noexcept
{
    return values;
}

/*
    Add a particular year's value to the Measure object. If a value already
    exists for the year, replace it.

    @param key
        The year to insert a value at

    @param value
        The value for the given year

    @return
        void
*/
void Measure::setValue(unsigned int year, double value)
{
    values[year] = value;
}

/*
    Retrieve the number of years data we have for this measure.

    @return
        The size of the measure
*/
const size_t Measure::size() const noexcept
{
    return values.size();
}

/*
    Calculate the difference between the first and last year imported. This
    function should be callable from a constant context and must promise to not
    change the state of the instance or throw an exception.

    @return
        The difference/change in value from the first to the last year, or 0 if it
        cannot be calculated
*/
const double Measure::getDifference() const noexcept
{
    if (size() < 2)
    {
        return 0;
    }

    return values.rbegin()->second - values.begin()->second;
}

/*
    Calculate the difference between the first and last year imported as a 
    percentage.

    @return
        The difference/change in value from the first to the last year as a decimal
        value, or 0 if it cannot be calculated
*/
const double Measure::getDifferenceAsPercentage() const noexcept
{
    if (size() < 2) 
    {
        return 0;
    }

    return (getDifference() / values.begin()->second) * 100;
}

/*
    Calculate the average/mean value for all the values.

    @return
        The average value for all the years, or 0 if it cannot be calculated
*/
const double Measure::getAverage() const noexcept
{
    if (size() < 1) 
    {
        return 0;
    }

    double total = 0;
    for (auto it : values)
    {
        total += it.second;
    }

    return total / size();
}

/*
    Years will be printed in chronological order. Three additional columns
    are be included at the end of the output, correspoding to the average
    value across the years, the difference between the first and last year,
    and the percentage difference between the first and last year.

    If there is no data in this measure, print the name and code, and 
    on the next line print: <no data>

    @param os
        The output stream to write to

    @param measure
        The Measure to write to the output stream

    @return
        Reference to the output stream
*/
std::ostream& operator<<(std::ostream &os, const Measure &measure)
{
    std::string details = measure.label + " (" + measure.codename + ")";
    std::string table;
    std::string headings;
    std::string values;

    if (measure.size() > 0)
    {
        std::string header;
        std::string value;

        for (auto it : measure.values)
        {
            header = std::to_string(it.first);
            value = std::to_string(it.second);

            measure.rightAlign(header, value);
            headings += header + " ";
            values += value + " ";
        }

        header = "Average";
        value = std::to_string(measure.getAverage());
        measure.rightAlign(header, value);
        headings += header + " ";
        values += value + " ";

        header = "Diff.";
        value = std::to_string(measure.getDifference());
        measure.rightAlign(header, value);
        headings += header + " ";
        values += value + " ";

        header = "% Diff.";
        value = std::to_string(measure.getDifferenceAsPercentage());
        measure.rightAlign(header, value);
        headings += header;
        values += value;

        table = headings + "\n" + values;
    }
    else
    {
        table = "<no data>";
    }

    return os << details << std::endl << table << std::endl;
}

/*
    Two Measure objects are only equal when their codename, 
    label and data are all equal.

    @param lhs
        A Measure object

    @param rhs
        A second Measure object

    @return
        true if both Measure objects have the same codename, label and data; false
        otherwise
*/
bool operator==(const Measure& lhs, const Measure& rhs)
{
    return lhs.codename == rhs.codename && lhs.label == rhs.label && lhs.values == rhs.values;
}

/*
    Merge the rhs measure into the lhs measure with the rhs
    taking precedence.

    @param lhs
        A Measure object

    @param rhs
        A second Measure object
*/
void operator+=(Measure& lhs, const Measure& rhs)
{
    lhs.label = rhs.label;

    for (auto it : rhs.values)
    {
        lhs.setValue(it.first, it.second);
    }
}

/*
    Takes two strings and aligns them both to the right by padding the longer
    of the two with spaces at the front.

    @param string1
        A refernce to the first string

    @param string2
        A refernce to the second string
        
    @return
        void
 */
void Measure::rightAlign(std::string &string1, std::string &string2) const noexcept
{
    int difference = string2.length() - string1.length();

    // Find the shorter string and pad the front with spaces to match length of longer string
    if (difference > 0)
    {
        string1 = std::string(difference, ' ') + string1;
    }
    else
    {
        string2 = std::string(std::abs(difference), ' ') + string2;
    }
}
