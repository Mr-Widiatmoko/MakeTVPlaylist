//
//  main.cpp
//  Make TV Playlist
//
//  Created by Mr-Widiatmoko on 11/26/21.
//
//  This file is part of Make TV Playlist. It is subject to the license terms
//  in the LICENSE file found in the top-level directory of this
//  distribution and at https://github.com/Mr-Widiatmoko/MakeTVPlaylist/LICENSE.txt
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
#include <thread>
#include <cstdarg>
#include <algorithm>
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
	unsigned long start{0}, end{s.size()};
	for (auto i{0}; i<end; ++i)
		if (not std::isspace(s[i])) {
			start=i;
			break;
		}
	for (auto i{end}; i>start; --i)
		if (not std::isspace(s[i])) {
			end=i;
			break;
		}
	return s.substr(start, end);
}

func isEqual(std::string source, const std::vector<std::string>* args) -> bool
{
	for (auto& check : *args)
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
	unsigned long pos{0};
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

func containInts(const std::string&& s) -> std::vector<int>
{
	std::string buffer;
	std::vector<int> nums;
	for (auto i{0}; i < s.size(); ++i) {
		
		if (const auto c{s[i]}; std::isdigit(c)) {
			buffer += c;
		} else {
			if (buffer.empty())
				continue;
			if (buffer.size() < 9)
			try {
				nums.push_back(std::stoi(buffer));
			} catch (std::out_of_range const& e) {
				std::cout << "stoi(" << s << ") Fail: " << e.what() << '\n';
			}
			buffer.clear();
		}
	}
	
	if (not buffer.empty())
		if (buffer.size() < 9)
		try {
			nums.push_back(std::stoi(buffer));
		} catch (std::out_of_range const& e) {
			std::cout << "stoi(" << s << ") Fail: " << e.what() << '\n';
		}
	
	return nums;
}

namespace fs = std::filesystem;

func excludeExtension(const fs::path& path) -> std::string
{
	return path.string().substr(0, path.string().size() - path.extension().string().size());
}

func parseCommaDelimited(const std::string&& literal, std::vector<std::string>* result)
{
	std::string buffer;
	for (auto& c : literal) {
		if (std::isspace(c))
			continue;
		else if (c == ',' and buffer.size() > 2) {
			if (buffer[0] == '*' and buffer[1] == '.')
				buffer.erase(buffer.begin());
			else if (buffer[0] not_eq '.')
				buffer.insert(buffer.begin(), '.');
			result->push_back(buffer);
			buffer = "";
		} else
			buffer += c;
	}
}

func isMediaFile(const fs::path& path, const std::string extensions,
				 char greaterThan = '\0', std::uintmax_t size = 0, std::uintmax_t sizeTo = 0) -> bool
{
	if (not fs::exists(path))
		return false;
	
	std::vector<std::string> x;
	if (extensions.empty()) {
		constexpr auto s{ ".mp4, .mkv, .mov, .m4v, .mpeg, .mpg, .mts, .ts, .webm,\
		.flv, .opus, .pcm, .wmv, .mp3, .aac, .aif, .aiff, .3gp,\
		.avi, .ape, .asf, .flac, .cue, divx, .dv, .mka, .ra,\
  .rmvb, .rm, .vox, .wma, .wav, .acm" };
		parseCommaDelimited(s, &x);
	} else
		parseCommaDelimited(tolower(extensions), &x);
	
	auto tmp{ path };
	if (fs::is_symlink(tmp)) { //TODO: is_symlink() cannot detect macOS alias file!
		tmp = fs::read_symlink(path);
		
		if (not fs::exists(tmp))
			return false;
	}

	auto fileSize{ fs::file_size(tmp) };
	return fs::is_regular_file(tmp) and isEqual(tolower(tmp.extension().string()), &x)
		and (size == 0 and sizeTo == 0 ? true
		 : (greaterThan == '\0' ? fileSize > size and fileSize < sizeTo
			: (greaterThan == '>' ? fileSize > size : fileSize < size))
		 );
}

func findSubtitleFile(const fs::path& original,
					  std::vector<fs::path>* result)
{
	if (auto parentPath{original.parent_path()}; not parentPath.empty()) {
		auto noext{excludeExtension(original)};
		std::vector<std::string> x{".srt", ".ass", ".vtt"};
		for (auto& f : fs::directory_iterator(parentPath)) {
			if (f.is_regular_file()
				and f.path().string().size() >= original.string().size()
				and f.path().string().substr(0, noext.size()) == noext
				and isEqual(tolower(f.path().extension().string()), &x))
			{
				result->push_back(f.path());
			}
		}
	}
}

func getAvailableFilename(const fs::path original, std::string prefix = " #", std::string suffix = "") -> std::string
{
	if (fs::exists(original)) {
		auto s{original.string()};
		auto ext{original.extension().string()};
		auto noext{s.substr(0, s.size() - ext.size())};
		unsigned i{0};
		while (true) {
			fs::path test(noext + prefix + std::to_string(++i) + suffix + ext);
			
			if (not fs::exists(test)) {
				return test.string();
			}
		}
	} else
		return original.string();
}

func isContainsSeasonDirs(const fs::path path) -> bool {
	std::vector<fs::directory_entry> sortedDir;
	for (auto& child : fs::directory_iterator(path))
		sortedDir.push_back(child);
	
	std::sort(sortedDir.begin(), sortedDir.end());
	
	auto hasDirs{false};
	
	auto isNum{true};
	std::vector<int> lastNum;
	std::vector<fs::path> bufferNum;
	
	for (auto& child : sortedDir)
		if (child.is_directory() and not fs::is_symlink(child.path())) {
			if (child.path().filename() == ".DS_Store"
				or (child.status().permissions()
					& (fs::perms::owner_read
					   | fs::perms::group_read
					   | fs::perms::others_read)) not_eq fs::perms::none
				)
				continue;
			
			hasDirs = true;
			if (isNum) {
				if (auto iNames{containInts(child.path().filename().string())};
					not iNames.empty()) {
					if (lastNum.empty()) {
						lastNum = std::move(iNames);
						bufferNum.push_back(child.path());
						continue;
					} else if (lastNum.size() == iNames.size()) {
						bool hasIncreased{false};
						for (auto xi{0}; xi < lastNum.size(); ++xi)
							if (lastNum[xi] < iNames[xi]) {
								hasIncreased = true;
								break;
							}
						
						if (hasIncreased) {
							bufferNum.push_back(child.path());
							continue;
						}
					}
				}
			}
			
			isNum = false;
			break;
		}

	return isNum and hasDirs;
}

/// Use std::set, just because it guarantee unique items and find-able.
std::set<fs::path> regularDirs 	= {};
std::set<fs::path> seasonDirs	= {};
std::vector<fs::path> selectFiles  = {};

func checkForSeasonDir(const fs::path& path) -> void {
	auto isNamedAsSeasonDir{ [](const fs::path& path) {
		auto source{path.string()};

		for (auto& keyword : {"season", "s"})
			if (auto suffix{after(keyword, source, true, true)};
				source not_eq suffix and isInt(suffix))
				return true;		
		
		return false;
	}};
	
	std::set<fs::path> possibleSeasonDirs;
	
	std::vector<std::thread> threads;
	
	if (not path.empty()) {
		auto hasDir{false};
		
		bool isNum{true};
		std::vector<int> lastNum;
		std::vector<fs::path> bufferNum;
		
		auto pullFromBUfferNum{ [&threads, &bufferNum, &isNum]() {
			isNum = false;
			for (auto& child : bufferNum) {
				regularDirs.insert(child);
				threads.emplace_back(checkForSeasonDir, child);
			}
		}};
		
		std::vector<fs::directory_entry> sortedDir;
		for (auto& child : fs::directory_iterator(path)) {
			if (child.path().filename() == ".DS_Store"
				or (child.status().permissions()
					& (fs::perms::owner_read
					   | fs::perms::group_read
					   | fs::perms::others_read)) not_eq fs::perms::none
				)
				continue;
			sortedDir.push_back(child);
		}
		
		std::sort(sortedDir.begin(), sortedDir.end());
		for (auto& child : sortedDir)
			if (child.is_directory()) {
				hasDir = true;
				if (isNamedAsSeasonDir(child))
				{
					possibleSeasonDirs.insert(child.path());
					pullFromBUfferNum();
				}
				else
				{
					if (isNum) {
						if (auto iNames{containInts(child.path().filename().string())};
							not iNames.empty()) {
							if (isContainsSeasonDirs(child.path())) {
								isNum = false;
							} else {
							if (lastNum.empty()) {
								lastNum = std::move(iNames);
								bufferNum.push_back(child.path());
								continue;
							} else if (lastNum.size() == iNames.size()) {
								bool hasIncreased{false};
								for (auto xi = 0; xi < lastNum.size(); ++xi)
									if (lastNum[xi] < iNames[xi]) {
										hasIncreased = true;
										break;
									}
								
								if (hasIncreased) {
									bufferNum.push_back(child.path());
									continue;
								}
							}
							}
						}
					}
					regularDirs.insert(child.path());
					threads.emplace_back(checkForSeasonDir, child.path());
					pullFromBUfferNum();
				}
			}
		
		if ((not hasDir and isNamedAsSeasonDir(path)) or (isNum and hasDir)) {
			regularDirs.erase(path);
			seasonDirs.insert(path);
		} else
			regularDirs.insert(path);
	}
	for (auto& t : threads)
		t.join();

	threads.clear();
	
	for (auto& dir : possibleSeasonDirs) {
		bool isValidSeasonDir{true};
		if (auto parentPath{dir.parent_path()}; not parentPath.empty())
			for (auto& pdc : fs::directory_iterator(parentPath))
				if (pdc.is_directory()
					and pdc.path().filename() not_eq ".DS_Store"
					and not isNamedAsSeasonDir(pdc)) {
					isValidSeasonDir = false;
					break;
				}

		if (isValidSeasonDir) {
			auto newSeasonDir{dir.parent_path()};
			seasonDirs.insert(newSeasonDir);
			
			if (auto findInRegular{regularDirs.find(newSeasonDir)};
				findInRegular not_eq regularDirs.end())
				regularDirs.erase(findInRegular);
		} else
			threads.emplace_back(checkForSeasonDir, dir);
	}
	
	for (auto& t : threads)
		t.join();
}

func processOption(const char *argv) -> const char *
{
	if (auto length{std::strlen(argv)};
		length > 2 and ( argv[0] == '-' and argv[1] == '-')) {
		argv++;
		argv++;
	} else if (length == 2 and argv[0] == '-')
		argv++;
	else {
		return nullptr;
	}
	
	return argv;
}

func getBytes(std::string s) -> uintmax_t
{
	std::string unit{"mb"};
	std::string value{s};
	if (s.size() > 2 and std::isalpha(s[s.size() - 2])) {
		unit = s.substr(s.size() - 2);
		value = s.substr(0, s.size() - 2);
	}
	
	if (not isInt(value))
		return 0.0f;
	
	uintmax_t result{0};
	if (unit == "kb")
		result = std::stof(value) * 1000;
	else if (unit == "gb")
		result = std::stof(value) * 1000000000;
	else
		result = std::stof(value) * 1000000;
	
	return result;
}

func getRange(std::string argv, std::string separator) -> std::shared_ptr<std::pair<uintmax_t, uintmax_t>>
{
	auto pos{argv.find(separator)};
	if (pos == std::string::npos)
		return nullptr;
	
	auto first{tolower(argv.substr(0, pos))};
	auto second{tolower(argv.substr(pos + separator.size()))};

	uintmax_t from{getBytes(std::move(first))};
	uintmax_t to{getBytes(std::move(second))};
	return std::make_shared<std::pair<uintmax_t, uintmax_t>>(from, to);
}

#undef func
constexpr auto VERSION="version 1 (Late 2021)\n™ and © 2021 Widiatmoko. \
All Rights Reserved. License and Warranty\n";

int main(int argc, char *argv[]) {
	std::vector<std::thread> threads;
	
	unsigned inputDirCount{0};
	
	std::map<std::string, std::string> state;
	constexpr auto OPT_HELP 			{"help"};
	constexpr auto OPT_VERSION 			{"version"};
	constexpr auto OPT_VERBOSE 			{"verbose"};
	constexpr auto OPT_OVERWRITE 		{"overwrite"};
	constexpr auto OPT_SKIPSUBTITLE 	{"skip-subtitle"};
	constexpr auto OPT_ONLYEXT 			{"only-ext"};
	constexpr auto OPT_FIXFILENAME 		{"fix-filename"};
	constexpr auto OPT_OUTDIR 			{"out-dir"};
	constexpr auto OPT_SIZE				{"size"};
	constexpr auto OPT_SIZETO			{"size_to"};
	constexpr auto OPT_SIZEOPGT			{"size_op"};
	
	state[OPT_SIZEOPGT] = "\0";
	state[OPT_SIZE] = "0";
	state[OPT_SIZETO] = "0";
	
	#if DEBUG
	auto start{std::chrono::system_clock::now()};
	#endif
	
	{
		auto opt_match{ [](const char* opt, const char* with, char mnemonic) {
			return 0 == std::strcmp(opt, with) or (strlen(opt) == 1 and opt[0] == mnemonic);
		}};
		for (int i{1}; i<argc; ++i) {
			if (auto opt{processOption(argv[i])}; opt) {if (opt_match(opt, OPT_HELP, 'h')) {
				std::cout <<
				fs::path(argv[0]).filename().string() << ' '
				<< VERSION <<
"\nCreate playlist file '.m3u8' from vary directories and files, \
then arranged one episode per title \
for all input titles.\nHosted in https://github.com/Mr-Widiatmoko/MakeTVPlaylist\n\n\
Usage:\n    "<< fs::path(argv[0]).filename()<<" [Option or Dir or File] ...\n\n\
If no argument was specified, the current directory will be use.\n\n\
Option:\n\
-O, --overwrite                 Overwrite output playlist file.\n\
-v, --version                   Display version.\n\
-V, --verbose                   Display playlist content.\n\
-x, --skip-subtitle		Dont include subtitle file.\n\
-e, --only-ext \"extension, ...\"	Filter only specific extensions, separated by comma.\n\
				  Example: --only-ext \"mp4, mkv\"\n\
-s, --size < OR > SIZE		Filter by size in \"KB\", \"MB\" (default), or \"GB\".\n\
       FROM-TO			  Example: --size < 750\n\
       FROM..TO				OR by specify the unit\n\
					   --size > 1.2gb\n\
					OR using range with '-' OR '..'\n\
					   --size 750-1.2gb\n\
-f, --fix-filename \"filename\"   Override output playlist filename.\n\
-d, --out-dir \"directory path\"  Override output directory for playlist file.\n";
				return 0;
			}
			else if (opt_match(opt, OPT_VERSION, 'v'))
				std::cout << VERSION << '\n';
			else if (opt_match(opt, OPT_OVERWRITE, 'O'))
				state[opt] = "true";
			else if (opt_match(opt, OPT_VERBOSE, 'V'))
				state[opt] = "true";
			else if (opt_match(opt, OPT_SKIPSUBTITLE, 'x'))
				state[opt] = "true";
			else if (opt_match(opt, OPT_ONLYEXT, 'e')) {
				if (i + 1 < argc) {
					i++;
					state[opt] = argv[i];
				} else
					std::cout << "Expecting extension after \"--" << opt << "\" option \
(eg: \"mp4, mkv\").\n";
				
			} else if (opt_match(opt, OPT_SIZE, 's') and i + 1 < argc) {
				if (std::strlen(argv[i + 1]) > 0) {
					if (auto nextArgv{argv[i + 1]};
						i + 2 < argc and (nextArgv[0] == '<' or nextArgv[0] == '>') )
					{
						i++;
						state[OPT_SIZEOPGT] = argv[i][0];

						i++;
						state[opt] = std::to_string(std::move(getBytes(argv[i])));
					} else {
						i++;
						auto range{getRange(argv[i], std::move("-"))};
						if (not range)
							range = getRange(argv[i], std::move(".."));
						if (range)
						{
							state[OPT_SIZE] = std::to_string(range->first);
							state[OPT_SIZETO] = std::to_string(range->second);
						} else {
							i--;
							std::cout << "Expecting operator '<' or '>' followed\
 by size in KB, MB, or GB.\nOr use value in range using form 'from-to' OR 'from..to'\n";
						}
					}
				}
			} else if (opt_match(opt, OPT_FIXFILENAME, 'f') and i + 1 < argc) {
				i += 1;
				state[opt] = argv[i];
				if (not fs::path(state[opt]).parent_path().string().empty())
				{
					state[opt] = fs::path(state[opt]).filename().string();
					state[OPT_OUTDIR] = fs::path(state[opt]).parent_path().string()
						+ fs::path::preferred_separator;
				}
				
			} else if (opt_match(opt, OPT_OUTDIR, 'd') and i + 1 < argc) {
				i += 1;
				state[opt] = fs::absolute(argv[i]);
				if (fs::exists(state[opt])) {
					if (auto tmp{state[opt]};
						tmp[tmp.size() - 1] not_eq fs::path::preferred_separator) {
						tmp += fs::path::preferred_separator;
						state[opt] = tmp;
					}
				} else
					std::cout << "Cannot set output dir to " << fs::path(state[opt]) << '\n';
			} else
				goto THERE;
		} else
THERE:		if (fs::is_directory(argv[i])) {
				if (i == 1 and 1 == argc - 1)
					state[OPT_OUTDIR] = fs::absolute(argv[i]).string() + fs::path::preferred_separator;
				inputDirCount += 1;
				threads.emplace_back(checkForSeasonDir, argv[i]);
			} else if (isMediaFile(fs::absolute(argv[i]), state[OPT_ONLYEXT],
								   state[OPT_SIZEOPGT][0],
								   std::stof(state[OPT_SIZE]),
								   std::stof(state[OPT_SIZETO])))
				selectFiles.push_back(fs::absolute(argv[i]));
			else
				std::cout << "What is this: \"" << argv[i] << "\"?, try type \""
						<< fs::path(argv[0]).filename().string() << " --help\"\n";
		}
	
		if (inputDirCount == 0 and selectFiles.empty()) {
			threads.emplace_back(checkForSeasonDir, fs::current_path());
			inputDirCount += 1;
		}
		
		if (state[OPT_OUTDIR].empty()) {
			if (not selectFiles.empty())
				state[OPT_OUTDIR] = fs::path(selectFiles[0]).parent_path().string() + fs::path::preferred_separator;
			else
				state[OPT_OUTDIR] = fs::current_path().string() + fs::path::preferred_separator;
		}
		
		for (auto& t : threads)
			t.join();
	}
	
	const auto regularDirSize{::regularDirs.size()};
	const auto seasonDirSize{::seasonDirs.size()};
	
	const auto maxDirSize{std::max(regularDirSize, seasonDirSize)};
	
	/// Convert std::set to classic array, to enable call by index subscript.
	fs::path regularDirs[regularDirSize];
	std::move(::regularDirs.begin(), ::regularDirs.end(), regularDirs);

	fs::path seasonDirs[seasonDirSize];
	std::move(::seasonDirs.begin(), ::seasonDirs.end(), seasonDirs);
	
	std::sort(selectFiles.begin(), selectFiles.end());

	auto swapDirs{seasonDirs[0].string() < regularDirs[0].string()};
	
	auto indexFile{0};
	auto indexFileSelected{0};
	
	std::map<std::string, std::shared_ptr<std::vector<fs::path>>> records;
	
	fs::path outputName{ state[OPT_OUTDIR] + (state[OPT_FIXFILENAME] != "" ? state[OPT_FIXFILENAME] : "playlist_from_" +
								(inputDirCount == 0
								 ? std::to_string(selectFiles.size()) + "_file"
								 : (inputDirCount == 1
									 ? fs::path(state[OPT_OUTDIR]).filename().string()
									 : std::to_string(inputDirCount)) + "_dir") + (inputDirCount > 2 ? "s" : "") + ".m3u8")};
	if (fs::exists(outputName) and state[OPT_OVERWRITE] == "true")
		fs::remove(outputName);
	else
		outputName = getAvailableFilename(outputName);
	
	std::ofstream outputFile(outputName, std::ios::out);
	
	unsigned long pleylistCount{0};
	
	auto putIntoPlaylist{ [&](const fs::path& file) {
		outputFile << fs::absolute(file).string() << '\n';
		#ifndef DEBUG
		if (state[OPT_VERBOSE] == "true")
		#endif
			std::cout << fs::absolute(file).string() << '\n';
		
		pleylistCount += 1;
		
		if (state[OPT_SKIPSUBTITLE] == "true")
			return;
		
		std::vector<fs::path> subtitleFiles = {};
		findSubtitleFile(file, &subtitleFiles);
		for (auto& sf : subtitleFiles) {
			outputFile << fs::absolute(sf).string() << '\n';
			#ifndef DEBUG
			if (state[OPT_VERBOSE] == "true")
			#endif
				std::cout << fs::absolute(sf).string() << '\n';
		}
	}};
	
	while (true) {
		auto finish{true};
		
		auto putSelectFile{ [&]() {
			putIntoPlaylist(std::move(selectFiles[indexFileSelected]));
			indexFileSelected += 1;
		}};
		
		for (auto i{0}; i < maxDirSize; ++i) {
			for (auto indexPass{1}; indexPass <= 2; ++indexPass) { ///pass 1 for regularDirs, pass 2 for seasonDirs

				auto passSwapDirs{(indexPass == 1 and swapDirs == false) or (indexPass == 2 and swapDirs)};

				if ((indexPass == 1 and i >= (not swapDirs ? regularDirSize : seasonDirSize))
					or (indexPass == 2 and i >= (not swapDirs ? seasonDirSize : regularDirSize))) {
					continue;
				}
				
				std::vector<fs::path> bufferFiles;

				auto filterChildFiles{ [&bufferFiles, &state](const fs::directory_entry& f) {
					if (f.is_regular_file() and isMediaFile(f.path(), state[OPT_ONLYEXT],
									state[OPT_SIZEOPGT][0],
									std::stof(state[OPT_SIZE]),
									std::stof(state[OPT_SIZETO])))
						bufferFiles.push_back(f.path());
				}};
				
				
				if (auto dir{passSwapDirs ? regularDirs[i] : seasonDirs[i]};
					dir.empty())
					continue;
								
				else if (auto found{records[dir.string()]}; found == nullptr) {
					if (passSwapDirs)
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
										
					putIntoPlaylist(std::move(bufferFiles[indexFile]));
				}
			} //end pass loop
		}
		
		if (indexFileSelected < selectFiles.size())
			putSelectFile();
		
		indexFile += 1;
		
		if (finish and indexFileSelected >= selectFiles.size())
			break;
	}
	
	if (outputFile.is_open())
		outputFile.close();
	
	if (pleylistCount == 0)
		fs::remove(outputName);
	else
		std::cout << fs::absolute(outputName).string() << '\n';
	
	#if DEBUG
	auto stop{std::chrono::system_clock::now()};
	std::chrono::duration<double, std::milli> time{stop - start};
	std::cout << std::fixed << std::setprecision(2) << time.count() << "  ms\n";
	#endif

	

	return 0;
}
