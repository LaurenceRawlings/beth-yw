/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains all the helper functions for initialising and running Beth Yw?
*/

#include <iostream>
#include <algorithm>
#include <regex>

#include "lib_cxxopts.hpp"

#include "bethyw.h"
#include "input.h"

/*
    Run Beth Yw?, parsing the command line arguments, importing the data,
    and outputting the requested data to the standard output/error.

    @param argc
        Number of program arguments

    @param argv
        Program arguments

    @return
        Exit code
*/
int BethYw::run(int argc, char *argv[])
{
    auto cxxopts = cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);

    // Print the help usage if requested
    if (args.count("help"))
    {
        std::cerr << cxxopts.help() << std::endl;
        return 0;
    }

    // Parse data directory argument
    std::string dir = args["dir"].as<std::string>() + DIR_SEP;

    // Parse other arguments and import data
    std::vector<InputFileSource> datasetsToImport;
    StringFilterSet areasFilter;
    StringFilterSet measuresFilter;
    YearFilterTuple yearsFilter;

    try
    {
        datasetsToImport = parseDatasetsArg(args);
        areasFilter = parseAreasArg(args);
        measuresFilter = parseMeasuresArg(args);
        yearsFilter = parseYearsArg(args);
    }
    catch(const std::invalid_argument& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Create the areas object and load the datasets
    Areas data = Areas();
    BethYw::loadDatasets(data, dir, datasetsToImport, areasFilter, measuresFilter, yearsFilter);

    if (args.count("json"))
    {
        // The output as JSON is the json flag is present
        std::cout << data.toJSON() << std::endl;
    }
    else
    {
        // The output as tables by default
        std::cout << data << std::endl;
    }

    return 0;
}

/*
    This function sets up and returns a valid cxxopts object.

    @return
        A constructed cxxopts object
*/
cxxopts::Options BethYw::cxxoptsSetup()
{
    cxxopts::Options cxxopts(
        "bethyw",
        "Student ID: " + STUDENT_NUMBER + "\n\n"
                                          "This program is designed to parse official Welsh Government"
                                          " statistics data files.\n");

    cxxopts.add_options()(
        "dir",
        "Directory for input data passed in as files",
        cxxopts::value<std::string>()->default_value("datasets"))(

        "d,datasets",
        "The dataset(s) to import and analyse as a comma-separated list of codes "
        "(omit or set to 'all' to import and analyse all datasets)",
        cxxopts::value<std::vector<std::string>>())(

        "a,areas",
        "The areas(s) to import and analyse as a comma-separated list of "
        "authority codes (omit or set to 'all' to import and analyse all areas)",
        cxxopts::value<std::vector<std::string>>())(

        "m,measures",
        "Select a subset of measures from the dataset(s) "
        "(omit or set to 'all' to import and analyse all measures)",
        cxxopts::value<std::vector<std::string>>())(

        "y,years",
        "Focus on a particular year (YYYY) or "
        "inclusive range of years (YYYY-ZZZZ)",
        cxxopts::value<std::string>()->default_value("0"))(

        "j,json",
        "Print the output as JSON instead of tables.")(

        "h,help",
        "Print usage.");

    return cxxopts;
}

/*
    Parse the datasets argument passed into the command line.

    The datasets argument is optional, and if it is not included, all datasets
    are imported. If it is included, it should be a comma-separated list of
    datasets to import. If the argument contains the value "all"
    (case-insensitive), all datasets are imported.

    @param args
        Parsed program arguments

    @return
        A std::vector of BethYw::InputFileSource instances to import

    @throws
        std::invalid_argument if the argument contains an invalid dataset with
        message: No dataset matches key <input code>
 */
std::vector<BethYw::InputFileSource> BethYw::parseDatasetsArg(cxxopts::ParseResult &args)
{
    auto &allDatasets = InputFiles::DATASETS;
    std::vector<InputFileSource> datasetsToImport;
    std::vector<std::string> inputDatasets;
    
    if (args.count("datasets"))
    {
        inputDatasets = args["datasets"].as<std::vector<std::string>>();
        stringVectorToLower(inputDatasets);
    }

    if (!args.count("datasets") || std::find(inputDatasets.begin(), inputDatasets.end(), "all") != inputDatasets.end())
    {
        for (auto dataset : allDatasets)
        {
            datasetsToImport.push_back(dataset);
        }

        return datasetsToImport;
    }

    // Check is passed datasets are valid
    for (auto inputDataset : inputDatasets)
    {
        bool found = false;

        for (auto dataset : allDatasets)
        {
            if (inputDataset == dataset.CODE)
            {
                datasetsToImport.push_back(dataset);
                found = true;
                break;
            }
        }

        if (!found)
        {
            throw std::invalid_argument("No dataset matches key: " + inputDataset);
        }
    }

    return datasetsToImport;
}

/*
    Parses the areas command line argument, which is optional. If it doesn't
    exist or exists and contains "all" as value (any case), all areas are
    imported.

    @param args
        Parsed program arguments

    @return
        An std::unordered_set of std::strings corresponding to specific areas
        to import, or an empty set if all areas should be imported.
*/
StringFilterSet BethYw::parseAreasArg(cxxopts::ParseResult &args)
{
    std::vector<std::string> inputAreas;

    if (args.count("areas"))
    {
        inputAreas = args["areas"].as<std::vector<std::string>>();
    }

    std::vector<std::string> inputAreasLower = inputAreas;
    stringVectorToLower(inputAreasLower);

    if (!args.count("areas") || std::find(inputAreasLower.begin(), inputAreasLower.end(), "all") != inputAreasLower.end())
    {
        return std::unordered_set<std::string>();
    }

    return stringVectorToUnorderedSet(inputAreas);
}

/*
    Parse the measures command line argument, which is optional. If it doesn't
    exist or exists and contains "all" as value (any case), all measures should
    be imported.

    @param args
        Parsed program arguments

    @return
        An std::unordered_set of std::strings corresponding to specific measures
        to import, or an empty set if all measures should be imported.

    @throws
        std::invalid_argument if the argument contains an invalid measures value
        with the message: Invalid input for measures argument
*/
StringFilterSet BethYw::parseMeasuresArg(cxxopts::ParseResult &args)
{
    std::vector<std::string> inputMeasures;

    if (args.count("measures"))
    {
        inputMeasures = args["measures"].as<std::vector<std::string>>();
    }

    std::vector<std::string> inputMeasuresLower = inputMeasures;
    stringVectorToLower(inputMeasuresLower);

    if (!args.count("measures") || std::find(inputMeasuresLower.begin(), inputMeasuresLower.end(), "all") != inputMeasuresLower.end())
    {
        return std::unordered_set<std::string>();
    }

    return stringVectorToUnorderedSet(inputMeasures);
}

/*
    Parse the years command line argument. Years is either a four digit year
    value, or two four digit year values separated by a hyphen (i.e. either
    YYYY or YYYY-ZZZZ). If one or both values are 0, then there is no filter 
    to be applied. If no year argument is given return <0,0> to import all years.

    @param args
        Parsed program arguments

    @return
        A std::tuple containing two unsigned ints

    @throws
        std::invalid_argument if the argument contains an invalid years value with
        the message: Invalid input for years argument
*/
YearFilterTuple BethYw::parseYearsArg(cxxopts::ParseResult &args)
{
    if (!args.count("years"))
    {
        return std::tuple<unsigned int, unsigned int>{0, 0};
    }

    // Use regex to check the year filter is valid as a whole
    std::string inputYears = args["years"].as<std::string>();
    std::regex yearsMatch("^([0-9]{4}|0)(-[0-9]{4}|-0)?$");

    if (!std::regex_match(inputYears.begin(), inputYears.end(), yearsMatch))
    {
        throw std::invalid_argument("Invalid input for years argument");
    }

    unsigned int startYear = 1;
    unsigned int endYear = 1;

    std::regex yearMatch("([0-9]+)");
    std::smatch year;

    // Use regex to extract the first year
    if (std::regex_search(inputYears, year, yearMatch))
    {
        startYear = (unsigned int)std::stoi(year[1]);
        inputYears = year.suffix().str();
    }

    // Use regex to extract the second year
    if (std::regex_search(inputYears, year, yearMatch))
    {
        endYear = (unsigned int)std::stoi(year[1]);
    }

    // Detect the year values and return the correct filter
    if (startYear == 0 || endYear == 0)
    {
        return std::tuple<unsigned int, unsigned int>{0, 0};
    }
    else if (endYear == 1)
    {
        return std::tuple<unsigned int, unsigned int>{startYear, startYear};
    }
    else
    {
        return std::tuple<unsigned int, unsigned int>{startYear, endYear};
    }
}

/*
    Load the areas.csv file from the directory `dir`. Parse the file and
    create the appropriate Area objects inside the Areas object passed to
    the function in the `areas` argument.

    @param areas
        An Areas instance that should be modified (i.e. the populate() function
        in the instance should be called)

    @param dir
        Directory where the areas.csv file is

    @param areasFilter
        An unordered set of areas to filter, or empty to import all areas

    @return
        void
*/
void BethYw::loadAreas(Areas& areas, const std::string& dir, const StringFilterSet areasFilter)
{
    InputFile file(dir + InputFiles::AREAS.FILE);
    areas.populate(file.open(), AuthorityCodeCSV, InputFiles::AREAS.COLS, &areasFilter);
}


/*
    Import datasets from `datasetsToImport` as files in `dir` into areas, and
    filtering them with the `areasFilter`, `measuresFilter`, and `yearsFilter`.

    This function should promise not to throw an exception. If there is an
    error/exception thrown in any function called by thus function, catch it and
    output 'Error importing dataset:', followed by a new line and then the output
    of the what() function on the exception.

    @param areas
        An Areas instance that should be modified (i.e. datasets loaded into it)

    @param dir
        The directory where the datasets are

    @param datasetsToImport
        A vector of InputFileSource objects

    @param areasFilter
        An unordered set of areas (as authority codes encoded in std::strings)
        to filter, or empty to import all areas

    @param measuresFilter
        An unordered set of measures (as measure codes encoded in std::strings)
        to filter, or empty to import all measures

    @param yearsFilter
        An two-pair tuple of unsigned ints corresponding to the range of years
        to import, which should both be 0 to import all years.

    @return
        void
*/
void BethYw::loadDatasets(Areas& areas, const std::string& dir,
                          const std::vector<BethYw::InputFileSource> datasetsToImport,
                          const StringFilterSet areasFilter,
                          const StringFilterSet measuresFilter,
                          const YearFilterTuple yearsFilter) noexcept
{
    try
    {   
        BethYw::loadAreas(areas, dir, areasFilter);

        for (auto dataset : datasetsToImport)
        {
            InputFile file(dir + dataset.FILE);
            areas.populate(file.open(), dataset.PARSER, dataset.COLS, &areasFilter, &measuresFilter, &yearsFilter);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error importing dataset:" << std::endl << e.what() << std::endl;
    }
}

/*
    Takes a string and converts it to lowercase.

    @param string
        A reference to the string to convert
        
    @return
        void
 */
void BethYw::stringToLower(std::string &string)
{
    transform(string.begin(), string.end(), string.begin(), ::tolower);
}

/*
    Takes a string vector and converts each string to lowercase.

    @param vector
        A reference to the string vector to process
        
    @return
        void
 */
void BethYw::stringVectorToLower(std::vector<std::string> &vector)
{
    for (auto &it : vector)
    {
        stringToLower(it);
    }
}

/*
    Takes a string vector and adds each value to an unordered string 
    set which is returned.

    @param vector
        A refernce to the string vector to be converted

    @return
        The unordered string set
 */
std::unordered_set<std::string> BethYw::stringVectorToUnorderedSet(const std::vector<std::string> &vector)
{
    std::unordered_set<std::string> set;

    for (auto it : vector)
    {
        set.insert(it);
    }

    return set;
}
