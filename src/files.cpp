#include <cstdio>
#include <filesystem>
#include <regex>

#include "File_Utils.hpp"

#include "include/files.hpp"

using std::vector;
using std::string;
using std::regex;
using WindowsRub::File::FilePath;
using WindowsRub::File::getFilePathU8;

regex wildcard2Regex(const std::string& pattern) {
    string strRegex;
    for (char ch : pattern) {
        switch (ch) {
            case '*': strRegex += ".*"; break;
            case '?': strRegex += ".";  break;
            default:  strRegex += ch;   break;
        }
    }
    return regex(strRegex, regex::icase);
}

vector<string> getFiles(const string& pathStr, const string& pattern
, const bool isVerbose) {
	FilePath path = getFilePathU8(pathStr);
	if(!std::filesystem::exists(path)) {
		fprintf(stderr, "%s not exists.\n", pathStr.c_str());
		return {};
	}
	if(isVerbose)
		printf("scanned files:\n");
    vector<string> files;
    string filename;
	regex regexPattern = wildcard2Regex(pattern);
	for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
		if(entry.is_regular_file()) {
			filename = entry.path().filename().u8string();
			if(isVerbose)
				printf("%s\n", filename.c_str());
			if(std::regex_match(filename, regexPattern))
				files.emplace_back(filename);
		}
    return files;
}