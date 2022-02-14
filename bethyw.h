#ifndef BETHYW_H_
#define BETHYW_H_

/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains declarations for the helper functions for initialising and
    running Beth Yw?
 */

#include <string>
#include <unordered_set>
#include <vector>

#include "lib_cxxopts.hpp"

#include "datasets.h"
#include "areas.h"

const char DIR_SEP =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

namespace BethYw
{
    const std::string STUDENT_NUMBER = "991368";
    int run(int argc, char *argv[]);
    cxxopts::Options cxxoptsSetup();
    std::vector<BethYw::InputFileSource> parseDatasetsArg(cxxopts::ParseResult &args);
    StringFilterSet parseAreasArg(cxxopts::ParseResult &args);
    StringFilterSet parseMeasuresArg(cxxopts::ParseResult &args);
    YearFilterTuple parseYearsArg(cxxopts::ParseResult &args);
    void loadAreas(Areas& areas, const std::string& dir, const StringFilterSet areasFilter);
    void loadDatasets(Areas& areas, const std::string& dir,
                      const std::vector<BethYw::InputFileSource> datasetsToImport,
                      const StringFilterSet areasFilter,
                      const StringFilterSet measuresFilter,
                      const YearFilterTuple yearsFilter) noexcept;

    void stringToLower(std::string &string);
    void stringVectorToLower(std::vector<std::string> &vector);
    std::unordered_set<std::string> stringVectorToUnorderedSet(const std::vector<std::string> &vector);
}

#endif // BETHYW_H_