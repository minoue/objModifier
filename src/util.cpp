#include "util.hpp"
#include <sstream>

Utils::Utils() {};

Utils::~Utils() {};

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

// https://marycore.jp/prog/cpp/vector-join/
std::string Utils::join(const std::vector<std::string>& v, const char* delim = 0)
{
    std::string s;
    if (!v.empty()) {
        s += v[0];
        for (decltype(v.size()) i = 1, c = v.size(); i < c; ++i) {
        if (delim) s += delim;
        s += v[i];
    }
  }
  return s;
}

std::string Utils::pathGetExt(const std::string path)
{

    std::vector<std::string> pathSplit;
    split(path, pathSplit, '.');
    std::string& last = pathSplit.back();
    return last;
}


std::string Utils::pathReplaceBody(std::string& path, const std::string& body)
{
    std::vector<std::string> pathSplit;
    split(path, pathSplit, '/');
    size_t n = pathSplit.size();
    std::string& last = pathSplit[n-1];

    std::vector<std::string> extSplit;
    split(last, extSplit, '.');
    extSplit[0] = body;

    std::string newBody = join(extSplit, ".");

    pathSplit[n-1] = newBody;

    std::string newPath = join(pathSplit, "/");
    return newPath;
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
