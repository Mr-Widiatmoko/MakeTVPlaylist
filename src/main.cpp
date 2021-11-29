//
//  main.cpp
//  Make Serial TV Playlist
//
//  Created by Mr-Widiatmoko on 11/26/21.
//
//  This file is part of Make Serial TV Playlist. It is subject to the license terms 
//  in the LICENSE file found in the top-level directory of this
//  distribution and at http://github.com/Mr-Widiatmoko/MakeSerialTVPlaylist/LICENSE.txt
//

#if DEBUG
#include <chrono>
#endif
#include <iostream>
#include <string>
#include <charconv>
#include <vector>
#include <set>
#include <map>
#include <filesystem>
//#include <atomic>
#include <thread>
#include <cstdarg>
#include <algorithm>
//#include <ranges>
#include <fstream>

#define func auto

func tolower(std::string s) -> std::string
{
	std::transform(s.begin(), s.end(), s.begin(),
				// static_cast<int(*)(int)>(std::tolower)         // wrong
				// [](int c){ return std::tolower(c); }           // wrong
				// [](char c){ return std::tolower(c); }          // wrong
				   [](unsigned char c){ return std::tolower(c); } // correct
				  );
	return s;
}

func trim(std::string s) -> std::string
{
	unsigned long start=0, end=s.size();
	for (auto i=0; i<end; ++i)
		if (s[i] not_eq ' ') {
			start=i;
			break;
		}
	for (auto i=end; i>start; --i)
		if (s[i] not_eq ' ') {
			end=i;
			break;
		}
	return s.substr(start, end);
}

func isEqual(std::string source, std::vector<std::string> args) -> bool
{
	for (auto& check : args)
		if (source == check)
			return true;

	return false;
}

func isInt(const std::string s) -> bool
{
	int result;
	if (auto [p, ec] = std::from_chars(s.c_str(), s.c_str()+s.size(), result);
		ec == std::errc())
		return true;
	
	return false;
}

func after(const std::string& keyword,
		   const std::string& source,
		   bool ignoreCase = false,
		   bool trimResult = false) -> std::string
{
	unsigned long pos=0;
	if (ignoreCase)
		pos=tolower(source).find(keyword);
	else
		pos=source.find(keyword);
	
	if (pos not_eq std::string::npos) {
		pos += keyword.size();
		
		auto result = source.substr(pos, source.size());
		
		return (trimResult ? trim(result) : result);
	}
	return source;
}

namespace fs = std::filesystem;

func excludeExtension(const fs::path& path) -> std::string
{
	return path.string().substr(0, path.string().size() - path.extension().string().size());
}

func isMediaFile(const fs::path& path) -> bool
{
	return fs::is_regular_file(path) and isEqual(tolower(path.extension().string()),
		{".mp4", ".mkv", ".mov", ".m4v", ".mpeg", ".mpg", ".mts", ".ts", ".webm",
		".flv", ".opus", ".pcm", ".wmv", ".mp3", ".aac", ".aif", ".aiff", ".3gp",
		".avi", ".ape", ".asf", ".flac", ".cue", "divx", ".dv", ".mka", ".ra",
		".rmvb", ".rm", ".vox", ".wma", ".wav", ".acm"});
}

func findSubtitleFile(const fs::path& original,
					  std::vector<fs::path>* result)
{
	auto noext = excludeExtension(original);
	auto parentPath = original.parent_path();
	if (not parentPath.empty())
		for (auto& f : fs::directory_iterator(parentPath)) {
			if (f.is_regular_file()
				and f.path().string().size() >= original.string().size()
				and f.path().string().substr(0, noext.size()) == noext
				and isEqual(tolower(f.path().extension().string()), {".srt", ".ass", ".vtt"}))
			{
				result->push_back(f.path());
			}
		}
}

func getAvailableFilename(const fs::path original, std::string prefix = " #", std::string suffix = "") -> std::string
{
	if (fs::exists(original)) {
		auto s = original.string();
		auto ext = original.extension().string();
		auto noext = s.substr(0, s.size() - ext.size());
		unsigned i = 0;
		while (true) {
			fs::path test(noext + prefix + std::to_string(++i) + suffix + ext);
			
			if (not fs::exists(test)) {
				return test.string();
			}
		}
	} else
		return original.string();
}

/// Use std::set, just because it guarantee unique items and find-able.
std::set<fs::path> regularDirs 	= {};
std::set<fs::path> seasonDirs	= {};
std::vector<fs::path> selectFiles  = {};

func checkForSeasonDir(const fs::path& path) -> void {
	regularDirs.insert(path);
	
	auto isNamedAsSeasonDir = [](const fs::path& path) {
		auto source = path.string();
		auto suffix = after("season", source, true, true);
		for (auto& keyword : {"season", "s"}) {
			suffix = after(keyword, source, true, true);
			if (source not_eq suffix and isInt(suffix))
				return true;
		}
		
		return false;
	};
	
	std::set<fs::path> possibleSeasonDirs;
	
	std::vector<std::thread> threads;
	
	if (not path.empty())
		for (auto& child : fs::directory_iterator(path))
			if (child.is_directory()) {
				if (isNamedAsSeasonDir(child))
					possibleSeasonDirs.insert(child);
				else {
					regularDirs.insert(child);
					threads.emplace_back(checkForSeasonDir, child);
				}
			}
	
	for (auto& t : threads)
		t.join();

	threads.clear();
	
	for (auto& dir : possibleSeasonDirs) {
		bool isValidSeasonDir = true;
		auto parentPath = dir.parent_path();
		if (not parentPath.empty())
			for (auto& pdc : fs::directory_iterator(parentPath))
				if (pdc.is_directory()
					and pdc.path().filename() not_eq ".DS_Store"
					and not isNamedAsSeasonDir(pdc)) {
					isValidSeasonDir = false;
					break;
				}

		if (isValidSeasonDir) {
			auto newSeasonDir = dir.parent_path();
			seasonDirs.insert(newSeasonDir);
			auto findInRegular = regularDirs.find(newSeasonDir);
			if (findInRegular not_eq regularDirs.end())
				regularDirs.erase(findInRegular);
		} else
			threads.emplace_back(checkForSeasonDir, dir);
	}
	
	for (auto& t : threads)
		t.join();
}

#undef func
constexpr auto VERSION="version 1 (Late 2021)\n™ and © 2021 Widiatmoko. \
All Rights Reserved. License and Warranty\n";

int main(int argc, char *argv[]) {
	std::vector<std::thread> threads;
	
	auto verbose = false;
	auto overwrite = false;
	std::string outDir = "";
	std::string fixFilename = "";
	unsigned inputDirCount = 0;
	
	#if DEBUG
	auto start = std::chrono::system_clock::now();
	#endif
	
	if (argc == 1) {
		checkForSeasonDir(fs::path(argv[0]).parent_path());
		inputDirCount += 1;
	} else {
		for (int i=1; i<argc; ++i) {
			if (0 == std::strcmp(argv[i], "--help")) {
				std::cout <<
				fs::path(argv[0]).filename().string() << ' '
				<< VERSION <<
"\nCreate playlist file '.m3u8' from vary directories and files, \
then arranged one episode per title \
for all input titles.\n\n\
Usage:\n    "<< fs::path(argv[0]).filename()<<" [Option or Dir or File] ...\n\n\
If no argument was specified, the current directory will be use.\n\n\
Option:\n\
--overwrite                 Overwrite output playlist file.\n\
--verbose                   Display playlist content.\n\
--fix-filename [filename]   Set output playlist filename.\n\
--out-dir [directory path]  Set output directory for playlist file.\n";
				return 0;
			} else if (0 == std::strcmp(argv[i], "--overwrite"))
				overwrite = true;
			else if (0 == std::strcmp(argv[i], "--verbose"))
				verbose = true;
			else if (0 == std::strcmp(argv[i], "--fix-filename") and i + 1 < argc) {
				i += 1;
				fixFilename = argv[i];
				if (not fs::path(fixFilename).parent_path().string().empty())
				{
					fixFilename = fs::path(fixFilename).filename().string();
					outDir = fs::path(fixFilename).parent_path().string() + fs::path::preferred_separator;
				}
				
			} else if (0 == std::strcmp(argv[i], "--out-dir") and i + 1 < argc) {
				i += 1;
				outDir = fs::absolute(argv[i]);
				if (fs::exists(outDir)) {
					if (outDir[outDir.size() - 1] not_eq fs::path::preferred_separator)
						outDir += fs::path::preferred_separator;
				} else
					std::cout << "Cannot set output dir to " << fs::path(outDir) << '\n';
			} else if (fs::is_directory(argv[i])) {
				if (i == 1 and 1 == argc - 1)
					outDir = fs::absolute(argv[i]).string() + fs::path::preferred_separator;
				inputDirCount += 1;
				threads.emplace_back(checkForSeasonDir, argv[i]);
			} else if (isMediaFile(argv[i]))
				selectFiles.push_back(argv[i]);
		}
	
		for (auto& t : threads)
			t.join();
	}
	
	const auto regularDirSize = ::regularDirs.size();
	const auto seasonDirSize = ::seasonDirs.size();
	
	const auto maxDirSize = (regularDirSize >= seasonDirSize ? regularDirSize : seasonDirSize);
	
	/// Convert std::set to classic array, to enable call by index subscript.
	auto index = 0;
	fs::path regularDirs[regularDirSize];
	for (auto& d : ::regularDirs)
		regularDirs[index++] = d;
	
	index = 0;
	fs::path seasonDirs[seasonDirSize];
	for (auto& sd : ::seasonDirs)
		seasonDirs[index++] = sd;

	auto indexFile = 0;
	
	std::map<std::string, std::shared_ptr<std::vector<fs::path>>> records;
	
	std::string outputName = outDir + (fixFilename != "" ? fixFilename : "playlist_from_" +
								(argc == 1
									  ? fs::path(argv[0]).parent_path().filename().string()
									  : (argc == 2
										 ? fs::path(argv[1]).filename().string()
										 : std::to_string(inputDirCount))) + "_dir" + (inputDirCount > 2 ? "s" : "") + ".m3u8");
	if (overwrite and fs::exists(outputName))
		fs::remove(outputName);
	else
		std::string outputName = getAvailableFilename(outputName);
	
	std::ofstream outputFile(outputName, std::ios::out);
	
	unsigned long pleylistCount = 0;
	
	auto putIntoPlaylist = [&](const fs::path& file) {
		outputFile << fs::absolute(file).string() << '\n';
		#ifndef DEBUG
		if (verbose)
		#endif
			std::cout << fs::absolute(file).string() << '\n';
		
		pleylistCount += 1;
		
		std::vector<fs::path> subtitleFiles = {};
		findSubtitleFile(file, &subtitleFiles);
		for (auto& sf : subtitleFiles) {
			outputFile << fs::absolute(sf).string() << '\n';
			#ifndef DEBUG
			if (verbose)
			#endif
				std::cout << fs::absolute(sf).string() << '\n';
		}
	};
	
	while (true) {
		auto finish = true;
		
		for (auto i = 0; i < maxDirSize; ++i) {
			for (auto indexPass = 1; indexPass <= 2; ++indexPass) { ///pass 1 for regularDirs, pass 2 for seasonDirs
				if ((indexPass == 1 and i >= regularDirSize)
					or (indexPass == 2 and i >= seasonDirSize)) {
					continue;
				}
				
				std::vector<fs::path> bufferFiles;

				auto filterChildFiles = [&bufferFiles](const fs::directory_entry& f) {
					if (isMediaFile(f.path()))
						bufferFiles.push_back(f.path());
				};
				
				auto dir = (indexPass == 1 ? regularDirs[i] : seasonDirs[i]);
				if (dir.empty())
					continue;
				
				auto found = records[dir.string()];
				if (found == nullptr) {
					if (indexPass == 1)
						for (auto& f : fs::directory_iterator(dir))
							filterChildFiles(f);
					else
						for (auto& f : fs::recursive_directory_iterator(dir))
							filterChildFiles(f);
					std::sort(bufferFiles.begin(), bufferFiles.end());
					records[dir.string()] = std::make_shared<std::vector<fs::path>>(bufferFiles);
				} else
					bufferFiles = *found;
				
				if (bufferFiles.size() > indexFile) {
					finish = false;
										
					putIntoPlaylist(bufferFiles[indexFile]);

				}
			} //end pass loop
		}
		
		if (indexFile < selectFiles.size()) {
			putIntoPlaylist(selectFiles[indexFile]);
			if (finish and indexFile < selectFiles.size() - 1) {
				finish = false;
			}
		}
		
		indexFile += 1;
		
		if (finish)
			break;
	}
	
	if (outputFile.is_open())
		outputFile.close();
	
	if (pleylistCount == 0)
		fs::remove(outputName);
	else
		std::cout << fs::absolute(outputName).string() << '\n';
	
	#if DEBUG
	auto stop = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> time = stop - start;
	std::cout << std::fixed << std::setprecision(2) << time.count() << "  ms\n";
	#endif

	

	return 0;
}
