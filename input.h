#ifndef INPUT_H_
#define INPUT_H_

/*
    +---------------------------------------+
    | BETH YW? WELSH GOVERNMENT DATA PARSER |
    +---------------------------------------+

    AUTHOR: 991368

    This file contains declarations for the input source handlers. There are
    two classes: InputSource and InputFile. InputSource is abstract 
    InputFile is a concrete derivation of InputSource, for input from files.
 */

#include <string>
#include <fstream>

/*
    InputSource is an abstract/purely virtual base class for 
    all input source types.
*/
class InputSource
{
protected:
    InputSource(const std::string &source);
    virtual ~InputSource() = default;
    std::string source;

public:
    virtual const std::string getSource() const;
};

/*
    Source data that is contained within a file.
*/
class InputFile : public InputSource
{
private:
    std::ifstream file;

public:
    InputFile(const std::string &filePath);
    ~InputFile();
    std::ifstream& open();
};

#endif // INPUT_H_