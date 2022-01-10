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
#include <regex>
#include <clocale>
#include <sys/stat.h>

std::map<std::string, std::string> state;
constexpr auto OPT_HELP 			{"help"};					// h
constexpr auto OPT_VERSION 			{"version"};				// v
constexpr auto OPT_VERBOSE 			{"verbose"};				// V
constexpr auto OPT_BENCHMARK 		{"benchmark"};				// b
constexpr auto OPT_OVERWRITE 		{"overwrite"};				// O
constexpr auto OPT_SKIPSUBTITLE 	{"skip-subtitle"};			// x
constexpr auto OPT_OUTDIR 			{"out-dir"};				// d
constexpr auto OPT_EXECUTION		{"execution"};				// c
constexpr auto OPT_THREAD			{"thread"};
constexpr auto OPT_ASYNC			{"async"};
constexpr auto OPT_EXCLHIDDEN		{"exclude-hidden"};			// n
constexpr auto OPT_REGEXSYNTAX		{"regex-syntax"};			// X

constexpr auto OPT_FIXFILENAME 		{"out-filename"};			// f
constexpr auto OPT_NOOUTPUTFILE 	{"no-output-file"};			// F

constexpr auto OPT_SIZE				{"size"};					// s
constexpr auto OPT_SIZEOPGT			{"size_op"};
constexpr auto OPT_EXCLSIZE			{"exclude-size"};			// S
constexpr auto OPT_EXCLSIZEOPGT		{"exclude-size_op"};

constexpr auto OPT_EXT 				{"ext"};					// e
constexpr auto OPT_EXCLEXT 			{"exclude-ext"};			// E

constexpr auto OPT_FIND				{"find"};					// i
constexpr auto OPT_EXCLFIND			{"exclude-find"};			// I

constexpr auto OPT_REGEX			{"regex"};					// r
constexpr auto OPT_EXCLREGEX		{"exclude-regex"};			// R

constexpr auto OPT_DCREATED			{"created"};				// t
constexpr auto OPT_DMODIFIED		{"modified"};				// m
constexpr auto OPT_DACCESSED		{"accessed"};				// a
constexpr auto OPT_DCHANGED			{"changed"};				// g

constexpr auto OPT_DEXCLCREATED			{"exclude-created"};	// T
constexpr auto OPT_DEXCLMODIFIED		{"exclude-modified"};	// M
constexpr auto OPT_DEXCLACCESSED		{"exclude-accessed"};	// A
constexpr auto OPT_DEXCLCHANGED			{"exclude-changed"};	// G

constexpr auto OPT_DEBUG			{"debug"};

//#include <format>

#if MAKE_LIB
#include "libtvplaylist.h"
#endif

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

func transformWhiteSpace(std::string s) -> std::string
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return std::isspace(c) ? ' ' : c;
	});
	return s;
}

func groupNumber(std::string number) -> std::string
{
	long long i = number.find_last_of('.');
	if (i == std::string::npos) {
		i = number.find_last_of('e');
		if (i == std::string::npos)
			i = number.size();
	}
	auto a{ number.size() > 1 and not std::isdigit(number[0]) ? 1
			: ( number.size() > 2 and std::tolower(number[1]) == 'x' ? 2
			   : 0) };
	while ( i - 3 > a ) {
		i-=3;
		number.insert(i, 1, ',');
	}
	return number;
}

func trim(const std::string& s) -> std::string
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

func isInt(const std::string& s) -> bool
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

struct Date
{
	unsigned short year;
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
	unsigned short ms;
	
	friend bool operator < (const Date& l, const Date& r) {
		auto ull { Date::get_ull(l, r) };
		return ull.first < ull.second;
	}
	
	friend bool operator > (const Date& l, const Date& r) {
		auto ull { Date::get_ull(l, r) };
		return ull.first > ull.second;
	}
	
	friend bool operator <= (const Date& l, const Date& r) {
		auto ull { Date::get_ull(l, r) };
		return ull.first <= ull.second;
	}
	
	friend bool operator >= (const Date& l, const Date& r) {
		auto ull { Date::get_ull(l, r) };
		return ull.first >= ull.second;
	}
	
	friend bool operator == (const Date& l, const Date& r) {
		#define is_equal(x, y) (x != 0 and y != 0 ? x == y : true)
		return is_equal(l.year, r.year)
			and is_equal(l.month, r.month)
			and is_equal(l.day, r.day)
			and is_equal(l.hour, r.hour)
			and is_equal(l.minute, r.minute)
			and is_equal(l.second, r.second)
			and is_equal(l.ms, r.ms)
			;
		#undef is_equal
	}
	
	func time_t(const Date* const d = nullptr) {
		std::tm tm{};
		tm.tm_year = (d ? d : this)->year-1900;
		tm.tm_mon = (d ? d : this)->month-1; // February
		tm.tm_mday = (d ? d : this)->day;
		tm.tm_hour = (d ? d : this)->hour;
		tm.tm_min = (d ? d : this)->minute;
		tm.tm_sec = (d ? d : this)->second;
		return std::mktime(&tm); //std::time(nullptr);
	}
	
	func string(const char* const format = nullptr) -> std::string {
		std::string s;
		if (format) {
			auto t { time_t() };
			char mbstr[100];
			if (std::strftime(mbstr, sizeof(mbstr), format, std::localtime(&t)))
				s = mbstr;
		} else {
			s.append(year > 0 ? std::to_string(year) : "?");
			s.append("/" + (month > 0 ? std::to_string(month) : "?"));
			s.append("/" + (day > 0 ? std::to_string(day) : "?"));
			
			s.append(" " + std::to_string(hour));
			s.append(":" + std::to_string(minute));
			s.append(":" + std::to_string(second));
		}
		return s;
	}
private:
	static
	func get_ull(const Date& l, const Date& r)
					-> std::pair<unsigned long long, unsigned long long>{
		std::string sleft, sright;
		auto go{[&](const unsigned short& x, const unsigned short& y) {
			if (x == 0 or y == 0) return;
			
			auto sx { std::to_string(x) };
			if (sx.size() < 2) sx.insert(0, 1, '0');
			sleft.append(sx);
			auto sy { std::to_string(y) };
			if (sy.size() < 2) sy.insert(0, 1, '0');
			sright.append(sy);
		}};
		
		go(l.year, r.year);
		go(l.month, r.month);
		go(l.day, r.day);
		go(l.hour, r.hour);
		go(l.minute, r.minute);
		go(l.second, r.second);
		go(l.ms, r.ms);
		
		return std::make_pair(std::stoull(sleft), std::stoull(sright));
	}
	func fold() const -> unsigned long long {
		auto Y = std::to_string(year);
		auto m = std::to_string(month);
		if (m.size() not_eq 2) m.insert(0, 1, '0');
		auto d = std::to_string(day);
		if (d.size() not_eq 2) d.insert(0, 1, '0');
		auto H = std::to_string(hour);
		if (H.size() not_eq 2) H.insert(0, 1, '0');
		auto M = std::to_string(minute);
		if (M.size() not_eq 2) M.insert(0, 1, '0');
		auto S = std::to_string(second);
		if (S.size() not_eq 2) S.insert(0, 1, '0');
		
		return std::stoull(Y + m + d + H + M + S);
	}
	
public:
	Date(const long* tl) {
		char a[5];
		unsigned short * property[] = {&year, &month, &day, &hour, &minute, &second};
		for (unsigned short i{ 0 }; auto& f : {"%Y", "%m", "%d", "%H", "%M", "%S"}) {
			std::strftime(a, sizeof(a), f, std::localtime(tl));
			*property[i++] = std::stoi(a);
		}
	}
		
	Date(const std::string& s): year{0}, month{0}, day{0},
								hour{0}, minute{0}, second{0}, ms{0}
	{
										
		auto isDigit{[](const std::string& s) -> bool
		{
			for (auto i{0}; i<s.size(); ++i)
				if (not isdigit(s[i]))
				return false;
			return s not_eq "";
		}};
		std::vector<unsigned short> date;
		std::vector<short> time;
		std::vector<std::string> others;
		char last = '\0';
		for (unsigned i{ 0 }, k{ 0 }; i<s.size(); ++i)
			if (not std::isdigit(s[i])) {
				if (s[i] == ':') {
					auto get{ s.substr(k, i - k) };
					if (isDigit(get))
						time.emplace_back(std::stoi(get));
					else
						others.emplace_back(get);
				} else if (std::ispunct(s[i])) {
					auto get{ s.substr(k, i - k) };
					if (isDigit(get))
						date.emplace_back(std::stoi(get));
					else
						others.emplace_back(get);
				} else if (std::isspace(s[i])) {
					if (k != i) {
						auto get{ tolower(s.substr(k, i - k)) };
						auto isNumber{ isDigit(get) };
						if (std::isalpha(last)) {
							if (get == "am" or get == "pm")
								time.emplace_back(get == "am" ? -1 : -2);
							else
								others.emplace_back(get);
						} else if (last == ':' and isNumber)
							time.emplace_back(std::stoi(get));
						else if ((last == '\0' or std::ispunct(last)) and isNumber)
							date.emplace_back(std::stoi(get));
						else
							others.emplace_back(get);
					}
					last = '\0';
					k = i + 1;
					continue;
				} else {
					if (i == s.size() - 1) {
						auto get = tolower(s.substr(k));
						if (get == "am" or get == "pm")
							time.emplace_back(get == "am" ? -1 : -2);
						else
							others.emplace_back(get);
					}
					if (last == '\0')
						last = s[i];
					continue;
				}
				
				last = s[i];
				k = i + 1;
				
			} else if (i == s.size() - 1) {
				auto get{ tolower(s.substr(k)) };
				auto isNumber{ isDigit(get) };
				if (last == ':' and isNumber)
					time.emplace_back(std::stoi(get));
				else if ((std::ispunct(last) or last == '\0') and isNumber)
					date.emplace_back(std::stoi(get));
				else
					others.emplace_back(get);
			}
			
			
			
			
		auto weekDay = -1;
		for (auto found{ 0 };
			auto& s : others)
		{
			if (weekDay == -1) {
				for (auto i{ 0 }; auto& m : {
					"sun", "mon", "tue", "wed", "thu", "fri", "sat", // en_US
					"min", "sen", "sel", "rab", "kam", "jum", "sab", // id_ID
					"dim", "lun", "mar", "mer", "jeu", "ven", "sam", // fr_FR
					"sön", "mån", "tis", "ons", "tor", "fre", "lör", // sv_SE
					"dom", "lun", "mar", "mié", "jue", "vie", "sáb", // es_ES
					"so", "mo", "di", "mi", "do", "fr", "sa",		 // de_DE
					"вс", "пн", "вт", "ср", "чт", "пт", "сб",		 // ru_RU
					"日", "一", "二", "三", "四", "五", "六",			 // zh_CN
					"日", "月", "火", "水", "木", "金", "土",  		 // ja_JP
				}) {
					if (s.starts_with(m))
					{
						weekDay = (i % 7) + 1;
						break;
					}
					i++;
				}
				
				if (weekDay != -1)
					continue;
			}
			
			for (auto i{ 0 };
				auto& m : {
					"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec", // en_US
					"jan", "feb", "mar", "apr", "mei", "jun", "jul", "agu", "sep", "okt", "nov", "des", // id_ID
					"jan", "feb", "mär", "apr", "mai", "jun", "jul", "aug", "sep", "okt", "nov", "dez", // de_DE
					"ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic", // es_ES
					"jan", "feb", "mar", "apr", "maj", "jun", "jul", "aug", "sep", "okt", "nov", "dec", // sv_SE
					"jan", "fév", "mar", "avr", "mai", "jui", "jul", "aoû", "sep", "oct", "nov", "déc", // fr_FR
					"янв", "фев", "мар", "апр", "май", "июн", "июл", "авг", "сен", "окт", "ноя", "дек", // ru_RU
				}) {
				if (s.starts_with(m))
				{
					found = (i % 12) + 1;
					break;
				}
				i++;
			}
			
			if (found > 0) {
				if (month == 0)
					month = found;
				else {
					if (day == 0) {
						day = month;
						month = found;
					} else
						month = found; // Just replace month, maybe previous month was day
				}
				continue;
			} else
				std::cout << "Unknown other: " << s << '\n';
		}
										
		for (auto& i : date)
			if (i > 35)
				year = i;
			else if (i <= 12) {
				if (month == 0 and day == 0)
					month = i;
				else {
					if (month > 0)
						day = i;
					else if (year == 0)
						year = i;
					else if (day == 0)
						day = i;
					else
						std::cout << "Unknown date<12: " << i << '\n';
				}
			} else if (i > 12) {
				if (day == 0)
					day = i;
				else
					if (month > 0) {
						day = month;
						if (year == 0)
							year = i;
						else
							std::cout << "Unknown date>12: " << i << '\n';
					}
			}
			
			
		if (weekDay > -1) {
			const std::chrono::weekday friday{ static_cast<unsigned int> (weekDay) };
			
			auto currentYear = 0;
			auto currentMonth = 0;
			if (year == 0) {
				const std::chrono::year_month_day ymd{floor<std::chrono::days>(std::chrono::system_clock::now())};
				currentYear = int(ymd.year());
				currentMonth = unsigned(ymd.month());
			}
			
			bool found{ false };
			for (auto& Y : {currentYear, 1970})
			for (int y {year > 0 ? year : Y}; y <= year > 0 ? year : Y; ++y) {
				const std::chrono::year cur_year{y};
				for (int cur_month{(Y != 1970 and year > 0) and month > 0 ? month
					: (Y == 1970 ? 1 : currentMonth)};
					 cur_month != (Y == 1970 ? 13 : cur_month + 1); ++cur_month) {
					const std::chrono::year_month_day 	ymd 		{ cur_year / cur_month / 13 };
					const std::chrono::weekday 			cur_weekday	{ std::chrono::sys_days{ ymd } };
					if (cur_weekday == friday) {
						if (year == 0) year = int(ymd.year());
						if (month == 0) month = unsigned(ymd.month());
						if (day == 0) day = unsigned(ymd.day());
						found = true;
						break;
					}
				}
				
				if (found)
					break;
			}
		}
			
		unsigned short* property[] = {&hour, &minute, &second, &ms };
		auto hasPM{ false };
		for (auto i { 0 }; auto& t : time)
			if (t < 0) {
				hasPM = t == -2;
			} else if (i < sizeof(property)/sizeof(property[0]))
				*property[i++] = t;
		if (hasPM)
			hour += 12;
		
		
#if DEBUG
		std::cout << "input: \"" << s << "\" -> " <<
		year << '/' << month << '/' << day << ' ' << hour << ':' << minute << ':' << second << ':' << ms
		<< '\n';
#endif

	}

	func isValid() -> bool {
		return (0 > year or month > 0 or day > 0 or hour <= 24 or minute <= 60 or second <= 60);
	}

	func getWeekDays(const char* const locale, const char* result[7]) {
		for (auto i{9}, k{0}; i<16; ++i, ++k) {
			std::tm tm{};
			tm.tm_year = 2020-1900; // 2020
			tm.tm_mon = 2-1; // February
			tm.tm_mday = i; // 15th
			//		tm.tm_hour = 10;
			//		tm.tm_min = 15;
			//		tm.tm_isdst = 0; // Not daylight saving
			std::time_t t = std::mktime(&tm); //std::time(nullptr);
			
			char mbstr[3];
			std::setlocale(LC_ALL, locale ? locale : std::locale().name().c_str());
			if (std::strftime(mbstr, sizeof(mbstr), "%a", std::localtime(&t)))
				result[k] = mbstr;
		}
	}

};


std::vector<std::string> EXCLUDE_EXT;
std::vector<std::string> DEFAULT_EXT {
	".mp4",  ".mkv", ".mov", ".m4v",  ".mpeg", ".mpg",  ".mts", ".ts",
	".webm", ".flv", ".wmv", ".avi",  ".divx", ".xvid", ".dv",  ".3gp",
	".tmvb", ".rm",  ".mpg4",".mqv",  ".ogm",  ".qt",
	".vox",  ".3gpp",".m2ts",".m1v",  ".m2v",  ".mpe",
	".mp3",	 ".m4a", ".aac", ".wav",  ".wma",  ".flac", ".ape", ".aiff"
	".jpg",  ".jpeg",".png", ".gif",  ".bmp"
};
bool EXCLUDE_EXT_REPLACED = false;
bool DEFAULT_EXT_REPLACED = false;

std::vector<std::regex> listRegex, listExclRegex;
std::vector<std::string> listFind, listExclFind;
std::vector<std::pair<char, Date>> 	listDCreated, listDModified, listDAccessed, listDChanged,
	listDExclCreated, listDExclModified, listDExclAccessed, listDExclChanged;
std::vector<std::pair<Date, Date>> listDCreatedR, listDModifiedR, listDAccessedR, listDChangedR,
	listDExclCreatedR, listDExclModifiedR, listDExclAccessedR, listDExclChangedR;
std::vector<std::pair<std::uintmax_t, std::uintmax_t>> listSize, listExclSize;

func isValidFile(const fs::path& path) -> bool
{
	if (not fs::exists(path))
		return false;

	if (not state[OPT_EXCLEXT].empty() and state[OPT_EXCLEXT] not_eq "*")
		if (not EXCLUDE_EXT_REPLACED) {
			parseCommaDelimited(tolower(state[OPT_EXCLEXT]), &EXCLUDE_EXT);
			EXCLUDE_EXT_REPLACED = true;
		}
	if (not state[OPT_EXT].empty() and state[OPT_EXT] not_eq "*")
		if (not DEFAULT_EXT_REPLACED) {
			parseCommaDelimited(tolower(state[OPT_EXT]), &DEFAULT_EXT);
			DEFAULT_EXT_REPLACED = true;
		}
	
	
	
	fs::path tmp{ path };
	if (fs::is_symlink(tmp)) { //TODO: is_symlink() cannot detect macOS alias file!
		tmp = fs::read_symlink(path);
		
		if (not fs::exists(tmp) or not fs::is_regular_file(tmp))
			return false;
	}
	
	if (state[OPT_EXT] not_eq "*"
		and not isEqual(tolower(tmp.extension().string()), &DEFAULT_EXT))
		return false;
	
	if (state[OPT_EXCLEXT] not_eq "*"
		and not state[OPT_EXCLEXT].empty())
		if (isEqual(tolower(tmp.extension().string()), &EXCLUDE_EXT))
			return false;
	
	if (state[OPT_DCREATED] 	not_eq ""
		or state[OPT_DACCESSED] not_eq ""
		or state[OPT_DMODIFIED] not_eq ""
		or state[OPT_DCHANGED] 	not_eq ""
		or state[OPT_DEXCLCREATED] 	not_eq ""
		or state[OPT_DEXCLACCESSED] not_eq ""
		or state[OPT_DEXCLMODIFIED] not_eq ""
		or state[OPT_DEXCLCHANGED] 	not_eq ""
		)
	{
		struct stat filestat;
		stat(tmp.string().c_str(), &filestat);
		
		std::string st[2][4] = {
			{ OPT_DCREATED, OPT_DACCESSED, OPT_DMODIFIED, OPT_DCHANGED },
			{ OPT_DEXCLCREATED, OPT_DEXCLACCESSED, OPT_DEXCLMODIFIED, OPT_DEXCLCHANGED },
		};

		Date _ft[4] = { 				Date(&filestat.st_birthtime),
										Date(&filestat.st_atime),
										Date(&filestat.st_mtime),
										Date(&filestat.st_ctime) };
		Date ft[2][4] = {
			{ _ft[0], _ft[1], _ft[2], _ft[3]},
			{ _ft[0], _ft[1], _ft[2], _ft[3]}
		};
			
		std::vector<std::pair<char, Date>>* ot[2][4] = {
			{ &listDCreated, &listDAccessed, &listDModified, &listDChanged },
			{ &listDExclCreated, &listDExclAccessed, &listDExclModified, &listDExclChanged },
		};

		
		std::vector<std::pair<Date, Date>>* rt[2][4] = {
			{ &listDCreatedR, &listDAccessedR, &listDModifiedR, &listDChangedR },
			{ &listDExclCreatedR, &listDExclAccessedR, &listDExclModifiedR, &listDExclChangedR },
		};

		bool found[2]{ false, false };
		for (auto& z : { 0, 1 })
		for (auto& i : { 0, 1, 2, 3 })
		{
			if (state[st[z][i]] not_eq "") {
				for (auto& r : *rt[z][i])
					if (ft[z][i] >= r.first and ft[z][i] <= r.second ) {
						found[z] = true;
						break;
					}
				for (auto& t : *ot[z][i])
					if (t.first == '=') {
						if (ft[z][i] == t.second) {
							found[z] = true;
							break;
						}
					}
					else if (t.first == '<') {
						if (ft[z][i] < t.second) {
							found[z] = true;
							break;
						}
					}
					else if (t.first == '>') {
						if (ft[z][i] > t.second) {
							found[z] = true;
							break;
						}
					}
			} // end st[] -> state[OPT_D?????]
		}
		
		if (not found[0] or found[1])
			return false;
	}
	
	
	if (bool found{ false }; not state[OPT_REGEX].empty()) {
		for (auto filename{ excludeExtension(tmp.filename()) };
			 auto& regex : listRegex)
			if (std::regex_search(filename, regex)) {
				found = true;
				break;
			}
		if (not found)
			return false;
	}
	
	if (not state[OPT_EXCLREGEX].empty())
		for (auto filename{ excludeExtension(tmp.filename()) };
			 auto& regex : listExclRegex)
			if (std::regex_search(filename, regex))
				return false;
	
	if (bool found{ false }; not state[OPT_FIND].empty()) {
		for (auto filename{ excludeExtension(tmp.filename()) };
			 auto& keyword : listFind)
			if (filename.find(keyword) != std::string::npos) {
				found = true;
				break;
			}
		if (not found)
			return false;
	}

	if (not state[OPT_EXCLFIND].empty())
		for (auto filename{ excludeExtension(tmp.filename()) };
			 auto& keyword : listExclFind)
			if (filename.find(keyword) != std::string::npos)
				return false;

	if (bool found{ false };
		not listSize.empty()
		or (state[OPT_SIZEOPGT][0] != '\0' and state[OPT_SIZE] != "0"))
	{
		const std::uintmax_t fileSize{ fs::file_size(tmp) };
		
		for (auto& range : listSize)
			if (fileSize > range.first and fileSize < range.second) {
				found = true;
				break;
			}
		
		if (not found)
			found = state[OPT_SIZEOPGT][0] == '>'
						? fileSize > std::stoul(state[OPT_SIZE])
						: fileSize < std::stoul(state[OPT_SIZE]);
		if (not found)
			return false;
	}
	
	if (bool found{ false };
		not listExclSize.empty()
		or (state[OPT_EXCLSIZEOPGT][0] != '\0' and state[OPT_EXCLSIZE] != "0"))
	{
		const std::uintmax_t fileSize{ fs::file_size(tmp) };
		
		for (auto& range : listExclSize)
			if (fileSize > range.first and fileSize < range.second)
				return false;
		
		if (not found)
			found = state[OPT_EXCLSIZEOPGT][0] == '>'
						? fileSize > std::stoul(state[OPT_EXCLSIZE])
						: fileSize < std::stoul(state[OPT_EXCLSIZE]);
		if (found)
			return false;
	}
	
//	if (not state[OPT_DCREATED].empty()) {
//		struct stat ftime;
//		stat(tmp.string().c_str(), &ftime);
//	}
	
	return /*fs::is_regular_file(tmp) and*/ true;
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

func getAvailableFilename(const fs::path& original, const std::string& prefix = " #",
						  const std::string& suffix = "") -> std::string
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
	containInts(bfn, &bv);

	if (av.size() == bv.size())
		for (auto i{0}; i<av.size(); ++ i) {
			if (av[i] == bv[i])
				continue;
			return av[i] < bv[i];
		}

	return afn < bfn;
}

func sortFiles(std::vector<fs::path>* const selectFiles)
{
	if (selectFiles->size() > 1) {
		std::map<std::string, std::shared_ptr<std::vector<fs::path>>> selectFilesDirs;
		std::sort(selectFiles->begin(), selectFiles->end());
		
		for (auto& f : *selectFiles) {
			auto parent { f.parent_path().string() };
			if (auto ptr { selectFilesDirs[parent] };
				ptr) {
				ptr->emplace_back(f);
			} else {
				std::vector<fs::path> flist;
				flist.emplace_back(f);
				selectFilesDirs[parent] = std::make_shared<std::vector<fs::path>>(std::move(flist));
			}
		}
		
		selectFiles->clear();
		for (auto& m : selectFilesDirs) {
			std::vector<fs::path> files {*(m.second)};
			std::sort(files.begin(), files.end(), ascending);
			for (auto& f : files) {
				selectFiles->emplace_back(f);
			}
		}
	}
}

func recursiveDirectory(const char* const dir,
						std::vector<std::string>* const out) -> void
{
	std::vector<fs::path> tmp;
	try {
		for (auto& d : fs::directory_iterator(dir))
			if (d.is_directory())
				tmp.emplace_back(d.path().string());
	} catch (fs::filesystem_error& e) {
		#ifndef DEBUG
		if (state[OPT_VERBOSE] == "all")
		#endif
			std::cout << e.what() << '\n';
	}
	
	if (tmp.size() > 1)
		std::sort(tmp.begin(), tmp.end(), ascending);
	
	for (auto& d : tmp) {
		out->emplace_back(d.string());
		
		try {
			std::vector<fs::path> inner_tmp;
			for (auto& inner : fs::recursive_directory_iterator(d))
				if (inner.is_directory())
					inner_tmp.emplace_back(inner.path().string());
			
			if (inner_tmp.size() > 1)
				std::sort(inner_tmp.begin(), inner_tmp.end(), ascending);
			
			for (auto& inner : inner_tmp)
				out->emplace_back(inner);
			
		} catch (fs::filesystem_error& e) {
			#ifndef DEBUG
			if (state[OPT_VERBOSE] == "all")
			#endif
				std::cout << e.what() << '\n';
		}
	}
}


func listDir(const fs::path& path, std::vector<fs::directory_entry>* const out,
			bool sorted=true)
{
	if (not out)
		return;
	if (fs::is_symlink(path)) {
		std::error_code ec;
		auto ori = fs::read_symlink(path, ec);
		if (ec)
			return;
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
			} else if (child.is_directory())
					out->emplace_back(child);
		}
	} catch (fs::filesystem_error& e) {
		#ifndef DEBUG
		if (state[OPT_VERBOSE] == "all")
		#endif
			std::cout << e.what() << '\n';
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
	
	if (sortedDir.size() <= 1)
		return false;
	
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

inline
func isValid(const fs::path& path) -> bool
{
	return not (
	((fs::status(path).permissions() & (fs::perms::owner_read
											| fs::perms::group_read
											| fs::perms::others_read))
			== fs::perms::none)
	or (state[OPT_EXCLHIDDEN] == "true" and path.filename().string()[0] == '.')
	);
}

func insertTo(std::set<fs::path>* const set, const fs::path& path)
{
	if (isValid(path))
		set->emplace(path);
}

func insertTo(std::vector<fs::path>* const vector, const fs::path& path)
{
	if (isValid(path))
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
		
		if (sortedDir.size() > 1)
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

func getBytes(const std::string& s) -> uintmax_t
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
	if (v <= 0)
		return result;
	
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

func getRange(const std::string& argv, const std::string& separator)
		-> std::shared_ptr<std::pair<uintmax_t, uintmax_t>>
{
	auto pos{argv.find(separator)};
	if (pos == std::string::npos)
		return nullptr;
	
	auto first{tolower(argv.substr(0, pos))};
	auto second{tolower(argv.substr(pos + separator.size()))};

	uintmax_t from{getBytes(std::move(first))};
	from = std::max(from, static_cast<uintmax_t>(0));
	uintmax_t to{getBytes(std::move(second))};
	to = std::max(to, static_cast<uintmax_t>(0));
	return std::make_shared<std::pair<uintmax_t, uintmax_t>>(from, to);
}

func expandArgs(const int argc, char* const argv[],
				const int startAt, std::vector<std::string>* const args)
{
	bool newFull{ false };
	unsigned lastOptCode{ 0 };
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
	for (int i{startAt}; i<argc; ++i) {
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
				if (lastOptCode > 0 and (arg[0] == ':' or arg[0] == '=')) {
					if (lastOptCode == 1)
						isMnemonic = true;
					else
						isFull = true;
					
					if (len == 1) {
						i++;
						arg = argv[i];
						len = std::strlen(arg);
						lastOptCode = 3;
					}
				} else {
					args->emplace_back(arg);
					continue;
				}
			}
		}
		
		
			
		unsigned index = lastOptCode == 3 ? -1 : (isFull and lastOptCode != 2 ? 1 : 0);
		int last{ -1 };
		while (++index < len) {
			if (last < 0 and std::isalpha(arg[index])) {
				if (isMnemonic) {
					lastOptCode = 1;
					std::string new_s {'-'};
					new_s += arg[index];
					args->emplace_back(new_s);
				} else if (isFull) {
					lastOptCode = 2;
					last = index;
				}
			} else {
				if (arg[index] == '=' or arg[index] == ':' or std::isspace(arg[index])) {
					if (isFull) {
						lastOptCode = 2;
						push(arg, index, last);
						last = -1;
					} else {
						lastOptCode = 1;
						last = index + 1;
					}
				} else if (arg[index] == ';') {
					lastOptCode = 1;
					push(arg, index, last);
					last = -1;
					if (isFull) {
						lastOptCode = 2;
						newFull = true;
					}
				} else if (isFull and last > 0 and (arg[index] == '<' or arg[index] == '>')) {
					lastOptCode = 0;
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
			   const std::string& msg,
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
		<< groupNumber(std::to_string(value)) << " " << tu[inc] << ".\n\n";
	
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
-h, --help       Display this screen.\n\
-c, --execution 'using'\n\
                 Specify execution, using 'thread', 'async' is default, or 'linear' to execute.\n\
-b, --benchmark  Benchmarking execution.\n\
-n, --exclude-hidden\n\
                 Exclude hidden folders or files.\n\
-O, --overwrite  Overwrite output playlist file.\n\
-v, --version    Display version.\n\
-V, --verbose [all | info]\n\
                 Display playlist content.\n\
                 Define as 'all' will show fail messages.\n\
                 Define as 'info' will display options info.\n\
-x, --skip-subtitle\n\
                 Dont include subtitle file.\n\
-i, --find 'keyword'\n\
                 Filter only files with filename contains find keyword.\n\
                 You can specifying this multiple times.\n\
                   Example: --find war; find invasion\n\
-I, --exclude-find 'keyword'\n\
                 Filter to exclude files with filename contains find keyword.\n\
                 You can specifying this multiple times.\n\
                   Example: -I love; I and; I home\n\
-X, --regex-syntax [type]\n\
                 Specify regular expression syntax to use.\n\
                 Available value are: 'ecma'(Default), 'awk', 'grep', 'egrep', 'basic', 'extended'.\n\
                 'basic' use the basic POSIX regex grammar and\n\
                 'extended' use the extended POSIX regex grammar.\n\
-r, --regex 'syntax'\n\
                 Filter only files with filename match regular expression.\n\
                 You can specifying this multiple times.\n\
-R, --exclude-regex 'syntax'\n\
                 Filter to exclude files with filename match regular expression.\n\
                 You can specifying this multiple times.\n\
-e, --ext \"'extension', 'extension', ...\"\n\
                 Filter only files that match specific extensions, separated by comma.\n\
                   Example: --ext \"pdf, docx\" or --ext=pdf,docx\n\
                 To process all files use *, example: --ext=* \n\
-E, --exclude-ext \"'extension', 'extension' ...\"\n\
                 Filter to exclude files that match specific extensions, separated by comma.\n\
-s, --size < | > 'size'\n\
           'min size'..'maz size'\n\
           'min size'-'max size'\n\
                 Filter only files that size match, in \"KB\", \"MB\" (default), or \"GB\".\n\
                 You can specifying this multiple times for 'Range' only based size.\n\
                   Example: --size < 750\n\
                     OR by specify the unit\n\
                      --size > 1.2gb\n\
                     OR using range with '-' OR '..'\n\
                      --size 750 - 1.2gb; size=30..200.2; size 2gb .. 4gb\n\
-S, --exclude-size < | > 'size'\n\
                  'min size'..'maz size'\n\
                  'min size'-'max size'\n\
                 Filter to exclude files that size match, in \"KB\", \"MB\" (default), or \"GB\".\n\
                 You can specifying this multiple times for 'Range' only based size.\n\
-t, --created = | < | > 'date and/or time'\n\
              'min' .. 'max'\n\
              'min' - 'max'\n\
                 Filter only files that was created on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-T, --exclude-created = | < | > 'date and/or time'\n\
                      'min' .. 'max'\n\
                      'min' - 'max'\n\
                 Filter to exclude only files that was created on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-a, --accessed = | < | > 'date and/or time'\n\
               'min' .. 'max'\n\
               'min' - 'max'\n\
                 Filter only files that was accessed on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-A, --exclude-accessed = | < | > 'date and/or time'\n\
                       'min' .. 'max'\n\
                       'min' - 'max'\n\
                 Filter to exclude only files that was accessed on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-m, --modified = | < | > 'date and/or time'\n\
              'min' .. 'max'\n\
              'min' - 'max'\n\
                 Filter only files that was modified on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-M, --exclude-modified = | < | > 'date and/or time'\n\
                      'min' .. 'max'\n\
                      'min' - 'max'\n\
                 Filter to exclude only files that was modified on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-g, --changed = | < | > 'date and/or time'\n\
              'min' .. 'max'\n\
              'min' - 'max'\n\
                 Filter only files that was changed on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-G, --exclude-changed = | < | > 'date and/or time'\n\
                      'min' .. 'max'\n\
                      'min' - 'max'\n\
                 Filter to exclude only files that was changed on specified date and/or time.\n\
                 You can specifying this multiple times, for both single value or range values.\n\
-f, --out-filename 'filename'\n\
                 Override output playlist filename.\n\
-F, --no-ouput-file [yes | no]\n\
                 Choose to create playlist file or no. Default 'yes' if option was declared or if was build as library.\n\
-d, --out-dir \"directory path\"\n\
                 Override output directory for playlist file.\n\
\n\
Options can be joined, and you replace option assignment separator [SPACE] with '=' \
or ':' and can be separated by ';' after assignment. For the example:\n\n\
  tvplaylist -hOVvc=async;xs<1.3gb;e=mp4,mkv;f:My-playlist.txt\n\n\
Thats it, -h -O -V -v -c are joined, and -c has assignment operator '=' instead of\
 using separator [SPACE]. \
Also -x -s are joined, and -x is continuing with ';' after option assignment \
'=async' and -s has remove [SPACE] separator for operator '<' and value '1.3gb'.\n\n\
Redefinition of option means, it will use the last option, except for options 'regex', 'exclude-regex', 'find', 'exclude-find', 'size', 'exclude-size', 'created', 'exclude-created', 'modified', 'exclude-modified', 'changed', 'exclude-changed', 'accessed', 'exclude-accessed'. For the example, \
this example will not use 'thread' execution at all':\n\n\
  tvplaylist -c=thread /usr/local/videos -c=none /Users/Shared/Videos\n\n\
Note, you cannot join mnemonic option with full option, for the example:\n\n\
  tvplaylist -bO;ext=mp3;version\t\tWONT WORK\n\n\
Instead try to separate mnemonic and full option, like this:\n\n\
  tvplaylist -bO --ext=mp3;version\n\n\
Posible value for 'date and/or time' are: 'Year', 'Month Name' (mnemonic or full), 'Month Number', 'Day', 'WeekDay Name' (mnemonic or full), 'Hour' (if AM/PM defined then it is 12 hours, otherwise 24 hours), 'Minute', 'Second', AM or PM.\n\
Example:\n\
  Filter only files created at 2009:\n\
     --created=2009\n\
  Filter only files created at 2009 with weekday is Sunday:\n\
     --created=\"Sunday 2009\"  OR  --created=sun/2009\n\
  Filter only files created from November 2019 thru January 2021:\n\
     --created \"nov 2019\" .. \"jan 2021\"\n\
  Filter only files created at January - March 1980 and May - June 2000 and after 2022:\n\
     --create=jan/1980..march/1980; create 2000/may - jun/2000;create>2022\n\
  Filter only files created after March 22 1990 20:44:\n\
	 --created>\"3/22/2019 20:44\"  OR  \"20:44 22/jan/2019\"\n\
It's up to you how to arrange date and/or time, At least you should use common time format (eg: 23:5:30). Here the possible arrangement you can use: \n\
Common normal: \"Monday Jan 15 2022 7:00 PM\"\n\
               \"Mon, 15/january/2022 7:0:0 pm\"\n\
               \"Monday 1/15/2022 19:0\"\n\
Equal and Acceptable: \"15 pm mon 7:0 2022/jan\"\n\
If you want to test 'date and/or time' use --debug=date 'date and/or time', for the example:\n\
     --debug=date \"pm 3:33\"\n\
     or --debug=date wed\n\
";

#if MAKE_LIB
void process(int argc, char *argv[], int *outc, char *outs[], unsigned long *maxLength) {
	state[OPT_NOOUTPUTFILE] = "true";
#else
int main(int argc, char *argv[]) {
#endif

	std::vector<fs::path> bufferDirs;

	state[OPT_SIZEOPGT] = '\0';
	state[OPT_SIZE] 	= "0";
	state[OPT_EXCLSIZEOPGT] = '\0';
	state[OPT_EXCLSIZE] 	= "0";

	state[OPT_EXECUTION]= OPT_ASYNC;
	state[OPT_REGEXSYNTAX] = "ecma";
	
	std::vector<std::string> args;
	expandArgs(argc, argv,
			   #if MAKE_LIB
			   0
			   #else
			   1
			   #endif
			   , &args);
	
	std::set<std::string_view> invalidArgs;
	auto getRegexSyntaxType{[](const std::string& s)
		-> std::regex_constants::syntax_option_type {
		if (s == "basic") return std::regex_constants::syntax_option_type::basic;
		if (s == "extended") return std::regex_constants::syntax_option_type::extended;
		if (s == "awk") return std::regex_constants::syntax_option_type::awk;
		if (s == "grep") return std::regex_constants::syntax_option_type::grep;
		if (s == "egrep") return std::regex_constants::syntax_option_type::egrep;
		else return std::regex_constants::syntax_option_type::ECMAScript;
	}};
	
	for (int i{0}; i<args.size(); ++i) {
		if (auto isMatch{ [&](const char* with,
							  char mnemonic,
							  bool writeBoolean=false,
							  const char* withOther=nullptr) {
			auto result { (args[i].length() > 3
					and args[i][0] == '-'
					and args[i][1] == '-'
					and (args[i].substr(2) == with
						 or (withOther and args[i].substr(2) == withOther)))
				or (args[i].length() == 2
					and args[i][0] == '-'
					and args[i][1] == mnemonic) };
			if (result) {
				args[i] = with;
				if (writeBoolean)
					state[with] = "true";
			}
			return result;
		} }; isMatch(OPT_HELP, 'h') or isMatch(OPT_VERSION, 'v'))
			{
				std::cout <<
				fs::path(argv[0]).filename().string() << ' '
				<< VERSION << '\n';
				
				if (args[i] == OPT_HELP)
					std::cout << HELP << '\n';
				
				if (i + 1 == args.size())
					return
#ifndef MAKE_LIB
					0
#endif
					;
			}
			else if (isMatch(OPT_DEBUG, 		'D', true)) {
				if (i + 1 < args.size() and
					(args[i + 1] == "args"
					 or args[i + 1] == "date"))
				{
					i++;
					state[OPT_DEBUG] = args[i];
					
					if (args[i] == "date" and i + 1 < args.size())
					{
						i++;
						Date date((args[i]));
						std::cout << '\"' << args[i] << '\"' << " -> "
						<< date.string() << " -> " << date.string("%c")
						<< " " << (date.isValid() ? "✅" : "❌") << '\n';
						return
#ifndef MAKE_LIB
						0
#endif
						;

					}
				}
			}
			else if (isMatch(OPT_NOOUTPUTFILE, 	'F', true)) {
				if (i + 1 < args.size())
				{
					i++;
					if (auto next { tolower(args[i + 1]) };
						next == "true" or next == "yes")
						state[OPT_NOOUTPUTFILE] = "true";
					else if (next == "false" or next == "no")
						state[OPT_NOOUTPUTFILE] = "false";
					else
						i--;
				}
			}
			else if (isMatch(OPT_FIND, 			'i')
					 or isMatch(OPT_EXCLFIND, 	'I')) {
				if (i + 1 < args.size())
				{
					(args[i] == OPT_FIND ? listFind : listExclFind)
						.emplace_back(args[i + 1]);
					state[args[i]] = "1";
					i++;
				} else
					std::cout << "Expecting keyword after \""
					<< args[i] << "\" option.\n";
			}
			else if (isMatch(OPT_REGEXSYNTAX, 	'X')) {
				if (i + 1 < args.size())
					for (auto arg { tolower(args[i + 1]) };
						 auto& s : {"ecma", "basic", "extended", "awk", "grep", "egrep"})
						if (s == arg) {
							state[OPT_REGEXSYNTAX] = s;
							i++;continue;
						}

				std::cout << "Expecting regular expression syntax after \""
					<< args[i] << "\" option.\n";
			}
			else if (isMatch(OPT_REGEX, 		'r')
					 or isMatch(OPT_EXCLREGEX, 	'R')) {
				if (i + 1 < args.size())
				{
					(args[i] == OPT_REGEX ? listRegex : listExclRegex)
						.emplace_back(std::regex(args[i + 1],
									getRegexSyntaxType(state[OPT_REGEXSYNTAX])));
					state[args[i]] = "1";
					i++;
				} else
					std::cout << "Expecting regular expression after \""
					<< args[i] << "\" option.\n";
			}
			else if (isMatch(OPT_DCREATED, 		't')
					 or isMatch(OPT_DCHANGED, 	'g')
					 or isMatch(OPT_DACCESSED, 	'a')
					 or isMatch(OPT_DMODIFIED, 	'm')
					 or isMatch(OPT_DEXCLCREATED, 	'T')
					 or isMatch(OPT_DEXCLCHANGED, 	'G')
					 or isMatch(OPT_DEXCLACCESSED, 	'A')
					 or isMatch(OPT_DEXCLMODIFIED, 	'M')
					 )
			{
				
				auto as_single{[&](const char opGt) -> bool {
					Date date(args[i + 1]);
					if (date.isValid()) {
						(args[i] == OPT_DCREATED ? listDCreated
						 : args[i] == OPT_DCHANGED ? listDChanged
						 : args[i] == OPT_DMODIFIED ? listDModified
						 : args[i] == OPT_DACCESSED ? listDAccessed
						 : args[i] == OPT_DEXCLCREATED ? listDExclCreated
						 : args[i] == OPT_DEXCLCHANGED ? listDExclChanged
						 : args[i] == OPT_DEXCLMODIFIED ? listDExclModified
						 : listDExclAccessed
						 ).emplace_back(std::make_pair(opGt, date));

						state[args[i]] = "1";
						i++;
						return true;
					}
					return false;
				}};
				if (i + 2 < args.size()
					and (args[i + 1][0] == '<'
							or args[i + 1][0] == '>'
							or args[i + 1][0] == '='))
				{
					if (as_single(args[i + 1][0]))
						continue;
				}
				else
				{
					auto push{[&](const Date& lower, const Date& upper) {
						(args[i] == OPT_DCREATED ? listDCreatedR
						 : args[i] == OPT_DCHANGED ? listDChangedR
						 : args[i] == OPT_DMODIFIED ? listDModifiedR
						 : args[i] == OPT_DACCESSED ? listDAccessedR
						 : args[i] == OPT_DEXCLCREATED ? listDExclCreatedR
						 : args[i] == OPT_DEXCLCHANGED ? listDExclChangedR
						 : args[i] == OPT_DEXCLMODIFIED ? listDExclModifiedR
						 : listDExclAccessedR
						 ).emplace_back(std::make_pair(lower, upper));
						state[args[i]] = "1";
					}};
					if (i + 3 < args.size() and
						(args[i + 2] == "-" or args[i + 2] == "..")) {
						if (Date lower(args[i + 1]); lower.isValid())
							if (Date upper(args[i + 3]); upper.isValid())
							{
								push(lower, upper);
								i += 3;
								continue;
							}
					} else if (i + 1 < args.size()) {
						auto pos = args[i + 1].find("..");
						if (pos not_eq std::string::npos) {
							if (Date lower(args[i + 1].substr(0, pos));
								lower.isValid())
							if (Date upper(args[i + 1].substr(pos + 2));
								upper.isValid())
							{
								push(lower, upper);
								state[args[i]] = "1";
								i++;
								continue;
							}
						}
						
						else {
							if (as_single('='))
								continue;
						}
					}
				}
				std::cout << "Expecting date and/or time after \""
				<< args[i] << "\" option.\n";
			}
			else if (isMatch(OPT_OVERWRITE, 	'O', true));
			else if (isMatch(OPT_BENCHMARK, 	'b', true));
			else if (isMatch(OPT_SKIPSUBTITLE, 	'x', true));
			else if (isMatch(OPT_EXCLHIDDEN, 	'n', true));
			else if (isMatch(OPT_VERBOSE, 		'V', true)) {
				if (i + 1 < args.size() and (args[i + 1] == "all" or args[i + 1] == "info"))
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
						state[OPT_EXECUTION] = "Linear";
				} else
					std::cout << "Expecting 'thread', 'async', or 'none' after \""
					<< args[i] << "\" option.\n";
			} else if (isMatch(OPT_EXT, 	'e')
					   or isMatch(OPT_EXCLEXT, 'E')) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1]] = args[i] == "*.*" ? "*" : args[i];
				} else
					std::cout << "Expecting extension after \""
					<< args[i] << "\" option (eg: \"mp4, mkv\").\n";
			}
			else if (isMatch(OPT_FIXFILENAME, 	'f', false, "fix-filename")) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1]] = args[i];
					if (not fs::path(args[i]).parent_path().string().empty())
					{
						state[args[i - 1]] = fs::path(args[i]).filename().string();
						state[OPT_OUTDIR] = fs::path(args[i]).parent_path().string();
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
			else if (isMatch(OPT_SIZE, 			's')
					 or isMatch(OPT_EXCLSIZE, 	'S')) {
				if (bool isExclude{ args[i] == OPT_EXCLSIZE };
					i + 1 < args.size()) {
					if (args[i + 1][0] == '<' or args[i + 1][0] == '>')
					{
						i++;
						const auto opGT { args[i][0] };
						
						uintmax_t value{ 0 };
						
						if (args[i].size() > 1) {
							value = getBytes(args[i].substr(1));
							if (value <= 0)
								goto SIZE_NEEDED;
						}
						
						if (value <= 0 and i + 1 < args.size()) {
							value = getBytes(args[i + 1]);
							
							if (value <= 0)
								goto SIZE_NEEDED;
							else
								i++;
						}
						
						state[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] = opGT;
						state[isExclude ? OPT_EXCLSIZE : OPT_SIZE] = std::to_string(std::move(value));
						(isExclude ? listExclSize : listSize).clear();
					} else {
						auto range{getRange(args[i + 1], std::move("-"))};
						if (not range)
							range = getRange(args[i + 1], std::move(".."));
						
						const std::string s_first = isExclude ? OPT_EXCLSIZE : OPT_SIZE;
						std::string first = "0", second = "0";
						
						if (range)
						{
							first = std::to_string(range->first);
							second = std::to_string(range->second);
							
							i++;
							
							if (second == "0") {
								uintmax_t value{ 0 };
								if (i + 1 < args.size())
									value = getBytes(args[i + 1]);
								
								if (value > 0) {
									second = std::to_string(value);
									i++;
								} else
									goto SIZE_NEEDED;
							}
						} else {
							if (i + 3 < args.size()) {
								first = std::to_string(getBytes(args[i + 1]));
								second = std::to_string(getBytes(args[i + 3]));
							} else if (i + 2 < args.size()) {
								first = std::to_string(getBytes(args[i + 1]));
								second = std::to_string(getBytes(args[i + 2]));
							}
							
							if (first != "0" and second != "0")
								i+=3;
							else if (first != "0" and second == "0") {
								if (args[i + 2].size() > 1
									and args[i + 2][0] == '-'
									and std::isdigit(args[i + 2][1]))
								{
									second = std::to_string(getBytes(args[i + 2].substr(1)));
								} else if (args[i + 2].size() > 2
										   and args[i + 2][0] == '.'
										   and args[i + 2][1] == '.'
										   and std::isdigit(args[i + 2][2]))
								{
									second = std::to_string(getBytes(args[i + 2].substr(2)));
								} else
									goto SIZE_NEEDED;
								i+=2;
							} else
								goto SIZE_NEEDED;
						}
						
						state[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] = '\0';
						state[s_first] = first;

						if (std::stoul(second) < std::stoul(first)) {
							std::cout << "Fail: Range is up side down! \""
								<< groupNumber(first) << " bytes greater than "
								<< groupNumber(second) << " bytes\"\n";
							state[s_first] = "0";
						} else if (first != "0" and second != "0")
							(isExclude ? listExclSize : listSize).emplace_back(std::make_pair(
										   std::stoul(first),
										   std::stoul(second)));
					}
				}
				else
SIZE_NEEDED:		std::cout << "Expecting operator '<' or '>' followed\
by size in KB, MB, or GB.\nOr use value in range using form 'from-to' OR 'from..to'\n";
		} else if (fs::is_directory(args[i]))
			insertTo(&bufferDirs, std::move(fs::path(args[i])));
		else if (fs::is_regular_file(std::move(fs::path(args[i])))) {
			if (isValidFile(fs::absolute(args[i])))
				insertTo(&selectFiles, std::move(fs::absolute(args[i])));
		} else
			invalidArgs.emplace(args[i]);
	}
	
	if (not invalidArgs.empty()) {
		std::string_view invalid_args[invalidArgs.size()];
		std::move(invalidArgs.begin(), invalidArgs.end(), invalid_args);
		std::cout << "\nWhat " << (invalidArgs.size() > 1 ? "are these" : "is this") << "? :\n";
		for (auto i{ 0 }; i<invalidArgs.size(); ++i) {
			std::cout << '"' << invalid_args[i] << '"'
				<< (i + 1 < invalidArgs.size() ? ", " : "");
		}
		std::cout << "\nFor more information, please try to type \""
			<< fs::path(argv[0]).filename().string() << " --help\"\n\n";
	}

	if (bufferDirs.empty() and selectFiles.empty())
		insertTo(&bufferDirs, fs::current_path());
	
	const auto inputDirsCount = bufferDirs.size();
	fs::path inputDirs[inputDirsCount + 1];
	std::copy(bufferDirs.begin(), bufferDirs.end(), inputDirs);

	while (bufferDirs.size() == 1) {
		state[OPT_OUTDIR] = fs::absolute(bufferDirs[0]).string();
		insertTo(&regularDirs, std::move(bufferDirs[0]));/// Assume single input dir is regularDir
		if (isContainsSeasonDirs(fs::path(state[OPT_OUTDIR]))) {
			break;
		}
		bufferDirs.clear();
		std::vector<fs::directory_entry> sortedDirs;
		listDir(fs::path(state[OPT_OUTDIR]), &sortedDirs);
				
		for (auto& child : sortedDirs)
			insertTo(&bufferDirs, std::move(child.path()));
	}

	if (auto dirOut{ inputDirsCount == 1
			? transformWhiteSpace(fs::path(state[OPT_OUTDIR]).filename().string())
			: std::to_string(inputDirsCount)};
		state[OPT_FIXFILENAME].empty())
	{
		#ifdef LIBCPP_FORMAT
		std::format_to(std::back_inserter(state[OPT_FIXFILENAME]),
					   "playlist_from_{0}{1}.m3u8",
					   inputDirsCount == 0
						 ? groupNumber(std::to_string(selectFiles.size())) + "_file"
						 : dirOut + "_dir",
					   inputDirsCount > 1 or selectFiles.size() > 1 ? "s" : "";
		#else
			state[OPT_FIXFILENAME] =
				state[OPT_NOOUTPUTFILE] == "true" ? "" :
				"playlist_from_"
				+ (inputDirsCount == 0
				   ? groupNumber(std::to_string(selectFiles.size())) + "_file"
				   : dirOut + "_dir")
		
				+ (inputDirsCount > 1 or selectFiles.size() > 1 ? "s" : "")
				+ ".m3u8";
		#endif
	}
	
	#ifndef DEBUG
	if (not invalidArgs.empty() or state[OPT_VERBOSE] == "all"
		or state[OPT_VERBOSE] == "info" or state[OPT_BENCHMARK] == "true"
		or state[OPT_DEBUG] == "true" or state[OPT_DEBUG] == "args")
	#endif
	{
		if (state[OPT_DEBUG] == "true" or state[OPT_DEBUG] == "args") {
			std::cout << "Original Arguments: ";
			for (auto i{1}; i<argc; ++i)
				std::cout << '"' << argv[i] << '"' << (i+1>=argc ? "" : ", ");
			std::cout << '\n';
			std::cout << "Parsed Arguments  : ";
			for (auto i{0}; i<args.size(); ++i)
				std::cout << '"' << args[i] << '"' << (i+1>=args.size() ? "" : ", ");
			std::cout << '\n';
		}
#define PRINT_OPT(x)	(x.empty() ? "false" : x)
	std::cout
		<< OPT_EXECUTION << "\t\t: " << state[OPT_EXECUTION] << '\n'
		<< OPT_VERBOSE << "\t\t\t: " << PRINT_OPT(state[OPT_VERBOSE]) << '\n'
		<< OPT_BENCHMARK << "\t\t: " << PRINT_OPT(state[OPT_BENCHMARK]) << '\n'
		<< OPT_OVERWRITE << "\t\t: " << PRINT_OPT(state[OPT_OVERWRITE]) << '\n'
		<< OPT_NOOUTPUTFILE << "\t\t: " << PRINT_OPT(state[OPT_NOOUTPUTFILE]) << '\n'
		<< OPT_FIXFILENAME << "\t\t: " << state[OPT_FIXFILENAME] << '\n'
		<< "Current Directory\t: " << fs::current_path().string() << '\n'
		<< OPT_OUTDIR << "\t\t\t: " << state[OPT_OUTDIR] << '\n'
		<< OPT_SKIPSUBTITLE << "\t\t: " << PRINT_OPT(state[OPT_SKIPSUBTITLE]) << '\n';
#undef PRINT_OPT
	std::cout << OPT_EXT << "\t\t\t: " << state[OPT_EXT];
	if (state[OPT_EXT].empty()) {
		for (auto i{0}; i<DEFAULT_EXT.size(); ++i)
			std::cout << DEFAULT_EXT[i] << (i < DEFAULT_EXT.size() - 1 ? ", " : "");
	}
	std::cout << '\n';

	for (auto& S : {OPT_FIND, OPT_EXCLFIND}) {
		std::cout << S << (S == OPT_FIND ? "\t\t\t: " : "\t\t: ");
		for (auto i{0}; i<(S == OPT_FIND ? listFind : listExclFind).size(); ++i)
			std::cout << (S == OPT_FIND ? listFind : listExclFind)[i]
			<< (i < (S == OPT_FIND ? listFind : listExclFind).size() - 1 ? ", " : "");
		std::cout << '\n';
	}

	for (auto& S : {OPT_REGEX, OPT_EXCLREGEX}) {
		std::cout << S << (S == OPT_REGEX ? "\t\t\t: " : "\t\t: ");
		for (auto i{0}; i<(S == OPT_REGEX ? listRegex : listExclRegex).size(); ++i)
			std::cout << (S == OPT_REGEX ? listRegex : listExclRegex)[i].mark_count()
			<< " expression"
			<< (i < (S == OPT_REGEX ? listRegex : listExclRegex).size() - 1 ? ", " : "");
		std::cout << '\n';
	}

	for (auto& S : {OPT_SIZE, OPT_EXCLSIZE}) {
		std::cout << S << (S == OPT_SIZE ? "\t\t\t: " : "\t\t: ")
				<< ((S == OPT_SIZE ? listSize : listExclSize).empty()
					or state[S == OPT_SIZE ? OPT_SIZEOPGT
							  : OPT_EXCLSIZEOPGT][0] != '\0'
					? (state[S == OPT_SIZE ? OPT_SIZEOPGT
							 : OPT_EXCLSIZEOPGT][0] == '<' ? "< " : "> ")
					+ state[S] : "");
		for (auto i{0}; i<(S == OPT_SIZE ? listSize : listExclSize).size(); ++i)
			std::cout << (S == OPT_SIZE ? listSize : listExclSize)[i].first
			<< ".." << (S == OPT_SIZE ? listSize : listExclSize)[i].second
			<< (i < (S == OPT_SIZE ? listSize : listExclSize).size() - 1 ? ", " : "");
		std::cout << '\n';
	}
			
	{
		std::string st[8] = { OPT_DCREATED, OPT_DCHANGED, OPT_DACCESSED, OPT_DMODIFIED,
			OPT_DEXCLCREATED, OPT_DEXCLCHANGED, OPT_DEXCLACCESSED, OPT_DEXCLMODIFIED };
		
		std::vector<std::pair<char, Date>>* ot[8] = { &listDCreated, &listDAccessed, &listDModified, &listDChanged,
			  &listDExclCreated, &listDExclAccessed, &listDExclModified, &listDExclChanged };

		
		std::vector<std::pair<Date, Date>>* rt[8] = { &listDCreatedR, &listDAccessedR, &listDModifiedR, &listDChangedR,
			  &listDExclCreatedR, &listDExclAccessedR, &listDExclModifiedR, &listDExclChangedR };
		
		for (auto i {0}; i<8; ++i) {
			std::cout << st[i] << ": ";
			for (auto k{0}; k<ot[i]->size(); ++k) {
				std::cout << ot[i]->at(k).first << ' ' << ot[i]->at(k).second.string() << ", ";
			}
			
			for (auto k{0}; k<rt[i]->size(); ++k) {
				std::cout << rt[i]->at(k).first.string() << ".." << rt[i]->at(k).second.string() << ", ";
			}
			std::cout << '\n';
		}
	}
	
	std::cout << "Inputs\t\t\t: ";
		for (auto i{0}; i<inputDirsCount + selectFiles.size(); ++i) {
			if (i < inputDirsCount) {
				std::cout << inputDirs[i];
			} else {
				std::cout << selectFiles[i - inputDirsCount];
			}
			
			std::cout  << (i < (inputDirsCount + selectFiles.size()) - 1 ? ", " : "");
		}
	std::cout << "\n\n";
	} // END Info
					   
	if (state[OPT_OUTDIR].empty()) {
		if (selectFiles.empty())
			state[OPT_OUTDIR] = fs::current_path().string();
		else
			state[OPT_OUTDIR] = fs::path(selectFiles[0]).parent_path().string();
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

	/// Convert std::set to classic array, to enable call by index subscript.
	fs::path regularDirs[regularDirSize];
	std::move(::regularDirs.begin(), ::regularDirs.end(), regularDirs);

	fs::path seasonDirs[seasonDirSize];
	std::move(::seasonDirs.begin(), ::seasonDirs.end(), seasonDirs);

	sortFiles(&selectFiles);

	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true" or state[OPT_DEBUG] == "true")
	#endif
	{
		if (inputDirsCount > 0)
			timeLapse(start, groupNumber(std::to_string(regularDirSize + seasonDirSize))
					  + " valid input dirs"
					  + (selectFiles.size() > 0
						 ? " and " + groupNumber(std::to_string(selectFiles.size()))
						 + " input files " : " " ) + "took ");
	}


	#ifndef DEBUG
	if (state[OPT_DEBUG] == "true")
	#endif
	for (auto i{0}; i<maxDirSize; ++i)
		for (auto& select : {1, 2}) {
			if ((select == 1 and i >= regularDirSize)
				or (select == 2 and i >= seasonDirSize))
				continue;
			std::cout << (select == 1 ? 'R' : 'S') << ':'
				<< (select == 1 ? regularDirs[i] : seasonDirs[i]) << '\n';
		}
	
	
	std::map<std::string, std::shared_ptr<std::vector<fs::path>>> records;
	
	std::ofstream outputFile;
	fs::path outputName;
	std::vector<std::string> outputArray;
					   
	if (state[OPT_NOOUTPUTFILE] != "true") {
		outputName = state[OPT_OUTDIR] + fs::path::preferred_separator
			+ state[OPT_FIXFILENAME];
		if (fs::exists(outputName) and state[OPT_OVERWRITE] == "true")
			fs::remove(outputName);
		else
			outputName = getAvailableFilename(outputName);
			
		outputFile = std::ofstream(outputName, std::ios::out);
	}
	
	unsigned long indexFile{0};
	unsigned long playlistCount{0};
	
	auto putIntoPlaylist{ [&](const fs::path& file) {
		auto putIt{ [&](const fs::path& file) {
			playlistCount++;
			if (state[OPT_NOOUTPUTFILE] == "true") {
				#if MAKE_LIB
				if (outc)
					*outc += 1;
				if (maxLength) {
					if (auto sz { fs::absolute(file).string().size() + 1 };
						*maxLength < sz)
						*maxLength = sz;
				}
				if (outs)
					std::strcpy(outs[playlistCount - 1], fs::absolute(file).string().c_str());
				#endif
			} else
				outputFile << fs::absolute(file).string() << '\n';
			#ifndef DEBUG
			if (state[OPT_VERBOSE] == "true" or state[OPT_DEBUG] == "true")
			#endif
				std::cout << fs::absolute(file).string() << '\n';
		}};
		
		putIt(file);
				
		if (state[OPT_SKIPSUBTITLE] != "true") {
			std::vector<fs::path> subtitleFiles;
			findSubtitleFile(file, &subtitleFiles);
			for (auto& sf : subtitleFiles)
				putIt(sf);
		}
	}};
	
	auto filterChildFiles{ [&records](const std::string& dir, bool recurive=false) {
		auto filter{ [](const fs::directory_entry& f) -> bool {
			if (not fs::is_regular_file(f.path())
				or not isValid(f.path()))
					return false;
			
			return isValidFile(f.path());
		}};
		
		std::vector<fs::path> bufferFiles;
		
		auto putToRecord{[&bufferFiles, &dir, &records](bool wantToSort) {
			if (bufferFiles.empty())
				return;
			
			if (wantToSort and bufferFiles.size() > 1)
				std::sort(bufferFiles.begin(), bufferFiles.end(), ascending);
			
			records.emplace(std::make_pair(std::move(dir),
										   std::make_shared<std::vector<fs::path>>(std::move(bufferFiles))
										   ));
		}};
		
		try {
			if (recurive) {
				std::vector<std::string> dirs;
				recursiveDirectory(dir.c_str(), &dirs);
				
				for (auto& d : dirs) {
					std::vector<fs::path> tmp;
					
					for (auto& f : fs::recursive_directory_iterator(d))
						if (filter(f))
							tmp.emplace_back(f);
					
					sortFiles(&tmp);
					
					for (auto& f : tmp)
						bufferFiles.emplace_back(f);
				}
				
				putToRecord(false);
			} else {
				for (auto& f : fs::directory_iterator(dir))
					if (filter(f))
						bufferFiles.emplace_back(f);
				
				putToRecord(true);
			}
			
		} catch (fs::filesystem_error& e) {
			#ifndef DEBUG
			if (state[OPT_VERBOSE] == "all")
			#endif
				std::cout << e.what() << '\n';
		}
	}};

	start = std::chrono::system_clock::now();
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
		//if (bufferSort.size() > 1) std::sort(bufferSort.begin(), bufferSort.end());
		for (auto& ok : bufferSort)
			putIntoPlaylist(std::move(ok));
		
		if (finish and indexFile >= selectFiles.size())
			break;
	}
	
	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true" or state[OPT_DEBUG] == "true")
	#endif
		timeLapse(start, groupNumber(std::to_string(playlistCount)) + " valid files took ");
					   
	if (state[OPT_NOOUTPUTFILE] != "true") {
		if (outputFile.is_open())
			outputFile.close();
			
		if (playlistCount == 0)
			fs::remove(outputName);
		else
			std::cout << fs::absolute(outputName).string() << '\n';
	}
}
