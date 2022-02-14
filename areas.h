#ifndef AREAS_H
#define AREAS_H

/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains the Areas class, which is responsible for parsing data
    from a standard input stream and converting it into a series of objects:

    Measure       — Represents a single measure for an area, e.g.
    |              population. Contains a human-readable label and a map of
    |              the measure accross a number of years.
    |
    +-> Area       Represents an area. Contains a unique local authority code
            |         used in national statistics, a map of the names of the area 
            |         (i.e. in English and Welsh), and a map of various Measure 
            |         objects.
            |
            +-> Areas A class that contains all Area objects.
*/

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_set>

#include "datasets.h"
#include "area.h"

/*
    An alias for filters based on strings such as categorisations e.g. area,
    and measures.
*/
using StringFilterSet = std::unordered_set<std::string>;

/*
    An alias for a year filter.
*/
using YearFilterTuple = std::tuple<unsigned int, unsigned int>;

/*
    An alias for the data within an Areas object stores Area objects.
*/
using AreasContainer = std::map<std::string, Area>;

/*
    Areas is a class that stores all the data categorised by area. The 
    underlying Standard Library container is customisable using the alias above.
*/
class Areas
{
private:
    AreasContainer areas;

public:
    Areas();
    const size_t size() const noexcept;
    Area& getArea(const std::string &localAuthorityCode);
    void setArea(const std::string &localAuthorityCode, const Area &area) noexcept;

    void populateFromAuthorityCodeCSV(
        std::istream &is,
        const BethYw::SourceColumnMapping &cols,
        const StringFilterSet *const areas = nullptr) noexcept(false);

    void populateFromWelshStatsJSON(
        std::istream &is, 
        const BethYw::SourceColumnMapping &cols, 
        const StringFilterSet *const areasFilter = nullptr, 
        const StringFilterSet *const measuresFilter = nullptr, 
        const YearFilterTuple *const yearsFilter = nullptr);

    void populateFromAuthorityByYearCSV(
        std::istream &is, 
        const BethYw::SourceColumnMapping &cols, 
        const StringFilterSet *const areasFilter = nullptr, 
        const StringFilterSet *const measuresFilter = nullptr, 
        const YearFilterTuple *const yearsFilter = nullptr);

    void populate(
        std::istream &is,
        const BethYw::SourceDataType &type,
        const BethYw::SourceColumnMapping &cols) noexcept(false);

    void populate(
        std::istream &is,
        const BethYw::SourceDataType &type,
        const BethYw::SourceColumnMapping &cols,
        const StringFilterSet *const areasFilter = nullptr,
        const StringFilterSet *const measuresFilter = nullptr,
        const YearFilterTuple *const yearsFilter = nullptr) noexcept(false);

    const std::string toJSON() const noexcept;
    friend std::ostream& operator<<(std::ostream &os, const Areas &areas);

    const bool checkFilter(const StringFilterSet *const filter, const std::string &x, 
                           const bool enhancedSearch = false, 
                           std::vector<std::string> extraSearch = {}) const noexcept;
    const bool checkFilter(const YearFilterTuple *const filter, int x) const noexcept;
    const std::vector<std::string> getExistingNames(const std::string &localAuthorityCode) noexcept;
};

#endif // AREAS_H