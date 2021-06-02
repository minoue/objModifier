#include "util.hpp"
#include <sstream>

Utils::Utils() {};

Utils::~Utils() {};

std::string Utils::getExt(const std::string path)
{

    std::vector<std::string> pathSplit;
    split(path, pathSplit, '.');
    std::string& last = pathSplit.back();
    return last;
}

size_t Utils::split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

// https://www.oreilly.com/library/view/c-cookbook/0596007612/ch03s06.html
float Utils::sciToFloat(const std::string& str)
{

    std::stringstream ss(str);
    float d = 0;
    ss >> d;

    if (ss.fail()) {
        std::string s = "Unable to format ";
        s += str;
        s += " as a number!";
        throw(s);
    }

    return (d);
}
