/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    The file contains the Areas class implementation. Areas are the top
    level of the data structure in Beth Yw? for now.

    Areas is also responsible for importing data from a stream (using the
    various populate() functions) and creating the Area and Measure objects.
*/

#include <stdexcept>
#include <iostream>
#include <string>
#include <stdexcept>
#include <tuple>
#include <unordered_set>

#include "lib_json.hpp"

#include "datasets.h"
#include "areas.h"
#include "measure.h"
#include "bethyw.h"

using json = nlohmann::json;

/*
    Constructor for an Areas object.
*/
Areas::Areas()
{
}

/*
    Add a particular Area to the Areas object.

    If an Area already exists with the same local authority code, overwrite all
    data contained within the existing Area with those in the new
    Area (i.e. they should be combined, but the new Area's data should take
    precedence, e.g. replace a name with the same language identifier).

    @param localAuthorityCode
        The local authority code of the Area

    @param area
        The Area object that will contain the Measure objects

    @return
        void
*/
void Areas::setArea(const std::string &localAuthorityCode, const Area &area) noexcept
{
    if (areas.count(localAuthorityCode))
    {
        getArea(localAuthorityCode) += area;
    }
    else
    {
        areas.insert(std::make_pair(localAuthorityCode, area));
    }
}

/*
    Retrieve an Area instance with a given local authority code.

    @param localAuthorityCode
        The local authority code to find the Area instance of

    @return
        An Area object

    @throws
        std::out_of_range if an Area with the set local authority code does not
        exist in this Areas instance
*/
Area& Areas::getArea(const std::string &localAuthorityCode)
{
    auto it = areas.find(localAuthorityCode);

    if (it == areas.end())
    {
        throw std::out_of_range("No area found matching " + localAuthorityCode);
    }

    return it->second;
}

/*
    Retrieve the number of Areas within the container.

    @return
        The number of Area instances
*/
const size_t Areas::size() const noexcept
{
    return areas.size();
}

/*
    This function specifically parses the compiled areas.csv file of local 
    authority codes, and their names in English and Welsh.

    This is a simple dataset that is a comma-separated values file (CSV), where
    the first row gives the name of the columns, and then each row is a set of
    data.

    @param is
        The input stream from InputSource

    @param cols
        A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
        that give the column header in the CSV file

    @param areasFilter
        An umodifiable pointer to set of umodifiable strings for areas to import,
        or an empty set if all areas should be imported

    @return
        void

    @throws 
        std::runtime_error if a parsing error occurs (e.g. due to a malformed file)
        std::out_of_range if there are not enough columns in cols
*/
void Areas::populateFromAuthorityCodeCSV(std::istream &is, const BethYw::SourceColumnMapping &cols, const StringFilterSet *const areasFilter)
{
    std::string cell;
    std::string line;
    std::vector<std::string> fileCols;

    if (!getline(is, line))
    {
        throw std::runtime_error("Malformed file!");
    }

    // Parese the first line to get the column headers
    std::stringstream sstr(line);
    while (getline(sstr, cell, ','))
    {
        fileCols.push_back(cell);
    }

    // If the cols passed in dont match the parsed cols invalid file
    if (fileCols.size() != cols.size()) 
    {
        throw std::out_of_range("Cols length mismatch!");
    }

    while (getline(is, line))
    {
        std::vector<std::string> areaData;
        std::stringstream row(line);

        while (getline(row, cell, ','))
        {
            areaData.push_back(cell);
        }

        if (areaData.size() < 3)
        {
            throw std::runtime_error("Malformed file!");
        }

        if (checkFilter(areasFilter, areaData[0], true, {areaData[1], areaData[2]}))
        {
            Area area = Area(areaData[0]);
            area.setName("eng", areaData[1]);
            area.setName("cym", areaData[2]);

            setArea(areaData[0], area);
        }
    }
}

/*
    Data from StatsWales is in the JSON format, and contains three
    top-level keys: odata.metadata, value, odata.nextLink. value contains the
    data we need.

    If areasFilter is a non-empty set only include areas matching the filter. If
    measuresFilter is a non-empty set only include measures matching the filter.
    If yearsFilter is not equal to <0,0>, only import years within the range
    specified by the tuple (inclusive).

    @param is
        The input stream from InputSource

    @param cols
        A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
        that give the column header in the CSV file

    @param areasFilter
        An umodifiable pointer to set of umodifiable strings of areas to import,
        or an empty set if all areas should be imported

    @param measuresFilter
        An umodifiable pointer to set of umodifiable strings of measures to import,
        or an empty set if all measures should be imported

    @param yearsFilter
        An umodifiable pointer to an umodifiable tuple of two unsigned integers,
        where if both values are 0, then all years should be imported, otherwise
        they should be treated as the range of years to be imported (inclusively)

    @return
        void

    @throws 
        std::runtime_error if a parsing error occurs (e.g. due to a malformed file)
        std::out_of_range if there are not enough columns in cols
*/
void Areas::populateFromWelshStatsJSON(std::istream &is, 
                                       const BethYw::SourceColumnMapping &cols, 
                                       const StringFilterSet *const areasFilter, 
                                       const StringFilterSet *const measuresFilter,
                                       const YearFilterTuple *const yearsFilter)
{
    json j;

    try
    {
        is >> j;
    }
    catch(const std::exception& e)
    {
        throw std::runtime_error("Malformed file!");
    }

    std::string localAuthorityCode;
    std::string areaName;
    std::string measureCode;
    std::string measureName;
    unsigned int year;
    double value;

    for (auto& el : j["value"].items()) {
        auto &data = el.value();

        try
        {
            localAuthorityCode = data[cols.at(BethYw::AUTH_CODE)];
            areaName = data[cols.at(BethYw::AUTH_NAME_ENG)];

            // Check measure type and parse
            if (cols.find(BethYw::MEASURE_CODE) != cols.end())
            {
                measureCode = data[cols.at(BethYw::MEASURE_CODE)];
                measureName = data[cols.at(BethYw::MEASURE_NAME)];
            }
            else 
            {
                measureCode = cols.at(BethYw::SINGLE_MEASURE_CODE);
                measureName = cols.at(BethYw::SINGLE_MEASURE_NAME);
            }

            // Parse and convert the year
            std::string yearString = data[cols.at(BethYw::YEAR)];
            try 
            {
                year = std::stoul(yearString);
            }
            catch(const std::invalid_argument& e)
            {
                throw std::runtime_error("Malformed file!");
            }

            // Parse and convert the value
            try
            {
                value = data[cols.at(BethYw::VALUE)];
            }
            catch(const nlohmann::detail::type_error& e)
            {
                std::string valueString = data[cols.at(BethYw::VALUE)];
                try 
                {
                    value = std::stod(valueString);
                }
                catch(const std::invalid_argument& e)
                {
                    throw std::runtime_error("Malformed file!");
                }
            }
        }
        catch(const std::out_of_range& e)
        {
            throw std::out_of_range("Not enough cols!");
        }

        // Find any existing names for the currect area
        // The names are passed into the filter check to be used in the extended argument filtering
        std::vector<std::string> existingNames = getExistingNames(localAuthorityCode);
        existingNames.push_back(areaName);
    
        if (checkFilter(areasFilter, localAuthorityCode, true, existingNames))
        {
            Area area = Area(localAuthorityCode);
            area.setName("eng", areaName);
            
            if (checkFilter(measuresFilter, measureCode))
            {
                Measure measure = Measure(measureCode, measureName);

                if (checkFilter(yearsFilter, year))
                {
                    measure.setValue(year, value);
                }
                
                area.setMeasure(measureCode, measure);
            }

            setArea(localAuthorityCode, area);
        }
    }
}

/*
    This function imports CSV files that contain a single measure. The 
    CSV file consists of columns containing the authority code and years.
    Each row contains an authority code and values for each year (or no value
    if the data doesn't exist).

    @param is
        The input stream from InputSource

    @param cols
        A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
        that give the column header in the CSV file

    @param areasFilter
        An umodifiable pointer to set of umodifiable strings for areas to import,
        or an empty set if all areas should be imported

    @param measuresFilter
        An umodifiable pointer to set of strings for measures to import, or an empty 
        set if all measures should be imported

    @param yearsFilter
        An umodifiable pointer to an umodifiable tuple of two unsigned integers,
        where if both values are 0, then all years should be imported, otherwise
        they should be treated as a the range of years to be imported

    @return
        void

    @throws 
        std::runtime_error if a parsing error occurs (e.g. due to a malformed file)
        std::out_of_range if there are not enough columns in cols
*/
void Areas::populateFromAuthorityByYearCSV(std::istream &is, const BethYw::SourceColumnMapping &cols, 
                                           const StringFilterSet *const areasFilter, 
                                           const StringFilterSet *const measuresFilter, 
                                           const YearFilterTuple *const yearsFilter)
{
    std::string cell;
    std::string line;
    std::vector<std::string> fileCols;

    if (!getline(is, line))
    {
        throw std::runtime_error("Malformed file!");
    }

    std::stringstream sstr(line);
    while (getline(sstr, cell, ','))
    {
        fileCols.push_back(cell);
    }

    if (cols.size() != 3)
    {
        throw std::out_of_range("Cols length mismatch!");
    }

    if (fileCols[0] != cols.at(BethYw::AUTH_CODE))
    {
        throw std::runtime_error("Malformed file!");
    }

    while (getline(is, line))
    {
        std::vector<std::string> data;
        std::stringstream row(line);

        while (getline(row, cell, ','))
        {
            data.push_back(cell);
        }        

        if (checkFilter(areasFilter, data[0], true, getExistingNames(data[0])))
        {
            Area area = Area(data[0]);

            if (checkFilter(measuresFilter, cols.at(BethYw::SINGLE_MEASURE_CODE)))
            {
                Measure measure = Measure(cols.at(BethYw::SINGLE_MEASURE_CODE), cols.at(BethYw::SINGLE_MEASURE_NAME));

                for (unsigned int i = 1; i < data.size(); i++)
                {
                    unsigned int year;

                    try 
                    {
                        year = std::stoul(fileCols[i]);
                    }
                    catch(const std::invalid_argument& e)
                    {
                        throw std::runtime_error("Malformed file!");
                    }

                    if (checkFilter(yearsFilter, year))
                    {
                        try
                        {
                            measure.setValue(year, std::stod(data[i]));
                        }
                        catch(const std::invalid_argument& e)
                        {
                            throw std::runtime_error("Malformed file!");
                        }
                    }
                }

                area.setMeasure(cols.at(BethYw::SINGLE_MEASURE_CODE), measure);
            }
            
            setArea(data[0], area);
        }
    }
}

/*
    Parse data from an standard input stream `is`, that has data of a particular
    `type`, and with a given column mapping in `cols`.

    @param is
        The input stream from InputSource

    @param type
        A value from the BethYw::SourceDataType enum which states the underlying
        data file structure

    @param cols
        A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
        that give the column header in the CSV file

    @return
        void

    @throws 
        std::runtime_error if a parsing error occurs (e.g. due to a malformed file),
        the stream is not open/valid/has any contents, or an unexpected type
        is passed in.
        std::out_of_range if there are not enough columns in cols
*/
void Areas::populate(std::istream &is, const BethYw::SourceDataType &type, const BethYw::SourceColumnMapping &cols)
{
    populate(is, type, cols, nullptr, nullptr, nullptr);
}

/*
    Parse data from an standard input stream, that is of a particular type,
    and with a given column mapping, filtering for specific areas, measures,
    and years, and fill the container.

    This overloaded function includes pointers to the three filters for areas,
    measures, and years.

    @param is
        The input stream from InputSource

    @param type
        A value from the BethYw::SourceDataType enum which states the underlying
        data file structure

    @param cols
        A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
        that give the column header in the CSV file

    @param areasFilter
        An umodifiable pointer to set of umodifiable strings for areas to import,
        or an empty set if all areas should be imported

    @param measuresFilter
        An umodifiable pointer to set of umodifiable strings for measures to import,
        or an empty set if all measures should be imported

    @param yearsFilter
        An umodifiable pointer to an umodifiable tuple of two unsigned integers,
        where if both values are 0, then all years should be imported, otherwise
        they should be treated as a the range of years to be imported

    @return
        void

    @throws 
        std::runtime_error if a parsing error occurs (e.g. due to a malformed file),
        the stream is not open/valid/has any contents, or an unexpected type
        is passed in.
        std::out_of_range if there are not enough columns in cols
*/
void Areas::populate(
    std::istream &is,
    const BethYw::SourceDataType &type,
    const BethYw::SourceColumnMapping &cols,
    const StringFilterSet *const areasFilter,
    const StringFilterSet *const measuresFilter,
    const YearFilterTuple *const yearsFilter)
{
    if (!is.good())
    {
        throw std::runtime_error("Input stream is not good!");
    }

    switch (type)
    {
        case BethYw::AuthorityCodeCSV:
            populateFromAuthorityCodeCSV(is, cols, areasFilter);
            break;
        
        case BethYw::WelshStatsJSON:
            populateFromWelshStatsJSON(is, cols, areasFilter, measuresFilter, yearsFilter);
            break;

        case BethYw::AuthorityByYearCSV:
            populateFromAuthorityByYearCSV(is, cols, areasFilter, measuresFilter, yearsFilter);
            break;

        default:
            throw std::runtime_error("Areas::populate: Unexpected data type");
            break;
    }
}

/*
    Convert this Areas object, and all its containing Area instances, and
    the Measure instances within those, to values.
    
    @return
        std::string of JSON
*/
const std::string Areas::toJSON() const noexcept
{
    json j;

    for (auto area : areas)
    {
        for (auto measure : area.second.getMeasures())
        {
            for (auto value : measure.second.getValues())
            {
                j[area.first]["measures"][measure.first][std::to_string(value.first)] = value.second;
            }
        }

        j[area.first]["names"] = area.second.getNames();
    }

    // If the json is empty return empty {}
    return j.empty() ? "{}" : j.dump();
}

/*
    Areas are printed, ordered alphabetically by their local authority code. 
    Measures within each Area are ordered alphabetically by their codename.

    @param os
        The output stream to write to

    @param areas
        The Areas object to write to the output stream

    @return
        Reference to the output stream
*/
std::ostream& operator<<(std::ostream &os, const Areas &areas)
{
    if (areas.size() > 0) 
    {
        for (auto it : areas.areas)
        {
            os << it.second << std::endl;
        }
    }
    else 
    {
        os << "<no areas>" << std::endl;
    }

    return os;
}

/*
    Searches a string filter set for a specific string, regardless of case. Has the
    option to enable partial match (disabled by default), which will also return
    true is the set contains a substring of the string passed in.

    @param filter
        A refernce to the set

    @param x
        The string to search for

    @param enhancedSearch
        Wheather to search for substrings

    @param extraSearch
        Also search for substrings within this vector

    @return bool
        True if the set contains the passed in string, false if not
 */
const bool Areas::checkFilter(const StringFilterSet *const filter, 
                              const std::string &x, const bool enhancedSearch, 
                              std::vector<std::string> extraSearch) const noexcept
{
    if (filter == nullptr || filter->empty())
    {
        return true;
    }

    std::string xLowerCase = x;
    BethYw::stringToLower(xLowerCase);

    for (auto it : *filter)
    {
        BethYw::stringToLower(it);

        if (it == xLowerCase)
        {
            return true;
        }

        // For the extended argument filtering also search substrings
        if (enhancedSearch)
        {
            if (xLowerCase.find(it) != std::string::npos) 
            {
                return true;
            }

            BethYw::stringVectorToLower(extraSearch);
            for (auto name : extraSearch)
            {
                if (name.find(it) != std::string::npos) 
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

/*
    Checks if an integer is within the given range (inclusive).

    @param range
        A tuple containing the high and low value of the range

    @param x
        The int to check

    @return bool
        True if the int is within the given range, false if not
 */
const bool Areas::checkFilter(const YearFilterTuple *const filter, int x) const noexcept
{
    if (filter == nullptr || *filter == std::tuple<unsigned int, unsigned int>{0, 0})
    {
        return true;
    }
    
    int low = std::get<0>(*filter);
    int high = std::get<1>(*filter);

    return ((x-low) <= (high-low));
}

/*
    If an area exists with the given local authority code its names will
    be returned in a vector, if the area doesn't exist an empty vector
    will be returned.

    @param localAuthorityCode
        The area to get the names of

    @return std::vector<std::string>
        A vector containing all the areas stored names
 */
const std::vector<std::string> Areas::getExistingNames(const std::string &localAuthorityCode) noexcept
{
    std::vector<std::string> existingNames;

    if (areas.count(localAuthorityCode))
    {
        Area existingArea = getArea(localAuthorityCode);
        for (auto existingName : existingArea.getNames())
        {
            existingNames.push_back(existingName.second);
        }
    }

    return existingNames;
}
