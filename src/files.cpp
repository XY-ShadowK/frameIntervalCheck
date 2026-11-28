#include <cstdio>
#include <filesystem>

#include "File_Path.hpp"
#include "File_Utils.hpp"
#include "Error_Utils.hpp"

#include "include/files.hpp"

using std::vector;
using std::string;
using WindowsRub::File::FilePath;
using WindowsRub::File::getFilePathU8;
using WindowsRub::File::getU8StringFilePath;
using WindowsRub::File::getPathFiles;
using WindowsRub::ErrorUtils::errorCode2String;

vector<string> getFiles(const string& pathStr, const string& pattern
, const bool isVerbose) {
	FilePath path = getFilePathU8(pathStr);
	if(!std::filesystem::exists(path)) {
		fprintf(stderr, "%s not exists.\n", pathStr.c_str());
		return {};
	}
    vector<FilePath> filesMatched, filesUnmatched;
	const std::error_code& errorCode = getPathFiles(path, pattern, filesMatched, filesUnmatched, false);
	if(errorCode) {
		fprintf(stderr, "scanning %s, %s\n", pathStr.c_str(), errorCode2String(errorCode).c_str());
		return {};
	}
	vector<string> files;
	files.reserve(filesMatched.size());
	if(isVerbose) {
		printf("scanned files:\n");
		for(const FilePath& filePath : filesUnmatched)
			printf("%s\n", getU8StringFilePath(filePath).c_str());
		for(const FilePath& filePath : filesMatched) {
			const string& filename = getU8StringFilePath(filePath);
			printf("%s\n", filename.c_str());
			files.emplace_back(filename);
		}
	}
	else
		for(const FilePath& filePath : filesMatched)
			files.emplace_back(getU8StringFilePath(filePath));
    return files;
}