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

func isEqual(const std::string& source, const std::vector<std::string>* args) -> bool
{
	for (auto& check : *args)
		if (source.size() == check.size()
			and source[1] == check[1]
			and source == check)
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
func containInts(const std::string& s, std::vector<MAXNUM>* const out)
{
	std::string buffer;
	auto make{[&]() -> void {
		if (buffer.size() > 0 and buffer.size() < 19) /// ULLONG_MAX length is 20
			out->emplace_back(std::stoull(std::move(buffer)));
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

inline
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
			result->emplace_back(buffer);
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
	if (extensions.empty())
		x = {".mp4",  ".mkv", ".mov", ".m4v",  ".mpeg", ".mpg",  ".mts", ".ts",
			 ".webm", ".flv", ".wmv", ".avi",  ".divx", ".xvid", ".dv",  ".3gp",
			 ".tmvb", ".rm",  ".mpg4", ".mqv",  ".ogm",  ".qt",
			 ".vox",  ".3gpp",".m2ts",".m1v",  ".m2v",  ".mpe"};
	else
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
		try {
		for (auto& f : fs::directory_iterator(parentPath))
			if (f.is_regular_file()
				and f.path().string().size() >= original.string().size()
				and f.path().string().substr(0, noext.size()) == noext
				and isEqual(tolower(f.path().extension().string()), &x))

				result->emplace_back(f.path());
		} catch (fs::filesystem_error& e) {
			#ifndef DEBUG
			if (state[OPT_VERBOSE] == "all")
			#endif
				std::cout << e.what() << '\n';
		}
	}
}

func getAvailableFilename(const fs::path& original, std::string prefix = " #",
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

func ascending(const fs::path& a, const fs::path& b)
{
	std::string afn { a.filename().string().substr(0,
	a.filename().string().size() - a.extension().string().size()) };

	std::string bfn { b.filename().string().substr(0,
	b.filename().string().size() - b.extension().string().size()) };

	std::vector<MAXNUM> av, bv;
	containInts(afn, &av);
	containInts(afn, &bv);

	if (av.size() == bv.size())
		for (auto i{0}; i<av.size(); ++ i) {
			if (av[i] == bv[i])
				continue;
			return av[i] < bv[i];
		}

	return afn < bfn;
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
			ignoredPaths.emplace_back(path.string());
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
					out->emplace_back(child);
				else
					ignoredPaths.emplace_back(ori.string());
			} else if (child.is_directory())
					out->emplace_back(child);
		}
	} catch (fs::filesystem_error& e) {
		#ifndef DEBUG
		if (state[OPT_VERBOSE] == "all")
		#endif
			std::cout << e.what() << '\n';
		
		ignoredPaths.emplace_back(path.string());
	}
	
	if (sorted and out->size() > 1)
		std::sort(out->begin(), out->end(), [](const fs::directory_entry& a,
											   const fs::directory_entry& b) {
			return ascending(a.path(), b.path());
		});
}

func isContainsSeasonDirs(const fs::path& path) -> bool {
	std::vector<fs::directory_entry> sortedDir;
	listDir(path, &sortedDir);
	
	auto hasDirs{false};
	
	auto isNum{true};
	std::vector<MAXNUM> lastNum;
	
	for (auto& child : sortedDir) {
		hasDirs = true;
		if (isNum) {
			std::vector<MAXNUM> iNames;
			containInts(child.path().filename().string(), &iNames);
			if (not iNames.empty()) {
				if (lastNum.empty()) {
					lastNum = std::move(iNames);
					continue;
				} else if (lastNum.size() == iNames.size()) {
					bool hasIncreased{false};
					for (auto xi{0}; xi < lastNum.size(); ++xi)
						if (hasIncreased = lastNum[xi] < iNames[xi];
							hasIncreased)
							break;
					
					if (hasIncreased)
						continue;
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
		vector->emplace_back(path);
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
						bufferNum.emplace_back(child.path());
						continue;
					} else if (lastNum.size() == iNames.size()) {
						bool hasIncreased{false};
						for (auto xi{0}; xi < lastNum.size(); ++xi)
							if (lastNum[xi] < iNames[xi]) {
								hasIncreased = true;
								break;
							}
						
						if (hasIncreased) {
							bufferNum.emplace_back(child.path());
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
		if (v <= (INT_MAX / 1000))
			result = v * 1000;
	} else if (unit == "gb") {
		if (v <= (INT_MAX / 1000000000))
			result = v * 1000000000;
	} else {
		if (v <= (INT_MAX / 1000000))
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
	bool newFull{ false };
	unsigned isLastOpt{ 0 };
	auto push{ [&](char* const arg, unsigned index, int last) {
		if (last < 0 or index - last <= 0) return;
				
		unsigned size = index - last + (newFull ? 2 : 0);
		char dst[size];

		unsigned i { 0 };
		if (newFull) {
			dst[i++] = '-';
			dst[i++] = '-';
		}
		
		for (auto inc{last}; i < size; ++i, ++inc)
			dst[i] = arg[inc];
		
		dst[i] = '\0';
		
		args->emplace_back(std::string(dst));
		
		if (newFull)
			newFull = false;
	}};
	for (int i{1}; i<argc; ++i) {
		auto arg { argv[i] };
		auto len { std::strlen(arg) };
		auto isMnemonic{ len > 1 and arg[0] == '-' and (std::isalpha(arg[1]) or arg[1] == ';') };
		auto isFull {false};
		if (not isMnemonic) {
			isFull = len > 2 and arg[0] == '-' and arg[1] == '-' and (std::isalpha(arg[2]) or arg[2] == ';');
			if (isFull)
				newFull = true;
			else
			{
				if (isLastOpt > 0 and (arg[0] == ':' or arg[0] == '=')) {
					if (isLastOpt == 1)
						isMnemonic = true;
					else
						isFull = true;
				} else {
					args->emplace_back(arg);
					continue;
				}
			}
		}
		
		
			
		unsigned index = isFull and isLastOpt != 2 ? 1 : 0;
		int last{ -1 };
		while (++index < len) {
			if (last < 0 and std::isalpha(arg[index])) {
				if (isMnemonic) {
					isLastOpt = 1;
					std::string new_s {'-'};
					new_s += arg[index];
					args->emplace_back(new_s);
				} else if (isFull) {
					isLastOpt = 2;
					last = index;
				}
			} else {
				if (arg[index] == '=' or arg[index] == ':') {
					if (isFull) {
						isLastOpt = 2;
						push(arg, index, last);
						last = -1;
					} else {
						isLastOpt = 1;
						last = index + 1;
					}
				} else if (arg[index] == ';') {
					isLastOpt = 1;
					push(arg, index, last);
					last = -1;
					if (isFull) {
						isLastOpt = 2;
						newFull = true;
					}
				} else if (isFull and last > 0 and (arg[index] == '<' or arg[index] == '>')) {
					isLastOpt = 0;
					push(arg, index, last);
					last = index;
				} else if (last < 0)
					last = index;
			}
		}
		if (last > 0)
			push(arg, index, last);
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
-V, --verbose [all]             Display playlist content. Define as 'all' will show fail messages.\n\
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
or ':' and can be separated by ';' after assignment. For the example:\n\
  tvplaylist -hOVvc=async;xs<1.3gb;r=mp4;f:My-playlist.m3u8\n\n\
Thats it, -h -O -V -v -c are joined, and -c has assignment operator '=' instead of\
 using separator [SPACE].\n\
Also -x -s are joined, and -x is continuing with ';' after option assignment \
'=async' and -s has remove [SPACE] separator for operator '<' and value '1.3gb'.\n\
Redefinition of option means, it will use the last option. For the example, \
this example will not use 'thread' execution at all':\n\
  tvplaylist -c=thread /usr/local/videos -c=none /Users/Shared/Videos\n\
Note, you cannot join mnemonic option with full option, for the example:\n\
  tvplaylist -ch;only-ext=m43;version\t\tWONT WORK\n\
Instead try to separate mnemonic and full option, like this:\n\
  tvplaylist -ch --only-ext=mp3;version\n"
;

int main(int argc, char *argv[]) {
	std::vector<fs::path> bufferDirs;

	state[OPT_SIZEOPGT] = '\0';
	state[OPT_SIZE] 	= "0";
	state[OPT_SIZETO] 	= "0";
	state[OPT_EXECUTION]= OPT_ASYNC;
	
	std::vector<std::string> args;
	expandArgs(argc, argv, &args);
	
	std::vector<std::string_view> invalidArgs;
	
	for (int i{0}; i<args.size(); ++i) {
		if (auto isMatch{ [&](const char* with,
							  char mnemonic,
							  bool writeBoolean=false) {
			auto result { (args[i].length() > 3
					and args[i][0] == '-'
					and args[i][1] == '-'
					and args[i].substr(2) == with)
				or (args[i].length() == 2
					and args[i][0] == '-'
					and args[i][1] == mnemonic) };
			if (result and writeBoolean)
				state[with] = "true";
			return result;
		} }; isMatch(OPT_HELP, 'h') or isMatch(OPT_VERSION, 'v'))
			{
				std::cout <<
				fs::path(argv[0]).filename().string() << ' '
				<< VERSION << '\n';
				
				if (isMatch(OPT_HELP, 'h'))
					std::cout << HELP << '\n';
				
				if (i + 1 == args.size())
					return 0;
			}
			else if (isMatch(OPT_OVERWRITE, 	'O', true));
			else if (isMatch(OPT_BENCHMARK, 	'b', true));
			else if (isMatch(OPT_SKIPSUBTITLE, 	'x', true));
			else if (isMatch(OPT_EXCLHIDDEN, 	'n', true));
			else if (isMatch(OPT_VERBOSE, 		'V', true)) {
				if (i + 1 < args.size() and args[i + 1] == "all")
				{
					i++;
					state[OPT_VERBOSE] = args[i];
				}
			}
			else if (isMatch(OPT_EXECUTION, 	'c')) {
				if (i + 1 < args.size()) {
					i++;
					if (args[i] == OPT_ASYNC or args[i] == OPT_THREAD)
						state[OPT_EXECUTION] = args[i];
					else
						state[OPT_EXECUTION] = "";
				} else
					std::cout << "Expecting 'thread', 'async', or 'none' after \""
					<< args[i] << "\" option.\n";
			} else if (isMatch(OPT_ONLYEXT, 	'e')) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1]] = args[i];
				} else
					std::cout << "Expecting extension after \""
					<< args[i] << "\" option (eg: \"mp4, mkv\").\n";
			}
			else if (isMatch(OPT_FIXFILENAME, 	'f')) {
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
			else if (isMatch(OPT_OUTDIR, 		'd')) {
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
			else if (isMatch(OPT_SIZE, 		's')) {
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
		} else if (fs::is_directory(args[i]))
			insertTo(&bufferDirs, std::move(fs::path(args[i])));
		
		else if (fs::is_regular_file(std::move(fs::path(args[i])))) {
			if (isMediaFile(fs::absolute(args[i]), state[OPT_ONLYEXT],
							   state[OPT_SIZEOPGT][0],
							   std::stof(state[OPT_SIZE]),
							   std::stof(state[OPT_SIZETO])))
				insertTo(&selectFiles, std::move(fs::absolute(args[i])));
		} else
			invalidArgs.emplace_back(args[i]);
	}
	
	if (not invalidArgs.empty()) {
		std::cout << "\nWhat " << (invalidArgs.size() > 1 ? "are these" : "is this") << "? :\n";
		for (auto i{ 0 }; i<invalidArgs.size(); ++i) {
			std::cout << '"' << invalidArgs[i] << '"'
				<< (i + 1 < invalidArgs.size() ? ", " : "");
		}
		std::cout << "\nFor more information, please try to type \""
			<< fs::path(argv[0]).filename().string() << " --help\"\n\n";
	}

	if (bufferDirs.empty() and selectFiles.empty())
		insertTo(&bufferDirs, fs::current_path());
	
	const auto inputDirsCount = bufferDirs.size();
	fs::path inputDirs[inputDirsCount];
	#ifndef DEBUG
	if (not invalidArgs.empty() or state[OPT_VERBOSE] == "true" or state[OPT_BENCHMARK] == "true")
	#endif
		std::copy(bufferDirs.begin(), bufferDirs.end(), inputDirs);

	while (bufferDirs.size() == 1) {
		state[OPT_OUTDIR] = fs::absolute(bufferDirs[0]).string();
		
		insertTo(&regularDirs, std::move(bufferDirs[0]));/// Assume single input dir is regularDir
		bufferDirs.clear();
		
		std::vector<fs::directory_entry> sortedDirs;
		listDir(fs::path(state[OPT_OUTDIR]), &sortedDirs);
		
		state[OPT_OUTDIR] += fs::path::preferred_separator;
		
		for (auto& child : sortedDirs)
			insertTo(&bufferDirs, std::move(child.path()));
	}

	#ifndef DEBUG
	if (not invalidArgs.empty() or state[OPT_VERBOSE] == "true" or state[OPT_BENCHMARK] == "true")
	#endif
	{
	std::cout
		<< OPT_EXECUTION << "\t\t: " << state[OPT_EXECUTION] << '\n'
		<< OPT_VERBOSE << "\t\t\t: " << state[OPT_VERBOSE] << '\n'
		<< OPT_BENCHMARK << "\t\t: " << state[OPT_BENCHMARK] << '\n'
		<< OPT_OVERWRITE << "\t\t: " << state[OPT_OVERWRITE] << '\n'
		<< OPT_FIXFILENAME << "\t\t: " << state[OPT_FIXFILENAME] << '\n'
		<< "Current Directory\t: " << fs::current_path().string() << '\n'
		<< OPT_OUTDIR << "\t\t\t: " << state[OPT_OUTDIR] << '\n'
		<< OPT_SKIPSUBTITLE << "\t\t: " << state[OPT_SKIPSUBTITLE] << '\n'
		<< OPT_ONLYEXT << "\t\t: " << state[OPT_ONLYEXT] << '\n'
		<< OPT_SIZE << "\t\t\t: " << (state[OPT_SIZETO] == "0" ? state[OPT_SIZEOPGT] : "")
			<< (state[OPT_SIZEOPGT].empty() ? "" : " ")
			<< state[OPT_SIZE]
			<< (state[OPT_SIZETO] != "0" ? ".." : " bytes")
		<< (state[OPT_SIZETO] == "0" ? "" : state[OPT_SIZETO] + " bytes") << '\n'
		<< "Inputs\t\t\t: ";
		for (auto i{0}; i<inputDirsCount + selectFiles.size(); ++i) {
			if (i < inputDirsCount) {
				std::cout << inputDirs[i];
			} else {
				std::cout << selectFiles[i - inputDirsCount];
			}
			
			std::cout  << (i < (inputDirsCount + selectFiles.size()) - 1 ? ", " : "");
		}
	std::cout << "\n\n";
	}
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

	if (state[OPT_EXECUTION] == OPT_THREAD) {
		for (auto& t : threads)
			t.join();
		threads.clear();
	} else if (state[OPT_EXECUTION] == OPT_ASYNC) {
		for (auto& a : asyncs)
			a.wait();
		asyncs.clear();
	}

		
	const auto regularDirSize{::regularDirs.size()};
	const auto seasonDirSize{::seasonDirs.size()};
	
	const auto maxDirSize{std::max(regularDirSize, seasonDirSize)};

	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true")
	#endif
	{
		if (inputDirsCount > 0)
			timeLapse(start, std::to_string(regularDirSize + seasonDirSize)
					  + " valid dirs took ", true);
	}

	/// Convert std::set to classic array, to enable call by index subscript.
	fs::path regularDirs[regularDirSize];
	std::move(::regularDirs.begin(), ::regularDirs.end(), regularDirs);

	fs::path seasonDirs[seasonDirSize];
	std::move(::seasonDirs.begin(), ::seasonDirs.end(), seasonDirs);
	
	if (selectFiles.size() > 1)
		std::sort(selectFiles.begin(), selectFiles.end(), ascending);
	
	std::map<std::string, std::shared_ptr<std::vector<fs::path>>> records;
	
	fs::path outputName{ state[OPT_OUTDIR]
							+ (state[OPT_FIXFILENAME] != ""
							   ? state[OPT_FIXFILENAME]
							   : "playlist_from_" + (inputDirsCount == 0
								 ? std::to_string(selectFiles.size()) + "_file"
								 : (inputDirsCount == 1
									 ? fs::path(state[OPT_OUTDIR].substr(0,
										state[OPT_OUTDIR].size() - 1)).filename().string()
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
	
	auto filterChildFiles{ [&records](const std::string& dir, bool recurive=false) {
		std::vector<fs::path> bufferFiles;
		auto filter{ [&](const fs::directory_entry& f) {
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
				bufferFiles.emplace_back(std::move(f.path()));
		}};
		try {
			if (recurive)
				for (auto& f : fs::recursive_directory_iterator(dir))
					filter(f);
			else
				for (auto& f : fs::directory_iterator(dir))
					filter(f);
			
		} catch (fs::filesystem_error& e) {
			#ifndef DEBUG
			if (state[OPT_VERBOSE] == "all")
			#endif
				std::cout << e.what() << '\n';
		}
		if (bufferFiles.empty())
			return;
		if (bufferFiles.size() > 1)
			std::sort(bufferFiles.begin(), bufferFiles.end(), ascending);
		#ifdef DEBUG
			std::cout << "+records[:" << dir << "] " << bufferFiles.size() << " files\n";
		#endif
		records.emplace(std::make_pair(std::move(dir),
									   std::make_shared<std::vector<fs::path>>(std::move(bufferFiles))
									   ));
	}};

	for (auto i{0}; i<maxDirSize; ++i)
		for (auto& x : {1, 2})
			if (i < (x == 1 ? regularDirSize : seasonDirSize) )
				if (auto dir { (x == 1 ? regularDirs[i] : seasonDirs[i]).string() };
					not dir.empty()) {
					if (state[OPT_EXECUTION] == OPT_THREAD)
						threads.emplace_back([&, dir]() {
							filterChildFiles(dir, x == 2);
						});
					else if (state[OPT_EXECUTION] == OPT_ASYNC)
						asyncs.emplace_back(std::async(std::launch::async, [&, dir]() {
							filterChildFiles(dir, x == 2);
						}));
					else
						filterChildFiles(dir, x == 2);
				}
	
	if (state[OPT_EXECUTION] == OPT_THREAD) {
		for (auto& t : threads)
			t.join();
	} else if (state[OPT_EXECUTION] == OPT_ASYNC) {
		for (auto& a : asyncs)
			a.wait();
	}

	
	while (true) {
		auto finish{true};
		std::vector<fs::path> bufferSort;
		
		for (auto i{0}; i < maxDirSize; ++i)
			for (auto& indexPass : {1, 2})
				///pass 1 for regularDirs, pass 2 for seasonDirs
			{
				if ((indexPass == 1 and i >= regularDirSize)
					or (indexPass == 2 and i >= seasonDirSize))
					continue;
				
				if (const auto dir{indexPass == 1 ? regularDirs[i] : seasonDirs[i]};
					dir.empty())
					continue;

				else if (auto found { records[dir.string()] }; found)
					if (indexFile < found->size()) {
						finish = false;

						bufferSort.emplace_back((*found)[indexFile]);
					}
			} //end pass loop
		
		if (indexFile < selectFiles.size())
			bufferSort.emplace_back(std::move(selectFiles[indexFile]));
		
		indexFile += 1;
		
		if (bufferSort.size() > 1)
			std::sort(bufferSort.begin(), bufferSort.end(), ascending);
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
