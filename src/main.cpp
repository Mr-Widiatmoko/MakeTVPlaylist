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
#include <future>
#include <cstdarg>
#include <algorithm>
#include <fstream>


std::map<std::string, std::string> state;
constexpr auto OPT_HELP 			{"help"};
constexpr auto OPT_VERSION 			{"version"};
constexpr auto OPT_VERBOSE 			{"verbose"};
constexpr auto OPT_BENCHMARK 		{"benchmark"};
constexpr auto OPT_OVERWRITE 		{"overwrite"};
constexpr auto OPT_SKIPSUBTITLE 	{"skip-subtitle"};
constexpr auto OPT_ONLYEXT 			{"only-ext"};
constexpr auto OPT_FIXFILENAME 		{"fix-filename"};
constexpr auto OPT_OUTDIR 			{"out-dir"};
constexpr auto OPT_SIZE				{"size"};
constexpr auto OPT_SIZETO			{"size_to"};
constexpr auto OPT_SIZEOPGT			{"size_op"};
constexpr auto OPT_THREAD			{"thread"};
constexpr auto OPT_ASYNC			{"async"};
constexpr auto OPT_EXECUTION		{"execution"};
constexpr auto OPT_EXCLHIDDEN		{"exclude-hidden"};

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
typedef unsigned long long MAXNUM;
func containInts(const std::string&& s, std::vector<MAXNUM>* const out)
{
	std::string buffer;
	auto make{[&]() -> void {
		if (buffer.size() > 0 and buffer.size() < 19) /// ULLONG_MAX length is 20
			out->push_back(std::stoull(std::move(buffer)));
	}};
	
	for (auto& c : s) {
		if (std::isdigit(c)) {
			buffer += c;
		} else {
			make();
			buffer.clear();
		}
	}
	
	make();
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

func isMediaFile(const fs::path& path,
				 const std::string extensions,
				 char greaterThan = '\0',
				 std::uintmax_t size = 0,
				 std::uintmax_t sizeTo = 0) -> bool
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
		
		if (not fs::exists(tmp) or not fs::is_regular_file(tmp))
			return false;
	}

	auto fileSize{ fs::file_size(tmp) };
	return /*fs::is_regular_file(tmp) and*/ isEqual(tolower(tmp.extension().string()), &x)
		and (size == 0 and sizeTo == 0 ? true
		 : (greaterThan == '\0' ? fileSize > size and fileSize < sizeTo
			: (greaterThan == '>' ? fileSize > size : fileSize < size))
		 );
}

func findSubtitleFile(const fs::path& original,
					  std::vector<fs::path>* const result)
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

func getAvailableFilename(const fs::path original, std::string prefix = " #",
						  std::string suffix = "") -> std::string
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



func listDir(const fs::path& path, std::vector<fs::directory_entry>* const out,
			bool sorted=true)
{
	static std::vector<std::string_view> ignoredPaths;
	if (not out or (std::find(ignoredPaths.begin(), ignoredPaths.end(),
							  path.string()) != ignoredPaths.end()))
		return;
	if (fs::is_symlink(path)) {
		std::error_code ec;
		auto ori = fs::read_symlink(path, ec);
		if (ec) {
			ignoredPaths.push_back(path.string());
			return;
		}
	}
	try {
		for (auto& child : fs::directory_iterator(path)) {
			if (child.path().filename() == ".DS_Store"
				//or child.is_symlink()
				or (child.status().permissions()
					& (fs::perms::owner_read
					   | fs::perms::group_read
					   | fs::perms::others_read)) == fs::perms::none)
				continue;
			else if (child.is_symlink()) {
				if (const auto ori { fs::read_symlink(child.path()) };
					fs::exists(ori) and fs::is_directory(ori))
					out->push_back(child);
				else
					ignoredPaths.push_back(ori.string());
			} else if (child.is_directory())
					out->push_back(child);
		}
	} catch (fs::filesystem_error& e) {
		#ifndef DEBUG
		if (state[OPT_VERBOSE] == "true")
		#endif
			std::cout << e.what() << '\n';
		
		ignoredPaths.push_back(path.string());
	}
	
	if (sorted)
		std::sort(out->begin(), out->end());
}

func isContainsSeasonDirs(const fs::path& path) -> bool {
	std::vector<fs::directory_entry> sortedDir;
	listDir(path, &sortedDir);
	
	auto hasDirs{false};
	
	auto isNum{true};
	std::vector<MAXNUM> lastNum;
	std::vector<fs::path> bufferNum;
	
	for (auto& child : sortedDir) {
		hasDirs = true;
		if (isNum) {
			std::vector<MAXNUM> iNames;
			containInts(child.path().filename().string(), &iNames);
			if (not iNames.empty()) {
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

func insertTo(std::set<fs::path>* const set, const fs::path& path)
{
	if (((fs::status(path).permissions() & (fs::perms::owner_read
											| fs::perms::group_read
											| fs::perms::others_read))
			== fs::perms::none)
		or (state[OPT_EXCLHIDDEN] == "true" and path.filename().string()[0] == '.'))
		;
	else
		set->insert(path);
}

func insertTo(std::vector<fs::path>* const vector, const fs::path& path)
{
	if (((fs::status(path).permissions() & (fs::perms::owner_read
											| fs::perms::group_read
											| fs::perms::others_read))
			== fs::perms::none)
		or (state[OPT_EXCLHIDDEN] == "true" and path.filename().string()[0] == '.'))
		;
	else
		vector->push_back(path);
}

func checkForSeasonDir(const fs::path& path) -> void {	
	if (not path.empty()) {
		auto hasDir{false};
		
		bool isNum{true};
		std::vector<MAXNUM> lastNum;
		std::vector<fs::path> bufferNum;
		
		auto pullFromBufferNum{ [&bufferNum, &isNum]() {
			isNum = false;
			for (auto& child : bufferNum) {
				insertTo(&regularDirs, child);
				checkForSeasonDir(child);
			}
		}};
		
		std::vector<fs::directory_entry> sortedDir;
		listDir(path, &sortedDir);
		
		for (auto& child : sortedDir) {
			hasDir = true;

			if (isNum) {
				std::vector<MAXNUM> iNames;
				containInts(child.path().filename().string(), &iNames);
				if (not iNames.empty()) {
					if (isContainsSeasonDirs(child.path())) {
						isNum = false;
					} else {
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
			}
			insertTo(&regularDirs, child.path());
			checkForSeasonDir(child.path());
			pullFromBufferNum();
		}
		
		if (isNum and hasDir) {
			regularDirs.erase(path);
			insertTo(&seasonDirs, path);
		} else
			insertTo(&regularDirs, path);
	}
}

func getOption(std::string_view s) -> std::optional<std::string_view>
{
	if (s.size() == 2 and s[0] == '-' and std::isalpha(s[1]))
		return s.substr(1);
	else if (s.size() > 2 and s[0] == '-' and s[1] == '-')
		return s.substr(2);
	else
		return std::nullopt;
}

func getBytes(std::string s) -> uintmax_t
{
	std::string unit{"mb"};
	std::string value{s};
	if (s.size() > 2 and std::isalpha(s[s.size() - 2])) {
		unit = tolower(s.substr(s.size() - 2));
		value = s.substr(0, s.size() - 2);
	}
	
	uintmax_t result{0};
	
	if (not isInt(value))
		return result;
	
	float v { std::stof(value) };
	
	if (unit == "kb") {
		if (v < (INT_MAX / 1000))
			result = v * 1000;
	} else if (unit == "gb") {
		if (v < (INT_MAX / 1000000000))
			result = v * 1000000000;
	} else {
		if (v < (INT_MAX / 1000000))
			result = v * 1000000;
	}
	
	return result;
}

func getRange(std::string argv, std::string separator)
		-> std::shared_ptr<std::pair<uintmax_t, uintmax_t>>
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

func expandArgs(const int argc, char* const argv[], std::vector<std::string>* const args)
{
	for (int i{1}; i<argc; ++i) {
		auto arg { argv[i] };
		if (auto len{std::strlen(arg)};
			len >= 2 and arg[0] == '-' and std::isalpha(arg[1])) {
			for (auto index{1}; index < len; ++index) {
				if (arg[index] == '=' or arg[index] == ':') {
					if (index + 1 < len) {
						char dst[len - index];
						std::memcpy(&dst, arg + index + 1, sizeof (len - index));
						args->push_back(dst);
					}
					break;
				} else if (std::isalpha(arg[index]))
					args->push_back({'-', arg[index]});
				else {
					char dst[len - index];
					std::memcpy(&dst, arg + index, sizeof (len - index));
					args->push_back(dst);
					break;
				}
			};
		} else {
			if (len > 3 and arg[0] == '-') {
				auto index{3};
				for (; index < len; ++index)
					if (arg[index] == '=' or arg[index] == ':')
						break;
				
				if (index > 3 and index < len - 1) {
					char dst1[index];
					std::memcpy(&dst1, arg, sizeof index);
					args->push_back(dst1);
					
					index++;
					char dst2[len - index];
					std::memcpy(&dst2, arg + index, sizeof (len - index));
					args->push_back(dst2);
					continue;
				}
			}
			
			args->push_back(arg);
		}
	}
}

func timeLapse(std::chrono::system_clock::time_point& start,
			   std::string_view msg,
			   bool resetStart=false)
{
	auto value { (std::chrono::system_clock::now() - start).count() };
	unsigned inc{ 0 };
	while (value / 1000 > 0 and inc < 2) {
		inc++;
		value /= 1000;
	}
	
	while (value / 60 > 0 and inc < 4) {
		inc++;
		value /= 60;
	}
	
	
	std::string_view tu[]{ "microseconds", "milliseconds", "seconds",
		"minutes", "hours" };
	std::cout << msg << std::fixed << std::setprecision(2)
		<< value << " " << tu[inc] << ".\n\n";
	
	if (resetStart)
		start = std::chrono::system_clock::now();
}

#undef func
constexpr auto VERSION="version 1 (Late 2021)\n™ and © 2021 Widiatmoko. \
All Rights Reserved. License and Warranty\n";

constexpr auto HELP=\
"Create playlist file '.m3u8' from vary directories and files, \
then arranged one episode per title \
for all input titles.\nHosted in https://github.com/Mr-Widiatmoko/MakeTVPlaylist\n\n\
Usage:\n    tvplaylist [Option or Dir or File] ...\n\n\
If no argument was specified, the current directory will be use.\n\n\
Option:\n\
-h, --help                      Display this screen.\n\
-c, --execution USING           Specify execution, using 'thread' or 'async' to process directories.\n\
-b, --benchmark                 Benchmarking execution.\n\
-n, --exclude-hidden            Exclude hidden folders or files.\n\
-O, --overwrite                 Overwrite output playlist file.\n\
-v, --version                   Display version.\n\
-V, --verbose                   Display playlist content, and fail messages.\n\
-x, --skip-subtitle		Dont include subtitle file.\n\
-e, --only-ext \"extension, ...\"	Filter only specific extensions, separated by comma.\n\
				  Example: --only-ext \"mp4, mkv\"\n\
-s, --size < OR > SIZE		Filter by size in \"KB\", \"MB\" (default), or \"GB\".\n\
	   FROM-TO	  	  Example: --size < 750\n\
	   FROM..TO			OR by specify the unit\n\
					   --size > 1.2gb\n\
					OR using range with '-' OR '..'\n\
					   --size 750-1.2gb\n\
-f, --fix-filename \"filename\"   Override output playlist filename.\n\
-d, --out-dir \"directory path\"  Override output directory for playlist file.\n\
\n\
Options can be joined, and replace option assignment separator [SPACE] with '=' \
or ':'. For the example:\n\
  tvplaylist -hOVvc=async -xs<1.3gb -r=mp4 \"-f:My-playlist.m3u8\"\n\n\
Thats it, -h -O -V -v -c are joined, and -c has assignment operator '=' instead of\
 using separator [SPACE].\n\
Also -x -s are joined, and -s has remove [SPACE] separator for operator '<' and \
value '1.3gb'.\n\
Redefinition of option means, it will use the last option. For the example, \
this example will not use 'thread' execution at all':\n\
  tvplaylist -c=thread /usr/local/videos -c=none /Users/Shared/Videos\n"
;

int main(int argc, char *argv[]) {
	std::vector<fs::path> bufferDirs;

	state[OPT_SIZEOPGT] = '\0';
	state[OPT_SIZE] 	= "0";
	state[OPT_SIZETO] 	= "0";
	state[OPT_EXECUTION]= OPT_ASYNC;
	
	auto isMatch{ [&](std::optional<std::string_view> opt,
					 const char* with,
					 char mnemonic,
					 bool writeBoolean=false) {
		auto result { opt.value() == with or opt.value()[0] == mnemonic };
		if (result and writeBoolean)
			state[with] = "true";
		return result;
	} };
	
	std::vector<std::string> args;
	expandArgs(argc, argv, &args);
	
	for (int i{0}; i<args.size(); ++i) {
		if (auto opt{ getOption(args[i]) }; opt) {
			if (auto isHelp{isMatch(opt, OPT_HELP, 	'h')};
				isHelp or isMatch(opt, OPT_VERSION, 'v'))
			{
				std::cout <<
				fs::path(args[0]).filename().string() << ' '
				<< VERSION << '\n';
				
				if (isHelp)
					std::cout << HELP << '\n';
				
				if (i + 1 == args.size())
					return 0;
			}
			else if (isMatch(opt, OPT_OVERWRITE, 	'O', true));
			else if (isMatch(opt, OPT_VERBOSE, 		'V', true));
			else if (isMatch(opt, OPT_BENCHMARK, 	'b', true));
			else if (isMatch(opt, OPT_SKIPSUBTITLE, 'x', true));
			else if (isMatch(opt, OPT_EXCLHIDDEN, 	'n', true));
			else if (isMatch(opt, OPT_EXECUTION, 	'c')) {
				if (i + 1 < args.size()) {
					i++;
					if (args[i] == OPT_ASYNC or args[i] == OPT_THREAD)
						state[OPT_EXECUTION] = args[i];
					else
						state[OPT_EXECUTION] = "";
				} else
					std::cout << "Expecting 'thread', 'async', or 'none' after \""
					<< args[i] << "\" option.\n";
			} else if (isMatch(opt, OPT_ONLYEXT, 	'e')) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1]] = args[i];
				} else
					std::cout << "Expecting extension after \""
					<< args[i] << "\" option (eg: \"mp4, mkv\").\n";
			}
			else if (isMatch(opt, OPT_FIXFILENAME, 	'f')) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1]] = args[i];
					if (not fs::path(args[i]).parent_path().string().empty())
					{
						state[args[i - 1]] = fs::path(args[i]).filename().string();
						state[OPT_OUTDIR] = fs::path(args[i]).parent_path().string()
							+ fs::path::preferred_separator;
					}
				} else
					std::cout << "Expecting file name after \""
					<< args[i] << "\" option (eg: \"my_playlist.m3u8\").\n";
			}
			else if (isMatch(opt, OPT_OUTDIR, 		'd')) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1]] = fs::absolute(args[i]);
					if (auto tmp{args[i]};
						tmp[tmp.size() - 1] not_eq fs::path::preferred_separator)
						state[args[i - 1]] += fs::path::preferred_separator;
				} else
					std::cout << "Expecting directory after \""
					<< args[i] << "\" option (eg: \"Downloads/\").\n";
			}
			else if (isMatch(opt, OPT_SIZE, 		's')) {
				if (i + 1 < args.size()) {
					if (args[i + 1][0] == '<' or args[i + 1][0] == '>')
					{
						i++;
						state[OPT_SIZEOPGT] = args[i][0];
						
						uintmax_t value{ 0 };
						
						if (args[i].size() > 1) {
							value = getBytes(args[i].substr(1));
							if (value == 0)
								goto SIZE_NEEDED;
						}
						
						if (value == 0 and i + 1 < args.size()) {
							value = getBytes(args[i + 1]);
							
							if (value == 0)
								goto SIZE_NEEDED;
							else
								i++;
						}
						
						state[OPT_SIZE] = std::to_string(std::move(value));
					} else {
						auto range{getRange(args[i + 1], std::move("-"))};
						if (not range)
							range = getRange(args[i + 1], std::move(".."));
						
						if (range)
						{
							state[OPT_SIZE] = std::to_string(range->first);
							state[OPT_SIZETO] = std::to_string(range->second);
							
							i++;
							
							if (state[OPT_SIZETO] == "0") {
								uintmax_t value{ 0 };
								if (i + 1 < args.size())
									value = getBytes(args[i + 1]);
								
								if (value != 0) {
									state[OPT_SIZETO] = std::to_string(value);
									i++;
								} else
									goto SIZE_NEEDED;
							}
						} else {
							if (i + 3 < args.size()) {
								state[OPT_SIZE] = std::to_string(getBytes(args[i + 1]));
								state[OPT_SIZETO] = std::to_string(getBytes(args[i + 3]));
							}
							
							if (state[OPT_SIZE] != "0" and state[OPT_SIZETO] != "0")
								i+=3;
							else if (state[OPT_SIZE] != "0" and state[OPT_SIZETO] == "0") {
								if (args[i + 2].size() > 1
									and args[i + 2][0] == '-'
									and std::isdigit(args[i + 2][1]))
								{
									state[OPT_SIZETO] = std::to_string(getBytes(args[i + 2].substr(1)));
								} else if (args[i + 2].size() > 2
										   and args[i + 2][0] == '.'
										   and args[i + 2][1] == '.'
										   and std::isdigit(args[i + 2][2]))
								{
									state[OPT_SIZETO] = std::to_string(getBytes(args[i + 2].substr(2)));
								} else
									goto SIZE_NEEDED;
								i+=2;
							} else
								goto SIZE_NEEDED;
						}
						
						if (std::stoul(state[OPT_SIZETO]) < std::stoul(state[OPT_SIZE])) {
							std::cout << "Fail: Range up side down! \""
								<< state[OPT_SIZE] << "bytes greater than "
								<< state[OPT_SIZETO] << "bytes\"\n";
							state[OPT_SIZE] = "0";
							state[OPT_SIZETO] = "0";
						}
					}
				}
				else
SIZE_NEEDED:		std::cout << "Expecting operator '<' or '>' followed\
by size in KB, MB, or GB.\nOr use value in range using form 'from-to' OR 'from..to'\n";
			}
			else goto CHECK_PATH;
		} else
CHECK_PATH:	if (fs::is_directory(args[i]))
			insertTo(&bufferDirs, std::move(fs::path(args[i])));
		
		else if (fs::is_regular_file(std::move(fs::path(args[i]))) and
					  isMediaFile(fs::absolute(args[i]), state[OPT_ONLYEXT],
							   state[OPT_SIZEOPGT][0],
							   std::stof(state[OPT_SIZE]),
							   std::stof(state[OPT_SIZETO])))
			insertTo(&selectFiles, std::move(fs::absolute(args[i])));
		else
			std::cout << "What is this: \"" << args[i] << "\"?\nTry to type \""
					<< fs::path(args[0]).filename().string() << " --help\"\n";
	}

	if (bufferDirs.empty() and selectFiles.empty())
		insertTo(&bufferDirs, fs::current_path());
	
	const auto inputDirsCount = bufferDirs.size();

	while (bufferDirs.size() == 1) {
		state[OPT_OUTDIR] = fs::absolute(bufferDirs[0]).string()
								+ fs::path::preferred_separator;
		
		insertTo(&regularDirs, std::move(bufferDirs[0]));/// Assume single input dir is regularDir
		bufferDirs.clear();
		
		std::vector<fs::directory_entry> sortedDirs;
		listDir(bufferDirs[0], &sortedDirs);
		
		for (auto& child : sortedDirs)
			insertTo(&bufferDirs, std::move(child.path()));
	}

	#ifndef DEBUG
	if (state[OPT_VERBOSE] == "true" or state[OPT_BENCHMARK] == "true")
	#endif
	std::cout
		<< OPT_EXECUTION << "\t\t: " << state[OPT_EXECUTION] << '\n'
		<< OPT_VERBOSE << "\t\t\t: " << state[OPT_VERBOSE] << '\n'
		<< OPT_BENCHMARK << "\t\t: " << state[OPT_BENCHMARK] << '\n'
		<< OPT_OVERWRITE << "\t\t: " << state[OPT_OVERWRITE] << '\n'
		<< OPT_FIXFILENAME << "\t\t: " << state[OPT_FIXFILENAME] << '\n'
		<< OPT_OUTDIR << "\t\t\t: " << state[OPT_OUTDIR] << '\n'
		<< OPT_SKIPSUBTITLE << "\t\t: " << state[OPT_SKIPSUBTITLE] << '\n'
		<< OPT_ONLYEXT << "\t\t: " << state[OPT_ONLYEXT] << '\n'
		<< OPT_SIZE << "\t\t\t: " << (state[OPT_SIZETO] == "0" ? state[OPT_SIZEOPGT] : "")
			<< (state[OPT_SIZEOPGT].empty() ? "" : " ")
			<< state[OPT_SIZE]
			<< (state[OPT_SIZETO] != "0" ? ".." : " bytes")
			<< (state[OPT_SIZETO] == "0" ? "" : state[OPT_SIZETO] + " bytes")
			<< '\n'
	<< '\n';
	
	if (state[OPT_OUTDIR].empty()) {
		if (selectFiles.empty())
			state[OPT_OUTDIR] = fs::current_path().string()
									+ fs::path::preferred_separator;
		else
			state[OPT_OUTDIR] = fs::path(selectFiles[0]).parent_path().string()
									+ fs::path::preferred_separator;
	}
	
	auto start{std::chrono::system_clock::now()};

	std::vector<std::thread> threads;
	std::vector<std::future<void>> asyncs;
	for (auto& child : bufferDirs)
		if (state[OPT_EXECUTION] == OPT_THREAD)
			threads.emplace_back(checkForSeasonDir, std::move(child));
		else if (state[OPT_EXECUTION] == OPT_ASYNC)
			asyncs.emplace_back(std::async(std::launch::async, checkForSeasonDir, std::move(child)));
		else
			checkForSeasonDir(std::move(child));

	if (state[OPT_EXECUTION] == OPT_THREAD)
		for (auto& t : threads)
			t.join();
	else if (state[OPT_EXECUTION] == OPT_ASYNC)
		for (auto& a : asyncs)
			a.wait();
		
	const auto regularDirSize{::regularDirs.size()};
	const auto seasonDirSize{::seasonDirs.size()};
	
	const auto maxDirSize{std::max(regularDirSize, seasonDirSize)};

	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true")
	#endif
		timeLapse(start, std::to_string(regularDirSize + seasonDirSize) + " valid dirs took ", true);

	/// Convert std::set to classic array, to enable call by index subscript.
	fs::path regularDirs[regularDirSize];
	std::move(::regularDirs.begin(), ::regularDirs.end(), regularDirs);

	fs::path seasonDirs[seasonDirSize];
	std::move(::seasonDirs.begin(), ::seasonDirs.end(), seasonDirs);
	
	std::sort(selectFiles.begin(), selectFiles.end());
	
	std::map<std::string, std::shared_ptr<std::vector<fs::path>>> records;
	
	fs::path outputName{ state[OPT_OUTDIR]
							+ (state[OPT_FIXFILENAME] != ""
							   ? state[OPT_FIXFILENAME]
							   : "playlist_from_" + (inputDirsCount == 0
								 ? std::to_string(selectFiles.size()) + "_file"
								 : (inputDirsCount == 1
									 ? fs::path(state[OPT_OUTDIR].substr(0,
										state[OPT_OUTDIR].size() - 2)).filename().string()
									 : std::to_string(inputDirsCount)) + "_dir")
										+ (inputDirsCount > 1
										   or selectFiles.size() > 1 ? "s" : "")
										+ ".m3u8")};
	if (fs::exists(outputName) and state[OPT_OVERWRITE] == "true")
		fs::remove(outputName);
	else
		outputName = getAvailableFilename(outputName);
	
	std::ofstream outputFile(outputName, std::ios::out);
	
	unsigned long indexFile{0};
	unsigned long playlistCount{0};
	
	auto putIntoPlaylist{ [&](const fs::path& file) {
		playlistCount += 1;
		outputFile << fs::absolute(file).string() << '\n';
		#ifndef DEBUG
		if (state[OPT_VERBOSE] == "true")
		#endif
			std::cout << fs::absolute(file).string() << '\n';
				
		if (state[OPT_SKIPSUBTITLE] != "true") {
			std::vector<fs::path> subtitleFiles = {};
			findSubtitleFile(file, &subtitleFiles);
			for (auto& sf : subtitleFiles) {
				outputFile << fs::absolute(sf).string() << '\n';
				#ifndef DEBUG
				if (state[OPT_VERBOSE] == "true")
				#endif
					std::cout << fs::absolute(sf).string() << '\n';
			}
		}
	}};
		
	while (true) {
		auto finish{true};
		std::vector<fs::path> bufferSort;
		
		for (auto i{0}; i < maxDirSize; ++i) {
			for (auto indexPass{1}; indexPass <= 2; ++indexPass)
				///pass 1 for regularDirs, pass 2 for seasonDirs
			{
				if ((indexPass == 1 and i >= regularDirSize)
					or (indexPass == 2 and i >= seasonDirSize))
					continue;
				
				std::vector<fs::path> bufferFiles;

				auto filterChildFiles{ [&bufferFiles](const fs::directory_entry& f) {
					if (not fs::is_regular_file(f.path())
						or (fs::status(f.path()).permissions()
							& (fs::perms::owner_read
							   | fs::perms::group_read
							   | fs::perms::others_read)) == fs::perms::none
						or (state[OPT_EXCLHIDDEN] == "true" and f.path().filename().string()[0] == '.'))
							return;
					
					if (isMediaFile(f.path(), state[OPT_ONLYEXT],
									state[OPT_SIZEOPGT][0],
									std::stof(state[OPT_SIZE]),
									std::stof(state[OPT_SIZETO])))
						bufferFiles.push_back(std::move(f.path()));
				}};

				if (auto dir{indexPass == 1 ? regularDirs[i] : seasonDirs[i]};
					dir.empty())
					continue;
								
				else if (auto found{records[dir.string()]}; found == nullptr) {
					try {
						if (indexPass == 1)
							for (auto& f : fs::directory_iterator(dir))
								filterChildFiles(std::move(f));
						else
							for (auto& f : fs::recursive_directory_iterator(dir))
								filterChildFiles(std::move(f));
					} catch (fs::filesystem_error& e) {
						#ifndef DEBUG
						if (state[OPT_VERBOSE] == "true")
						#endif
							std::cout << e.what() << '\n';
					}
					std::sort(bufferFiles.begin(), bufferFiles.end());
					records[dir.string()] = std::make_shared<std::vector<fs::path>>(bufferFiles);
				} else
					bufferFiles = *found;
				
				if (bufferFiles.size() > indexFile) {
					finish = false;
										
					bufferSort.push_back(std::move(bufferFiles[indexFile]));
				}
			} //end pass loop
		}
		
		if (indexFile < selectFiles.size())
			bufferSort.push_back(std::move(selectFiles[indexFile]));
		
		indexFile += 1;
		
		std::sort(bufferSort.begin(), bufferSort.end());
		for (auto& ok : bufferSort)
			putIntoPlaylist(std::move(ok));
		
		if (finish and indexFile >= selectFiles.size())
			break;
	}
	
	if (outputFile.is_open())
		outputFile.close();
	
	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true")
	#endif
		timeLapse(start, std::to_string(playlistCount) + " valid files took ");
	
	if (playlistCount == 0)
		fs::remove(outputName);
	else
		std::cout << fs::absolute(outputName).string() << '\n';
	
	return 0;
}
