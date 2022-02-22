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

#include <chrono>
#include <iostream>
#include <string>
#include <charconv>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <future>
#include <cstdarg>
#include <algorithm>
#include <regex>
#include <clocale>
#include <sys/stat.h> // TODO: I dont know in Windows, should change to use FileAttribute?
#include <fstream>
#include <random>
#include <dirent.h>
#define let static constexpr auto
let TRUE_FALSE						= {	"true",
										"false"
};

let OPT_LIST						{"list"};
let OPT_INSTALLMAN					{"install-man"};
let OPT_UNINSTALLMAN				{"uninstall-man"};
let OPT_OPEN						{"open"};
let OPT_OPENWITH					{"open-with"};

let OPT_SHOWCONFIG					{"show-config"};
let OPT_SHOWCONFIG_ALTERNATIVE 		= {	"show-defaults",
										"display-config",
										"display-defaults",
										"print-config",
										"print-defaults",
};
let OPT_WRITEDEFAULTS				{"write-defaults"};	// W
let OPT_WRITEDEFAULTS_ALTERNATIVE 	= {	"write-config"
};
let OPT_WRITEDEFAULTS_ARGS 			= { "new",
										"edit",
										"remove",
										"reset",
										"add"
};

let OPT_LOADCONFIG					{"load-config"};			// L
let OPT_ARRANGEMENT 				{"arrangement"};			// w
	let MODE_ARRANGEMENT_DEFAULT				{"default"};
	let MODE_ARRANGEMENT_UNORDERED				{"unordered"};
	let MODE_ARRANGEMENT_ASCENDING				{"ascending"};
	let MODE_ARRANGEMENT_PERTITLE				{"per-title"};
	let MODE_ARRANGEMENT_SHUFFLE				{"shuffle"};
	let MODE_ARRANGEMENT_SHUFFLE_DEFAULT		{"default-shuffle"};
	let MODE_ARRANGEMENT_SHUFFLE_PERTITLE		{"per-title-shuffle"};
	let MODE_ARRANGEMENT_DESCENDING				{"descending"};
	let MODE_ARRANGEMENT_DESCENDING_DEFAULT		{"default-descending"};
	let MODE_ARRANGEMENT_DESCENDING_PERTITLE	{"per-title-descending"};
let OPT_HELP 						{"help"};					// h
let OPT_VERSION 					{"version"};				// v
let OPT_VERBOSE 					{"verbose"};				// V
let OPT_BENCHMARK 					{"benchmark"};				// b
let OPT_OVERWRITE 					{"overwrite"};				// O
let OPT_SKIPSUBTITLE 				{"skip-subtitle"};			// x
let OPT_OUTDIR 						{"out-dir"};				// d
let OPT_CURRENTDIR 					{"current-dir"};
let OPT_ADSDIR 						{"ads-dir"};				// D
let OPT_ADSCOUNT					{"ads-count"};				// C
let OPT_EXECUTION					{"execution"};				// c
	let MODE_EXECUTION_THREAD					{"thread"};
	let MODE_EXECUTION_ASYNC					{"async"};
let OPT_EXCLHIDDEN					{"exclude-hidden"};			// n
let OPT_REGEXSYNTAX					{"regex-syntax"};		// X
let OPT_REGEXSYNTAX_ARGS 			= {	"ecma",
										"basic",
										"extended",
										"awk",
										"grep",
										"egrep"
};
let OPT_CASEINSENSITIVE				{"make-case-insensitive"};	// N
let OPT_CASEINSENSITIVE_ALTERNATIVE = {	"case-insensitive",
										"caseinsensitive",
										"ignore-case",
										"ignorecase"
};

let OPT_SEARCH						{"search"};					// q

let OPT_OUTFILENAME 				{"out-filename"};			// f
let OPT_NOOUTPUTFILE 				{"no-output-file"};			// F

let OPT_SIZE						{"size"};					// s
let OPT_SIZEOPGT					{"size_op"};
let OPT_EXCLSIZE					{"exclude-size"};			// S
let OPT_EXCLSIZEOPGT				{"exclude-size_op"};

let OPT_EXT 						{"ext"};					// e
let OPT_EXCLEXT 					{"exclude-ext"};			// E

let OPT_FIND						{"find"};					// i
let OPT_EXCLFIND					{"exclude-find"};			// I

let OPT_REGEX						{"regex"};					// r
let OPT_EXCLREGEX					{"exclude-regex"};			// R

let OPT_DATE						{"date"};					// z
let OPT_EXCLDATE					{"exclude-date"};			// Z

let OPT_DCREATED					{"created"};				// t
let OPT_DMODIFIED					{"modified"};				// m
let OPT_DACCESSED					{"accessed"};				// a
let OPT_DCHANGED					{"changed"};				// g

let OPT_DEXCLCREATED				{"exclude-created"};		// T
let OPT_DEXCLMODIFIED				{"exclude-modified"};		// M
let OPT_DEXCLACCESSED				{"exclude-accessed"};		// A
let OPT_DEXCLCHANGED				{"exclude-changed"};		// G

let OPT_DEBUG						{"debug"};                  // B
let OPT_DEBUG_ARGS 					= {	"date",
										"args",
										"id3"
};


#if defined(_WIN32) || defined(_WIN64)
#define CONFIG_PATH	"C:\\ProgramData\\tvplaylist.conf"
#define INSTALL_PATH "C:\\Windows"
#define SEP_PATH "\\"
#define PE_EXT ".exe"
#else
#define CONFIG_PATH	"/usr/local/etc/tvplaylist.conf"
#define INSTALL_PATH "/usr/local/bin"
#define SEP_PATH "/"
#define PE_EXT
#endif
#define INSTALL_FULLPATH	INSTALL_PATH SEP_PATH "tvplaylist" PE_EXT

let OPT_INSTALL						{"install"};
let INSTALL="\
--install\n\
        Install tvplaylist into \"" INSTALL_PATH "\".\n\
";
let OPT_UNINSTALL					{"uninstall"};
let UNINSTALL="\
--uninstall\n\
        Uninstall tvplaylist from \"" INSTALL_PATH "\".\n\
";
let OPT_UPDATE						{"update"};
let OPT_UPDATE_ALTERNATIVE			= {	"upgrade",
};
let UPDATE="\
--update\n\
--upgrade\n\
        Update tvplaylist.\n\
";

let OPEN="\
--open\n\
        Open generated playlist file with default app.\n\
        See also: --open-with\n\
";
let OPEN_WITH="\
--open-with 'application'\n\
        Open with specified application.\n\
        See also: --open\n\
";
let LIST="\
--list\n\
        Display list of options state, then exit.\n\
        See also: --debug=args or --display-config\n\
";
let SHOW="\
--show-defaults\n\
--show-config\n\
--display-defaults\n\
--display-config\n\
--print-defaults\n\
--print-config\n\
        Display options summary and configuration file contents, then quit.\n\
        See also --verbose or --verbose=info or --debug=args or --benchmark.\n\
        Example: --print-defaults\n\
";
let WRITE="\
-W, --write-defaults [new | reset | edit | add | remove]\n\
    --write-config [new | reset | edit | add | remove]\n\
        Write anything you defined to \"" CONFIG_PATH\
"\" as new defaults (if no argument passed), instead executing it.\n\
        Argument \"new\" is used by default.\n\
        Example: --write-config\n\
        To reset to factory default, pass \"reset\".\n\
        Example: --write-defaults reset\n\
        To edit existing options, pass \"edit\".\n\
        Example: --ads-count=1-3:write-config=edit\n\
        To add or append new options, pass \"add\".\n\
        Example: -W=add -D=/tmp/Downloads:D=/tmp/Backup\n\
        To remove the exact pairs of options and values, pass \"remove\"\n\
        Example: --debug=args --write-config=remove\n\
";
let LOAD="\
-L, --load-config 'custom file'\n\
        Load configuration from custom file.\n\
        Example:\n\
            --load-config tuesday_night.txt\n\
        Default configuration file located in \"" CONFIG_PATH "\".\n\
        If config file exist in default location, then it will be automatic loaded.\n\
        Content of configuration are list of options (with/out \"--\") separated by new line.\n\
        To write as defaults instead executing defined options, you can use option --write-config, either by specifying \"edit\", \"reset\", or \"add\" as parameter.\n\
        Example:\n\
            /* Multi lines\n\
               comment */\n\
            // Single line Comment\n\
            ads-dir   = \"D:\\Videos\\Funny Advertise Collections\"\n\
            ads-dir   = \"D:\\Downloads\\Trailers 2022\"\n\
            ads-count = 2-10 # Single line Comment\n\
        To set custom path for config file, use --write-defaults.\n\
        Example: --write-defaults:load-config=/tmp/setting.txt\n\
";
let VERSION="\
tvplaylist version 1.1 (Early 2022)\nTM and (C) 2022 Widiatmoko. \
All Rights Reserved. License and Warranty\n";

let HELP="\
Create playlist file from vary directories and files, \
then by default, arranged one episode per Title.\nHosted in https://github.com/Mr-Widiatmoko/MakeTVPlaylist\n\n\
Usage:\n    tvplaylist [Option or Dir or File] ...\n\n\
If no argument was specified, the current directory will be use.\n\n\
Option:\n\
";
let A_HELP="\
-h, --help ['keyword']\n\
        Display options description.\n\
";
let A_VERSION="\
-v, --version\n\
        Display version.\n\
";

let ADS_DIR="\
-D, --ads-dir 'directory constains advertise'\n\
        Add advertise directory. Ads file will be inserted between each ordered files.\n\
        You can specifying this multiple times.\n\
        To set the numbers how many ads shown use --ads-count.\n\
";
let ADS_COUNT="\
-C, --ads-count 'fixed count'\n\
                 'min count' .. 'max count'\n\
                 'min count' - 'max count'\n\
        Set the number of how many ads will be shown per insertion. Set 0 to disable ads.\n\
        If you set using range, then ads will be shown randomly between 'min count' to 'max count'.\n\
        To set where the advertise directories location path, use --ads-dir.\n\
";
let VERBOSE="\
-V, --verbose [all | info]\n\
        Display playlist content.\n\
        Define as 'all' will also show fail messages.\n\
        Define as 'info' will also display options info summary.\n\
";
let BENCHMARK="\
-b, --benchmark\n\
        Benchmarking execution.\n\
";
let ARRANGEMENT="\
-w, --arrangement [default | unordered | per-title | ascending | shuffle] \n\
        Specifying how playlist content will be arranged.\n\
        default   : One file per Title, Title sorted ascending.\n\
                    To set more than one file per Title, pass \"=\"'Count'.\n\
                    Example: --arrangement default=3\n\
        unordered : One file per Title, Title sorted unordered.\n\
        per-title : All files sorted ascending grouped per Title/Directory.\n\
        ascending : All files sorted ascending by filenames, regardless directory name order.\n\
        shuffle           : Shuffle all files.\n\
        shuffle-default   : \"default\" but each Title has files shuffled.\n\
        shuffle-per-title : \"per-title\" but each Title has files shuffled.\n\
        descending           : All files sorted descending by filenames, regardless directory name order.\n\
        descending-default   : \"default\" but each Title has files sorted descending.\n\
        descending-per-title : \"per-title\" but each Title has files sorted descending.\n\
";
let SEARCH="\
--search 'keywords'\n\
        tvplaylist search engine.\n\
        --search is alias from --no-output-file:verbose:arrangement=ascending:ignore-case\n\
          Example:\n\
            --search \"ext=* medalion or title=medalion exclude=horse\"\n\
        You can specifying this multiple times.\n\
";
let OVERWRITE="\
-O, --overwrite\n\
        Overwrite output playlist file.\n\
";
let SKIP_SUBTITLE="\
-x, --skip-subtitle\n\
        Dont include subtitle file.\n\
";
let OUT_DIR="\
-d, --out-dir \"directory path\"\n\
        Override output directory for playlist file.\n\
";
let CURRENT_DIR="\
-d, --current-dir \"directory path\"\n\
        Set current directory.\n\
";
let EXECUTION="\
-c, --execution [thread | async | linear]\n\
        Specify execution, \"async\" is selected by default.\n\
";
let EXCLUDE_HIDDEN="\
-n, --exclude-hidden\n\
        Exclude hidden folders or files (not for Windows).\n\
";
[[deprecated("Recognized in --find or --exlude-find.")]] [[maybe_unused]]
let CASEINSENSITIVE="\
-N, --make-case-insensitive\n\
     --case-insensitive\n\
     --caseinsensitive\n\
     --ignore-case\n\
     --ignorecase\n\
        Make --find and --exclude-find case-insensitive.\n\
        To make case-sensitive again, pass --ignorecase=false.\n\
";
let FIND="\
-i, --find 'keyword'\n\
-I, --exclude-find 'keyword'\n\
        Filter only files with (or exclude) filename contains find keyword.\n\
        You can specifying this multiple times.\n\
            Example: --find=war:find=invasion\n\
        To filter only for directory name, pass dir='value'.\n\
            Example: --find dir=war  or  --find=dir=war\n\
        To filter files extension, pass ext='ext-comma-delimited'\n\
            Example: --find ext=mp3,mp4,mkv  or --find ext=*\n\
        To find by using MP3 ID3 tags, pass 'key'='value'.\n\
        Possible 'key' are: \"id3\", \"title\", \"artist\", \"album\", \"genre\", \"comment\", \"year\", and \"track\".\n\
            Example: --find artist=Koko  or  --find=artist=Koko\n\
        If you specify tag \"id3\", it will search in all tags, eg:: title, album, etc.\n\
        To enable case-insensitive, pass ignore-case=true:\n\
            Example: --find ignore-case=true\n\
";
[[deprecated("Recognized in --regex or --exlude-regex.")]] [[maybe_unused]]
let REGEX_SYNTAX="\
-X, --regex-syntax [ecma | awk | grep | egrep | basic | extended]\n\
        Specify regular expression syntax type to use.\n\
        Available value are: 'ecma'(Default), 'awk', 'grep', 'egrep', 'basic', 'extended'.\n\
            'basic' use the basic POSIX regex grammar and\n\
            'extended' use the extended POSIX regex grammar.\n\
        You can also specifying this inside --regex or --exclude-regex, for example\n\
            --regex type=basic  or  --exclude-regex type=extended\n\
";
let REGEX="\
-r, --regex 'syntax'\n\
-R, --exclude-regex 'syntax'\n\
        Filter only files with (or exclude) filename match regular expression.\n\
        To set regex syntax type, pass type=[ecma | awk | grep | egrep | basic | extended]\n\
            Example: --regex type=grep  or  --regex=type=grep\n\
        You can specifying this multiple times.\n\
";
let EXT="\
-e, --ext \"'extension', 'extension', ...\"\n\
          += \"'extension', 'extension', ...\"\n\
          -= \"'extension', 'extension', ...\"\n\
-E, --exclude-ext \"'extension', 'extension' ...\"\n\
                  += \"'extension', 'extension', ...\"\n\
                  -= \"'extension', 'extension', ...\"\n\
        Filter only (or exclude) files that match specific extensions, separated by comma.\n\
            Example: --ext \"pdf, docx\" or --ext=pdf,docx\n\
        To process all files use character *.\n\
            Example: --ext=* \n\
        To enable get contents from other playlists, you must include the playlist extensions.\n\
            Example: --ext=mp4,mkv,mp3,m3u,m3u8,pls,wpl,xspf\n\
        To add[s] or remove[s] from existing extensions, use operator += or -=.\n\
            Example: --ext-=mp3,mpa,aac:ext+=flac,ape\n\
        You can specifying this multiple times for operators += and -= only.\n\
";
let SIZE="\
-s, --size < | > 'size'\n\
           'min size'..'maz size'\n\
           'min size'-'max size'\n\
-S, --exclude-size < | > 'size'\n\
                   'min size'..'maz size'\n\
                   'min size'-'max size'\n\
        Filter only (or exclude) files that size match, in \"KB\", \"MB\" (default), or \"GB\".\n\
        You can specifying this multiple times for 'Range' only based size.\n\
            Example: --size<750\n\
                OR by specify the unit\n\
                    --size>1.2gb\n\
                OR using range with '-' OR '..'\n\
                    --size 750 - 1.2gb; size=30..200.2; size 2gb .. 4gb\n\
";
let DATE="\
-z, --date = | < | > 'date and/or time'\n\
           'min' .. 'max'\n\
           'min' - 'max'\n\
-Z, --exclude-date = | < | > 'date and/or time'\n\
                   'min' .. 'max'\n\
                   'min' - 'max'\n\
        Filter only (or exclude) files that was created or accessed or modified or changed at specified date[s] and/or time[s].\n\
        If you need only specific type of date, then use --[exclude-]created, --[exclude-]accessed, --[exclude-]modified, or --[exclude-]changed respectively.\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
            Example, to filter all files that has created/accessed/modified/changed with minutes from 20 to 35 for any date time and from 47 to 56 for year 2021:\n\
                    --date=0:20-0:35 --date=\"0:47 2021\" - \"0:56 2021\"\n\
                OR\n\
                    --date=0:20..0:35:date=0:47/2022..0:56/2022\n\
";
let CREATED="\
-t, --created = | < | > 'date and/or time'\n\
              'min' .. 'max'\n\
              'min' - 'max'\n\
-T, --exclude-created = | < | > 'date and/or time'\n\
                      'min' .. 'max'\n\
                      'min' - 'max'\n\
        Filter only (or exclude) files that was created on specified date[s] and/or time[s].\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
";
let ACCESSED="\
-a, --accessed = | < | > 'date and/or time'\n\
               'min' .. 'max'\n\
               'min' - 'max'\n\
-A, --exclude-accessed = | < | > 'date and/or time'\n\
                        'min' .. 'max'\n\
                        'min' - 'max'\n\
        Filter only (or exclude) files that was accessed on specified date[s] and/or time[s].\n\
        You can specifying this multiple times, for both single value or range values.\n\
        For more information about 'date and/or time' possible values, see below.\n\
";
let MODIFIED="\
-m, --modified = | < | > 'date and/or time'\n\
               'min' .. 'max'\n\
               'min' - 'max'\n\
-M, --exclude-modified = | < | > 'date and/or time'\n\
                       'min' .. 'max'\n\
                       'min' - 'max'\n\
        Filter only (or exclude) files that was modified on specified date[s] and/or time[s].\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
";
let CHANGED="\
-g, --changed = | < | > 'date and/or time'\n\
              'min' .. 'max'\n\
              'min' - 'max'\n\
-G, --exclude-changed = | < | > 'date and/or time'\n\
                      'min' .. 'max'\n\
                      'min' - 'max'\n\
        Filter only (or exclude) files that was changed on specified date[s] and/or time[s].\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
";
let OUT_FILENAME="\
-f, --out-filename 'filename'\n\
        Override output playlist filename, by default it will crreate \".m3u8\" playlist.\n\
        To create \".pls\", \".wpl\", or others type of playlist, pass it as extension filename.\n\
            Example: --out-filename=my_playlist.xspf.\n\
        To convert or grab files inside another playlist file, \n\
        first add playlist ext[s] and media file ext[s] you desired into --ext.\n\
            Example: --ext=mp4,pls,wpl,xspf another.pls another.wpl another.xspf\n\
        Here the example to convert from different type of playlist to another type:\n\
            Example: --ext=mp3,m3u old_musics.m3u --out-filename=old_music.html\n\
";
let NO_OUTPUT_FILE="\
-F, --no-ouput-file [yes | no]\n\
        Choose to create playlist file or no. Default 'yes' if option was declared or if was build as library.\n\
";
let HELP_REST="\
\n\
Arguments can be surrounded by \"\" (eg. \"--verbose;benchmark\") to enable using character ';' or <ENTER> as multiline or another characters that belongs to Terminal or shell. To view how arguments was deduced you can pass option --debug=args.\n\
Options can be joined, and option assignment separator [SPACE] can be replaced with '=' \
and options can be separated by ':' or ';' after assignment. For the example:\n\n\
  tvplaylist -hOVvc=async:xs<1.3gb:e=mp4,mkv:f=My-playlist.txt\n\n\
Thats it, -h -O -V -v -c are joined, and -c has assignment operator '=' instead of\
 using separator [SPACE]. \
Also -x -s are joined, and -x is continuing with ':' after option assignment \
'=async' and -s has remove [SPACE] separator for operator '<' and value '1.3gb'.\n\n\
Redefinition of option means, it will use the last option, except for options that marked to be enable to specified multiple times. For the example, \
this example will not use 'thread' execution at all':\n\n\
  tvplaylist -c=thread /usr/local/videos -c=none /Users/Shared/Videos\n\n\
Note, you cannot join mnemonic option with full option, for the example:\n\n\
  tvplaylist -bO:ext=mp3:version\t\tWONT WORK\n\n\
Instead try to separate mnemonic and full option, like this:\n\n\
  tvplaylist -bO --ext=mp3:version\n\n\
";
let HELP_DATE_REST="\
Posible value for 'date and/or time' are: 'Year', 'Month Name' (mnemonic or full), 'Month Number', 'Day', 'WeekDay Name' (mnemonic or full), 'Hour' (if AM/PM defined then it is 12 hours, otherwise 24 hours), 'Minute', 'Second', AM or PM, now, today.\n\
Example:\n\
  Filter only files created in 2009:\n\
	 --created=2009\n\
  Filter only files created on Friday:\n\
	--created friday  OR  --created==friday\n\
  Filter only files created in 2009 with weekday is Sunday:\n\
	 --created=\"Sunday 2009\"  OR  --created=sun/2009\n\
  Filter only files created from November 2019 thru January 2021:\n\
	 --created \"nov 2019\" .. \"jan 2021\"\n\
  Filter only files created at January - March 1980 and May - June 2000 and after 2022:\n\
	 --created=jan/1980..march/1980:created 2000/may - jun/2000 --created>2022\n\
  Filter only files created after March 22 1990 20:44:\n\
	 --created>\"3/22/2019 20:44\"  OR  --created \"20:44 22/jan/2019\"\n\
It's up to you how to arrange date and/or time, At least you should use common time format (eg: 23:5:30). Here the possible arrangement you can use: \n\
Common normal: \"Monday Jan 15 2022 7:00 PM\"\n\
               \"Mon, 15/january/2022 7:0:0 pm\"\n\
               \"Monday 1/15/2022 19:0\"\n\
Equal and Acceptable: \"15 pm mon 7:0 2022/jan\"\n\
If you want to test 'date and/or time' use --debug=date 'date and/or time' ['format'], for the example:\n\
	 --debug=date \"pm 3:33\"\n\
	 or --debug=date now \"Date: %F, Time: %T.\"\n\
It will showing internal tvplaylist date time recognizer, with format \"Weekday Year/Month/Day Hour:Minute:Second\".\n\
";

#define CString											char*
#define ReadOnlyCString									const char* const
typedef std::vector<std::string> 						ListString;
typedef std::vector<std::filesystem::path> 				ListPath;
typedef std::vector<std::filesystem::directory_entry> 	ListEntry;
typedef std::vector<std::thread> 						ListThread;
typedef std::vector<std::future<void>> 					ListAsync;
typedef std::vector<std::regex> 						ListRegex;
typedef std::vector<int> 								ListInt;
typedef std::vector<unsigned short> 					ListUShort;
typedef std::vector<short> 								ListShort;
typedef std::vector<unsigned> 							ListUInt;
typedef std::vector<std::pair<uintmax_t, uintmax_t>> 	ListPaitUIntMax;
typedef std::vector<std::pair<float, std::string>> 		ListPairFloatString;
struct 	Date;
typedef std::vector<std::pair<char, Date>> 				ListPairCharDate;
typedef std::vector<std::pair<Date, Date>> 				ListPairDate;
typedef unsigned long long 								MAXNUM;
typedef std::vector<MAXNUM> 							ListMAXNUM;
typedef std::unordered_set<std::string> 				SetString;
typedef std::unordered_set<std::string_view> 			SetStringView;
typedef std::unordered_map<std::string, std::string> 	MapString;
typedef std::unordered_map<std::string, std::shared_ptr<ListPath>>
														MapStringPListPath;
#define ARRAYLEN(x)	(sizeof(x)/sizeof(x[0]))

constexpr ReadOnlyCString* OPTS[] = { &OPT_VERSION, &OPT_HELP, &OPT_ARRANGEMENT,
	&OPT_SEARCH, &OPT_VERBOSE, &OPT_BENCHMARK, & OPT_OVERWRITE,
	&OPT_SKIPSUBTITLE, &OPT_OUTDIR, &OPT_CURRENTDIR, &OPT_ADSDIR, &OPT_ADSCOUNT,
	&OPT_EXECUTION, &OPT_LOADCONFIG, &OPT_WRITEDEFAULTS, &OPT_SHOWCONFIG,
	&OPT_LIST, &OPT_OUTFILENAME, &OPT_NOOUTPUTFILE, &OPT_OPEN, &OPT_OPENWITH,
	&OPT_SIZE, &OPT_EXCLSIZE, &OPT_EXT, &OPT_EXCLEXT,
	&OPT_FIND, &OPT_EXCLFIND, &OPT_REGEX, &OPT_EXCLREGEX, &OPT_EXCLHIDDEN,
	&OPT_INSTALL, &OPT_UNINSTALL, &OPT_UPDATE,
	&OPT_DATE, &OPT_EXCLDATE,
	&OPT_DCREATED, &OPT_DMODIFIED, &OPT_DACCESSED, &OPT_DCHANGED,
	&OPT_DEXCLCREATED, &OPT_DEXCLMODIFIED, &OPT_DEXCLACCESSED, &OPT_DEXCLCHANGED,
	
	/// Ignored
	&OPT_DEBUG
};

/// Conjunction with OPTS, to enable access OPTS[index] == HELPS[index]
constexpr ReadOnlyCString* HELPS[] = { &VERSION, &A_HELP, &ARRANGEMENT,
	&SEARCH, &VERBOSE, &BENCHMARK, & OVERWRITE,
	&SKIP_SUBTITLE, &OUT_DIR, &CURRENT_DIR, &ADS_DIR, &ADS_COUNT,
	&EXECUTION, &LOAD, &WRITE, &SHOW, &LIST, &OUT_FILENAME,
	&NO_OUTPUT_FILE, &OPEN, &OPEN_WITH, &SIZE, &SIZE, &EXT, &EXT,
	&FIND, &FIND, &REGEX, &REGEX, &EXCLUDE_HIDDEN,
	&INSTALL, &UNINSTALL, &UPDATE,
	&DATE, &DATE,
	&CREATED, &MODIFIED, &ACCESSED, &CHANGED,
	&CREATED, &MODIFIED, &ACCESSED, &CHANGED,
	
	/// Ignored
	&HELP_REST, &HELP_DATE_REST
};

static_assert((ARRAYLEN(OPTS) - 1) == (ARRAYLEN(HELPS) - 2),
			  "Size need to be equal!, to be able accessed by index");

constexpr ReadOnlyCString* SINGLE_VALUE_OPT[] = {&OPT_LOADCONFIG, &OPT_SHOWCONFIG,
	&OPT_ARRANGEMENT, &OPT_OPEN, &OPEN_WITH, &OPT_CURRENTDIR,
	&OPT_VERBOSE, &OPT_BENCHMARK, &OPT_OVERWRITE, &OPT_SKIPSUBTITLE,
	&OPT_OUTDIR, &OPT_ADSCOUNT, &OPT_EXECUTION, &OPT_OUTFILENAME, &OPT_EXCLHIDDEN,
	&OPT_EXT, &OPT_EXCLEXT, &OPT_LIST};

[[maybe_unused]]
constexpr ReadOnlyCString* MULTI_VALUE_OPT[] = {
	&OPT_ADSDIR,
	&OPT_SIZE, &OPT_EXCLSIZE,
	&OPT_FIND, &OPT_EXCLFIND, &OPT_REGEX, &OPT_EXCLREGEX,
	&OPT_DATE, &OPT_EXCLDATE,
	&OPT_DCREATED, &OPT_DMODIFIED, &OPT_DACCESSED, &OPT_DCHANGED,
	&OPT_DEXCLCREATED, &OPT_DEXCLMODIFIED, &OPT_DEXCLACCESSED, &OPT_DEXCLCHANGED,};

constexpr ReadOnlyCString* ALL_HELPS[] = {
	&HELP, &A_HELP, &A_VERSION, &LOAD, &WRITE, &SHOW, &LIST, &ARRANGEMENT, &SEARCH, &VERBOSE, &BENCHMARK,
	&OVERWRITE, &SKIP_SUBTITLE, &OUT_DIR, &CURRENT_DIR, &ADS_DIR, &ADS_COUNT, &EXECUTION, &OUT_FILENAME,
	&NO_OUTPUT_FILE, &OPEN, &OPEN_WITH, &EXCLUDE_HIDDEN,
	
	&SIZE, &EXT, &FIND, &REGEX, &DATE, &CREATED, &MODIFIED, &ACCESSED, &CHANGED,
	
	&INSTALL, &UNINSTALL, &UPDATE,

	&HELP_REST, &HELP_DATE_REST
};

//#include <format>

#if MAKE_LIB
#include "libtvplaylist.h"
#endif

#define var  auto
#define func auto

func tolower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
				// static_cast<int(*)(int)>(std::tolower)         // wrong
				// [](int c){ return std::tolower(c); }           // wrong
				// [](char c){ return std::tolower(c); }          // wrong
				   [](unsigned char c){ return std::tolower(c); } // correct
				  );
	return s;
}

func toupper(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
				   [](unsigned char c){ return std::toupper(c); }
				  );
	return s;
}


func transformWhiteSpace(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return std::isspace(c) ? ' ' : c;
	});
	return s;
}

func groupNumber(std::string number)
{
	var i { (long long) number.find_last_of('.') };
	if (i == std::string::npos) {
		i = number.find_last_of('e');
		if (i == std::string::npos)
			i = number.size();
	}
	
	var a { number.size() > 1 and not std::isdigit(number[0]) ? 1
			: ( number.size() > 2 and std::tolower(number[1]) == 'x' ? 2
			   : 0) };
	
	while ( i - 3 > a ) {
		i-=3;
		number.insert(i, 1, ',');
	}
	
	return number;
}

func trim(const std::string& s)
{
	var start { (unsigned long) 0 }, end{ s.size() };
	for (var i { 0 }; i<end; ++i)
		if (not std::isspace(s[i])) {
			start=i;
			break;
		}
	for (var i { end }; i>start; --i)
		if (not std::isspace(s[i])) {
			end=i;
			break;
		}
	return s.substr(start, end);
}

enum class IgnoreCase { None, Both, Left, Right };

/// Is left contains right
/// Parameters:
/// - ignoreChar usage, for example for {'.', ' '} -> "La.la.Li" equal "La La Li"
func isContains(const std::string& l, const std::string& r,
				const IgnoreCase ic = IgnoreCase::None,
				const std::pair<char, char>* const ignoreChar = nullptr)
{
	if (r.size() > l.size()) return std::string::npos;
	
	var k { (unsigned) 0 };
	var i { (unsigned) 0 };
	if (l.size() > r.size())
		for (; i<l.size(); ++i) {
			var match { ignoreChar and
				(  (l[i] == ignoreChar->first 	and r[k] == ignoreChar->second)
				or (l[i] == ignoreChar->second 	and r[k] == ignoreChar->first)
				or (l[i] == ignoreChar->first 	and r[k] == ignoreChar->first)
				or (l[i] == ignoreChar->second 	and r[k] == ignoreChar->second)
				)};
			if (not match)
				switch (ic) {
					case IgnoreCase::Both:
						match = std::tolower(l[i]) == std::tolower(r[k]);
						break;
					case IgnoreCase::Left:
						match = std::tolower(l[i]) == r[k];
						break;
					case IgnoreCase::Right:
						match = l[i] == std::tolower(r[k]);
						break;
					default:
						match = l[i] == r[k];
						break;
				}
			if (match and ((k == 0 and l.size() - i >= r.size()) or k > 0)) {
				if (++k == r.size())
					break;
			} else
				k = 0;
		}
	
	return k == r.size() ? i - k + 1 : std::string::npos;
}

func isEqual(ReadOnlyCString l, ReadOnlyCString r,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long max = 0)
{
	if (not l or not r) return false;
	const var sz1 { std::strlen(l) };
	const var sz2 { std::strlen(r) };
	if (max == 0 and sz1 not_eq sz2) return false;
	var total { max > 0 ? max : sz1 };
	
	const CString _l = l;
	const CString _r = r;
	while (total-- > 0) {
		switch (ic){
			case IgnoreCase::Both:
				if (std::tolower(*_l) not_eq std::tolower(*_r))
					return false;
				break;
			case IgnoreCase::Left:
				if (std::tolower(*_l) not_eq *_r)
					return false;
				break;
			case IgnoreCase::Right:
				if (*_l not_eq std::tolower(*_r))
					return false;
				break;
			default:
				if (*_l not_eq *_r)
					return false;
		}
		_l++;
		_r++;
	}
	return true;
}

func isEqual(const std::string& l, const std::string& r,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long startLeft = 0,
			 const unsigned long startRight = 0,
			 const unsigned long max = 0)
{
	const var sizeLeft 	{ l.size() - startLeft };
	const var sizeRIght { r.size() - startRight };
	if (max == 0 and sizeLeft not_eq sizeRIght) return false;
	var total { std::max(sizeLeft, sizeRIght) };
	for (var i { 0 }; i<(max > 0 ? std::min(max, total) : total); ++i)
		switch (ic) {
			case IgnoreCase::Both:
				if (std::tolower(l[i + startLeft]) not_eq std::tolower(r[i + startRight]))
					return false;
				break;
			case IgnoreCase::Left:
				if (std::tolower(l[i + startLeft]) not_eq r[i + startRight])
					return false;
				break;
			case IgnoreCase::Right:
				if (l[i + startLeft] not_eq std::tolower(r[i + startRight]))
					return false;
				break;
			default:
				if (l[i + startLeft] not_eq r[i + startRight])
					return false;
		}
	return true;
}

func isEqual(ReadOnlyCString source,
			 const std::initializer_list<const CString>& list,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long startLeft = 0,
			 const unsigned long startRight = 0,
			 const unsigned long max = 0)
{
	for (var& s : list)
		if (isEqual(source, s, ic, startLeft, startRight, max))
			return true;

	return false;
}

template <template <class ...> class Container, class ... Args>
func isEqual(const std::string& source,
			 const Container<std::string, Args ...>* const args,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long startLeft = 0,
			 const unsigned long startRight = 0,
			 const unsigned long max = 0)
{
	for (var& check : *args)
		if (isEqual(source, check, ic, startLeft, startRight, max))
			return true;

	return false;
}

func isInt(const std::string& s, int* const value = nullptr)
{
	var result { 0 };
	if (const var [p, ec] { std::from_chars(s.c_str(), s.c_str()+s.size(), result) };
		ec == std::errc()) {
		if (value)
			*value = result;
		return true;
	}
	
	return false;
}

func getLikely(const std::string_view& src, const std::string_view& with)
{
	var first { src.begin() }, last { src.end() };
	var d_first { with.begin() };
	var score { 0.f }, scoreOpenent { 0.f };
	if (first not_eq last) {
		decltype (d_first) d_last = std::next(d_first, std::distance(first, last));
		for (decltype (first) i = first; i not_eq last; ++i) {
			if (i not_eq std::find(first, i, *i))
				continue;
			scoreOpenent += std::count(d_first, d_last, *i);
			score += std::count(i, last, *i);
		}
	}
	
	return scoreOpenent / score * 100;
}

func printHelp(ReadOnlyCString arg = nullptr)
{
	func print{[](const int i) {
		std::cout << *HELPS[i];
		if (isEqual(*OPTS[i], { OPT_DATE, OPT_EXCLDATE, OPT_DCREATED,
			OPT_DEXCLCREATED, OPT_DCHANGED, OPT_DEXCLCHANGED, OPT_DMODIFIED,
			OPT_DEXCLMODIFIED, OPT_DACCESSED, OPT_DEXCLACCESSED}))
			std::cout << HELP_DATE_REST;
	}};
	if (var i { 0 }; arg) {
		for (var& opt : OPTS) {
			if (isEqual(arg, *opt)) {
				if (isEqual(arg, OPT_HELP))
					for (var& help : ALL_HELPS)
						std::cout << *help;
				else
					print(i);
				break;
			}
			i++;
			if (i >= (ARRAYLEN(OPTS) - 1)) {
				var found { ListString() };
				var k { 0 };
				var indexFound { k };
				for (var& opt : OPTS) {
					if (getLikely(arg, *opt) > 80) {
						found.emplace_back(*opt);
						indexFound = k;
					}
					k++;
				}
				if (found.size() == 1) {
					print(indexFound);
				}
				else {
					var foundIndexes { ListInt() };
					if (found.empty()) {
						for (var h { 0 }; ; ++h) {
							if (h >= (ARRAYLEN(ALL_HELPS) - 2))
								break;
							if (isContains(*ALL_HELPS[h], arg, IgnoreCase::Both)
								not_eq std::string::npos)
								foundIndexes.emplace_back(h);
						}
					}
					
					if (foundIndexes.empty())
						std::cout << "⚠️ No option named \""
									<< arg
									<< '\"';
					else {
						std::cout << "Found "
									<< foundIndexes.size()
									<< " result containing \""
									<< arg
									<< "\":\n\n";
						for (var& id : foundIndexes) {
							print(id);
							std::cout << '\n';
						}
					}
						
					if (foundIndexes.empty() and not found.empty()) {
						std::cout << ", do you mean ";
						for (var k { 0 }; k<found.size(); ++k)
							std::cout << '\"'
										<< found[k]
										<< '\"'
										<< (k + 1 == found.size() ? "." : " or ");
					}
				}
				std::cout << '\n';
				break;
			}
		}
	}
}



func containInts(const std::string& s, ListMAXNUM* const out)
{
	var buffer { std::string() };
	func make{[&]() -> void {
		if (buffer.size() > 0 and buffer.size() < 19) /// ULLONG_MAX length is 20
			out->emplace_back(std::stoull(std::move(buffer)));
	}};
	
	for (var& c : s) {
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
func excludeExtension(const fs::path& path)
{
	return path.string().substr(0,
					path.string().size() - path.extension().string().size());
}

/// Automatically lower case in result
template <template <class ...> class Container, class ... Args>
func parseExtCommaDelimited(const std::string& literal,
							Container<std::string, Args...>* const result)
{
	var buffer { std::string() };
	
	func emplace{[&buffer, &result]() {
		if (buffer.size() > 2 and buffer[0] == '*' and buffer[1] == '.')
			buffer.erase(buffer.begin());
		else if (buffer.size() > 1 and buffer[0] not_eq '.')
			buffer.insert(buffer.begin(), '.');

		if (buffer.empty())
			return;
		
		std::fill_n(std::inserter(*result, result->end()), 1, std::move(buffer));
		buffer = "";
	}};
	
	for (var& c : literal) {
		if (std::isspace(c))
			continue;
		else if (c == ',')
			emplace();
		else
			buffer += std::tolower(c);
	}
	
	emplace();
}

#define property

struct Date
{
	property unsigned short weekday;
	property unsigned short year;
	property unsigned short month;
	property unsigned short day;
	property unsigned short hour;
	property unsigned short minute;
	property unsigned short second;
	
	friend
	func operator < (const Date& l, const Date& r) -> bool {
		var ull { Date::get_ull(l, r) };
		return ull.first < ull.second;
	}
	
	friend
	func operator > (const Date& l, const Date& r) -> bool {
		var ull { Date::get_ull(l, r) };
		return ull.first > ull.second;
	}
	
	friend
	func operator <= (const Date& l, const Date& r) -> bool {
		var ull { Date::get_ull(l, r) };
		return ull.first <= ull.second;
	}
	
	friend
	func operator >= (const Date& l, const Date& r) -> bool {
		var ull { Date::get_ull(l, r) };
		return ull.first >= ull.second;
	}
	
	friend
	func operator == (const Date& l, const Date& r) -> bool {
		#define is_equal(x, y) (x not_eq 0 and y not_eq 0 ? x == y : true)
		return (is_equal(l.year, r.year)
			and is_equal(l.month, r.month)
			and is_equal(l.day, r.day)
			and is_equal(l.hour, r.hour)
			and is_equal(l.minute, r.minute)
			and is_equal(l.second, r.second))
			or is_equal(l.weekday, r.weekday)
			;
		#undef is_equal
	}
	
	func time_t(const Date* const d = nullptr) {
		var tm { std::tm() };
		if ((d ? d : this)->year > 0)
			tm.tm_year = (d ? d : this)->year-1900;
		if ((d ? d : this)->month > 0)
			tm.tm_mon = (d ? d : this)->month-1; // February
		if ((d ? d : this)->day > 0)
			tm.tm_mday = (d ? d : this)->day;
		tm.tm_hour = (d ? d : this)->hour;
		tm.tm_min = (d ? d : this)->minute;
		tm.tm_sec = (d ? d : this)->second;
		if ((d ? d : this)->weekday > 0)
			tm.tm_wday = (d ? d : this)->weekday;
		return std::mktime(&tm);
	}

	func string(ReadOnlyCString format = nullptr, const bool UTC = false) const {
		var s { std::string() };
		if (format and std::strlen(format) > 0) {
			var tm { std::tm() };
			if (year > 0)
				tm.tm_year = year-1900;
			if (month > 0)
				tm.tm_mon = month-1; // February
			if (day > 0)
				tm.tm_mday = day;
			tm.tm_hour = hour;
			tm.tm_min = minute;
			tm.tm_sec = second;
			if (weekday > 0)
				tm.tm_wday = weekday;
			var t { std::mktime(&tm) };
			char mbstr[100];
			if (std::strftime(mbstr, sizeof(mbstr), format,
							  UTC ? std::gmtime(&t) : std::localtime(&t)))
				s = mbstr;
		} else {
			if (weekday > 0)
				s.append(getWeekDayName(weekday));
			
			if (year > 0 or month > 0 or day > 0) {
				s.append((s.empty() ? "" : " ") + (year > 0 ? std::to_string(year) : "?"));
				s.append("/" + (month > 0 ? std::to_string(month) : "?"));
				s.append("/" + (day > 0 ? std::to_string(day) : "?"));
			}
			if (hour > 0 or minute > 0 or second > 0) {
				s.append((s.empty() ? "" : " ") + std::to_string(hour));
				s.append(":" + std::to_string(minute));
				s.append(":" + std::to_string(second));
			}
		}
		return s;
	}
	
	static
	func getWeekDayName(const unsigned short index, ReadOnlyCString locale = nullptr)
		-> std::string
	{
		if (index >= 1 or index <= 7) {
			var tm { std::tm() };
			tm.tm_year = 2020-1900; // 2020
			tm.tm_mon = 2-1; // February
			tm.tm_mday = 8 + index; // 15th
			std::time_t t { std::mktime(&tm) }; //std::time(nullptr);
			
			char mbstr[20];
			std::setlocale(LC_ALL, locale ? locale : std::locale().name().c_str());
			if (std::strftime(mbstr, sizeof(mbstr), "%A", std::localtime(&t)))
				return std::string(mbstr);
		}
		return std::string();
	}
	
	static
	func getWeekDayNames(const CString result[7], ReadOnlyCString locale = nullptr)
	{
		for (var i { 9 }, k { 0 }; i<16; ++i, ++k) {
			var tm { std::tm() };
			tm.tm_year = 2020-1900; // 2020
			tm.tm_mon = 2-1; // February
			tm.tm_mday = i; // 15th
			var t { std::mktime(&tm) }; //std::time(nullptr);
			
			char mbstr[3];
			std::setlocale(LC_ALL, locale ? locale : std::locale().name().c_str());
			if (std::strftime(mbstr, sizeof(mbstr), "%a", std::localtime(&t)))
				result[k] = mbstr;
		}
	}

	static
	func getWeekDayIndex(const int aYear = 0,
						 const unsigned short aMonth = 0,
						 const unsigned short aDay = 0,
						 const unsigned short expectedWeekDay = 0)
	{
		for (var Y { aYear > 0 ? aYear : 1970 };
			 Y <= aYear > 0 ? aYear : 1970; ++Y)
		{
			var curr_year { std::chrono::year(Y) };
			
			for (var cur_month{ aMonth > 0 ? aMonth : 1 };
				 cur_month not_eq (aYear > 0 ? aMonth + 1 : 13); ++cur_month)
			
				for (var d { aDay > 0 ? aDay : 1 };
					 d<(aDay > 0 ? aDay + 1 : 32); ++d)
				{
					const var ymd
						{ std::chrono::year_month_day(curr_year / cur_month / d) };
					const var cur_weekday
						{ std::chrono::weekday(std::chrono::sys_days{ ymd }) };
					
					if (expectedWeekDay > 0) {
						if (cur_weekday.iso_encoding() == expectedWeekDay)
							return cur_weekday.iso_encoding();
					} else
						return cur_weekday.iso_encoding();
				}
		}
			
		return unsigned(0);
	}

private:
	let WEEKDAYS = {
	/*en_US.UTF-8:*/ "sun", "mon", "tue", "wed", "thu", "fri", "sat",
	/*id_ID       */ "min", "sen", "sel", "rab", "kam", "jum", "sab",
	/*de_DE.UTF-8:*/ "so", "mo", "di", "mi", "do", "fr", "sa",
	/*fr_FR.UTF-8:*/ "dim", "lun", "mar", "mer", "jeu", "ven", "sam",
	/*sv_SE.UTF-8:*/ "sön", "mån", "tis", "ons", "tor", "fre", "lör",
	/*es_ES.UTF-8:*/ "dom", "lun", "mar", "mié", "jue", "vie", "sáb",
	/*ru_RU.UTF-8:*/ "вс", "пн", "вт", "ср", "чт", "пт", "сб",
	/*ja_JP.UTF-8:*/ "日", "月", "火", "水", "木", "金", "土",
	/*zh_CN.UTF-8:*/ "日", "一", "二", "三", "四", "五", "六",
	/*id_ID.UTF-8:*/ "日", "一", "二", "三", "四", "五", "六",


	/*en_US.UTF-8:*/ "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday",
	/*id_ID       */ "minggu", "senin", "selasa", "rabu", "kamis", "jumat", "sabtu",
	/*de_DE.UTF-8:*/ "sonntag", "montag", "dienstag", "mittwoch", "donnerstag", "freitag", "samstag",
	/*fr_FR.UTF-8:*/ "dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi",
	/*sv_SE.UTF-8:*/ "söndag", "måndag", "tisdag", "onsdag", "torsdag", "fredag", "lördag",
	/*es_ES.UTF-8:*/ "domingo", "lunes", "martes", "miércoles", "jueves", "viernes", "sábado",
	/*ru_RU.UTF-8:*/ "воскресенье", "понедельник", "вторник", "среда", "четверг", "пятница", "суббота",
	/*ja_JP.UTF-8:*/ "日曜日", "月曜日", "火曜日", "水曜日", "木曜日", "金曜日", "土曜日",
	/*zh_CN.UTF-8:*/ "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六",
	/*id_ID.UTF-8:*/ "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六",
	};
	
	let MONTHS = {
	/*en_US.UTF-8:*/ "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec",
	/*id_ID       */ "jan", "feb", "mar", "apr", "mei", "jun", "jul", "agu", "sep", "okt", "nov", "des",
	/*de_DE.UTF-8:*/ "jan", "feb", "mär", "apr", "mai", "jun", "jul", "aug", "sep", "okt", "nov", "dez",
	/*fr_FR.UTF-8:*/ "jan", "fév", "mar", "avr", "mai", "jui", "jul", "aoû", "sep", "oct", "nov", "déc",
	/*sv_SE.UTF-8:*/ "jan", "feb", "mar", "apr", "maj", "jun", "jul", "aug", "sep", "okt", "nov", "dec",
	/*es_ES.UTF-8:*/ "ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic",
	/*ru_RU.UTF-8:*/ "янв", "фев", "мар", "апр", "май", "июн", "июл", "авг", "сен", "окт", "ноя", "дек",

	/*en_US.UTF-8:*/ "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december",
	/*id_ID       */ "januari", "februari", "maret", "april", "mei", "juni", "juli", "agustus", "september", "oktober", "november", "desember",
	/*de_DE.UTF-8:*/ "januar", "februar", "märz", "april", "mai", "juni", "juli", "august", "september", "oktober", "november", "dezember",
	/*fr_FR.UTF-8:*/ "janvier", "février", "mars", "avril", "mai", "juin", "juillet", "août", "septembre", "octobre", "novembre", "décembre",
	/*sv_SE.UTF-8:*/ "januari", "februari", "mars", "april", "maj", "juni", "juli", "augusti", "september", "oktober", "november", "december",
	/*es_ES.UTF-8:*/ "enero", "febrero", "marzo", "abril", "mayo", "junio", "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre",
	/*ru_RU.UTF-8:*/ "января", "февраля", "марта", "апреля", "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря",
	/*ja_JP.UTF-8:*/ "1月", "2月", "3月", "4月", "5月", "6月", "7月", "8月", "9月", "10月", "11月", "12月",
	/*zh_CN.UTF-8:*/ "一月", "二月", "三月", "四月", "五月", "六月", "七月", "八月", "九月", "十月", "十一月", "十二月",
	};
	
	static
	func get_ull(const Date& l, const Date& r)
					-> std::pair<unsigned long long, unsigned long long>
	{
		var sleft { std::string() }, sright { std::string() };
		func go{[&sleft, &sright](const unsigned short& x, const unsigned short& y)
		{
			if (x == 0 or y == 0) return;
			
			var sx { std::to_string(x) };
			if (sx.size() < 2) sx.insert(0, 1, '0');
			sleft.append(sx);
			var sy { std::to_string(y) };
			if (sy.size() < 2) sy.insert(0, 1, '0');
			sright.append(sy);
		}};
		
		go(l.year, r.year);
		go(l.month, r.month);
		go(l.day, r.day);
		go(l.hour, r.hour);
		go(l.minute, r.minute);
		go(l.second, r.second);
		go(l.weekday, r.weekday);
		
		return std::make_pair(std::stoull(sleft), std::stoull(sright));
	}
	
	func fold() const
	{
		var Y { std::to_string(year) };
		var m { std::to_string(month) };
		if (m.size() not_eq 2) m.insert(0, 1, '0');
		var d { std::to_string(day) };
		if (d.size() not_eq 2) d.insert(0, 1, '0');
		var H { std::to_string(hour) };
		if (H.size() not_eq 2) H.insert(0, 1, '0');
		var M { std::to_string(minute) };
		if (M.size() not_eq 2) M.insert(0, 1, '0');
		var S { std::to_string(second) };
		if (S.size() not_eq 2) S.insert(0, 1, '0');
		
		return std::stoull(Y + m + d + H + M + S);
	}
	
	func setWeekday()
	{
		const var ymd
			{ std::chrono::year_month_day(std::chrono::year(year) / month / day) };
		const var aweekday
			{ std::chrono::weekday(std::chrono::sys_days{ ymd }) };
		
		if (weekday not_eq aweekday.iso_encoding())
			weekday = aweekday.iso_encoding();
	}
public:
	static
	func now()
	{
		const var now { std::chrono::time_point<std::chrono::system_clock>(
											std::chrono::system_clock::now()) };
		const var t { std::chrono::system_clock::to_time_t(now) };

		return Date(&t);
	}
	
	func set(const std::time_t* t, const bool UTC = false)
	{
		char a[5];
		unsigned short * unit[] = {&year, &month, &day, &hour, &minute, &second};
		for (var i { 0 }; var& f : {"%Y", "%m", "%d", "%H", "%M", "%S"}) {
			std::strftime(a, sizeof(a), f,
						  UTC ? std::gmtime(t) : std::localtime(t));
			*unit[i++] = std::stoi(a);
		}
		
		// Just not trust weekday returned from strftime("%u")
		setWeekday();
	}
	
	Date(const std::time_t* const t, const bool UTC = false) {
		set(t, UTC);
	}
		
	Date(const std::string& s): year{0}, month{0}, day{0}, weekday{0},
								hour{0}, minute{0}, second{0}
	{
		if (isEqual(s.c_str(), {"now", "today"}, IgnoreCase::Left)) {
			const var now { std::chrono::time_point<std::chrono::system_clock>(
											std::chrono::system_clock::now()) };
			const var t { std::chrono::system_clock::to_time_t(now) };
			set(&t);
			if (isEqual(s, "today")) {
				hour = 0;
				minute = 0;
				second = 0;
			}
				
			return;
		}
			
		func isDigit{[](const std::string& s)
		{
			for (var i { 0 }; i<s.size(); ++i)
				if (not isdigit(s[i]))
					return false;
			return s not_eq "";
		}};
		
		func isPM{[](const std::string& get)
		{
			if (get.size() == 2 and std::tolower(get[1]) == 'm') {
				if (std::tolower(get[0]) == 'a')
					return -1;
				else if (std::tolower(get[0]) == 'p')
					return -2;
			}
			return 0;
		}};
		var date { ListUShort() };
		var time { ListShort() };
		var others { ListString() };
		var last { '\0' };
		for (var i { (unsigned) 0 }, k { (unsigned) 0 }; i<s.size(); ++i)
			if (not std::isdigit(s[i])) {
				if (s[i] == ':') {
					const var get { s.substr(k, i - k) };
					if (isDigit(get))
						time.emplace_back(std::stoi(get));
					else if (not get.empty())
						others.emplace_back(get);
				} else if (std::ispunct(s[i])) {
					const var get { s.substr(k, i - k) };
					if (isDigit(get)) {
						if (last == ':')
							time.emplace_back(std::stoi(get));
						else
							date.emplace_back(std::stoi(get));
					} else if (not get.empty())
						others.emplace_back(get);
				} else if (std::isspace(s[i])) {
					if (k not_eq i) {
						const var get { s.substr(k, i - k) };
						const var isNumber { isDigit(get) };
						if (std::isalpha(last)) {
							if (const var pm { isPM(get) }; pm)
								time.emplace_back(pm);
							else if (not get.empty())
								others.emplace_back(get);
						} else if (last == ':' and isNumber)
							time.emplace_back(std::stoi(get));
						else if ((last == '\0' or std::ispunct(last)) and isNumber)
							date.emplace_back(std::stoi(get));
						else if (not get.empty())
							others.emplace_back(get);
					}
					last = '\0';
					k = i + 1;
					continue;
				} else {
					if (i == s.size() - 1) {
						const var get { s.substr(k) };
						if (const var pm { isPM(get) }; pm)
							time.emplace_back(pm);
						else if (not get.empty())
							others.emplace_back(get);
					}
					if (last == '\0')
						last = s[i];
					continue;
				}
				
				last = s[i];
				k = i + 1;
				
			} else if (i == s.size() - 1) {
				const var get { s.substr(k) };
				const var isNumber { isDigit(get) };
				if (last == ':' and isNumber)
					time.emplace_back(std::stoi(get));
				else if ((std::ispunct(last) or last == '\0') and isNumber)
					date.emplace_back(std::stoi(get));
				else if (not get.empty())
					others.emplace_back(get);
			}
			
			
			
			
		var weekDay { -1 };
		for (var found { 0 };
			var& s : others)
		{
			s[0] = std::tolower(s[0]);
			
			if (weekDay == -1) {
				for (var i { 0 }; var& m : WEEKDAYS) {
					if (s == m)
					{
						weekDay = (i % 7) + 1;
						break;
					}
					i++;
				}
				
				if (weekDay not_eq -1) {
					assert(weekday == 0);
					weekday = weekDay;
					continue;
				}
			}
			
			for (var i { 0 };
				var& m : MONTHS) {
				if (s == m)
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
				std::cout << "⚠️  Cannot understand literal \""
							<< s
							<< "\"!.\n";
		}
		
		
		for (var& i : date)
			if (i > 32)
				year = i;
			else if (i <= 12) {
				if (month == 0)
					month = i;
				else {
					if (day == 0)
						day = i;
					else if (year == 0)
						year = i;
					else //if (time.empty())
						time.emplace_back(i);
				}
			} else if (i > 12) {
				if (day == 0)
					day = i;
				else {
					if (year == 0)
						year = i;
					else //if (time.empty())
						time.emplace_back(i);
				}
			} else //if (time.empty())
					time.emplace_back(i);
			
		if (year > 0 and year < 1000) {
			const std::chrono::year_month_day ymd{floor<std::chrono::days>(std::chrono::system_clock::now())};
			const var Y { std::to_string(year) };
			const var nowy { std::to_string(int(ymd.year())) };
			year = std::stoi(nowy.substr(0, nowy.size() - Y.size()) + Y);
		}
			
		if (year > 0 and month > 0 and day > 0 and weekday == 0) {
			weekday = std::chrono::weekday{ std::chrono::sys_days{
				std::chrono::year_month_day{std::chrono::year(year)/month/day}
			}}.iso_encoding();
			
			if (weekDay > -1 and weekDay not_eq weekday)
				std::cout << "⚠️  Invalid Weekday was defined for "
							<< string("%B %d %Y")
							<< "!\n";
		}
			
		unsigned short* unit[] = {&hour, &minute, &second };
		var hasPM { false };
		for (var i { 0 }; var& t : time)
			if (t < 0) {
				hasPM = t == -2;
			} else if (i < ARRAYLEN(unit))
				*unit[i++] = t;
			else
				std::cout << "⚠️  Invalid unit time on "
							<< hour
							<< ':'
							<< minute
							<< ':'
							<< second
							<< '\n';
		if (hasPM)
			hour += 12;
	}

	func isValid() const -> bool
	{
		return not (year == 0 and month == 0 and day == 0 and hour == 0 and minute == 0 and second == 0 and weekday == 0)
		and (weekday <= 7 or year >= 10 or month <= 12 or day <= 33 or hour <= 24 or minute <= 60 or second <= 60);
	}

};

static const var SET_OF_ID3_TAGS {
	SetString {
	"id3",
	"version",								// both		2.3		2.4
	"audio_encryption", 					// AENC
	"attached_picture",						// APIC
	"audio_seek_point_index",				//					ASPI
	"comment", 								// COMM
	"commercial_frame",						// COMR
	"size",									//			TSIZ
	"encryption_method_registration", 		// ENCR
	"equalization",							//			EQUA	EQU2
	"event_timing_codes",					// ETCO
	"general_encapsulation_object",			// GEOB
	"group_identification_registration",	// GRID
	"linked_information", 					// LINK
	"music_cd_identifier",					// MCDI
	"mpeg_location_lookup_table",			// MLLT
	"ownership_frame", 						// OWNE
	"play_counter",							// PCNT
	"popularimeter", 						// POPM
	"position_sync_frame",					// POSS
	"private_frame",						// PRIV
	"recommended_buffer_size",				// RBUF
	"relative_volume_adjustment",			//			RVAD	RVA2
	"reverb",								// RVRB
	"seek_frame",							//					SEEK
	"signature_frame",						//					SIGN
	"sync_lyric",							// SYLT
	"sync_tempo_codes",						// SYTC
	"album", 								// TALB
	"bpm",									// TBPM
	"composer",								// TCOM
	"genre",								// TCON
	"copyright_message",					// TCOP
	"encoding_time",						//					TDEN
	"playlist_delay",						// TDLY
	"original_release_year",				//			TORY	TDOR
	"date",									// 			TDAT	TDRC
	"recording_date",						// 			TRDA	TDRC
	"recording_time",						//					TDRC
	"time",									//			TIME	TDRC
	"year", 								//			TYER	TDRC
	"release_time",							//					TDRL
	"tagging_time",							//					TDTG
	"encoder",								// TENC
	"lyric",								// TEXT
	"file_type",							// TFLT
	"involved_people_list",					//			IPLS	TIPL
	"content_group_description",			// TIT1
	"title",								// TIT2
	"subtitle_refinement",					// TIT3
	"initial_key",							// TKEY
	"language",								// TLAN
	"length",								// TLEN
	"musician_credits_list",				//					TMCL
	"media_type",							// TMED
	"mood",									//					TMOO
	"original_album_title",					// TOAL
	"original_filename",					// TOFN
	"original_lyricist_writer",				// TOLY
	"original_artist",						// TOPE
	"file_license",							// TOWN
	"artist",								// TPE1
	"band",									// TPE2
	"conductor_refinement",					// TPE3
	"remixed",								// TPE4
	"part_of_set",							// TPOS
	"produced_notice",						//					TPRO
	"publisher",							// TPUB
	"track",								// TRCK
	"internet_radio_station_name",			// TRSN
	"internet_radio_station_owner",			// TRSO
	"album_sort_order",						//					TSOA
	"performer_sort_order",					//					TSOP
	"title_sort_order",						//					TSOT
	"isrc",									// TSRC
	"encoding_setting",						// TSSE
	"set_subtitle",							//					TSST
	"user_defined_text_info_frame",			// TXXX
	"unique_file_identifier",				// UFID
	"term_of_use",							// USER
	"unsync_lyric_transcription",			// USLT
	"commercial_information",				// WCOM
	"copyright_information",				// WCOP
	"official_audio_file_webpage",			// WOAF
	"official_atist_webpage",				// WOAR
	"official_audio_source_webpage",		// WOAS
	"official_internet_radio_station_homepage", // WORS
	"payment",								// WPAY
	"publisher_official_webpage",			// WPUB
	"user_defined_url_link_frame"			// WXXX
}};
struct ID3
{
	static constexpr ReadOnlyCString GENRES[] = {
		"Blues", "Classic rock", "Country", "Dance", "Disco", "Funk", "Grunge",
		"Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop",
		"Rhythm and Blues", "Rap", "Reggae", "Rock", "Techno", "Industrial",
		"Alternative", "Ska", "Death metal", "Pranks", "Soundtrack", "Euro-Techno",
		"Ambient", "Trip-Hop", "Vocal", "Jazz & Funk", "Fusion", "Trance", "Classical",
		"Instrumental", "Acid", "House", "Game", "Sound clip", "Gospel", "Noise",
		"Alternative Rock", "Bass", "Soul", "Punk", "Space", "Meditative",
		"Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic", "Darkwave",
		"Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance", "Dream",
		"Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap",
		"Pop/Funk", "Jungle", "Native US", "Cabaret", "New Wave", "Psychedelic",
		"Rave", "Show tunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz",
		"Polka", "Retro", "Musical", "Rock ’n’ Roll", "Hard rock",
		"Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebop", "Latin",
		"Revival", "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", "Progressive Rock",
		"Psychedelic Rock", "Symphonic Rock","Slow rock", "Big Band", "Chorus",
		"Easy Listening", "Acoustic", "119", "Humour", "Speech", "Chanson", "Opera",
		"Chamber music", "Sonata", "Symphony", "Booty bass", "Primus", "Porn groove",
		"Satire", "Slow jam", "Club", "Tango", "Samba", "Folklore", "Ballad",
		"Power ballad", "Rhythmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum solo",
		"A cappella", "Euro-House", "Dancehall", "Goa", "Drum & Bass", "Club-House",
		"Hardcore Techno", "Terror", "Indie", "BritPop", "Negerpunk", "Polsk Punk",
		"Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal", "Crossover",
		"Contemporary Christian", "Christian rock", "Merengue", "Salsa", "Thrash Metal",
		"Anime", "Jpop", "Synthpop", "Abstract", "Art Rock", "Baroque", "Bhangra",
		"Big beat", "Breakbeat", "Chillout", "Downtempo", "Dub", "EBM", "Eclectic",
		"Electro", "Electroclash", "Emo", "Experimental", "Garage", "Global", "IDM",
		"Illbient", "Industro-Goth", "Jam Band", "Krautrock", "Leftfield", "Lounge",
		"Math Rock", "New Romantic", "Nu-Breakz", "Post-Punk", "Post-Rock", "Psytrance",
		"Shoegaze", "Space Rock", "Trop Rock", "World Music", "Neoclassical",
		"Audiobook", "Audio theatre", "Neue Deutsche Welle", "Podcast", "Indie-Rock",
		"G-Funk", "Dubstep", "Garage Rock", "Psybient",
	};

	
	func parseInput(const std::string& keyword) const
		-> std::shared_ptr<std::pair<std::string, std::string>>
	{
		const var pos { keyword.find('=') };
		var found { pos > 2 and pos not_eq std::string::npos };
		
		if (found) {
			const var selectedTag { keyword.substr(0, pos) };
			found = SET_OF_ID3_TAGS.find(selectedTag) not_eq SET_OF_ID3_TAGS.end();
			if (found) {
				std::string val { keyword.substr(pos + 1) };
				return std::make_shared<std::pair<std::string,
					std::string>>(std::make_pair(selectedTag, val));
			}
		}
		
		return nullptr;
	}
	
	func get(const std::string& tag) const
	{
		const var found { tags.find(tag) };
		if (found not_eq tags.end())
			return found->second;
		else
			return std::string();
	}
	
	func set(const std::string& tag, const std::string& value)
	{
		const var found { tags.find(tag) };
		if (found not_eq tags.end())
			found->second = value;
		else
			tags.emplace(std::make_pair(tag, value));
	}
	
	func add(const std::string& tag, const std::string& value)
	{
		tags.emplace(std::make_pair(tag, value));
	}
	
//	func operator [](const std::string& tag) -> std::string&
//	{
//		var found { tags.find(tag) };
//		if (found not_eq tags.end())
//			return found->second;
//		else {
//			var newItem { std::make_pair(tag, "") };
//			tags.emplace(std::move(newItem));
//			return tags.find(tag)->second;
//		}
//	}
	
	func operator [](const std::string& tag) const -> const std::string
	{
		return get(tag);
	}

	friend
	func operator % (const ID3& l, const std::string& keyword) -> bool
	{
		const var keyVal { l.parseInput(keyword) };
		if (keyVal) {
			if (keyVal->first == "id3") {
				for (var& tag : l.tags)
					if (isContains(	tag.second, keyVal->second,
									l.isCaseInsensitive ? IgnoreCase::Both
									: IgnoreCase::None)
						not_eq std::string::npos)
						return true;
			} else
				return isContains(l[keyVal->first], keyVal->second,
								  l.isCaseInsensitive ? IgnoreCase::Both
								  : IgnoreCase::None)
						not_eq std::string::npos;
		}
		
		return false;
	}
	
private:
	std::unordered_multimap<std::string_view, std::string> tags;
	const CString path;
	bool isCaseInsensitive;
	
	static
	func btoi(ReadOnlyCString bytes, const int size, const int offset)
	{
		var result { (unsigned int) 0x00 };
		for (var i { 0 }; i < size; ++i)
		{
			result = result << 8;
			result = result | (unsigned char) bytes[offset + i];
		}
		
		return result;
	}
	
	//#if 0
	static
	func itob(const int integer, CString result[4])
	{
		// We need to reverse the bytes because Intel uses little endian.
		let size { 4 };
		var aux { (CString) &integer };
		for (var i { size - 1 }; i >= 0; i--)
			*result[size - 1 - i] = aux[i];
	}
	
	static
	func syncint_encode(int value)
	{
		var out { 0 }, mask { 0x7F };

		while (mask ^ 0x7FFFFFFF) {
			out = value & ~mask;
			out <<= 1;
			out |= value & mask;
			mask = ((mask + 1) << 8) - 1;
			value = out;
		}

		return out;
	}
	//#endif
	
	static
	func syncint_decode(const int value)
	{
		unsigned int a, b, c, d, result = 0x0;
		a = value & 0xFF;
		b = (value >> 8) & 0xFF;
		c = (value >> 16) & 0xFF;
		d = (value >> 24) & 0xFF;
		
		result = result | a;
		result = result | (b << 7);
		result = result | (c << 14);
		result = result | (d << 21);
		
		return result;
	}
	
	struct V2 {
		let SZ_HEADER 				= 10;
		let SZ_TAG 					= 3;
		let SZ_VERSION 				= 1;
		let SZ_REVISION				= 1;
		let SZ_FLAGS 				= 1;
		let SZ_HEADER_SIZE 			= 4;
		let SZ_EXTENDED_HEADER_SIZE = 4;
		
		
		property char 	tag[SZ_TAG];
		property char 	majorVersion,
						minorVersion,
						flags;
		property int 	tag_size,
						extended_header_size;
		
		V2(ReadOnlyCString buffer) :
			majorVersion{0x0}, minorVersion{0x0}, flags{0x0},
			tag_size{0}, extended_header_size{0}
		{
			var position { 0 };
			std::memcpy(tag, buffer, SZ_TAG);
			majorVersion = buffer[position += SZ_TAG];
			minorVersion = buffer[position += SZ_VERSION];
			flags = buffer[position += SZ_REVISION];
			tag_size = syncint_decode(
				btoi(buffer, SZ_HEADER_SIZE, position += SZ_FLAGS));
			
			if ((flags & (1 << 6)) == (1 << 6))
				extended_header_size = syncint_decode(
					btoi(buffer, SZ_EXTENDED_HEADER_SIZE, position += SZ_HEADER_SIZE));
		}
		
		struct Frame {
			let SZ_HEADER 					= 10;
			let SZ_ID 						= 4;
			let SZ_SIZE 					= 4;
			let SZ_FLAGS 					= 2;
			let SZ_ENCODING 				= 1;
			let SZ_LANGUAGE 				= 3;
			let SZ_SHORT_DESCRIPTION 		= 1;
			
			let INVALID_FRAME 	= 0;
			let TEXT_FRAME 		= 1;
			let COMMENT_FRAME 	= 2;
			let APIC_FRAME 		= 3;
			
			let ISO_ENCODING 	= 0;
			let UTF_16_ENCODING = 1;
			
			property char id[SZ_ID];
			property char flags[SZ_FLAGS];
			property int size;
			property std::string data;
			
			func isValid() const
			{
				return size not_eq 0;
			}
			
			Frame(ReadOnlyCString raw, int offset, const int major_version) :
				size{0}
			{
				std::memcpy(id, raw + offset, SZ_ID);
				if (0 == std::memcmp(id, "\0\0\0\0", 4))
					return;
				size = btoi(raw, 4, offset += SZ_ID);
				if (major_version == 4)
					size = syncint_decode(size);
				
				std::memcpy(flags, raw + (offset += SZ_SIZE), 2);
				
				// Load frame data
				var tmp { raw };
				if (*(tmp + (offset + SZ_FLAGS)) == '\0')
					offset++;
				
				char _data[size];
				std::memcpy(_data, raw + (offset += SZ_FLAGS), size);
				_data[size - 1] = '\0';
				data = std::string(_data);
			}
		};
		#if 0
		property std::vector<Frame> frames;
		#endif
	};
	
public:
	
	func write(ReadOnlyCString a_path = nullptr)
	{
		if (not path and not a_path)
			return;
		var file { std::fstream(a_path ? a_path : path) };
//		if (not file.good()) {
//			std::cout << "⚠️  Cannot write \""
//						<< (a_path ? a_path : path)
//						<< "\"\n";
//			return;
//		}
		
		file.seekp(0, std::ios_base::end);
		const var end { (int) file.tellp() };
		
		file.seekp(end - 128);
		var pos { (int) file.tellp() };
		
		func set{[&file, &pos](ID3* const id3,
							   ReadOnlyCString key,
							   const int size,
							   ReadOnlyCString value = nullptr)
		{
			const CString currVal { nullptr };
			if (key and id3->tags.find(key) == id3->tags.end())
				id3->add(key, value ? value : "");
			else if (key)
				currVal = id3->get(key).c_str();
			
			var val { key ? (value ? value : (currVal ? currVal
							  : nullptr)) : nullptr };
			
			if (val) {
				if (size == 1) {
					char ONE[1];
					ONE[0] = std::stoi(val);
					file.write(ONE, size);
				} else
					file.write(val, size);
			}
			pos += size;
			file.seekp(pos);
		}};
		
		set(this, nullptr, 3);
		set(this, "title", 30);
		set(this, "artist", 30);
		set(this, "album", 30);
		set(this, "year", 4);
		set(this, "comment",  28);
		set(this, nullptr, 1);
		set(this, "track", 1);
		
		if (const var genre { get("genre") };
			not genre.empty())
			for (var i { 0 }; var& g : GENRES) {
				if (genre == g) {
					set(this, "genre", 1, std::to_string(i).c_str());
					break;
				}
				i++;
			}
		
		file.seekp(0);
		// TODO: Write v2.x
		
		file.close();
	}
	
	func string() const
	{
		var s { std::string() };
		
		for (var& tag : tags)
		{
			if (tag.second.empty())
				continue;
			s.append(tag.first);
			
			s += " : \"";
			s.append(tag.second);
			s += "\"\n";
		}
		
		return s;
	}
	
	ID3(): path{nullptr} { }
	
	ID3(ReadOnlyCString path, const bool case_insensitive = false)
	{
		this->path = path;
		isCaseInsensitive = case_insensitive;
		var file { std::ifstream(path, std::ios::in | std::ios::ate | std::ios::binary) };
		if (not file.good()) {
			this->path = nullptr;
			return;
		}
		
		file.seekg(0);

		if (char s_tag[V2::SZ_HEADER];
			file.read(s_tag, V2::SZ_HEADER)
			and isEqual(s_tag, "ID3", IgnoreCase::None, 3))
		{
			var tag { V2(s_tag) };
			if (char bytes[tag.tag_size + 10];
				
				tag.majorVersion not_eq 0
				and file.read(bytes, tag.tag_size + 10))
			{
				
				add("version", "2." + std::to_string(tag.majorVersion)
									+ (tag.minorVersion > 0
										? "." + std::to_string(tag.minorVersion)
										: ""));
				
				var isv24 { tag.majorVersion == 4 };
				var isv23 { tag.majorVersion == 3 };
				
				var offset{ tag.extended_header_size };
				
				while (offset < tag.tag_size)
				{
					var frame { V2::Frame(bytes, offset, tag.majorVersion) };
					if (frame.isValid())
					{
						offset += frame.size + 10;
						
						if (isEqual(frame.id, "AENC"))
							add("audio_encryption", frame.data);
						else if (isEqual(frame.id, "APIC"))
							add("attached_picture", frame.data);
						else if (isEqual(frame.id, "ASPI") and isv24)
							add("audio_seek_point_index", frame.data);
						else if (isEqual(frame.id, "COMM"))
							add("comment", frame.data);
						else if (isEqual(frame.id, "COMR"))
							add("commercial_frame", frame.data);
						else if (isEqual(frame.id, "TSIZ") and isv23)
							add("size", frame.data);
						else if (isEqual(frame.id, "ENCR"))
							add("encryption_method_registration", frame.data);
						else if (isEqual(frame.id, "EQU2") and isv24)
							add("equalization", frame.data);
						else if (isEqual(frame.id, "EQUA") and isv23)
							add("equalization", frame.data);
						else if (isEqual(frame.id, "ETCO"))
							add("event_timing_codes", frame.data);
						else if (isEqual(frame.id, "GEOB"))
							add("general_encapsulation_object", frame.data);
						else if (isEqual(frame.id, "GRID"))
							add("group_identification_registration", frame.data);
						else if (isEqual(frame.id, "LINK"))
							add("linked_information", frame.data);
						else if (isEqual(frame.id, "MCDI"))
							add("music_cd_identifier", frame.data);
						else if (isEqual(frame.id, "MLLT"))
							add("mpeg_location_lookup_table", frame.data);
						else if (isEqual(frame.id, "OWNE"))
							add("ownership_frame", frame.data);
						else if (isEqual(frame.id, "PCNT"))
							add("play_counter", frame.data);
						else if (isEqual(frame.id, "POPM"))
							add("popularimeter", frame.data);
						else if (isEqual(frame.id, "POSS"))
							add("position_sync_frame", frame.data);
						else if (isEqual(frame.id, "PRIV"))
							add("private_frame", frame.data);
						else if (isEqual(frame.id, "RBUF"))
							add("recommended_buffer_size", frame.data);
						else if (isEqual(frame.id, "RVA2") and isv24)
							add("relative_volume_adjustment", frame.data);
						else if (isEqual(frame.id, "RVAD") and isv23)
							add("relative_volume_adjustment", frame.data);
						else if (isEqual(frame.id, "RVRB"))
							add("reverb", frame.data);
						else if (isEqual(frame.id, "SEEK") and isv24)
							add("seek_frame", frame.data);
						else if (isEqual(frame.id, "SIGN") and isv24)
							add("signature_frame", frame.data);
						else if (isEqual(frame.id, "SYLT"))
							add("sync_lyric", frame.data);
						else if (isEqual(frame.id, "SYTC"))
							add("sync_tempo_codes", frame.data);
						else if (isEqual(frame.id, "TALB"))
							add("album", frame.data);
						else if (isEqual(frame.id, "TBPM"))
							add("bpm", frame.data);
						else if (isEqual(frame.id, "TCOM"))
							add("composer", frame.data);
						else if (isEqual(frame.id, "TCON") and not frame.data.empty())
						{
							var index { -1 };
							
							if (frame.data[0] == '(')
								index = std::stoi(frame.data.substr(
												1, frame.data.size() - 2));
							else if (isInt(frame.data))
								index = std::stoi(frame.data);
							
							if (index > -1)
								add("genre", GENRES[index]);
						}
						else if (isEqual(frame.id, "TCOP"))
							add("copyright_message", frame.data);
						else if (isEqual(frame.id, "TDEN") and isv24)
							add("encoding_time", frame.data);
						else if (isEqual(frame.id, "TDLY"))
							add("playlist_delay", frame.data);
						else if (isEqual(frame.id, "TDOR") and isv24)
							add("original_release_year", frame.data);
						else if (isEqual(frame.id, "TORY") and isv23)
							add("original_release_year", frame.data);
						
						else if (isEqual(frame.id, "TDAT") and isv23)
							add("date", frame.data);
						else if (isEqual(frame.id, "TRDA") and isv23)
							add("recording_date", frame.data);
						else if (isEqual(frame.id, "TIME") and isv23)
							add("time", frame.data);
						else if (isEqual(frame.id, "TYER") and isv23)
							add("year", frame.data);
						
						
						else if (isEqual(frame.id, "TDRC") and isv24)
						{
							add("date", frame.data);
							add("recording_date", frame.data);
							add("recording_time", frame.data);
							add("time", frame.data);
							add("year", frame.data);
						}
						
						else if (isEqual(frame.id, "TDRL") and isv24)
							add("release_time", frame.data);
						else if (isEqual(frame.id, "TDTG") and isv24)
							add("tagging_time", frame.data);
						else if (isEqual(frame.id, "TENC"))
							add("encoder", frame.data);
						else if (isEqual(frame.id, "TEXT"))
							add("lyric", frame.data);
						else if (isEqual(frame.id, "TFLT"))
							add("file_type", frame.data);
						else if (isEqual(frame.id, "TIPL") and isv24)
							add("involved_people_list", frame.data);
						else if (isEqual(frame.id, "IPLS") and isv23)
							add("involved_people_list", frame.data);
						
						else if (isEqual(frame.id, "TIT1"))
							add("content_group_description", frame.data);
						else if (isEqual(frame.id, "TIT2"))
							add("title", frame.data);
						else if (isEqual(frame.id, "TIT3"))
							add("subtitle_refinement", frame.data);
						else if (isEqual(frame.id, "TKEY"))
							add("initial_key", frame.data);
						else if (isEqual(frame.id, "TLAN"))
							add("language", frame.data);
						else if (isEqual(frame.id, "TLEN"))
							add("length", frame.data);
						else if (isEqual(frame.id, "TMCL") and isv24)
							add("musician_credits_list", frame.data);
						else if (isEqual(frame.id, "TMED"))
							add("media_type", frame.data);
						else if (isEqual(frame.id, "TMOO") and isv24)
							add("mood", frame.data);
						else if (isEqual(frame.id, "TOAL"))
							add("original_album_title", frame.data);
						else if (isEqual(frame.id, "TOFN"))
							add("original_filename", frame.data);
						else if (isEqual(frame.id, "TOLY"))
							add("original_lyricist_writer", frame.data);
						else if (isEqual(frame.id, "TOPE"))
							add("original_artist", frame.data);
						else if (isEqual(frame.id, "TOWN"))
							add("file_license", frame.data);
						else if (isEqual(frame.id, "TPE1"))
							add("artist", frame.data);
						else if (isEqual(frame.id, "TPE2"))
							add("band", frame.data);
						else if (isEqual(frame.id, "TPE3"))
							add("conductor_refinement", frame.data);
						else if (isEqual(frame.id, "TPE4"))
							add("remixed", frame.data);
						else if (isEqual(frame.id, "TPOS"))
							add("part_of_set", frame.data);
						else if (isEqual(frame.id, "TPRO") and isv24)
							add("produced_notice", frame.data);
						else if (isEqual(frame.id, "TPUB"))
							add("publisher", frame.data);
						else if (isEqual(frame.id, "TRCK"))
							add("track", frame.data);
						else if (isEqual(frame.id, "TRSN"))
							add("internet_radio_station_name", frame.data);
						else if (isEqual(frame.id, "TPSO"))
							add("internet_radio_station_owner", frame.data);
						else if (isEqual(frame.id, "TSOA") and isv24)
							add("album_sort_order", frame.data);
						else if (isEqual(frame.id, "TSOP") and isv24)
							add("performer_sort_order", frame.data);
						else if (isEqual(frame.id, "TSOT") and isv24)
							add("title_sort_order", frame.data);
						else if (isEqual(frame.id, "TSRC"))
							add("isrc", frame.data);
						else if (isEqual(frame.id, "TSSE"))
							add("encoding_setting", frame.data);
						else if (isEqual(frame.id, "TSST") and isv24)
							add("set_subtitle", frame.data);
						else if (isEqual(frame.id, "TXXX"))
							add("user_defined_text_info_frame", frame.data);
						else if (isEqual(frame.id, "UFID"))
							add("unique_file_identifier", frame.data);
						else if (isEqual(frame.id, "USER"))
							add("term_of_use", frame.data);
						else if (isEqual(frame.id, "USLT"))
							add("unsync_lyric_transcription", frame.data);
						else if (isEqual(frame.id, "WCOM"))
							add("commercial_information", frame.data);
						else if (isEqual(frame.id, "WCOP"))
							add("copyright_information", frame.data);
						else if (isEqual(frame.id, "WOAF"))
							add("official_audio_file_webpage", frame.data);
						else if (isEqual(frame.id, "WOAR"))
							add("official_atist_webpage", frame.data);
						else if (isEqual(frame.id, "WOAS"))
							add("official_audio_source_webpage", frame.data);
						else if (isEqual(frame.id, "WORS"))
							add("official_internet_radio_station_homepage", frame.data);
						else if (isEqual(frame.id, "WPAY"))
							add("payment", frame.data);
						else if (isEqual(frame.id, "WPUB"))
							add("publisher_official_webpage", frame.data);
						else if (isEqual(frame.id, "WXXX"))
							add("user_defined_url_link_frame", frame.data);
						#if 0
						std::cout << "Frame: "
									<< frame.frame_id
									<< " = "
									<< frame.data
									<< '\n';
						
						tag.frames.emplace_back(std::move(frame));
						#endif
					} else
						break;
				}
			}
		}
		else
		{
			func get{[&file](const int size, const bool isGenre = false) -> std::string
			{
				char buffer[size + 1];
				buffer[size] = '\0';
				for (var i { 0 }; i < size; ++i)
					buffer[i] = file.get();
				
				if (size == 1) {
					var result { buffer[0] };
					if (var genreIndex { 0 }; isGenre)
						for (var& g : GENRES) {
							if (result == genreIndex)
								return g;
							genreIndex++;
						}
					else
						return std::to_string(result);
				}
				
				return std::string(buffer);
			}};
		
			file.seekg(0, std::ios::end);
			const var end { (int) file.tellg() };
			file.seekg(end - 128);
			if (get(3) == "TAG")
			{
				add("version", "1.0");
				add("title", get(30));
				add("artist", get(30));
				add("album", get(30));
				add("year", get(4));
				add("comment", get(28));
				get(/*Zero Track*/ 1);
				add("track", get(1));
				const var genre { get(1, true) };
				var index { 0 };
				if (const var [p, ec] { std::from_chars(genre.c_str(),
										genre.c_str()+genre.size(), index) };
					ec == std::errc())
					add("genre", GENRES[index]);
				
				file.seekg(0, std::ios::end);
				file.seekg(end - (128 + 227));
				if (get(4) == "TAG+")
				{
					add("version", "1.1 Extended");
					add("title", get(60));
					add("artist", get(60));
					add("album", get(60));
					/*"Speed*/get(1);
					add("genre", get(30));
					/*"Start*/get(6);
					/*"End*/get(6);
				}
			}
		}
			
		file.close();
	}
};


namespace opt {
	var EXCLUDE_EXT { SetString() };
	var DEFAULT_EXT { SetString{
		".mp4",  ".mkv", ".mov", ".m4v",  ".mpeg", ".mpg",  ".mts", ".ts",
		".webm", ".flv", ".wmv", ".avi",  ".divx", ".xvid", ".dv",  ".3gp",
		".tmvb", ".rm",  ".mpg4",".mqv",  ".ogm",  ".qt",
		".vox",  ".3gpp",".m2ts",".m1v",  ".m2v",  ".mpe",
		".mp3",	 ".m4a", ".aac", ".wav",  ".wma",  ".flac", ".ape", ".aiff"
		".jpg",  ".jpeg",".png", ".gif",  ".bmp"
	}};
	var EXCLUDE_EXT_REPLACED { false };
	var DEFAULT_EXT_REPLACED { false };

	ListRegex 	listRegex, 	listExclRegex;
	
	ListString 	listFind, 	listExclFind,
				listFindDir,listExclFindDir;

	ListPaitUIntMax listSize, listExclSize;

	ListPairCharDate
		listDCreated, 	  listDModified, 	 listDAccessed, 	listDChanged,
		listDExclCreated, listDExclModified, listDExclAccessed, listDExclChanged;
	ListPairDate
		listDCreatedR, 	   listDModifiedR, 	   listDAccessedR, 	   listDChangedR,
		listDExclCreatedR, listDExclModifiedR, listDExclAccessedR, listDExclChangedR;

	namespace date {
		ReadOnlyCString* SINGLE_DATE[2][4] = {
			{ &OPT_DCREATED, &OPT_DACCESSED, &OPT_DMODIFIED, &OPT_DCHANGED },
			{ &OPT_DEXCLCREATED, &OPT_DEXCLACCESSED, &OPT_DEXCLMODIFIED, &OPT_DEXCLCHANGED },
		};

			
		const ListPairCharDate* const OPERATOR_DATE[2][4] = {
			{ &opt::listDCreated, &opt::listDAccessed, &opt::listDModified, &opt::listDChanged },
			{ &opt::listDExclCreated, &opt::listDExclAccessed, &opt::listDExclModified, &opt::listDExclChanged },
		};

		
		const ListPairDate* const RANGE_DATE[2][4] = {
			{ &opt::listDCreatedR, &opt::listDAccessedR, &opt::listDModifiedR, &opt::listDChangedR },
			{ &opt::listDExclCreatedR, &opt::listDExclAccessedR, &opt::listDExclModifiedR, &opt::listDExclChangedR },
		};
	}

	var valueOf { MapString() };
}



func directory_iterator(const fs::path& path, const unsigned char type)
{
	var result { ListEntry() };
		
	if(const var folder { opendir(path.string().c_str()) }; folder) {
		const var parentPath { path.string() + fs::path::preferred_separator };
		struct dirent* entry;
		readdir(folder);
		readdir(folder);
		while( (entry = readdir(folder)) ) {
			if ((entry->d_type & type) == entry->d_type)
			{
				if (entry->d_type == DT_REG
					and isEqual(entry->d_name, ".DS_Store"))
					;
				else {
					var name { parentPath };
					name += entry->d_name;
					const var path_name { fs::path(std::move(name)) };
					var d { fs::directory_entry(std::move(path_name)) };
					d.refresh();
					if (((d.status().permissions() & ( fs::perms::owner_read
													| fs::perms::group_read
													| fs::perms::others_read))
							== fs::perms::none)
						or (opt::valueOf[OPT_EXCLHIDDEN] == "true"
							and entry->d_name[0] == '.'))
						continue;
					if (d.is_symlink()) {
						const var ori { fs::directory_entry(
										std::move(fs::read_symlink(d.path()))) };
						if ((ori.path().empty() or not ori.exists())
							and
							(((type & DT_DIR) == DT_DIR and not ori.is_directory())
							 or
							 ((type & DT_REG) == DT_REG and not ori.is_regular_file()))
							)
							continue;
						
						d.assign(ori);
						d.refresh();
					}
					
					result.emplace_back(std::move(d));
				}
			}
		}
		closedir(folder);
	}
	return result;
}

func isValidFile(const fs::path& path)
{
	if (not fs::exists(path))
		return false;

	if ((not opt::valueOf[OPT_EXCLEXT].empty() and opt::valueOf[OPT_EXCLEXT] not_eq "*")
		and not opt::EXCLUDE_EXT_REPLACED) {
		parseExtCommaDelimited(opt::valueOf[OPT_EXCLEXT], &opt::EXCLUDE_EXT);
		opt::EXCLUDE_EXT_REPLACED = true;
	}
	if ((not opt::valueOf[OPT_EXT].empty() and opt::valueOf[OPT_EXT] not_eq "*")
		 and not opt::DEFAULT_EXT_REPLACED) {
		parseExtCommaDelimited(opt::valueOf[OPT_EXT], &opt::DEFAULT_EXT);
		opt::DEFAULT_EXT_REPLACED = true;
	}
	
	
	
	var tmp { path };
	if (fs::is_symlink(tmp)) { //TODO: is_symlink() cannot detect macOS alias file!
		tmp = fs::read_symlink(path);
		
		if (not fs::exists(tmp) or not fs::is_regular_file(tmp))
			return false;
	}
	
	const var fileExt { tolower(tmp.extension().string()) };
	if (opt::valueOf[OPT_EXT] not_eq "*"
		and opt::DEFAULT_EXT.find(fileExt)
		== opt::DEFAULT_EXT.end())
		return false;
	
	if (opt::valueOf[OPT_EXCLEXT] not_eq "*"
		and not opt::valueOf[OPT_EXCLEXT].empty())
		if (opt::EXCLUDE_EXT.find(fileExt)
			not_eq opt::EXCLUDE_EXT.end())
			return false;
	
	if (opt::valueOf[OPT_DCREATED] 		not_eq ""
		or opt::valueOf[OPT_DACCESSED] 	not_eq ""
		or opt::valueOf[OPT_DMODIFIED] 	not_eq ""
		or opt::valueOf[OPT_DCHANGED] 	not_eq ""
		or opt::valueOf[OPT_DEXCLCREATED] not_eq ""
		or opt::valueOf[OPT_DEXCLACCESSED]not_eq ""
		or opt::valueOf[OPT_DEXCLMODIFIED]not_eq ""
		or opt::valueOf[OPT_DEXCLCHANGED] not_eq ""
		)
	{
		struct stat filestat;
		stat(tmp.string().c_str(), &filestat);
		
		const Date _ft[4] = { 			Date(&filestat.st_birthtime),
										Date(&filestat.st_atime),
										Date(&filestat.st_mtime),
										Date(&filestat.st_ctime) };
		const Date ft[2][4] = {
			{ _ft[0], _ft[1], _ft[2], _ft[3]},
			{ _ft[0], _ft[1], _ft[2], _ft[3]}
		};


		bool found[2]{ false, false };
		for (var& z : { 0, 1 })
			for (var& i : { 0, 1, 2, 3 })
			{
				if (opt::valueOf[*opt::date::SINGLE_DATE[z][i]] not_eq "")
				{
					for (var& r : *opt::date::RANGE_DATE[z][i])
						if (ft[z][i] >= r.first and ft[z][i] <= r.second ) {
							found[z] = true;
							break;
						}
					for (var& t : *opt::date::OPERATOR_DATE[z][i])
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
				} // end st[] -> valueOf[OPT_D?????]
			}
		
		if (not found[0] or found[1])
			return false;
	}
	
	
	if (var found { false }; not opt::valueOf[OPT_REGEX].empty()) {
		for (var filename { excludeExtension(tmp.filename()) };
			 var& regex : opt::listRegex)
			if (std::regex_search(filename, regex)) {
				found = true;
				break;
			}
		if (not found)
			return false;
	}
	
	if (not opt::valueOf[OPT_EXCLREGEX].empty())
		for (const var filename { excludeExtension(tmp.filename()) };
			 var& regex : opt::listExclRegex)
			if (std::regex_search(filename, regex))
				return false;
	
	if (not opt::listFind.empty() or not opt::listExclFind.empty()) // MARK: Find statement
	{
		const var ismp3 { isEqual(fileExt.c_str(), {".mp3", ".aac", ".m4a"}) };
		const var filename { excludeExtension(tmp.filename()) };
		const var isCaseInsensitive { opt::valueOf[OPT_CASEINSENSITIVE] == "true" };
		
		var id3 { ID3() };
		if (ismp3)
			id3 = ID3(tmp.string().c_str(), isCaseInsensitive);
	
		if (var found { false }; not opt::listFind.empty())
		{
			for (var& keyword : opt::listFind)
			{
				var handled { keyword[0] == char(1) };
				if (not handled and ismp3 and (handled = true))
					found = id3 % keyword;
				
				if (not handled and isContains(filename, keyword,
					isCaseInsensitive ? IgnoreCase::Both : IgnoreCase::None)
					not_eq std::string::npos)
				{
					found = true;
					break;
				}
			}
			if (not found)
				return false;
		}

		for (var& keyword : opt::listExclFind)
		{
			var handled { keyword[0] == char(1) };
			if (var found { false }; not handled and ismp3 and (handled = true)) {
				found = id3 % keyword;
				if (handled and found)
					return false;
			}
			
			if (not handled and isContains(filename, keyword, isCaseInsensitive
				? IgnoreCase::Both : IgnoreCase::None) not_eq std::string::npos)
				return false;
		}
	}


	if (var found { false };
		not opt::listSize.empty()
		or (opt::valueOf[OPT_SIZEOPGT][0] not_eq '\0' and opt::valueOf[OPT_SIZE] not_eq "0"))
	{
		const var fileSize { fs::file_size(tmp) };
		
		for (var& range : opt::listSize)
			if (fileSize > range.first and fileSize < range.second) {
				found = true;
				break;
			}
		
		if (not found)
			found = opt::valueOf[OPT_SIZEOPGT][0] == '>'
						? fileSize > std::stoul(opt::valueOf[OPT_SIZE])
						: fileSize < std::stoul(opt::valueOf[OPT_SIZE]);
		if (not found)
			return false;
	}
	
	if (var found { false };
		not opt::listExclSize.empty()
		or (opt::valueOf[OPT_EXCLSIZEOPGT][0] not_eq '\0' and opt::valueOf[OPT_EXCLSIZE] not_eq "0"))
	{
		const var fileSize { fs::file_size(tmp) };
		
		for (var& range : opt::listExclSize)
			if (fileSize > range.first and fileSize < range.second)
				return false;
		
		if (not found)
			found = opt::valueOf[OPT_EXCLSIZEOPGT][0] == '>'
						? fileSize > std::stoul(opt::valueOf[OPT_EXCLSIZE])
						: fileSize < std::stoul(opt::valueOf[OPT_EXCLSIZE]);
		if (found)
			return false;
	}
		
	return true;
}

func getAvailableFilename(const fs::path& original,
						  const std::string& prefix = " #",
						  const std::string& suffix = "")
{
	if (fs::exists(original)) {
		const var s { original.string() };
		const var ext { original.extension().string() };
		const var noext { s.substr(0, s.size() - ext.size()) };
		var i { (unsigned) 0 };
		while (true) {
			var test { fs::path(noext + prefix + std::to_string(++i) + suffix + ext) };
			
			if (not fs::exists(test)) {
				return test.string();
			}
		}
	} else
		return original.string();
}

inline
func sort(const fs::path& a, const fs::path& b, bool ascending)
{
	const var afn { a.filename().string().substr(0,
	a.filename().string().size() - a.extension().string().size()) };

	const var bfn { b.filename().string().substr(0,
	b.filename().string().size() - b.extension().string().size()) };

	var av { ListMAXNUM() };
	var bv { ListMAXNUM() };
	containInts(afn, &av);
	containInts(bfn, &bv);

	if (av.size() == bv.size())
		for (var i { 0 }; i<av.size(); ++ i) {
			if (av[i] == bv[i])
				continue;
			return ascending ? av[i] < bv[i] : av[i] > bv[i];
		}

	return afn < bfn;
}

func ascending(const fs::path& a, const fs::path& b)
{
	return sort(a, b, true);
}

func descending(const fs::path& a, const fs::path& b)
{
	return sort(a, b, false);
}


func sortFiles(ListPath* const selectFiles)
{
	if (selectFiles->size() > 1) {
		std::sort(selectFiles->begin(), selectFiles->end());
		
		var selectFilesDirs { MapStringPListPath() };
		for (var& f : *selectFiles) {
			var parent { f.parent_path().string() };
			if (var ptr { selectFilesDirs[parent] };
				ptr)
				ptr->emplace_back(f);
			else {
				var flist { ListPath{f} };
				
				selectFilesDirs[parent] = std::make_shared<ListPath>
													(std::move(flist));
			}
		}
		
		selectFiles->clear();
		for (var& m : selectFilesDirs) {
			var files { *(m.second) };
			std::sort(files.begin(), files.end(), ascending);
			for (var& f : files)
				selectFiles->emplace_back(f);
		}
	}
}

func isDirNameValid(const fs::path& dir)
{
	if (opt::listFindDir.empty() and opt::listExclFindDir.empty())
		return true;
	
	const var filename { dir.filename().string() };
	const var ignoreCase { opt::valueOf[OPT_CASEINSENSITIVE] == "true" };
	for (var& keyword : opt::listFindDir)
		if (isContains(filename, keyword, ignoreCase ? IgnoreCase::Left
					   : IgnoreCase::None) not_eq std::string::npos)
			return true;
	
	var result { true };
	for (var& keyword : opt::listExclFindDir)
		if (isContains(filename, keyword, ignoreCase ? IgnoreCase::Left
					   : IgnoreCase::None) not_eq std::string::npos)
		{
			result = false;
			break;
		}
	
	if (not result)
		return true;
		
	return false;
}

inline
func isValid(const fs::path& path)
{
	return /*(fs::is_regular_file(path) and path.filename().string() not_eq ".DS_Store")
	or*/ not (((fs::status(path).permissions() & (  fs::perms::owner_read
												| fs::perms::group_read
												| fs::perms::others_read))
			== fs::perms::none)
	or (opt::valueOf[OPT_EXCLHIDDEN] == "true" and path.filename().string()[0] == '.')
	);
}

func listDirInto(const fs::path& ori,
				 ListEntry* const out,
				 const bool sorted = true,
				 const bool includeRegularFiles = false)
{
	if (not out)
		return;
	
	var path { std::move(ori) };
	if (fs::is_symlink(ori)) {
		var ec { std::error_code() };
		path = fs::read_symlink(ori, ec);
		if (ec or not fs::is_directory(path))
			return;
	}

	for (var& child : directory_iterator(path, (includeRegularFiles
				? (DT_DIR | DT_REG) : DT_DIR)))
		out->emplace_back(child);
	
	if (sorted and out->size() > 1)
		std::sort(out->begin(), out->end(), [](const fs::directory_entry& a,
											   const fs::directory_entry& b) {
			return ascending(a.path(), b.path());
		});
}

func listDirRecursivelyInto(const fs::path& path,
							ListPath* const out,
							const bool includeRegularFiles)
{
	if (not out or path.empty())
		return;
	
	var head { path };
	out->emplace_back(std::move(path));
	
	/// Try to expand single dir and put into list
	var list { ListEntry() };
	var dirs { ListPath() };
	var threads { ListThread() };
	var asyncs { ListAsync() };
	
	
	func emplace{[&list, &out]()
	{
		for (var& d : list)
			out->emplace_back(std::move(d.path()));
	}};
	
	do {
		do {
			list.clear();
			dirs.clear();
			listDirInto(head, &list, false, includeRegularFiles);
			if (list.size() == 1)
				out->emplace_back(std::move(list[0].path()));
			if (not list.empty()) {
				for (var& child : list)
					if (child.is_directory())
						dirs.emplace_back(std::move(child.path()));
				if (dirs.empty()) {
					emplace();
					return;
				} else
					head = dirs[0];
			}
		} while(list.size() == 1);
		
		if (list.empty())
			break;
		
		/// If single dir was expanded, then expand each child dir
		
		for (var& d : dirs)
			if (d.empty())
				continue;
			else if (opt::valueOf[OPT_EXECUTION] == MODE_EXECUTION_THREAD)
				threads.emplace_back(listDirInto, d, &list,
									 false, includeRegularFiles);
			else if (opt::valueOf[OPT_EXECUTION] == MODE_EXECUTION_ASYNC)
				asyncs.emplace_back(std::async(
						std::launch::async, listDirInto, d, &list,
											   false, includeRegularFiles));
			else
				listDirInto(d, &list, false, includeRegularFiles);
		
		
		if (opt::valueOf[OPT_EXECUTION] == MODE_EXECUTION_THREAD) {
			for (var& t : threads)
				t.join();
			threads.clear();
		}
		else if (opt::valueOf[OPT_EXECUTION] == MODE_EXECUTION_ASYNC) {
			for (var& a : asyncs)
				a.wait();
			asyncs.clear();
		}
		
		emplace();
	} while(list.size() > 0);
}

func isContainsSeasonDirs(const fs::path& path)
{
	if (path.empty())
		return false;
	
	var sortedDir { ListEntry() };
	listDirInto(path, &sortedDir);
	
	if (sortedDir.size() <= 1)
		return false;
	
	var hasDirs { false };
	
	var isNum { true };
	var lastNum { ListMAXNUM() };
	
	for (var& child : sortedDir) {
		hasDirs = true;
		if (isNum) {
			var iNames { ListMAXNUM() };
			containInts(child.path().filename().string(), &iNames);
			if (not iNames.empty()) {
				if (lastNum.empty()) {
					lastNum = std::move(iNames);
					continue;
				} else if (lastNum.size() == iNames.size()) {
					var hasIncreased { false };
					for (var xi { 0 }; xi < lastNum.size(); ++xi)
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

namespace in {
	ListPath 				regularDirs,
							seasonDirs,
							selectFiles,
							listAdsDir,
							inputDirs;
	
	var fileCountPerTurn 	{ (unsigned long) 1 };
	var args				{ ListString() };
	var invalidArgs			{ SetStringView() };
}

namespace def {
	const var SUBTITLES_EXT { ListString{ ".srt", ".ass", ".vtt" } };
	const var FILENAME_IGNORE_CHAR { std::pair<char, char>{'.', ' '} };
	
	let ARG_SEP { ':' };
	let ARG_ASSIGN { '=' };

	let ARGS_SEPARATOR {"\x0a\x0aARGS-SEPARATOR\x0a\x0a"};

	const std::string XML_CHARS_ALIAS[] = {"&quot", "&apos", "&lt", "&gt", "&amp"};
	const std::string XML_CHARS_NORMAL[] = {"\"", "\\", "<", ">", "&"};
	let NETWORK_PROTOCOLS = {"http:", "https:", "ftp:", "ftps:", "rtsp:", "mms:"};
}

func findSubtitleFileInto(const fs::path& original,
						  ListPath* const result)
{
	if (const var parentPath { original.parent_path() }; not parentPath.empty()) {
		const var noext { excludeExtension(original.filename()) };
		for (var& f : directory_iterator(parentPath, DT_REG))
			if (isValid(f)
				and f.path().string().size() >= original.string().size()
				and isEqual(f.path().extension().string(), &def::SUBTITLES_EXT,
							IgnoreCase::Left)
				and isContains(f.path().filename().string(), noext,
				IgnoreCase::Both, &def::FILENAME_IGNORE_CHAR) not_eq std::string::npos)

				result->emplace_back(std::move(f));
	}
}



func insertInto(ListPath* const out, const fs::path& path)
{
	const var isOk { isValid(path) };
	if (isOk)
		out->emplace_back(std::move(path));

	return isOk;
}


func checkForSeasonDir(const fs::path& path) -> void
{
	if (not path.empty()) {
		var hasDir { false };
		
		var isNum { true };
		var lastNum { ListMAXNUM() };
		var bufferNum { ListPath() };
		
		func pullFromBufferNum{ [&bufferNum, &isNum]()
		{
			isNum = false;
			for (var& child : bufferNum) {
				insertInto(&in::regularDirs, child);
				checkForSeasonDir(child);
			}
		}};
		
		var sortedDir { ListEntry() };
		listDirInto(path, &sortedDir);
		
		if (sortedDir.size() == 1)
			checkForSeasonDir(sortedDir[0].path());
		else if (sortedDir.size() > 1)
			for (var& child : sortedDir) {
				hasDir = true;

				if (isNum) {
					var iNames { ListMAXNUM() };
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
								var hasIncreased { false };
								for (var xi { 0 }; xi < lastNum.size(); ++xi)
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
				insertInto(&in::regularDirs, child.path());
				checkForSeasonDir(child.path());
				pullFromBufferNum();
			}
		
		if (isNum and hasDir) {
			for (var it { in::regularDirs.begin() };
				 it not_eq in::regularDirs.end(); )
				if (*it == path)
					it = in::regularDirs.erase(it);
				else
					++it;
			
			insertInto(&in::seasonDirs, path);
		} else
			insertInto(&in::regularDirs, path);
	}
}

func getBytes(const std::string& s)
{
	var unit { std::string("mb") };
	var value { s };
	if (s.size() > 2 and std::isalpha(s[s.size() - 2])) {
		unit = s.substr(s.size() - 2);
		value = s.substr(0, s.size() - 2);
	}
	
	var result { (uintmax_t) 0 };
	
	if (not isInt(value))
		return result;
	
	const var v { std::stof(value) };
	if (v <= 0)
		return result;
	
	if (isEqual(unit.c_str(), "kb", IgnoreCase::Left)) {
		if (v <= (INT_MAX / 1000))
			result = v * 1000;
	} else if (isEqual(unit.c_str(), "gb", IgnoreCase::Left)) {
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
	const var pos { argv.find(separator) };
	if (pos == std::string::npos)
		return nullptr;
	
	const var first { argv.substr(0, pos) };
	const var second { argv.substr(pos + separator.size()) };

	var from { getBytes(std::move(first)) };
	from = std::max(from, static_cast<uintmax_t>(0));
	var to { getBytes(std::move(second)) };
	to = std::max(to, static_cast<uintmax_t>(0));
	return std::make_shared<std::pair<uintmax_t, uintmax_t>>(from, to);
}

func deduceArgsInto(const int argc, CString const argv[],
				const int startAt, ListString* const args)
{
	var newFull { false };
	var lastOptCode { (unsigned) 0 };
	func push{ [&newFull, &args](ReadOnlyCString arg,
								 const unsigned index,
								 const int last)
	{
		if (last < 0 or index - last <= 0) return;
				
		const var size { (unsigned) (index - last) };
		var newArg { std::string() };

		if (newFull)
			newArg = "--";
		
		char dst[size];
		dst[size] = '\0';
		std::memcpy(dst, arg + last, sizeof dst);
		newArg += dst;

		args->emplace_back(std::move(newArg));
		
		if (newFull)
			newFull = false;
	}};
	
	const var isMultiLine {
		startAt < argc
		and std::string(argv[startAt]).find("\x0a") not_eq std::string::npos
	};
	
	func isNextOption{[](ReadOnlyCString arg) {
		var tmp { arg };
		
		while (tmp++) {
			if (std::isalpha(*tmp))
				return true;
			else if (std::isspace(*tmp))
				continue;
			else
				break;
		}
		return false;
	}};
	
	for (var i { startAt }; i<argc; ++i) {
		var arg { argv[i] };
		
		if (isMultiLine)
			while (arg[0] == 0x0a or arg[0] == 0x0d)
				arg++;
		
		var len { std::strlen(arg) };
		var isMnemonic { len > 1 and arg[0] == '-'
			and (std::isalpha(arg[1]) or arg[1] == def::ARG_SEP or arg[1] == ';' ) };
		var isFull { false };
		if (not isMnemonic) {
			isFull = len > 2 and arg[0] == '-' and arg[1] == '-'
				and (std::isalpha(arg[2]) or arg[2] == def::ARG_SEP or arg[1] == ';');
			if (isFull)
				newFull = true;
			else
			{
				if (lastOptCode > 0 and arg[0] == def::ARG_ASSIGN) {
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
					lastOptCode = 0;
					continue;
				}
			}
		}
		
		var equalOpCount { (unsigned) 0 };
		var fallthrough { (unsigned) 0 };
		var index { (unsigned) lastOptCode == 3 ? -1
								: (isFull and lastOptCode not_eq 2 ? 1 : 0) };
		var last { -1 };
		while (++index < len) {
			if (fallthrough > 0) {
				if (arg[index] == '"') {
					fallthrough--;
					if (fallthrough == 0) {
						push(arg, index, last);
						last = -1;
					}
				}
				continue;
			}
			if (const var c { arg[index] };
				last < 0 and std::isalpha(c)) {
				if (isMnemonic) {
					lastOptCode = 1;
					var new_s { std::string() };
					new_s += '-';
					new_s += c;
					args->emplace_back(new_s);
				} else if (isFull) {
					last = index;
				}
			} else {
				if (const var isArithmetic { c == '+' or c == '-' };
					equalOpCount == 0
					and (c == def::ARG_ASSIGN
						 or std::isspace(c)
						 or (index + 1 < len
							 and isArithmetic
							 and arg[index + 1] == '=')
					))
				{
					if (isFull) {
						lastOptCode = 2;
						push(arg, index, last);
						last = -1;
					} else {
						lastOptCode = 1;
						last = index + 1;
					}
					equalOpCount++;
					
					if (const var lastNewFull { newFull };
						isArithmetic)
					{
						newFull = false;
						push(arg, index + 2, index);
						newFull = lastNewFull;
						index++;
					}
				} else if (const var foundEnter { c == 0x0a };
						   ((c == def::ARG_SEP or c == ';')
						   and (index + 1 == len or
								(index + 1 < len and isNextOption(arg + index + 1) ))
						   )
						   or foundEnter)
				{
					lastOptCode = 1;
					push(arg, index, last);
					last = -1;
					
					if (foundEnter and arg[index + 1] == 0x0d)
						index++;

					if (isFull) {
						lastOptCode = 2;
						newFull = foundEnter ? false :
						index + 1 < std::strlen(arg) and isNextOption(arg + index + 1);
					}
					equalOpCount = 0;
				} else if (isFull and last > 0 and (c == '<' or c == '>')) {
					if (arg[last - 1] == def::ARG_ASSIGN
						or arg[last - 1] == '<'
						or arg[last - 1] == '>')
						continue;
					
					lastOptCode = 0;
					push(arg, index, last);
					last = index;
				} else {
					if (c == '"') {
						if (fallthrough == 0) {
							push(arg, index, last);
							last = -1;
						}
						fallthrough++;
					}
					if (last < 0)
						last = index + (fallthrough > 0 ? 1 : 0);
				}
			}
		}
		if (last > 0)
			push(arg, index, last);
	}
}

func timeLapse(std::chrono::system_clock::time_point& start,
			   const std::string& msg,
			   const bool resetStart=false)
{
	var value { (std::chrono::system_clock::now() - start).count() };
	var inc { (unsigned) 0 };
	while (value / 1000 > 0 and inc < 2) {
		inc++;
		value /= 1000;
	}
	
	while (value / 60 > 0 and inc < 4) {
		inc++;
		value /= 60;
	}
	
	
	const std::string_view tu[] { 	"microseconds",
									"milliseconds",
									"seconds",
									"minutes",
									"hours" };
	std::cout << msg
				<< std::fixed << std::setprecision(2)
				<< groupNumber(std::to_string(value))
				<< " "
				<< tu[inc]
				<< ".\n\n";
	
	if (resetStart)
		start = std::chrono::system_clock::now();
}

func parseKeyValue(std::string* const s, const bool isExclude)
{
	if (not s or s->empty())
		return false;
		
	const CString keyword { "" };
	var value { std::string() };
	var isKeyValue { true };
	for (var& key : {"dir", OPT_EXT, OPT_EXCLEXT,
		OPT_CASEINSENSITIVE, OPT_EXCLHIDDEN, OPT_SIZE, OPT_EXCLSIZE,
		"case-insensitive", "caseinsensitive",
		"ignore-case", "ignorecase", OPT_DATE, OPT_EXCLDATE,
		OPT_DCREATED, OPT_DCHANGED, OPT_DACCESSED, OPT_DMODIFIED,
		OPT_DEXCLCREATED, OPT_DEXCLCHANGED, OPT_DEXCLMODIFIED, OPT_DEXCLACCESSED})
	{
		const var sz { std::strlen(key) };
		
		if (s->size() > sz + 2)
			if (const var c { s->at(sz) };
				s->starts_with(key) and (c == '=' or c == '<' or c == '>'))
			{
				keyword = key;
				value = s->substr(sz + (c == '=' ? 1 : 0));
				break;
			}
	}
	if (isEqual(keyword, { OPT_SIZE,OPT_EXCLSIZE}))
	{
		var pos { value.find("..") };
		var next { 2 };
		if (pos == std::string::npos) {
			pos = value.find('-');
			next = 1;
		}
		if (pos not_eq std::string::npos) {
			const var lower { getBytes(value.substr(0, pos))};
			const var upper { getBytes(value.substr(pos + next)) };
			if (lower > upper)
				isKeyValue = false;
			else {
				opt::valueOf[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] = '\0';
				opt::valueOf[keyword] = std::to_string(lower);
				
				(isExclude ? opt::listExclSize : opt::listSize).emplace_back(std::make_pair(
							   lower,
							   upper));
			}
		}
		else if (const var havePrefix { value[0] == '<' or value[0] == '>'};
				 havePrefix) {
			if (const var number { getBytes(value.substr(havePrefix ? 1 : 0)) };
				number < 0)
				isKeyValue = false;
			else {
				opt::valueOf[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] =
				opt::valueOf[keyword] = std::to_string(number);
				(isExclude ? opt::listExclSize : opt::listSize).clear();
			}
		}
		else
			isKeyValue = false;
	}
	else if (isEqual(keyword, "dir"))
	{
		if (value.empty())
			isKeyValue = false;
		else {
			if (opt::valueOf[OPT_CASEINSENSITIVE] == "true")
				value = tolower(value);
			(isExclude ? opt::listExclFindDir : opt::listFindDir).emplace_back(value);
		}
	}
	else if (isEqual(keyword, {OPT_EXCLHIDDEN, OPT_EXT, OPT_EXCLEXT})) {
		opt::valueOf[keyword] = value;
	}
	else if (isEqual(keyword, {OPT_DATE, OPT_EXCLDATE,
		OPT_DCREATED, OPT_DCHANGED, OPT_DACCESSED, OPT_DMODIFIED,
		OPT_DEXCLCREATED, OPT_DEXCLCHANGED, OPT_DEXCLMODIFIED, OPT_DEXCLACCESSED}))
   {
	   const var havePrefix { value[0] == '<' or value[0] == '>'};
	   if (const var opGt { value[0] };
		   havePrefix or opGt == '=')
	   {
		   if (const var date { Date(value) };
			   not date.isValid())
			   isKeyValue = false;
		   else {
			   if (isEqual(keyword, OPT_DATE)) {
				   opt::listDCreated.emplace_back(std::make_pair(opGt, date));
				   opt::listDChanged.emplace_back(std::make_pair(opGt, date));
				   opt::listDModified.emplace_back(std::make_pair(opGt, date));
				   opt::listDAccessed.emplace_back(std::make_pair(opGt, date));
				   opt::valueOf[OPT_DCREATED]  = "1";
				   opt::valueOf[OPT_DMODIFIED] = "1";
				   opt::valueOf[OPT_DACCESSED] = "1";
				   opt::valueOf[OPT_DCHANGED]  = "1";
			   } else if (isEqual(keyword, OPT_EXCLDATE)) {
				   opt::listDExclCreated.emplace_back(std::make_pair(opGt, date));
				   opt::listDExclChanged.emplace_back(std::make_pair(opGt, date));
				   opt::listDExclModified.emplace_back(std::make_pair(opGt, date));
				   opt::listDExclAccessed.emplace_back(std::make_pair(opGt, date));
				   opt::valueOf[OPT_DEXCLCREATED]  = "1";
				   opt::valueOf[OPT_DEXCLMODIFIED] = "1";
				   opt::valueOf[OPT_DEXCLACCESSED] = "1";
				   opt::valueOf[OPT_DEXCLCHANGED]  = "1";
			   } else {
				 (isEqual(keyword, OPT_DCREATED) ? opt::listDCreated
				: isEqual(keyword, OPT_DCHANGED) ? opt::listDChanged
				: isEqual(keyword, OPT_DMODIFIED) ? opt::listDModified
				: isEqual(keyword, OPT_DACCESSED) ? opt::listDAccessed
				: isEqual(keyword, OPT_DEXCLCREATED) ? opt::listDExclCreated
				: isEqual(keyword, OPT_DEXCLCHANGED) ? opt::listDExclChanged
				: isEqual(keyword, OPT_DEXCLMODIFIED) ? opt::listDExclModified
				: opt::listDExclAccessed
				).emplace_back(std::make_pair(opGt, date));

				   opt::valueOf[keyword] = "1";
			   }
		   }
	   }
	   else {
		   var pos { value.find("..") };
		   var next { 2 };
		   
		   if (pos == std::string::npos) {
			   next = 1;
			   var strip { (unsigned) 0 };
			   pos = 0;
			   for (var m { 0 }; m < value.size(); ++m)
				   if (value[m] == '-') {
					   strip++;
					   pos = m;
				   }
			   if (strip not_eq 1)
				   pos = std::string::npos;
		   }
		   
		   if (pos == std::string::npos)
			   isKeyValue = false;
		   else {
			   if (const var lower { Date(value.substr(0, pos)) };
				   not lower.isValid())
				   isKeyValue = false;
			   else {
				   if (const var upper { Date(value.substr(pos + next)) };
					   not upper.isValid() or lower > upper)
					   isKeyValue = false;
				   else {
					   if (isEqual(keyword, OPT_DATE)) {
						   opt::listDCreatedR.emplace_back(std::make_pair(lower, upper));
						   opt::listDChangedR.emplace_back(std::make_pair(lower, upper));
						   opt::listDModifiedR.emplace_back(std::make_pair(lower, upper));
						   opt::listDAccessedR.emplace_back(std::make_pair(lower, upper));
						   opt::valueOf[OPT_DCREATED]  = "1";
						   opt::valueOf[OPT_DMODIFIED] = "1";
						   opt::valueOf[OPT_DACCESSED] = "1";
						   opt::valueOf[OPT_DCHANGED]  = "1";
					   } else if (isEqual(keyword, OPT_EXCLDATE)) {
						   opt::listDExclCreatedR.emplace_back(std::make_pair(lower, upper));
						   opt::listDExclChangedR.emplace_back(std::make_pair(lower, upper));
						   opt::listDExclModifiedR.emplace_back(std::make_pair(lower, upper));
						   opt::listDExclAccessedR.emplace_back(std::make_pair(lower, upper));
						   opt::valueOf[OPT_DEXCLCREATED]  = "1";
						   opt::valueOf[OPT_DEXCLMODIFIED] = "1";
						   opt::valueOf[OPT_DEXCLACCESSED] = "1";
						   opt::valueOf[OPT_DEXCLCHANGED]  = "1";
					   } else {
							 (isEqual(keyword, OPT_DCREATED) ? opt::listDCreatedR
							: isEqual(keyword, OPT_DCHANGED) ? opt::listDChangedR
							: isEqual(keyword, OPT_DMODIFIED) ? opt::listDModifiedR
							: isEqual(keyword, OPT_DACCESSED) ? opt::listDAccessedR
							: isEqual(keyword, OPT_DEXCLCREATED) ? opt::listDExclCreatedR
							: isEqual(keyword, OPT_DEXCLCHANGED) ? opt::listDExclChangedR
							: isEqual(keyword, OPT_DEXCLMODIFIED) ? opt::listDExclModifiedR
							: opt::listDExclAccessedR
							).emplace_back(std::make_pair(lower, upper));
						   opt::valueOf[keyword] = "1";
					   }
				   }
			   }
		   }
	   }
   }
   else if (0 not_eq std::strlen(keyword)) {
		opt::valueOf[OPT_CASEINSENSITIVE] = value;
		if (isEqual(value, "true", IgnoreCase::Left)) {
			for (var& k : opt::listFindDir) k = tolower(k);
			for (var& k : opt::listExclFindDir) k = tolower(k);
			for (var& k : opt::listFind) k = tolower(k);
			for (var& k : opt::listExclFind) k = tolower(k);
		}
	}
	
	if (isKeyValue and 0 not_eq std::strlen(keyword))
		s->insert(0, 1, char(1));
	
	return 0 not_eq std::strlen(keyword);
}

template <typename  T>
func getLinesInto(std::basic_istream<T>* const inputStream,
			  ListString* const lines,
			  const std::initializer_list<std::string>&& excludePrefix)
{
	var commentCount { 0 };
	var isComment { false };
	var buff { std::string() };
	T c;
		
	func push{[&lines, &buff]()
	{
		if (buff.empty())
			return;
		lines->emplace_back(buff);
		buff.clear();
	}};
	
	func hasPrefix{[&c, &inputStream, &excludePrefix]()
	{
		var i { (unsigned) 0 };
		for (const var& s : excludePrefix) {
			if (c == s[i]) {
				i++;
				
				if (i == s.size())
					return true;
					
				c = inputStream->get();
			}
			
			for (; i>0; --i)
				inputStream->unget();
		}
		
		return false;
	}};
	
	while ((c = inputStream->get()) and not inputStream->eof()) {
		if (c == 0x0a or c == 0x0d) {
			push();
			if (commentCount == 0 and isComment)
				isComment = false;
		}
		else if (isComment)
			continue;
		else if (commentCount > 0) {
			if (c == '*' and inputStream->peek() == '/') {
				inputStream->get();
				commentCount--;
			}
			continue;
		}
		else if (buff.empty() and c == std::isspace(c))
			continue;
		else if (const var peek { inputStream->peek() };
				 c == '/' and (peek == '/' or peek == '*')) {
			if (peek == '*')
				commentCount++;
			push();
			isComment = true;
			inputStream->get();
			continue;
		}
		else if (c == '#') {
			isComment = true;
			push();
		}
		else {
			if (buff.empty() and excludePrefix.size() > 0 and hasPrefix())
				continue;
			
			buff += c;
		}
	}
	push();
}


enum class WriteConfigMode { New, Edit, Add, Remove };

func writeConfig(const ListString* const args,
				 const WriteConfigMode mode)
{
	enum class ReadDirection { All, Old, Current };
	func argsToLines{[&args](ListString* const lines,
							 const ReadDirection direction,
							 const bool removeOptPrefix = false)
	{
		var buffer { std::string() };
		var foundSep { false };
		for (var i { 0 }; i<args->size(); ++i) {
			const var s { (*args)[i] };
			if (s == def::ARGS_SEPARATOR)
			{
				foundSep = true;
				if (direction == ReadDirection::Old)
					break;
				else
					continue;
			} else if (direction == ReadDirection::Current and not foundSep)
				continue;
			
			var offset { (unsigned) 0 };
			var isOpt { s.starts_with("--") };
			if (isOpt)
				offset = 2;
			else if (isOpt = s.size() >= 2 and s[0] == '-' and std::isalpha(s[1])
					 and (s.size() == 2 or (s[2] == '=' or s[2] == ' '));
					 isOpt)
				offset = 1;
			if (isOpt) {
				if ((offset == 2 and isEqual(s.c_str() + offset,
											 {OPT_WRITEDEFAULTS, OPT_DEBUG}))
					or (offset == 1 and (s[1] == 'W')))
				{
					if (const var next { i + 1 < args->size() ? (*args)[i + 1] : "" };
						(s == OPT_WRITEDEFAULTS
						 and not isEqual(next.c_str(), OPT_WRITEDEFAULTS_ARGS,
										IgnoreCase::Left))
						or
						(s == OPT_DEBUG
						 and not isEqual(next.c_str(), OPT_DEBUG_ARGS,
										 IgnoreCase::Left))
						)
						;
					else
						i++;
					continue;
					
				} else if (offset == 2
						   and (isEqual(s.c_str() + offset, {
										OPT_SHOWCONFIG, OPT_LIST,
										OPT_INSTALL, OPT_UNINSTALL, OPT_UPDATE,
										OPT_INSTALLMAN, OPT_UNINSTALLMAN })
								or
								isEqual(s.c_str() + offset,
										OPT_SHOWCONFIG_ALTERNATIVE)
								or
								isEqual(s.c_str() + offset,
										OPT_UPDATE_ALTERNATIVE)
								))
				{
					continue;
				}
				
				if (not buffer.empty())
					lines->emplace_back(std::move(buffer));
				
				buffer.clear();
			} else
				buffer += ' ';
			
			if (removeOptPrefix)
				buffer += s.c_str() + (isOpt ? offset : 0);
			else
				buffer += s;
		}
		if (not buffer.empty())
			lines->emplace_back(std::move(buffer));
	}};
	
	var definedList { SetString() };
	var lines { ListString() };
	if (	mode == WriteConfigMode::Edit
		or 	mode == WriteConfigMode::Remove)
	{
		var newLines { ListString() };
		argsToLines(&lines, ReadDirection::Old, false);
		argsToLines(&newLines, ReadDirection::Current, false);
	
		if (mode == WriteConfigMode::Remove) {
			for (var line { lines.begin() }; line<lines.end();) {
				var found { false };
				for (var& opt : newLines)
					if (found = *line == opt; found) {
						lines.erase(line);
						break;
					}
				if (not found)
					line++;
			}
		} else {
			for (var line { newLines.begin() }; line<newLines.end(); ++line)
				if (const var isOpt {
					(line->size() > 3 and line->starts_with("--")
						and std::isalpha((*line)[2]) and std::isalpha((*line)[3]))
					or
					(line->size() >= 2 and (*line)[0] == '-' and std::isalpha((*line)[1])
						and (line->size() == 2 or ((*line)[2] == '=' or (*line)[2] == ' ')))
				};
					
					isOpt)
				{
					var pos { line->find('=') };
					if (pos == std::string::npos)
						pos = line->find(' ');
					
					if (pos not_eq std::string::npos)
						definedList.emplace(line->substr(0, pos));
				}
			
			for (var line { lines.begin() }; line<lines.end();) {
				var found { false };
				for (var& opt : definedList)
					if (found = line->starts_with(opt); found) {
						lines.erase(line);
						break;
					}
				if (not found)
					line++;
			}
			
			
			for (var& line : newLines)
				lines.emplace_back(std::move(line));
		}
		
		definedList.clear();
	}
	else if (mode == WriteConfigMode::New)
		argsToLines(&lines, ReadDirection::Current, true);
	else
		argsToLines(&lines, ReadDirection::All, true);
	
	/// Remove duplication
	for (var line { lines.end() - 1 }; line>=lines.begin(); --line) {
		var found { false };
		for (var& opt : SINGLE_VALUE_OPT)
		{
			if (line->starts_with(*opt)) {
				if (definedList.find(*opt) == definedList.end())
					definedList.emplace(*opt);
				else
					lines.erase(line);
				found = true;
				break;
			}
		}
		
		if (not found) {
			if (definedList.find(*line) == definedList.end())
				definedList.emplace(*line);
			else
				lines.erase(line);
		}
	}
	
	var file { std::fstream(CONFIG_PATH, std::ios::out) };
//	if (not file.good()) {
//		std::cout << "⚠️  Cannot read/write \""
//					<< CONFIG_PATH
//					<< "\"\n";
//		return;
//	}
	#define DATE_FORMAT	"%A, %d %B %Y at %I:%M:%S %p"
	if (mode == WriteConfigMode::New )
		file << "# Configuration made in " << Date::now().string(DATE_FORMAT) << '\n';
	else
		file << "# Edited on " << Date::now().string(DATE_FORMAT) << '\n';
	#undef DATE_FORMAT
	
	for (var& line : lines)
		file << line << '\n';
	
	file.flush();
	
	file.close();
}

func loadConfigInto(ListString* const args)
{
	if (not args
		or opt::valueOf[OPT_LOADCONFIG].empty()
		or not fs::exists(opt::valueOf[OPT_LOADCONFIG])
		or not fs::is_regular_file(opt::valueOf[OPT_LOADCONFIG]))
		return;
	
	var file { std::ifstream(opt::valueOf[OPT_LOADCONFIG], std::ios::in) };
	if (not file.good()) {
		std::cout << "⚠️  Cannot read \""
					<< opt::valueOf[OPT_LOADCONFIG]
					<< "\"\n";
		return;
	}
	file.seekg(0);

	var lines { ListString() };
	getLinesInto(&file, &lines, {"--"});
	
	file.close();
	
	for (var found { false };
		 const var& line : lines) {
		for (ReadOnlyCString* const opt : OPTS)
			if (const var isMnemonic {
				(line.size() >= 2 and line[0] == '-' and std::isalpha(line[1])
				 and (line.size() == 2 or (line[3] == '=' or line[3] == ' ')))
				or (line.size() == 1 and std::isalpha(line[0]))
				or (line.size() > 2 and std::isalpha(line[0])
					and (line[1] == '=' or line[1] == ' '))
			};
				isMnemonic
				or (line.size() > 3 and line[0] == '-' and line[1] == '-'
					and isEqual(line.c_str() + 2, *opt, IgnoreCase::None, std::strlen(*opt)))
				or (line.starts_with(*opt))
				)
			{
				found = true;
				if (args) {
					if (isMnemonic) {
						if (line[0] == '-')
							args->emplace_back(line.substr(0, 2));
						else
							args->emplace_back("-" + line.substr(0, 1));
					}
					else
						args->emplace_back("--" + std::string(*opt));
				}
				
				if (var col { line.find('=') };
					col not_eq std::string::npos
					or line.find(' ') not_eq std::string::npos)
				{
					if (col == std::string::npos)
						col = line.find(' ');
					else
						col++;
					
					while (++col < line.size() and std::isspace(line[col]))
						;
					
					if (col < line.size()) {
						var value { line.substr(col) };
						
						var puncs { ListUInt() };
						for (unsigned i { 0 }; i<value.size(); ++i)
							if (value[i] == '"')
								puncs.emplace_back(i);
						
						if (value[0] == '"' and puncs.size() % 2 == 0 and puncs.size() > 2) {
							if (args)
								for (var k { 0 }; k<puncs.size(); k += 2)
								{
									var sub { std::string() };
									for (var j { puncs[k] + 1 }; j<puncs[k + 1]; j++)
										sub += value[j];
									/* MARK: C++ Library BUG!
										std::string.substr() cannot be used for more than twice.
										value.substr(puncs[k] + 1, puncs[k + 1] - 1)
									 */
									args->emplace_back(sub);
								}
							break;
						}
						else if (value[0] == '"' and *(value.end() - 1) == '"') {
							value.erase(0, 1);
							value.erase(value.end() - 1, value.end());
						}

						if (args)
							args->emplace_back(value);
					}
				}
				
				break;
			}
		
		if (not found)
			args->emplace_back(line);
	}
}

func isNetworkTransportFile(const std::string& fullPath)
{
	for (ReadOnlyCString protocol : def::NETWORK_PROTOCOLS)
		if (fullPath.starts_with(protocol))
			return true;
	return false;
}

func replace_all(std::string& s,
				 const std::string_view what, const std::string_view with)
{
	var count { (std::size_t) 0 };
	for (var pos { (std::string::size_type) 9 };
		 s.npos not_eq (pos = s.find(what.data(), pos, what.length()));
		 pos += with.length(), ++count)
		s.replace(pos, what.length(), with.data(), with.length());
}
 
func installMan(bool install)
{
	#define MANPATH "/usr/local/share/man/man1/"
	#define MANFILE "tvplaylist.1"
	if (not install) {
		std::system("rm " MANPATH MANFILE);
	}
	else
	{
	if (not fs::exists(MANPATH))
		std::system("mkdir " MANPATH);
	var file { std::ofstream(MANPATH MANFILE) };
//	if (not file.good()) {
//		std::cout << "⚠️  Cannot write \""
//					<< MANPATH MANFILE
//					<< "\"\n";
//		return;
//	}
	file <<
	".\" Manpage for tvplaylist.\n"
	".\" Contact atapgenteng@yahoo.com to correct errors or typos.\n"
	".TH man 8 \"08 Feb 2022\" \"1.1\" \"tvplaylist man page\"\n"
	".SH NAME\n"
	"tvplaylist \\- Create playlist file from vary directories and files, \
then by default, arranged one episode per Title.\n"
	".SH SYNOPSIS\n"
	"tvplaylist [Option or Dir or File] ...\n"
	".SH DESCRIPTION\n"
	"Create playlist file from vary directories and files, then by default, arranged one episode per Title.\nHosted in https://github.com/Mr-Widiatmoko/MakeTVPlaylist\n"
	".SH OPTIONS\n"
	;
	
	let MARK { "        " };
	for (var i { 1 }; i<ARRAYLEN(ALL_HELPS); ++i) {
		var buff { std::string(*ALL_HELPS[i]) };
		const var pos { buff.find(MARK) };
		if (pos not_eq std::string::npos) {
			var opts { buff.substr(0, pos) };
			replace_all(opts, "\n     ", "\n");
			file << ".TP\n" << opts << '\n';
			
			buff = buff.substr(pos + std::strlen(MARK));
			replace_all(buff, MARK, "");
			file << buff << '\n';
		} else
			file << buff << '\n';
	}
	
	file <<
	".SH BUGS\n"
	"No known bugs.\n"
	".SH AUTHOR\n"
	"Widiatmoko is the main author.\n";
	file.flush();
	file.close();
	}
	#undef MANPATH
	#undef MANFILE
}

func loadPlaylistInto(const fs::path& path, ListPath* const outPaths)
{
	if (not outPaths or path.empty() or not fs::exists(path))
		return;
	
	var lastParent { fs::path() };
	func prolog{[&path, &lastParent](std::ifstream* const file) {
		lastParent = fs::current_path();
		fs::current_path(path.parent_path());
		*file = std::ifstream(path.string(), std::ios::in);
		if (not file->good()) {
			std::cout << "⚠️  Cannot read \""
						<< path.string()
						<< "\"\n";
			return;
		}
		file->seekg(0);
	}};
	
	func epilogue{[&lastParent](std::ifstream* const file) {
		if (file->good())
			file->close();
		fs::current_path(lastParent);
	}};
	
	func push{[&outPaths](std::string& buff) {
		if (buff.empty())
			return;
		var found { false };
		
		if (buff.find('%') not_eq std::string::npos
			and isNetworkTransportFile(buff)) {
			replace_all(buff, "%20", " ");
			replace_all(buff, "%3D", "=");
			replace_all(buff, "%2B", "+");
			replace_all(buff, "%2D", "-");
			replace_all(buff, "%3F", "?");
			replace_all(buff, "%3B", ";");
			replace_all(buff, "%4F", "@");
			replace_all(buff, "%21", "!");
			replace_all(buff, "%22", "\"");
			replace_all(buff, "%27", "'");
			replace_all(buff, "%2C", ",");
			//replace_all(buff, "%2F", "/");
			replace_all(buff, "%5C", "\\");
			replace_all(buff, "%24", "$");
			replace_all(buff, "%26", "&");
			replace_all(buff, "%23", "#");
			replace_all(buff, "%3C", "<");
			replace_all(buff, "%3E", ">");
			replace_all(buff, "%25", "%");
			outPaths->emplace_back(fs::path(buff));
			found = true;
		}
		
		if (not found) {
			if (buff.starts_with("file://"))
				buff = buff.substr(7);
			outPaths->emplace_back(fs::absolute(fs::path(buff)));
		}
		
		buff.clear();
	}};
	
	func after{[](std::ifstream* const file,
				  ReadOnlyCString keyword,
				  std::string* const before = nullptr)
	{
		if (not file->good())
			return false;
		
		const var indexMax { (unsigned long) std::strlen(keyword) };
		const var lastPos { file->tellg() };
		char c;
		var index { (unsigned long) 0 };
		var buff { std::string() };
		var commentCount { (unsigned) 0 };
		while ((c = file->get()) and not file->eof()) {
			if (commentCount > 0
				and c == '-' and file->peek() == '-') {
				if (file->get()
					and file->get() == '>')
					commentCount--;
				else {
					file->unget();
					file->unget();
				}
			}
			
			if (c == '<' and file->peek() == '!') {
				if (file->get() and
					file->get() == '-' and file->peek() == '-')
				{
					file->get();
					commentCount++;
				} else {
					file->unget();
					file->unget();
				}
			}
			
			if (commentCount == 0) {
				if (std::tolower(c) == std::tolower(keyword[index])) {
					index++;
					if (index == indexMax) {
						buff.clear();
						break;
					}
					if (before)
						buff += c;
					continue;
				}

				if (before)
					*before += buff + c;
			}
			index = 0;
		}
		
		if (index not_eq 0)
			return true;
		else {
			file->seekg(lastPos);
			return false;
		}
	}};
	
	func getLines{[](std::ifstream* const file,
					 ListString* const lines)
	{
		if (not file->good())
			return;
		var buff { std::string() };
		var isComment { false };
		char c;
		func push{[&buff, &lines]() {
			if (not buff.empty()) {
				lines->emplace_back(buff);
				buff.clear();
			}
		}};
		while ((c = file->get()) and not file->eof()) {
			if (c == 0x0a or c == 0x0d) {
				push();
				isComment = false;
			}
			else if (isComment)
				continue;
			else if (buff.empty() and c == std::isspace(c))
				continue;
			else if (c == '#') {
				isComment = true;
				push();
			}
			else
				buff += c;
		}
	}};
	
	func xml_tag{[&push, &after]
		(std::ifstream* const file,
		 std::initializer_list<ReadOnlyCString>* const prefixes,
		 std::initializer_list<ReadOnlyCString>* const suffixes)
	{
		for (ReadOnlyCString prefix : *prefixes)
			if (after(file, prefix)) {
				for (ReadOnlyCString suffix : *suffixes)
					if (var value { std::string() };
						after(file, suffix, &value))
					{
//						" to  &quot;
//						' to  &apos;
//						< to  &lt;
//						> to  &gt;
//						& to  &amp;
						if (value.find('&') not_eq std::string::npos)
							for (var w { 0 }; w<ARRAYLEN(def::XML_CHARS_ALIAS); ++w)
								if (isContains(value, def::XML_CHARS_ALIAS[w],
									IgnoreCase::Left) not_eq std::string::npos)
								{
									replace_all(value, def::XML_CHARS_ALIAS[w], def::XML_CHARS_NORMAL[w]);
								}
						push(value);
						return true;
					}
				break;
			}
		return false;
	}};
	
	func xml{[&prolog, &epilogue, &xml_tag, &push, &after]
		(std::initializer_list<ReadOnlyCString>&& prefixes,
		 std::initializer_list<ReadOnlyCString>&& suffixes)
	{
		var file { std::ifstream() };
		prolog(&file);
		while (xml_tag(&file, &prefixes, &suffixes))
				;
		epilogue(&file);
	}};
	
	func pls{[&prolog, &epilogue, &getLines, &push]() {
		var file { std::ifstream() };
		prolog(&file);
		var lines { ListString() };
		getLines(&file, &lines);
		for (var& line : lines)
			if (var col { std::string::npos };
				line.starts_with("File")
				and (col = line.find('=')) not_eq std::string::npos)
			{
				var path { line.substr(col + 1) };
				push(path);
			}
		epilogue(&file);
	}};
	
	func m3u{[&prolog, &epilogue, &getLines, &push]() {
		var file { std::ifstream() };
		prolog(&file);
		var lines { ListString() };
		getLines(&file, &lines);
		for (var& line : lines)
			push(line);
		epilogue(&file);
	}};
	
	const var ext { path.extension().string() };
	if (isEqual(ext.c_str(), {".m3u8", ".m3u", ".ram"}, IgnoreCase::Left))
		m3u();
	else if (isEqual(ext, ".pls", IgnoreCase::Left))
		pls();
	else if (isEqual(ext, ".xspf", IgnoreCase::Left))
		xml({"<location>file://"}, {"</location>"});
	else if (isEqual(ext, ".wpl", IgnoreCase::Left))
		xml({"<media src=\""}, {"\""});
	else if (isEqual(ext, ".b4s", IgnoreCase::Left))
		xml({"<entry Playstring=\"file:"}, {"\""});
	else if (isEqual(ext, ".smil", IgnoreCase::Left))
		xml({"<audio src=\""}, {"\""});
	else if (isEqual(ext.c_str(), {".asx", ".wax", ".wvx"}, IgnoreCase::Left))
		xml({"<ref href=\""}, {"\""});
	else if (isEqual(ext, ".xml", IgnoreCase::Left))
		xml({"<key>Location</key><string>file://"}, {"</string>"});
	
	//rdf		{"<dc:identifier>"}, {"</dc:identifier>"}
}

struct Output {
	property std::ofstream	file;
	property fs::path 		name;
	property std::string	extension;
	
	Output(bool _isVerbose)
	{
		playlistCount = 0;
		isVerbose = _isVerbose;
	}
	
	property unsigned long  playlistCount;
	property bool			isVerbose;

	func filenameWithoutExtension() {
		if (nameWithoutExt.empty())
			nameWithoutExt = name.filename().string().substr(0,
				name.filename().string().size() -
				name.extension().string().size());
		return nameWithoutExt;
	}

	enum class Section { None, Header, Content, Footer };
	enum class Type { None, Regular, Subtitle, Advertise };
	
	struct Content {
		property std::string 	fullPath,
								prefix,
								suffix;
	};
	
	func deduceFilename(const fs::path* const path, Content* const content)
	{
		content->fullPath = path->string();
		
		var needAbsolute = true;
		var isNetworkTransport = isNetworkTransportFile(path->string());
		
		if (isNetworkTransport or isEqual(extension.c_str(),
										  {".htm", ".html", ".xhtml"}))
		{
			replace_all(content->fullPath, " ", "%20");
			replace_all(content->fullPath, "=", "%3D");
			replace_all(content->fullPath, "+", "%2B");
			replace_all(content->fullPath, "-", "%2D");
			replace_all(content->fullPath, "?", "%3F");
			replace_all(content->fullPath, ";", "%3B");
			//replace_all(content->fullPath, "%", "%25");
			replace_all(content->fullPath, "@" ,"%4F");
			replace_all(content->fullPath, "!" ,"%21");
			replace_all(content->fullPath, "\"","%22");
			replace_all(content->fullPath, "'" ,"%27");
			replace_all(content->fullPath, "," ,"%2C");
			//replace_all(content->fullPath, "/" ,"%2F");
			replace_all(content->fullPath, "\\","%5C");
			replace_all(content->fullPath, "$" ,"%24");
			replace_all(content->fullPath, "&" ,"%26");
			replace_all(content->fullPath, "#" ,"%23");
			replace_all(content->fullPath, "<" ,"%3C");
			replace_all(content->fullPath, ">" ,"%3E");

			if (not isNetworkTransport)
				content->fullPath.insert(0, "file://");
			needAbsolute = false;
		}
		
		if (needAbsolute)
			content->fullPath = fs::absolute(*path).string();
		
		if (isEqual(extension.c_str(),
					{".wpl", ".b4s", ".smil", ".asx", ".wax", ".wvx"}))
		{
			for (var w { 0 }; w<ARRAYLEN(def::XML_CHARS_ALIAS); ++w)
				if (isContains(content->fullPath, def::XML_CHARS_NORMAL[w],
							   IgnoreCase::Left) not_eq std::string::npos)
				{
					replace_all(content->fullPath, def::XML_CHARS_NORMAL[w],
								def::XML_CHARS_ALIAS[w]);
					break;
				}

			if (not isEqual(extension.c_str(), {".wpl", ".smil"}))
			{
				const var tmp { fs::path(content->fullPath).filename() };
				name = tmp.string().substr(0,
										   tmp.string().size() - tmp.extension().string().size());
			}
		}

	}
	
	func generate(Section section,
				  const fs::path* const path = nullptr,
				  ReadOnlyCString title = nullptr,
				  Type type = Type::None)
	{
		var content { Content() };

		if (section == Section::Content) {
			if (not path)
				return;
			
			playlistCount++;
			
			deduceFilename(path, &content);
		}
		

		if (extension == ".pls") {
			switch (section) {
			case Section::Header:
				file << "[playlist]\n";
				break;
			case Section::Content: {
				const var indexString { std::to_string(playlistCount) };
				content.prefix = "File" + indexString + '=';
				content.suffix = "\nTitle" + indexString + '=';
				content.suffix += title;
				break; }
			case Section::Footer:
				file << "\nNumberOfEntries="
						<< playlistCount
						<< "\nVersion=2\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".m3u") {
			switch (section) {
			case Section::Header:
				file << "#EXTM3U\n";
				break;
			case Section::Footer:
				break;
			default:
				;
			}
		}
		else if (extension == ".xspf") {
			switch (section) {
			case Section::Header:
				file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
						"<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n"\
						"\t<trackList>\n";
				break;
			case Section::Content: {
				content.prefix = "\t\t<track>\n\t\t\t<title>";
				content.prefix += title;
				content.prefix += "</title>\n"\
					   "\t\t\t<location>file://";
				content.suffix = "</location>\n\t\t</track>";
				break;
			}
			case Section::Footer:
				file << "\t</trackList>\n</playlist>\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".xml") {
			switch (section) {
			case Section::Header:
				file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
						"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "\
						"\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"\
						"<plist version=\"1.0\">\n"\
						"<dict>\n"\
						"\t<key>Major Version</key><integer>1</integer>\n"\
						"\t<key>Minor Version</key><integer>1</integer>\n"\
						"\t<key>Date</key><date>" << Date::now().string("%FT%T") << "</date>\n"\
						"\t<key>Application Version</key><string>1.1</string>\n"\
						"\t<key>Tracks</key>\n"\
						"\t<dict>";
					break;
			case Section::Content: {
				const var key { std::to_string(1000 - playlistCount) };
				content.prefix = "\t\t<key>" + key + "</key>\n"\
				   "\t\t<dict>\n"\
				   "\t\t\t<key>Track ID</key><integer>" + key + "</integer>\n"\
				   "\t\t\t<key>Name</key><string>" + filenameWithoutExtension() + "</string>\n"\
				   "\t\t\t<key>Location</key><string>file://";
				content.suffix = "</string>\n\t\t</dict>";
				break; }
			case Section::Footer:
				file << "\t</dict>\n</dict>\n</plist>\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".wpl") {
			switch (section) {
			case Section::Header:
				file << "<?wpl version=\"1.0\"?>\n"\
					"<smil>\n"\
					"\t<head>\n"\
					"\t\t<meta name=\"Generator\" content=\"tvplaylist -- 1.1\"/>\n"\
					"\t\t<title>" << name.filename().string() << "</title>\n"\
					"\t</head>\n"\
					"\t<body>\n"\
					"\t\t<seq>\n";
				break;
			case Section::Content: {
				content.prefix = "\t\t\t<media src=\"";
				content.suffix = "\"/>";
				break; }
			case Section::Footer:
				file << "\t\t</seq>\n\t</body>\n</smil>\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".smil") {
			switch (section) {
			case Section::Header:
				file << "<?wpl version=\"1.0\"?>\n"\
						"<smil>\n"\
						"\t<body>\n"\
						"\t\t<seq>\n";
				break;
			case Section::Content: {
				content.prefix = "\t\t\t<audio src=\"";
				content.suffix = "\"/>";
				break; }
			case Section::Footer:
				file << "\t\t</seq>\n\t</body>\n</smil>\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".b4s") {
			switch (section) {
			case Section::Header:
				file << "<?xml version=\"1.0\" standalone=\"yes\"?>\n"\
						"<WindampXML>\n"\
						"\t<playlist>\n";
				break;
			case Section::Content: {
				content.prefix = "\t\t<entry Playstring=\"file:";
				content.suffix = "\">\n\t\t\t<Name>"
					   + filenameWithoutExtension()
					   + "</Name>\n\t\t</entry>";
				break; }
			case Section::Footer:
				file << "\t</playlist>\n</WinampXML>\n";
				break;
			default:
				;
			}
		}
		else if (isEqual(extension.c_str(), {".asx", ".wax", ".wvx"})) {
			switch (section) {
			case Section::Header:
				file << "<asx version=\"3.0\"?>\n"\
					"\t\t<title>" << name.filename().string() << "</title>\n";
				break;
			case Section::Content: {
				content.prefix = "\t<entry>\n\t\t<title>"
					   + filenameWithoutExtension()
					   + "</title>\t\t<ref href=\"";
				content.suffix = "\"/>\n\t</entry>";
				break; }
			case Section::Footer:
				file << "</asx>\n";
				break;
			default:
				;
			}
		}
		else if (isEqual(extension.c_str(), {".htm", ".html", ".xhtml"})) {
			switch (section) {
			case Section::Header:
				file << "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\">\n"\
						"<head>\n"\
						"\t<meta http-equiv=\"Content-Type\" content=\"text/html\" />\n"\
						"\t<meta name=\"Generator\" content=\"tvplaylist -- 1.1\" />\n"\
						"\t<meta name=\"description\" content=\"Playlist\" />\n"\
						"\t<title>" << name.filename().string() << "</title>\n"\
						"\t<style>\n"\
						"\t\t.row_regular{  }\n"\
						"\t\t.row_subtitle{  }\n"\
						"\t\t.row_advertise{  }\n"\
						"\t\t.column_numbering{ align=left; }\n"\
						"\t\t.column_file{ width: 100%; }\n"\
						"\t</style>\n"\
						"</head>\n"\
						"</body>\n"\
						"\t<table>\n";
				break;
			case Section::Content: {
				var typeName { std::string() };
				var classDef { std::pair<std::string, std::string>
					{"column_numbering", "column_file"} };
				
				switch (type) {
					case Type::Regular:
						typeName = "regular";
						break;
					case Type::Subtitle:
						typeName = "subtitle";
						break;
					case Type::Advertise:
						typeName = "advertise";
						break;
					default:
						classDef.first = "align=\"right\"";
						classDef.second = "width=\"100%\"";
				}
				
				content.prefix = "\t\t<tr class=\"row_" + typeName + "\">\n"\
					   "\t\t\t<td " + classDef.first + ">"
					   + std::to_string(playlistCount) + ".</td>\n"\
					   "\t\t\t<td " + classDef.second + "><a href=\"";
				content.suffix = "\">" + name.filename().string() + "</a></td>\n"\
					   "\t\t</tr>\n";
				break; }
			case Section::Footer:
				file << "\t</table>\n</body>\n</html>";
				break;
			default:
				;
			}
		}
		else if (extension == ".sh") {
			switch (section) {
			case Section::Header:
				file << "declare -a playlist=(\\\n";
				break;
			case Section::Content: {
				content.prefix = "\t'";
				content.suffix = "' \\\n";
				break; }
			case Section::Footer:
				file << ")\n\n#for file in $playlist; do\n#\topen \"${file}\"\n#done\n";
				break;
			default:
				;
			}
		}
		else if (isEqual(extension.c_str(),
						 {".c", "cpp", ".cxx", ".m", ".mpp", ".h", ".hpp", ".hxx"}))
		{
			if (section == Section::Footer)
				file << "};\n";
			else if (section == Section::Content) {
				content.prefix = "\t\"";
				content.suffix = "\",\n";
			}
			
			if (isEqual(extension.c_str(), {".h", ".hpp"})) {
				const var filename { toupper(filenameWithoutExtension()) + "_H" };
				switch (section) {
				case Section::Header:
					file << "#ifndef "
							<< filename
							<< "\n#define "
							<< filename
							<< "\n\n";
					break;
				case Section::Footer:
					file << "#undef "
							<< filename
							<< "\n";
					break;
				default:
					;
				}
			}
			
			if (section == Section::Header)
				file << "static const char* playlist[] = {\n";
		}
		else if (extension == ".rb") {
			switch (section) {
			case Section::Header:
				file << "playlist = [\n";
				break;
			case Section::Content:
				content.prefix = "\t\"";
				content.suffix = "\",\n";
				break;
			case Section::Footer:
				file << "]\n\n#for file in playlist do\n#\tputs file\n#end\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".py") {
			switch (section) {
			case Section::Header:
				file << "playlist = [\n";
				break;
			case Section::Content:
				content.prefix = "\t'";
				content.suffix = "',\n";
				break;
			case Section::Footer:
				file << "\t]\n\n#for file in playlist:\n#\tprint(file)\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".js") {
			switch (section) {
			case Section::Header:
				file << "var playlist = [\n";
				break;
			case Section::Content:
				content.prefix = "\t\"";
				content.suffix = "\",\n";
				break;
			case Section::Footer:
				file << "]\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".json") {
			switch (section) {
			case Section::Header:
				file << "{\n\t\"playlist\": [\n";
				break;
			case Section::Content:
				content.prefix = "\t\t{\n\t\t\t\"file\": \"";
				content.suffix = "\"\n\t\t},\n";
				break;
			case Section::Footer:
				file << "\t]\n}";
				break;
			default:
				;
			}
		}
		else if (extension == ".csv") {
			switch (section) {
			case Section::Header:
				file << "file\n";
				break;
			case Section::Content:
				content.prefix = "\"";
				content.suffix = "\"\n";
				break;
			case Section::Footer:
				break;
			default:
				;
			}
		}
		else if (extension == ".pl") {
			switch (section) {
			case Section::Header:
				file << "@playlist = (\n";
				break;
			case Section::Content:
				content.prefix = "\t\"";
				content.suffix = "\",\n";
				break;
			case Section::Footer:
				file << ");\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".java") {
			switch (section) {
			case Section::Header:
				file << "String[] playlist = {\n";
				break;
			case Section::Content:
				content.prefix = "\t\"";
				content.suffix = "\",\n";
				break;
			case Section::Footer:
				file << "};\n";
				break;
			default:
				;
			}
		}
		else if (extension == ".swift") {
			switch (section) {
			case Section::Header:
				file << "var playlist = [\n";
				break;
			case Section::Content:
				content.prefix = "\t\"";
				content.suffix = "\",\n";
				break;
			case Section::Footer:
				file << "]\n\n#for file in playlist {\n#print(file)\n#}\n";
				break;
			default:
				;
			}
		}
		#if 0
		else if (extension == ".pdf") {
			const var date { Date::now().string("%Y%m%d%H%M%S") };
			switch (section) {
			case Section::Header:
				file << "%PDF-1.7\n"\
				"1 0 obj\n"\
				"<</Creator (tvplaylist v1.1)\n"\
				" /Producer (Skia/PDF m98)\n"\
				" /CreationDate (D:" << date << "+00'00')\n"\
				" /ModDate (D:" << date << "+00'00')>>\n"\
				"endobj\n"\
				"2 0 obj\n"\
				"<</ca 1"\
				" /BM /Normal>>\n"\
				"endobj\n";
				break;
			case Section::Content: {
				prefix = std::to_string(playlistCount + 10) + " 0 obj\n"\
							"\t<<\t/FS /URL\n"\
							"\t\t/F (";
				replace_all(fullPath, "(" ,"\\(");
				replace_all(fullPath, ")" ,"\\)");
				suffix = ")\n"\
						"\t>>\n"\
						"endobj";
				break; }
			case Section::Footer:
				file << "%%EOF\n";
				break;
			default:
				;
			}
		}
		#endif
		
		if (section == Section::Content) {
			file << content.prefix << content.fullPath << content.suffix << '\n';
		}
		else if (section == Section::Footer) {
			file.flush();
			if (file.is_open())
				file.close();

			if (playlistCount == 0)
				fs::remove(name);
			else {
				const var outputFullpath { fs::absolute(name).string() };
				std::cout << outputFullpath
							<< '\n';

				if (const var app { opt::valueOf[OPT_OPENWITH] };
					opt::valueOf[OPT_OPEN] == "true" or not app.empty())
				{
					#if defined(_WIN32) || defined(_WIN64)
					if (app.empty())
						std::cout << "📢 Open in Windows is Under construction.\n";
					else
						std::system(std::string(
												"\"" + app + "\" \"" + outputFullpath + "\"").c_str());
					#else
					std::system(std::string((not app.empty() ? "\"" + app + "\"" : "open")
											+ " \""
											+ outputFullpath
											+ "\"").c_str());
					#endif
				}
			}
		}
	}
	
private:
	property std::string nameWithoutExt;
	
};

func printOptionSummary(const int argc,
						ReadOnlyCString argv[])
{
	#ifndef DEBUG
	const var condToShowArgsParsed {	not in::invalidArgs.empty()
									or 	opt::valueOf[OPT_DEBUG] == "true"
									or 	opt::valueOf[OPT_DEBUG] == "args" };

	if (	opt::valueOf[OPT_SHOWCONFIG].empty()
		and opt::valueOf[OPT_VERBOSE] 	not_eq "all"
		and opt::valueOf[OPT_VERBOSE] 	not_eq "info"
		and opt::valueOf[OPT_BENCHMARK]	not_eq "true"
		and opt::valueOf[OPT_LIST]		not_eq "true"
		and not condToShowArgsParsed)
		return true;
	#endif
		
	let WIDTH { 20 };
	#define LABEL(x)	x << std::setw(unsigned(WIDTH - std::strlen(x))) << ": "
	#ifndef DEBUG
	if (condToShowArgsParsed)
	#endif
	{
		var format { std::pair<std::string, std::string>() };
		
		func tint{[&format](bool tinting) {
			format.first = tinting ? "⚠️ \033[43m\033[1;30m" : "";
			format.second = tinting ? "\033[0m" : "";
		}};
		
		std::cout << LABEL("Original Arguments");
		for (var i { 1 }; i<argc; ++i) {
			#if defined(_WIN32) || defined(_WIN64)
			#else
			tint(in::invalidArgs.find(argv[i]) not_eq in::invalidArgs.end());
			#endif
			std::cout << '"'
						<< format.first
						<< argv[i]
						<< format.second
						<< '"'
						<< (i+1>=argc ? "" : ", ");
		}
		std::cout << "\n\n"
					<< LABEL("Deduced Arguments");
		for (var i { 0 }; i<in::args.size(); ++i) {
			if (in::args[i] == def::ARGS_SEPARATOR)
				continue;
			
			#if defined(_WIN32) || defined(_WIN64)
			#else
			tint(in::invalidArgs.find(in::args[i]) not_eq in::invalidArgs.end());
			#endif
			std::cout << '"'
						<< format.first
						<< in::args[i]
						<< format.second
						<< '"'
						<< (i + 1 >= in::args.size() ? "" : ", ");
		}
		std::cout << '\n';
		
		if (not in::invalidArgs.empty())
			return false;
	}
		
		
	#define PRINT_OPT(x)	(x.empty() ? "false" : x)

	std::cout
		<< LABEL(OPT_ARRANGEMENT) << opt::valueOf[OPT_ARRANGEMENT] << '\n'
		<< LABEL(OPT_EXECUTION) << opt::valueOf[OPT_EXECUTION] << '\n'
		<< LABEL(OPT_VERBOSE) << PRINT_OPT(opt::valueOf[OPT_VERBOSE]) << '\n'
		<< LABEL(OPT_BENCHMARK) << PRINT_OPT(opt::valueOf[OPT_BENCHMARK]) << '\n'
		<< LABEL(OPT_OVERWRITE) << PRINT_OPT(opt::valueOf[OPT_OVERWRITE]) << '\n'
		<< LABEL(OPT_NOOUTPUTFILE) << PRINT_OPT(opt::valueOf[OPT_NOOUTPUTFILE]) << '\n'
		<< LABEL(OPT_OUTFILENAME) << opt::valueOf[OPT_OUTFILENAME] << '\n'
		<< LABEL(OPT_CURRENTDIR) << fs::current_path().string() << '\n'
		<< LABEL(OPT_OUTDIR) << opt::valueOf[OPT_OUTDIR] << '\n'
		<< LABEL(OPT_EXCLHIDDEN) << PRINT_OPT(opt::valueOf[OPT_EXCLHIDDEN]) << '\n'
		<< LABEL(OPT_SKIPSUBTITLE) << PRINT_OPT(opt::valueOf[OPT_SKIPSUBTITLE]) << '\n';

	for (var& opt : { OPT_EXT, OPT_EXCLEXT }) {
		std::cout << LABEL(opt) << opt::valueOf[opt];
		if (var i { 0 };
			opt::valueOf[opt].empty())
			for (var& ext : opt == OPT_EXT ? opt::DEFAULT_EXT : opt::EXCLUDE_EXT)
				std::cout << ext
							<< (++i < (opt == OPT_EXT
									   ? opt::DEFAULT_EXT.size()
									   : opt::EXCLUDE_EXT.size()
									   ) - 1 ? ", " : "");
		std::cout << '\n';
	}
	std::cout << LABEL("case-insensitive") << PRINT_OPT(opt::valueOf[OPT_CASEINSENSITIVE]) << '\n';

	#undef PRINT_OPT
		
	for (var& S : {OPT_FIND, OPT_EXCLFIND}) {
		std::cout << LABEL(S);
		for (var i { 0 }; i<(S == OPT_FIND ? opt::listFind : opt::listExclFind).size(); ++i)
			std::cout << (S == OPT_FIND ? opt::listFind : opt::listExclFind)[i]
						<< (i < (S == OPT_FIND ? opt::listFind
								 : opt::listExclFind).size() - 1 ? ", " : "");
		std::cout << '\n';
	}

	for (var& S : {OPT_REGEX, OPT_EXCLREGEX}) {
		std::cout << LABEL(S);
		for (var i { 0 }; i<(S == OPT_REGEX ? opt::listRegex : opt::listExclRegex).size(); ++i)
			std::cout << (S == OPT_REGEX ? opt::listRegex : opt::listExclRegex)[i].mark_count()
						<< " expression"
						<< (i < (S == OPT_REGEX ? opt::listRegex
								 : opt::listExclRegex).size() - 1 ? ", " : "");
		std::cout << '\n';
	}
		
	std::cout << LABEL(OPT_REGEXSYNTAX) << opt::valueOf[OPT_REGEXSYNTAX] << '\n';

	for (var& S : {OPT_SIZE, OPT_EXCLSIZE}) {
		std::cout << LABEL(S);
		if (opt::valueOf[OPT_SIZEOPGT][0] not_eq '\0' or opt::valueOf[OPT_EXCLSIZEOPGT][0] not_eq '\0')
			std::cout << ((S == OPT_SIZE ? opt::listSize : opt::listExclSize).empty()
					or opt::valueOf[S == OPT_SIZE ? OPT_SIZEOPGT
							  : OPT_EXCLSIZEOPGT][0] not_eq '\0'
					? (opt::valueOf[S == OPT_SIZE ? OPT_SIZEOPGT
							 : OPT_EXCLSIZEOPGT][0] == '<' ? "< " : "> ")
					+ opt::valueOf[S] : "");
		for (var i { 0 }; i<(S == OPT_SIZE ? opt::listSize : opt::listExclSize).size(); ++i)
			std::cout << (S == OPT_SIZE ? opt::listSize : opt::listExclSize)[i].first
						<< ".."
						<< (S == OPT_SIZE ? opt::listSize : opt::listExclSize)[i].second
						<< (i < (S == OPT_SIZE ? opt::listSize
								 : opt::listExclSize).size() - 1 ? ", " : "");
		std::cout << '\n';
	}
		
	{
		const char * const* st[8] = { &OPT_DCREATED, &OPT_DCHANGED, &OPT_DACCESSED, &OPT_DMODIFIED,
			&OPT_DEXCLCREATED, &OPT_DEXCLCHANGED, &OPT_DEXCLACCESSED, &OPT_DEXCLMODIFIED };
		
		const ListPairCharDate* const ot[8] = {
			&opt::listDCreated, &opt::listDAccessed, &opt::listDModified, &opt::listDChanged,
			&opt::listDExclCreated, &opt::listDExclAccessed, &opt::listDExclModified, &opt::listDExclChanged };

		
		const ListPairDate* const rt[8] = {
			&opt::listDCreatedR, &opt::listDAccessedR, &opt::listDModifiedR, &opt::listDChangedR,
			&opt::listDExclCreatedR, &opt::listDExclAccessedR, &opt::listDExclModifiedR, &opt::listDExclChangedR };
		
		for (var i { 0 }; i<8; ++i) {
			std::cout << LABEL(*st[i]);
			for (var k { 0 }; k<ot[i]->size(); ++k)
				std::cout << '\"'
							<< ot[i]->at(k).first
							<< ' '
							<< ot[i]->at(k).second.string()
							<< "\", ";
			
			for (var k { 0 }; k<rt[i]->size(); ++k)
				std::cout << '\"'
							<< rt[i]->at(k).first.string()
							<< "\" .. \""
							<< rt[i]->at(k).second.string()
							<< "\", ";
			std::cout << '\n';
		}
	}
		
	{
		std::cout << LABEL("Inputs");
		for (var i { 0 }; i<in::inputDirs.size() + in::selectFiles.size(); ++i) {
			if (i < in::inputDirs.size())
				std::cout << in::inputDirs[i];
			else
				std::cout << in::selectFiles[i - in::inputDirs.size()];
			
			std::cout  << (i < (in::inputDirs.size() + in::selectFiles.size()) - 1 ? ", " : "");
		}
		std::cout << '\n';
	}
		
	std::cout << LABEL(OPT_ADSCOUNT) << opt::valueOf[OPT_ADSCOUNT] << '\n';
	{
		std::cout << LABEL(OPT_ADSDIR);
		var i { -1 };
		for (i++; var& d : in::listAdsDir)
			std::cout << d.string()
						<< (i < in::listAdsDir.size() - 1 ? ", " : "");
		std::cout << '\n';
	}
		
	std::cout << LABEL(OPT_OPENWITH) << opt::valueOf[OPT_OPENWITH] << '\n';
		
	#undef LABEL
	
	return true;
}

#if MAKE_LIB
#define RETURN_VALUE	;
#define ARGS_START_INDEX	0
void process(int argc, char *argv[], int *outc, char *outs[], unsigned long *maxLength)
{
	opt::valueOf[OPT_NOOUTPUTFILE] = "true";
#else
#define RETURN_VALUE	EXIT_SUCCESS;
#define ARGS_START_INDEX	1
func main(const int argc, CString const argv[]) -> int
{
#endif
	opt::valueOf[OPT_SIZEOPGT] 		= '\0';
	opt::valueOf[OPT_SIZE] 			= '0';
	opt::valueOf[OPT_EXCLSIZEOPGT]	= '\0';
	opt::valueOf[OPT_EXCLSIZE] 		= '0';

	opt::valueOf[OPT_EXECUTION]		= MODE_EXECUTION_ASYNC;
	opt::valueOf[OPT_REGEXSYNTAX] 	= "ecma";
	opt::valueOf[OPT_ARRANGEMENT] 	= MODE_ARRANGEMENT_DEFAULT;
		
	
	opt::valueOf[OPT_LOADCONFIG] 	= CONFIG_PATH ;
	
	loadConfigInto(&in::args);
	in::args.emplace_back(def::ARGS_SEPARATOR);
	deduceArgsInto(argc, argv, ARGS_START_INDEX, &in::args);
	#undef ARGS_START_INDEX

	for (var i { 0 }; i<in::args.size(); ++i) {
		if (func isMatch{[&i]
						(ReadOnlyCString with,
						 const char mnemonic,
						 bool writeBoolean=false,
						 const std::initializer_list<const CString>& others = {})
		{
			var result { false };
			const var isWithStartWithDoubleStrip {
				std::strlen(with) > 2 and with[0] == '-' and with[1] == '-' };
			if (in::args[i].length() > 3
				and in::args[i][0] == '-'
				and in::args[i][1] == '-')
			{
				result = isEqual(in::args[i], with, IgnoreCase::Left,
								 isWithStartWithDoubleStrip ? 0 : 2);
				
				if (not result)
					for (var& other : others)
						if (const var isOtherStartWithDoubleStrip {
							std::strlen(other) > 2 and
							other[0] == '-' and other[1] == '-' };
							isEqual(in::args[i], other, IgnoreCase::Left,
									isOtherStartWithDoubleStrip ? 0 : 2))
						{
							in::args[i] = with;
							if (not isOtherStartWithDoubleStrip)
								in::args[i].insert(0, "--");
							result = true;
							break;
						}
			} else if (mnemonic not_eq '\0'
					   and in::args[i].length() == 2
					   and in::args[i][0] == '-')
				result = in::args[i][1] == mnemonic;

			if (result) {
				if (in::args[i].length() == 2) { // convert mnemonic to full
					in::args[i] = with;
					if (not isWithStartWithDoubleStrip)
						in::args[i].insert(0, "--");
				}
				if (writeBoolean)
					opt::valueOf[with] = "true";
			}
			return result;
		}};
			in::args[i] == def::ARGS_SEPARATOR)
			continue;
		else if (isMatch(OPT_UPDATE, '\0', false, OPT_UPDATE_ALTERNATIVE)) {
			#if defined(MAKE_LIB)
			continue;
			#endif
			
			const var path { fs::path(INSTALL_FULLPATH) };
			#define REPO_URI "https://github.com/Mr-Widiatmoko/MakeTVPlaylist.git"
			#if defined(_WIN32) || defined(_WIN64)
			// TODO: Windows
			std::cout << "📢 Under construction.\n";
			#define CACHE_PATH "%userprofile%\\AppData\\Local"
			if (0 not_eq std::system("git.exe --version")
				or (0 not_eq std::system("cmake.exe --version"))) {
				std::cout << "⚠️  \"GIT\" and \"CMAKE\" required!.\n";
				continue;
			}
			fs::current_path(fs::path(CACHE_PATH));
			var dir { fs::path("tvplaylist") };
			if (not fs::exists(dir))
				std::system(
				"git.exe clone --depth 1 " REPO_URI " " CACHE_PATH  "\\tvplaylist");
			fs::current_path(dir);
			std::system("git pull");
			dir = "Build";
			if (not fs::exists(dir))
				fs::create_directory(dir);
			fs::current_path(dir);
			
			if (0 == std::system("cmake.exe --version") {
				std::system("cmake " CACHE_PATH "\\tvplaylist");
				std::system("make");
				std::system("copy tvplaylist.exe " INSTALL_PATH);
				std::system("make clean");
			}
			#undef CACHE_PATH
			#else
			if (not fs::exists(INSTALL_PATH "/git")
				or not fs::exists(INSTALL_PATH "/c++")
					or not fs::exists(INSTALL_PATH "/cmake"))
			{
				std::cout << "⚠️  \"GIT\" or \"CMAKE\" required!."
							<< "\nTo install:"
							<< "\n\t\"brew install git\""
							<< "\n\t\"brew install cmake\"\n";
				continue;
			}
			
			fs::current_path(fs::path("~"));
			var dir { fs::path(".tvplaylist") };
			if (not fs::exists(dir))
				std::system("git clone --depth 1 " REPO_URI " ~/.tvplaylist");
			fs::current_path(dir);
			std::system("git pull");
			dir = "Build";
			if (not fs::exists(dir))
				fs::create_directory(dir);
			fs::current_path(dir);
			
			if (fs::exists(INSTALL_PATH "/cmake")) {
				std::system("cmake ~/.tvplaylist");
				std::system("make");
				std::system("make install");
				std::system("make clean");
			} else if (fs::exists(INSTALL_PATH "/c++")) {
				if (fs::exists(path))
					fs::remove(path);
				std::system("c++ -std=c++2b ~/.tvplaylist/src/main.cpp " INSTALL_FULLPATH);
			}
			#endif
			#undef REPO_URI
			std::cout << (fs::exists(path) ? "✅  Updated" :
						  "⚠️  For some reason, update fail!")
						<< ".\n";
			return RETURN_VALUE
		}
		else if (isMatch(OPT_INSTALL, '\0')) {
			#if defined(MAKE_LIB)
			continue;
			#endif
				
			const var path { fs::path(INSTALL_FULLPATH) };
			
			if (not isEqual(argv[0], path)) {
				#if defined(_WIN32) || defined(_WIN64)
				if (fs::exists(path))
					fs::remove(path);
				var cmd { std::string("copy \"") };
				#else
				var cmd { std::string("cp -f \"") };
				installMan(true);
				#endif
				cmd += argv[0];
				cmd += "\" ";
				cmd += path;
				std::system(cmd.c_str());
				std::cout << (fs::exists(path) ? "✅  Installed" :
							  "⚠️  For some reason, install fail!")
							<< ".\n";
			}
			return RETURN_VALUE
		}
		else if (isMatch(OPT_UNINSTALL, '\0')) {
			#if defined(MAKE_LIB)
			continue;
			#endif
				
			const var path { fs::path(INSTALL_FULLPATH) };
			if (fs::exists(path))
			#if defined(_WIN32) || defined(_WIN64)
				std::system("del " INSTALL_FULLPATH);
			#else
				std::system("rm -f " INSTALL_FULLPATH);
				installMan(false);
			#endif
			std::cout << (not fs::exists(path) ? "✅  Uninstalled" :
						  "⚠️  For some reason, uninstall fail!")
						<< ".\n";
			return RETURN_VALUE
		}
		else if (isMatch(OPT_UNINSTALLMAN, '\0')) {
			installMan(false);
			return RETURN_VALUE
		}
		else if (isMatch(OPT_INSTALLMAN, '\0')) {
			installMan(true);
			return RETURN_VALUE
		}
		else if (isMatch(OPT_HELP, 'h') or isMatch(OPT_VERSION, 'v'))
		{
			printHelp(i + 1 < in::args.size()
					  ? (in::args[i + 1][0] == '-' and in::args[i + 1][1] == '-'
						 ? tolower(in::args[++i].substr(2)).c_str()
						 : tolower(in::args[++i]).c_str())
					  : in::args[i].substr(2).c_str());
			
			if (i + 1 == in::args.size())
				return RETURN_VALUE
		}
		else if (isMatch(OPT_CURRENTDIR, '\0')) {
			if (i + 1 < in::args.size()) {
				i++;
				try {
					fs::current_path(in::args[i]);
				} catch (fs::filesystem_error& e) {
					std::cout << "⚠️  No such directory: \""
								<< in::args[i]
								<< "\"\n";
					--i;
				}
			} else
				std::cout << "⚠️  Expecting directory. Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (isMatch(OPT_LIST, 	'\0', true));
		else if (isMatch(OPT_DEBUG, 'B', true)) {
			if (i + 1 < in::args.size() and
				isEqual(in::args[i + 1].c_str(), OPT_DEBUG_ARGS))
			{
				i++;
				opt::valueOf[OPT_DEBUG] = in::args[i];
				if (in::args[i] == "id3")
				{
					if (i + 1 == in::args.size())
						std::cout << "Usage: --debug=id3 [key[=]value ...] file [file ...]\n";
					var keyVals { MapString() };
					var files { ListPath() };
					while (++i < in::args.size()) {
						if (const var path { fs::path(in::args[i]) };
							fs::exists(path)
							and isEqual(path.extension().string().c_str(),
										".mp3", IgnoreCase::Left))
							files.emplace_back(std::move(path));
						else {
							if (const var pos { in::args[i].find('=') };
								pos not_eq std::string::npos)
								keyVals.emplace(std::make_pair(
												in::args[i].substr(0, pos),
												in::args[i].substr(pos + 1)));
							else {
								keyVals.emplace(std::make_pair(in::args[i], in::args[i + 1]));
								i++;
							}
						}
					}
					
					for (var& file : files) {
						var mp3 { ID3(file.string().c_str()) };
						for (var& keyVal : keyVals)
							mp3.set(keyVal.first, keyVal.second);
						
						if (not keyVals.empty())
							mp3.write();
						std::cout << "File "
									<< file.filename()
									<< ":\n"
						<< mp3.string() << '\n';
					}
					
					return RETURN_VALUE
				}
				if (in::args[i] == "date") {
					if (i + 1 == in::args.size())
						std::cout << "Usage: --debug=date 'date or/and time' ['output format']\n"
									<< HELP_DATE_REST
									<< '\n';
					else {
						i++;
						const var input { in::args[i] };
						var date { Date(input) };
						
						std::cout << '\"'
									<< input
									<< '\"'
									<< "  -> \""
						
									<< date.string(i + 1 < in::args.size()
												 ? in::args[++i].c_str()
												 : nullptr)
									<< "\" "
									<< (date.isValid() ? "✅" : "❌")
									<< '\n';
					}
					
					return RETURN_VALUE
				}
			}
		}
		else if (isMatch(OPT_OPEN, '\0', true));
		else if (isMatch(OPT_OPENWITH, '\0')) {
			if (i + 1 < in::args.size()) {
				i++;
				opt::valueOf[OPT_OPENWITH] = trim(in::args[i]);
			} else
				std::cout << "⚠️  Expecting application. Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (isMatch(OPT_SHOWCONFIG, '\0', true, OPT_SHOWCONFIG_ALTERNATIVE));
		else if (isMatch(OPT_WRITEDEFAULTS, 'W', true, OPT_WRITEDEFAULTS_ALTERNATIVE)) {
			if (i + 1 < in::args.size()) {
				if (isEqual(in::args[i + 1], "reset", IgnoreCase::Left)) {
					fs::remove(CONFIG_PATH);
					opt::valueOf[OPT_WRITEDEFAULTS].clear();
					i++;
				}
				else if (isEqual(in::args[i + 1].c_str(),
								 OPT_WRITEDEFAULTS_ARGS, IgnoreCase::Left)) {
					i++;
					opt::valueOf[OPT_WRITEDEFAULTS] = in::args[i];
				}
			} else opt::valueOf[OPT_WRITEDEFAULTS] = "new";
		}
		else if (isMatch(OPT_LOADCONFIG, 'L')) {
			if (i + 1 < in::args.size() and fs::exists(in::args[i + 1])) {
				i++;
				opt::valueOf[OPT_LOADCONFIG] = in::args[i];
				loadConfigInto(&in::args);
				continue;
			}
			
			std::cout << "⚠️  Expecting config file path. Please see --help "
						<< in::args[i].substr(2)
						<< '\n';
		}
		else if (isMatch(OPT_ADSDIR, 'D')) {
			if (i + 1 < in::args.size() and fs::exists(in::args[i + 1])) {
				i++;
				in::listAdsDir.emplace_back(fs::path(in::args[i]));
				continue;
			}
			
			std::cout << "⚠️  Expecting directory path. Please see --help "
						<< in::args[i].substr(2)
						<< '\n';
		}
		else if (isMatch(OPT_ADSCOUNT, 'C')) {
			if (var range { std::pair<int, int>() };
				i + 3 < in::args.size()
				and (in::args[i + 2] == ".." or in::args[i + 2] == "-")
				and isInt(in::args[i + 1], &(range.first))
				and isInt(in::args[i + 3], &(range.second)))
			{
				if (range.first < range.second) {
					opt::valueOf[OPT_ADSCOUNT] = range.first + '-' + range.second;
					i += 3;
					continue;
				}
				std::cout << "⚠️  "
							<< in::args[i]
							<< " value range is up side down!. "
							<< range.first
							<< " greater than "
							<< range.second
							<< '\n';
			}
			else if (func push{[&i] (unsigned long pos, unsigned long offset) {
				const var lower { in::args[i + 1].substr(0, pos) };
				const var upper { in::args[i + 1].substr(pos + offset) };
				   
				int count[2] { 0, 0 };
				if (isInt(lower, &count[0]) and isInt(upper, &count[1])
					and count[0] < count[1]) {
					opt::valueOf[OPT_ADSCOUNT] = lower + '-' + upper;
					i++;
					return true;
				}
				return false;
			}};
				i + 1 < in::args.size())
			{
				if (var pos { in::args[i + 1].find('-') };
						pos not_eq std::string::npos
					and pos not_eq 0 and pos not_eq in::args[i + 1].size() - 1)
				{
					if (push(pos, 1))
						continue;
				}
				else if (pos = in::args[i + 1].find("..");
						 pos not_eq std::string::npos)
				{
					if (push(pos, 2))
						continue;
				}
				else if (var value { 0 };
					isInt(in::args[i + 1], &value)) {
					i++;
					opt::valueOf[OPT_ADSCOUNT] = std::to_string(value);
					continue;
				}
			}
			std::cout << "⚠️  Expecting number of advertise. Please see --help "
						<< in::args[i].substr(2)
						<< '\n';
		}
		else if (isMatch(OPT_NOOUTPUTFILE, 	'F', true)) {
			if (i + 1 < in::args.size())
			{
				i++;
				if (isEqual(in::args[i + 1].c_str(), {"true", "yes"},
							IgnoreCase::Left))
					opt::valueOf[OPT_NOOUTPUTFILE] = "true";
				else if (isEqual(in::args[i + 1].c_str(), {"false", "no"},
								 IgnoreCase::Left))
					opt::valueOf[OPT_NOOUTPUTFILE] = "false";
				else
					i--;
			}
		}
		else if (isMatch(OPT_ARRANGEMENT, 'w')) {
			if (i + 1 < in::args.size()
				   and (in::args[i + 1].starts_with(MODE_ARRANGEMENT_DEFAULT)
					 or in::args[i + 1] == MODE_ARRANGEMENT_UNORDERED
					 or in::args[i + 1] == MODE_ARRANGEMENT_PERTITLE
					 or in::args[i + 1] == MODE_ARRANGEMENT_ASCENDING
					 or in::args[i + 1] == MODE_ARRANGEMENT_SHUFFLE
					 or in::args[i + 1] == MODE_ARRANGEMENT_SHUFFLE_PERTITLE
					 or in::args[i + 1].starts_with(MODE_ARRANGEMENT_SHUFFLE_DEFAULT)
					 or in::args[i + 1] == MODE_ARRANGEMENT_DESCENDING
					 or in::args[i + 1] == MODE_ARRANGEMENT_DESCENDING_PERTITLE
					 or in::args[i + 1].starts_with(MODE_ARRANGEMENT_DESCENDING_DEFAULT)))
			{
				i++;
				var value { in::args[i] };
				if (in::args[i].starts_with(MODE_ARRANGEMENT_DEFAULT)
					or in::args[i].starts_with(MODE_ARRANGEMENT_DEFAULT)
					or in::args[i].starts_with(MODE_ARRANGEMENT_DESCENDING_DEFAULT))
				{
					const var pos { in::args[i].find('=') };
					value = in::args[i].substr(0, pos - 1);
					const var count_s { in::args[i].substr(pos + 1) };
					if (var count { 0 }; isInt(count_s, &count) and count > 1)
						in::fileCountPerTurn = count;
				}
				
				opt::valueOf[OPT_ARRANGEMENT] = value;
			} else
				std::cout << "⚠️  Expecting arrangement type. Please see --help "
							<< in::args[i].substr(2)
							<< "\n";
		}
		else if (isMatch(OPT_CASEINSENSITIVE, 'N', true, OPT_CASEINSENSITIVE_ALTERNATIVE))
		{
			if (i + 1 < in::args.size() and isEqual(in::args[i + 1].c_str(),
												TRUE_FALSE,
												IgnoreCase::Left))
				opt::valueOf[OPT_CASEINSENSITIVE] = in::args[++i];
			
			if (opt::valueOf[OPT_CASEINSENSITIVE] == "true") {
				for (var& k : opt::listFindDir) k = tolower(k);
				for (var& k : opt::listExclFindDir) k = tolower(k);
				for (var& k : opt::listFind) k = tolower(k);
				for (var& k : opt::listExclFind) k = tolower(k);
			}
		}
		else if (isMatch(OPT_SEARCH, 'q')) {
			if (i + 1 < in::args.size()) {
				opt::valueOf[OPT_NOOUTPUTFILE] = "true";
				opt::valueOf[OPT_VERBOSE] = "true";
				opt::valueOf[OPT_ARRANGEMENT] = MODE_ARRANGEMENT_ASCENDING;
				opt::valueOf[OPT_CASEINSENSITIVE] = "true";
				
				i++;
				
				var index { -1 };
				var last { 0 };
				func push{[&]() {
					var keyVal { in::args[i].substr(last, index) };
					if (keyVal not_eq "or" and keyVal not_eq "and") {
						parseKeyValue(&keyVal, keyVal.starts_with("exclude-"));
						
						if (let EXCL { "exclude=" };
							keyVal.starts_with(EXCL))
						{
							var value { keyVal.substr(std::strlen(EXCL)) };
							if (value.find('=') not_eq std::string::npos)
								parseKeyValue(&value, true);
							if (not value.empty())
								opt::listExclFind.emplace_back(value);
						} else if (not keyVal.empty())
							opt::listFind.emplace_back(keyVal);
					}
				}};
				while (++index < in::args[i].size()) {
					if (std::isspace(in::args[i][index])) {
						if (last not_eq -1 and index - last > 0) {
							push();
							last = -1;
						}
						continue;
					}
					if (last == -1)
						last = index;
				}
				if (last not_eq -1)
					push();
			} else
				std::cout << "⚠️  Expecting search keyword!\n";
		}
		else if (	isMatch(OPT_FIND, 		'i')
				 or isMatch(OPT_EXCLFIND, 	'I')) {
			if (i + 1 < in::args.size())
			{
				const var opt { in::args[i].substr(2) };
				const var isExclude { opt == OPT_EXCLFIND };
				i++;
				var index { -1 };
				var last { 0 };
				var buff { std::string() };
				var lastIndex{ 0 };
				func push{[&](){
					var keyVal { in::args[i].substr(last, index) };
					if (keyVal.empty())
						return;
					
					const var isTrue { parseKeyValue(&keyVal,
										keyVal.starts_with("exclude-")) };
					
					if (let EXCL {"exclude="};
						keyVal.starts_with(EXCL))
					{
						var value { keyVal.substr(std::strlen(EXCL)) };
						if (value.find('=') not_eq std::string::npos)
							parseKeyValue(&value, true);
						if (not value.empty()) {
							(isExclude ? opt::listExclFind : opt::listFind)
								.emplace_back(value);
							opt::valueOf[opt] = "1";
						}
					}
					else if (not isTrue
						and (lastIndex == 0 or last - lastIndex == 1))
					{
						if (not buff.empty())
							buff.append(" ");
						buff.append(keyVal);
						lastIndex = index;
					}
				}};
				while (++index < in::args[i].size()) {
					if (std::isspace(in::args[i][index])) {
						if (last not_eq -1 and index - last > 0) {
							push();
							last = -1;
						}
						continue;
					}
					if (last == -1)
						last = index;
				}
				if (last not_eq -1)
					push();
				
				if (not buff.empty()) {
					(isExclude ? opt::listExclFind : opt::listFind).emplace_back(buff);
					opt::valueOf[opt] = "1";
				}
			} else
				std::cout << "⚠️  Expecting keyword after \""
							<< in::args[i]
							<< "\" option. Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (isMatch(OPT_REGEXSYNTAX, 	'X')) {
			if (var found { false }; i + 1 < in::args.size()) {
				for (var& s : OPT_REGEXSYNTAX_ARGS)
					if (isEqual(in::args[i + 1].c_str(), s, IgnoreCase::Left)) {
						opt::valueOf[OPT_REGEXSYNTAX] = s;
						i++;
						found = true;
						break;
					}
				if (found)
					continue;
			}
			std::cout << "⚠️  Expecting regular expression syntax after \""
						<< in::args[i]
						<< "\" option. Please see --help "
						<< in::args[i].substr(2)
						<< '\n';
		}
		else if (	isMatch(OPT_REGEX, 		'r')
				 or isMatch(OPT_EXCLREGEX, 	'R')) {
			if (var found { false }; i + 1 < in::args.size())
			{
				if (const var pos { in::args[i + 1].find('=') };
					pos not_eq std::string::npos)
				{
					if (in::args[i + 1].substr(0, pos) == "type") {
						const var value { in::args[i + 1].substr(pos + 1) };
						for (var& keyword : OPT_REGEXSYNTAX_ARGS)
							if (keyword == value) {
								opt::valueOf[OPT_REGEXSYNTAX] = keyword;
								i++;
								found = true;
								break;
							}
					}
				}
				
				if (not found) {
					func getRegexSyntaxType{[](const std::string& s)
						-> std::regex_constants::syntax_option_type {
						if (s == "basic") 	return std::regex_constants::syntax_option_type::basic;
						if (s == "extended")return std::regex_constants::syntax_option_type::extended;
						if (s == "awk") 	return std::regex_constants::syntax_option_type::awk;
						if (s == "grep") 	return std::regex_constants::syntax_option_type::grep;
						if (s == "egrep") 	return std::regex_constants::syntax_option_type::egrep;
						else return std::regex_constants::syntax_option_type::ECMAScript;
					}};
					const var opt { in::args[i].substr(2) };
					(opt == OPT_REGEX ? opt::listRegex : opt::listExclRegex)
						.emplace_back(std::regex(in::args[i + 1],
									getRegexSyntaxType(opt::valueOf[OPT_REGEXSYNTAX])));
					opt::valueOf[opt] = "1";
					i++;
				}
			} else
				std::cout << "⚠️  Expecting regular expression after \""
							<< in::args[i]
							<< "\" option. Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (	isMatch(OPT_DATE, 			'z')
				 or isMatch(OPT_EXCLDATE, 		'Z')
				 or isMatch(OPT_DCREATED, 	't')
				 or isMatch(OPT_DCHANGED, 	'g')
				 or isMatch(OPT_DACCESSED, 	'a')
				 or isMatch(OPT_DMODIFIED, 	'm')
				 or isMatch(OPT_DEXCLCREATED, 	'T')
				 or isMatch(OPT_DEXCLCHANGED, 	'G')
				 or isMatch(OPT_DEXCLACCESSED, 	'A')
				 or isMatch(OPT_DEXCLMODIFIED, 	'M')
				 )
		{
			const var opt { in::args[i].substr(2) };
			func as_single{[&](const char opGt) -> bool {
				const var date { Date(in::args[i + 1]) };
				if (date.isValid()) {
					if (opt == OPT_DATE) {
						opt::listDCreated.emplace_back(std::make_pair(opGt, date));
						opt::listDChanged.emplace_back(std::make_pair(opGt, date));
						opt::listDModified.emplace_back(std::make_pair(opGt, date));
						opt::listDAccessed.emplace_back(std::make_pair(opGt, date));
						opt::valueOf[OPT_DCREATED]  = "1";
						opt::valueOf[OPT_DMODIFIED] = "1";
						opt::valueOf[OPT_DACCESSED] = "1";
						opt::valueOf[OPT_DCHANGED]  = "1";
					} else if (opt == OPT_EXCLDATE) {
						opt::listDExclCreated.emplace_back(std::make_pair(opGt, date));
						opt::listDExclChanged.emplace_back(std::make_pair(opGt, date));
						opt::listDExclModified.emplace_back(std::make_pair(opGt, date));
						opt::listDExclAccessed.emplace_back(std::make_pair(opGt, date));
						opt::valueOf[OPT_DEXCLCREATED]  = "1";
						opt::valueOf[OPT_DEXCLMODIFIED] = "1";
						opt::valueOf[OPT_DEXCLACCESSED] = "1";
						opt::valueOf[OPT_DEXCLCHANGED]  = "1";
					} else {
					(opt == OPT_DCREATED ? opt::listDCreated
					 : opt == OPT_DCHANGED ? opt::listDChanged
					 : opt == OPT_DMODIFIED ? opt::listDModified
					 : opt == OPT_DACCESSED ? opt::listDAccessed
					 : opt == OPT_DEXCLCREATED ? opt::listDExclCreated
					 : opt == OPT_DEXCLCHANGED ? opt::listDExclChanged
					 : opt == OPT_DEXCLMODIFIED ? opt::listDExclModified
					 : opt::listDExclAccessed
					 ).emplace_back(std::make_pair(opGt, date));

						opt::valueOf[opt] = "1";
					}
					i++;
					return true;
				}
				return false;
			}};
			if (i + 1 < in::args.size()
				and (	in::args[i + 1][0] == '<'
					or 	in::args[i + 1][0] == '>'
					or 	in::args[i + 1][0] == '='))
			{
				if (in::args[i + 1] == "<" or in::args[i + 1] == ">"
					or in::args[i + 1] == "=")
				{
					if (i + 2 < in::args.size())
						i++;
					else
						goto DATE_NEEDED;
				}

				if (as_single(in::args[i + 1][0]))
					continue;
			}
			else
			{
				func push{[&](const Date& lower, const Date& upper) {
					if (lower > upper) {
						std::cout << "⚠️  Date range up side down!, "
									<< lower.string()
									<< " greater than "
									<< upper.string()
									<< '\n';
						return false;
					}
					if (opt == OPT_DATE) {
						opt::listDCreatedR.emplace_back(std::make_pair(lower, upper));
						opt::listDChangedR.emplace_back(std::make_pair(lower, upper));
						opt::listDModifiedR.emplace_back(std::make_pair(lower, upper));
						opt::listDAccessedR.emplace_back(std::make_pair(lower, upper));
						opt::valueOf[OPT_DCREATED]  = "1";
						opt::valueOf[OPT_DMODIFIED] = "1";
						opt::valueOf[OPT_DACCESSED] = "1";
						opt::valueOf[OPT_DCHANGED]  = "1";
					} else if (opt == OPT_EXCLDATE) {
						opt::listDExclCreatedR.emplace_back(std::make_pair(lower, upper));
						opt::listDExclChangedR.emplace_back(std::make_pair(lower, upper));
						opt::listDExclModifiedR.emplace_back(std::make_pair(lower, upper));
						opt::listDExclAccessedR.emplace_back(std::make_pair(lower, upper));
						opt::valueOf[OPT_DEXCLCREATED]  = "1";
						opt::valueOf[OPT_DEXCLMODIFIED] = "1";
						opt::valueOf[OPT_DEXCLACCESSED] = "1";
						opt::valueOf[OPT_DEXCLCHANGED]  = "1";
					} else {
					(opt == OPT_DCREATED ? opt::listDCreatedR
					 : opt == OPT_DCHANGED ? opt::listDChangedR
					 : opt == OPT_DMODIFIED ? opt::listDModifiedR
					 : opt == OPT_DACCESSED ? opt::listDAccessedR
					 : opt == OPT_DEXCLCREATED ? opt::listDExclCreatedR
					 : opt == OPT_DEXCLCHANGED ? opt::listDExclChangedR
					 : opt == OPT_DEXCLMODIFIED ? opt::listDExclModifiedR
					 : opt::listDExclAccessedR
					 ).emplace_back(std::make_pair(lower, upper));
						opt::valueOf[opt] = "1";
					}
					return true;
				}};
				if (i + 3 < in::args.size() and
					(in::args[i + 2] == "-" or in::args[i + 2] == "..")) {
					if (const var lower { Date(in::args[i + 1]) }; lower.isValid())
						if (const var upper { Date(in::args[i + 3]) }; upper.isValid())
							if (push(lower, upper)) {
								i += 3;
								continue;
							}
				} else if (i + 1 < in::args.size()) {
					var pos { in::args[i + 1].find("..") };
					var next { 2 };
					if (pos == std::string::npos) {
						next = 1;
						var strip { (unsigned) 0 };
						pos = 0;
						for (var m { 0 }; m < in::args[i + 1].size(); ++m)
							if (in::args[i + 1][m] == '-') {
								strip++;
								pos = m;
							}
						if (strip not_eq 1)
							pos = std::string::npos;
					}
						
					if (pos not_eq std::string::npos) {
						if (const var lower { Date(in::args[i + 1].substr(0, pos)) };
							lower.isValid())
							if (const var upper { Date(in::args[i + 1].substr(pos + next)) };
								upper.isValid())
							{
								if (push(lower, upper)) {
									i++;
									continue;
								}
							}
					}
					
					if (as_single('=')) {
						continue;
					}
					
				}
			}
DATE_NEEDED:	std::cout << "⚠️  Expecting date and/or time after \""
						<< in::args[i]
						<< "\" option. Please see --help "
						<< in::args[i].substr(2)
						<< '\n';
		}
		else if (isMatch(OPT_OVERWRITE, 	'O', true)) {
			if (i + 1 < in::args.size() and isEqual(in::args[i + 1].c_str(),
												TRUE_FALSE, IgnoreCase::Left))
				opt::valueOf[OPT_OVERWRITE] = in::args[++i];
		}
		else if (isMatch(OPT_BENCHMARK, 	'b', true)) {
			if (i + 1 < in::args.size() and isEqual(in::args[i + 1].c_str(),
												TRUE_FALSE, IgnoreCase::Left))
				opt::valueOf[OPT_OVERWRITE] = in::args[++i];
		}
		else if (isMatch(OPT_SKIPSUBTITLE, 	'x', true)) {
			if (i + 1 < in::args.size() and isEqual(in::args[i + 1].c_str(),
												TRUE_FALSE, IgnoreCase::Left))
				opt::valueOf[OPT_OVERWRITE] = in::args[++i];
		}
		else if (isMatch(OPT_EXCLHIDDEN, 	'n', true)) {
			if (i + 1 < in::args.size() and isEqual(in::args[i + 1].c_str(),
												TRUE_FALSE, IgnoreCase::Left))
				opt::valueOf[OPT_OVERWRITE] = in::args[++i];
		}
		else if (isMatch(OPT_VERBOSE, 		'V', true)) {
			if (i + 1 < in::args.size()
				and (in::args[i + 1] == "all" or in::args[i + 1] == "info"))
			{
				i++;
				opt::valueOf[OPT_VERBOSE] = in::args[i];
			}
		}
		else if (isMatch(OPT_EXECUTION, 	'c')) {
			if (i + 1 < in::args.size()) {
				i++;
				if (in::args[i] == MODE_EXECUTION_ASYNC or in::args[i] == MODE_EXECUTION_THREAD)
					opt::valueOf[OPT_EXECUTION] = in::args[i];
				else
					opt::valueOf[OPT_EXECUTION] = "Linear";
			} else
				std::cout << "⚠️  Expecting 'thread', 'async', or 'none' after \""
							<< in::args[i]
							<< "\" option. Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (	isMatch(OPT_EXT,		'e')
				 or isMatch(OPT_EXCLEXT, 	'E')) {
			if (i + 1 < in::args.size()) {
				const var opt { in::args[i].substr(2) == OPT_EXT ? OPT_EXT : OPT_EXCLEXT };
				i++;
				if (in::args[i].size() == 2 and in::args[i][1] == '=' and i + 1 < in::args.size())
				{
					enum class OP { Add, Sub };
					
					const var op { in::args[i][0] == '+' ? OP::Add : OP::Sub };
					
					i++;
					
					var newList { ListString() };
					parseExtCommaDelimited(in::args[i], &newList);
					
					if (not opt::valueOf[opt].empty()) {
						if (op == OP::Sub) {
							var list { ListString() };
							parseExtCommaDelimited(opt::valueOf[opt], &list);
							opt::valueOf[opt].clear();
							
							for (var& ext : newList) {
								const var iterator { std::find(list.begin(), list.end(), ext) };
								if (iterator not_eq list.end())
									list.erase(iterator);
							}
							
							for (var& ext : list) {
								if (not opt::valueOf[opt].empty())
									opt::valueOf[opt] += ",";
								opt::valueOf[opt] += std::move(ext);
							}
						} else
							for (var& ext : newList)
								opt::valueOf[opt] += std::move(ext);
						
					} else {
						var extensions { opt == OPT_EXT ? &opt::DEFAULT_EXT
														: &opt::EXCLUDE_EXT };
						for (var& ext : newList)
							if (op == OP::Sub)
								extensions->erase(std::move(ext));
							else
								extensions->insert(std::move(ext));
					}
				}
				else
					opt::valueOf[opt] = in::args[i] == "*.*" ? "*" : in::args[i];
				
			} else
				std::cout << "Expecting extension after \""
							<< in::args[i]
							<< "\" option (eg: \"mp4, mkv\"). Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (isMatch(OPT_OUTFILENAME, 	'f', false, {"fix-filename"})) {
			if (i + 1 < in::args.size()) {
				i++;
				opt::valueOf[OPT_OUTFILENAME] = in::args[i];
				if (var parent { fs::path(in::args[i]).parent_path().string() };
					not parent.empty())
				{
					opt::valueOf[OPT_OUTFILENAME] = fs::path(in::args[i]).filename().string();
					parent += fs::path::preferred_separator;
					in::args.insert(in::args.begin() + 1 + i, std::move(parent));
					
					parent = "--";
					parent += OPT_OUTDIR;
					in::args.insert(in::args.begin() + 1 + i, std::move(parent));
				}
			} else
				std::cout << "⚠️  Expecting file name after \""
							<< in::args[i]
							<< "\" option (eg: \"my_playlist.m3u8\"). Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (isMatch(OPT_OUTDIR, 		'd')) {
			if (i + 1 < in::args.size()) {
				i++;
				const var dir { fs::path(in::args[i]) };
				if (not fs::exists(dir))
					fs::create_directories(dir);
				opt::valueOf[OPT_OUTDIR] = fs::absolute(dir);
				if (const var tmp { in::args[i] };
					tmp[tmp.size() - 1] not_eq fs::path::preferred_separator)
					opt::valueOf[OPT_OUTDIR] += fs::path::preferred_separator;
			} else
				std::cout << "⚠️  Expecting directory after \""
							<< in::args[i]
							<< "\" option (eg: \"Downloads/\"). Please see --help "
							<< in::args[i].substr(2)
							<< '\n';
		}
		else if (	isMatch(OPT_SIZE, 		's')
				 or isMatch(OPT_EXCLSIZE, 	'S')) {
			if (bool isExclude{ in::args[i].substr(2) == OPT_EXCLSIZE };
				i + 1 < in::args.size()) {
				if (in::args[i + 1][0] == '<' or in::args[i + 1][0] == '>')
				{
					i++;
					const var opGT { in::args[i][0] };
					
					var value{ (uintmax_t) 0 };
					
					if (in::args[i].size() > 1) {
						value = getBytes(in::args[i].substr(1));
						if (value <= 0)
							goto SIZE_NEEDED;
					}
					
					if (value <= 0 and i + 1 < in::args.size()) {
						value = getBytes(in::args[i + 1]);
						
						if (value <= 0)
							goto SIZE_NEEDED;
						else
							i++;
					}
					
					opt::valueOf[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] = opGT;
					opt::valueOf[isExclude ? OPT_EXCLSIZE : OPT_SIZE] = std::to_string(std::move(value));
					(isExclude ? opt::listExclSize : opt::listSize).clear();
				} else {
					var range { getRange(in::args[i + 1], std::move("-")) };
					if (not range)
						range = getRange(in::args[i + 1], std::move(".."));
					
					const var s_first { std::string(isExclude ? OPT_EXCLSIZE : OPT_SIZE) };
					var first { std::string("0") };
					var second { std::string("0") };
					
					if (range)
					{
						first = std::to_string(range->first);
						second = std::to_string(range->second);
						
						i++;
						
						if (second == "0") {
							var value { (uintmax_t) 0 };
							if (i + 1 < in::args.size())
								value = getBytes(in::args[i + 1]);
							
							if (value > 0) {
								second = std::to_string(value);
								i++;
							} else
								goto SIZE_NEEDED;
						}
					} else {
						if (i + 3 < in::args.size()) {
							first = std::to_string(getBytes(in::args[i + 1]));
							second = std::to_string(getBytes(in::args[i + 3]));
						} else if (i + 2 < in::args.size()) {
							first = std::to_string(getBytes(in::args[i + 1]));
							second = std::to_string(getBytes(in::args[i + 2]));
						}
						
						if (first not_eq "0" and second not_eq "0")
							i+=3;
						else if (first not_eq "0" and second == "0") {
							if (in::args[i + 2].size() > 1
								and in::args[i + 2][0] == '-'
								and std::isdigit(in::args[i + 2][1]))
							{
								second = std::to_string(getBytes(in::args[i + 2].substr(1)));
							} else if (in::args[i + 2].size() > 2
									   and in::args[i + 2][0] == '.'
									   and in::args[i + 2][1] == '.'
									   and std::isdigit(in::args[i + 2][2]))
							{
								second = std::to_string(getBytes(in::args[i + 2].substr(2)));
							} else
								goto SIZE_NEEDED;
							i+=2;
						} else
							goto SIZE_NEEDED;
					}
					
					opt::valueOf[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] = '\0';
					opt::valueOf[s_first] = first;

					if (std::stoul(second) < std::stoul(first)) {
						std::cout << "⚠️  Range is up side down! \""
									<< groupNumber(first)
									<< " bytes greater than "
									<< groupNumber(second)
									<< " bytes\"\n";
						opt::valueOf[s_first] = "0";
					} else if (first not_eq "0" and second not_eq "0")
						(isExclude ? opt::listExclSize : opt::listSize).emplace_back(std::make_pair(
									   std::stoul(first),
									   std::stoul(second)));
				}
			}
			else
SIZE_NEEDED:	std::cout << "⚠️  Expecting operator '<' or '>' followed"\
							"by size in KB, MB, or GB.\nOr use value in"\
							"range using form 'from-to' OR 'from..to'"\
							"Please see --help "
							<< OPT_SIZE
							<< '\n';
		}
		else if (isNetworkTransportFile(in::args[i]))
			insertInto(&in::selectFiles, fs::path(in::args[i]));
		else if (fs::is_directory(in::args[i]))
			insertInto(&in::inputDirs, fs::path(in::args[i]));
		else if (const var path { fs::path(in::args[i]) };
				 fs::is_regular_file(path)
				 and isValid(path) and isValidFile(path))
		{
			var list { ListPath() };
			loadPlaylistInto(path, &list);
			if (list.empty())
				insertInto(&in::selectFiles, fs::absolute(path));
			else
				for (var& f : list)
					if (isValid(f) and isValidFile(f))
						insertInto(&in::selectFiles, f);
		}
		else
			in::invalidArgs.emplace(in::args[i]);
	}
	// MARK: End option matching
	
	if (not in::invalidArgs.empty()) {
		std::string_view invalid_args[in::invalidArgs.size()];
		std::move(in::invalidArgs.begin(), in::invalidArgs.end(), invalid_args);
		std::cout << "⚠️  What "
					<< (in::invalidArgs.size() > 1 ? "are these" : "is this")
					<< ":\n";
		for (var i { 0 }; i<in::invalidArgs.size(); ++i) {
			const var item { &invalid_args[i] };
			var others { std::string() };
			if (item->size() > 4)
			{
				const var isStartWithDoubleStrip { item->starts_with("--") };
				const var pos { item->find("=") };
				var substr_item { std::string() };
				var substr_next { std::string() };
				if (pos == std::string::npos)
					substr_item = std::move(item->substr(
									isStartWithDoubleStrip ? 2 : 0));
				else {
					substr_item = std::move(item->substr(
									isStartWithDoubleStrip ? 2 : 0, pos - 1));
					substr_next = std::move(item->substr(pos));
				}
				var possible { ListPairFloatString() };
				for (var& opt : OPTS)
					if (const var percentage { getLikely(substr_item, *opt) };
						percentage > 80)
						possible.emplace_back(std::make_pair(percentage, *opt));
				
				if (not possible.empty()) {
					std::sort(possible.begin(), possible.end(), [](
						const std::pair<float, std::string>& a,
						const std::pair<float, std::string>& b)
					{
						return a.first > b.first;
					});
					others = " do you mean ";
					for (var k { 0 }; var&& s : possible)
						others.append("\"--" + s.second + substr_next + "\""
									+ (++k == possible.size() ? "." : " or "));
				}
			}
				
			std::cout << '"'
						<< *item
						<< '"'
						<< (others.empty() ? " ❔" : others)
						<< '\n';
		}
		std::cout << "\nFor more information, please try to type \""
					<< fs::path(argv[0]).filename().string()
					<< " --help ['keyword']\"\n\n";
	}
	
	if (in::inputDirs.empty() and in::selectFiles.empty())
		insertInto(&in::inputDirs, fs::current_path());
	
	/// Saving state for original inputDirs
	var bufferInputDirs { ListPath() };
	std::copy(in::inputDirs.begin(), in::inputDirs.end(),
			  std::back_inserter(bufferInputDirs));

	/// Expand InputDir if only one dir.
	while (bufferInputDirs.size() == 1) {
		var dir { fs::absolute(bufferInputDirs[0]).string() };
		insertInto(&in::regularDirs, bufferInputDirs[0]);/// Assume single input dir is regularDir
		if (isContainsSeasonDirs(dir)) {
			break;
		}
		bufferInputDirs.clear();
		var sortedDirs { ListEntry() };
		listDirInto(fs::path(dir), &sortedDirs);
				
		for (var& child : sortedDirs)
			insertInto(&bufferInputDirs, child.path());
	}

	if (const var dirOut { in::inputDirs.size() == 1
			? transformWhiteSpace(fs::path(opt::valueOf[OPT_OUTDIR]).filename().string())
			: std::to_string(in::inputDirs.size()) };
		opt::valueOf[OPT_OUTFILENAME].empty())
	{
		#ifdef LIBCPP_FORMAT
		std::format_to(std::back_inserter(opt::valueOf[OPT_FIXFILENAME]),
					   "playlist_from_{0}{1}.m3u8",
					   inputDirsCount == 0
						 ? groupNumber(std::to_string(in::selectFiles.size())) + "_file"
						 : dirOut + "_dir",
					   inputDirsCount > 1 or in::selectFiles.size() > 1 ? "s" : "";
		#else
		opt::valueOf[OPT_OUTFILENAME] =
				opt::valueOf[OPT_NOOUTPUTFILE] == "true" ? "" :
				"playlist_from_"
				+ (in::inputDirs.size() == 0
				   ? groupNumber(std::to_string(in::selectFiles.size())) + "_file"
				   : dirOut + "_dir")
		
				+ (in::inputDirs.size() > 1 or in::selectFiles.size() > 1 ? "s" : "")
				+ ".m3u8";
		#endif
	}
	
	
	if (	not	printOptionSummary(argc, argv)
		or 	not in::invalidArgs.empty())
		return RETURN_VALUE
					   
					   
	if (const var mode { opt::valueOf[OPT_WRITEDEFAULTS] };
		not mode.empty())
		writeConfig(&in::args,
					  mode == "edit" 	? WriteConfigMode::Edit
					: mode == "add" 	? WriteConfigMode::Add
					: mode == "remove" 	? WriteConfigMode::Remove
					: 					  WriteConfigMode::New);

	if (not opt::valueOf[OPT_SHOWCONFIG].empty()) {
		func printFile{[](ReadOnlyCString path) {
			if (not fs::exists(fs::path(path)))
				return;
			
			std::cout << "\nContents of file \""
						<< path
						<< "\":\n";
			var file { std::ifstream(path) };
			if (not file.good())
				return;
			std::cout << file.rdbuf()
						<< '\n';
			file.close();
		}};
			
		printFile(CONFIG_PATH);
		var otherFile { opt::valueOf[OPT_LOADCONFIG].c_str() };
		if (not isEqual(CONFIG_PATH, otherFile))
			printFile(otherFile);
	}
					   
    if (		opt::valueOf[OPT_LIST] == "true"
		or	not opt::valueOf[OPT_SHOWCONFIG].empty()
		or 	not opt::valueOf[OPT_WRITEDEFAULTS].empty())
        return RETURN_VALUE

	{///Clean up <key=val> dir=??? in listFind and listExclFind
		func cleanUp{[](ListString* list) -> void {
			var tmp { ListString() };
			for (var i { 0 }; i<list->size(); ++i)
				if (list->at(i)[0] not_eq char(1))
					tmp.emplace_back(std::move(list->at(i)));
			*list = std::move(tmp);
		}};
		cleanUp(&opt::listFind);
		cleanUp(&opt::listExclFind);
	}
					   
	if (opt::valueOf[OPT_OUTDIR].empty()) {
		if (in::selectFiles.empty())
			opt::valueOf[OPT_OUTDIR] = fs::current_path().string();
		else
			opt::valueOf[OPT_OUTDIR] = fs::path(in::selectFiles[0]).parent_path().string();
		opt::valueOf[OPT_OUTDIR] += fs::path::preferred_separator;
	}
	
	var start { std::chrono::system_clock::now() };

	var threads { ListThread() };
	var asyncs { ListAsync() };
					   
	{
		const var list { std::move(in::listAdsDir) };
		
		for (var& child : list)
			if (const var mode { opt::valueOf[OPT_EXECUTION] };
				mode == MODE_EXECUTION_THREAD)
				threads.emplace_back([&, child]() {
					listDirRecursivelyInto(child, &in::listAdsDir, false); });
			else if (mode == MODE_EXECUTION_ASYNC)
				asyncs.emplace_back(std::async(std::launch::async, [&, child]() {
					listDirRecursivelyInto(child, &in::listAdsDir, false); }));
			else
				listDirRecursivelyInto(child, &in::listAdsDir, false);
	}
	
	for (const var isByPass { opt::valueOf[OPT_ARRANGEMENT] == MODE_ARRANGEMENT_ASCENDING };
		var& child : bufferInputDirs)
		if (const var mode { opt::valueOf[OPT_EXECUTION] };
			mode == MODE_EXECUTION_THREAD) {
			if (isByPass)
				threads.emplace_back([&, child]() {
					listDirRecursivelyInto(child, &in::regularDirs, false);
				});
			else
				threads.emplace_back(checkForSeasonDir, child);
		}
		else if (mode == MODE_EXECUTION_ASYNC)
			if (isByPass)
				asyncs.emplace_back(std::async(std::launch::async, [&, child]() {
					listDirRecursivelyInto(child, &in::regularDirs, false);
				}));
			else
				asyncs.emplace_back(std::async(std::launch::async,
							checkForSeasonDir, child));
		else
		{
			if (isByPass)
				listDirRecursivelyInto(child, &in::regularDirs, false);
			else
				checkForSeasonDir(child);
		}

	if (const var mode { opt::valueOf[OPT_EXECUTION] };
		mode == MODE_EXECUTION_THREAD) {
		for (var& t : threads)
			t.join();
		threads.clear();
	} else if (mode == MODE_EXECUTION_ASYNC) {
		for (var& a : asyncs)
			a.wait();
		asyncs.clear();
	}

	
	const var maxDirSize { std::max(in::regularDirs.size(), in::seasonDirs.size()) };

	if (const var mode { opt::valueOf[OPT_ARRANGEMENT] };
			mode == MODE_ARRANGEMENT_DEFAULT
		or 	mode == MODE_ARRANGEMENT_SHUFFLE_DEFAULT
		or 	mode == MODE_ARRANGEMENT_DESCENDING_DEFAULT)
	{
		std::sort(in::regularDirs.begin(), in::regularDirs.end());
		std::sort(in::seasonDirs.begin(), in::seasonDirs.end());
			
		sortFiles(&in::selectFiles);
	}

	#ifndef DEBUG
	if (const var isDebugTrue { opt::valueOf[OPT_DEBUG] == "true" };
		opt::valueOf[OPT_BENCHMARK] == "true" or isDebugTrue)
	#endif
	{
		if (in::inputDirs.size() > 0)
			timeLapse(start, groupNumber(
						std::to_string(in::regularDirs.size() + in::seasonDirs.size()))
					  + " valid input dirs"
					  + (in::selectFiles.size() > 0
						 ? " and " + groupNumber(std::to_string(in::selectFiles.size()))
						 + " input files " : " " ) + "took ");
		#ifndef DEBUG
		if (isDebugTrue)
		#endif
		for (var i { 0 }; i<maxDirSize; ++i)
			for (var& select : {1, 2}) {
				if ((select == 1 and i >= in::regularDirs.size())
					or (select == 2 and i >= in::seasonDirs.size()))
					continue;
				std::cout
					<< (i == 0 and select == 1 ? "BEGIN valid dirs-----\n" : "")
					<< (select == 1 ? 'R' : 'S') << ':'
					<< (select == 1 ? in::regularDirs[i] : in::seasonDirs[i])
					<< (i + 1 == maxDirSize ? "\nEND valid dirs-----\n\n" : "\n");
			}
	}
	
	
	var records { MapStringPListPath() };
	
	func filterChildFiles{ [&records](const fs::path& dir, bool recurive=false) {
		var bufferFiles { ListPath() };
		
		func putToRecord{[&bufferFiles, &dir, &records](bool wantToSort) {
			if (bufferFiles.empty())
				return;
			
			if (wantToSort and bufferFiles.size() > 1)
				std::sort(bufferFiles.begin(), bufferFiles.end(), ascending);
			
			records.emplace(std::make_pair(dir.string(),
										   std::make_shared<ListPath>(
												std::move(bufferFiles))
										   ));
		}};
		
		if (recurive) {
			var dirs { ListPath() };
			listDirRecursivelyInto(dir, &dirs, false);
			std::sort(dirs.begin(), dirs.end());
			
			for (var& d : dirs) {
				var tmp { ListPath() };
				
				for (var& f : directory_iterator(d, DT_REG))
					if (isValidFile(f.path())) {
						var list { ListPath() };
						loadPlaylistInto(f.path().string(), &list);
						if (list.empty())
							tmp.emplace_back(std::move(f));
						else
							for (var& p : list)
								if (fs::is_regular_file(p) and isValid(p) and isValidFile(p))
									tmp.emplace_back(std::move(p));
					}
				
				sortFiles(&tmp);
				
				std::move(tmp.begin(), tmp.end(), std::back_inserter(bufferFiles));
			}
			
			putToRecord(false);
		}
		
		for (var& f : directory_iterator(dir, DT_REG))
			if (isValidFile(f.path())) {
				var list { ListPath() };
				loadPlaylistInto(f.path().string(), &list);
				if (list.empty())
					bufferFiles.emplace_back(std::move(f));
				else
					for (var& p : list)
						if (fs::is_regular_file(p) and isValid(p) and isValidFile(p))
							bufferFiles.emplace_back(std::move(p));
			}
		
		putToRecord(true);
	}};
					   
					   
	start = std::chrono::system_clock::now();
					   
	for (var i { 0 }; i<std::max(maxDirSize, in::listAdsDir.size()); ++i)
		for (var& x : {0, 1, 2})
			if (i < (x == 1 ? in::regularDirs.size() : (x == 2 ? in::seasonDirs.size()
														: in::listAdsDir.size())) )
				if (const var dir { x == 1 ? in::regularDirs[i] : (x == 2 ? in::seasonDirs[i]
															  : in::listAdsDir[i]) };
					not dir.empty() and isDirNameValid(dir)) {
					if (const var mode { opt::valueOf[OPT_EXECUTION] };
						mode == MODE_EXECUTION_THREAD)
						threads.emplace_back([&, dir]() {
							filterChildFiles(dir, x == 2);
						});
					else if (mode == MODE_EXECUTION_ASYNC)
						asyncs.emplace_back(std::async(std::launch::async, [&, dir]() {
							filterChildFiles(dir, x == 2);
						}));
					else
						filterChildFiles(dir, x == 2);
				}
					   
	if (const var mode { opt::valueOf[OPT_EXECUTION] };
		mode == MODE_EXECUTION_THREAD)
		for (var& t : threads)
			t.join();
	else if (mode == MODE_EXECUTION_ASYNC)
		for (var& a : asyncs)
			a.wait();

	var output { Output(	opt::valueOf[OPT_VERBOSE]	== "all"
						or 	opt::valueOf[OPT_VERBOSE]	== "true"
						or 	opt::valueOf[OPT_DEBUG] 	== "true") };

	
	const var isDontWrite { opt::valueOf[OPT_NOOUTPUTFILE] == "true" };
	if (not isDontWrite) {
		output.name = opt::valueOf[OPT_OUTDIR] + opt::valueOf[OPT_OUTFILENAME];
			
		if (fs::exists(output.name) and opt::valueOf[OPT_OVERWRITE] == "true")
		   fs::remove(output.name);
		else
		   output.name = getAvailableFilename(output.name);
		   
		output.extension = tolower(output.name.extension().string());
		output.file = std::ofstream(output.name, std::ios::out);
//		if (not output.file.good()) {
//			std::cout << "⚠️  Cannot write \""
//						<< outputName
//						<< "\"\n";
//			return RETURN_VALUE
//		}
			
		output.generate(Output::Section::Header);
	}

	
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 mersenneTwisterEngine(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distrib;
	std::uniform_int_distribution<> distribCount;
	unsigned long adsCount[2] {0, 0};


	func putIntoPlaylist{ [&](const fs::path& file)
	{
		func putIt{ [&](const fs::path& file,
						ReadOnlyCString title = nullptr,
						Output::Type type = Output::Type::None)
		{
			if (not isDontWrite)
				output.generate(Output::Section::Content, &file, title, type);
			
			else {
				output.playlistCount++;
				var content { Output::Content() };
				output.deduceFilename(&file, &content);
				
				#if MAKE_LIB
				if (outc)
					*outc += 1;
				if (maxLength) {
					if (const var sz { content.fullPath.size() + 1 };
						*maxLength < sz)
						*maxLength = sz;
				}
				if (outs)
					std::strcpy(outs[output.playlistCount - 1],
								content.fullPath.c_str());
				#endif
			}
			
			#ifndef DEBUG
			if (output.isVerbose)
			#endif
				std::cout << file.string()
							<< '\n';
		}};
		
		#if defined(_WIN32) || defined(_WIN64)
		#define OS_NAME	"Windows"
		#elif defined(__APPLE__)
		#define OS_NAME	"macOS"
		#else
		#define OS_NAME	"Linux"
		#endif

		putIt(file,
			  OS_NAME " Path",
			  Output::Type::Regular);
		   

		if (opt::valueOf[OPT_SKIPSUBTITLE] not_eq "true") {
			var subtitleFiles { ListPath() };
			findSubtitleFileInto(file, &subtitleFiles);
			for (var& sf : subtitleFiles)
				putIt(std::move(sf),
					  "Subtitle Path on " OS_NAME,
					  Output::Type::Subtitle);
		}
		   
		if (not in::listAdsDir.empty())
		   for (var i { 0 }; i<(adsCount[1] == 0
							  ? adsCount[0]
							  : distribCount(mersenneTwisterEngine));
			   ++i, ++output.playlistCount)
			   putIt(fs::absolute(
						   in::listAdsDir[distrib(mersenneTwisterEngine)]).string(),
					 "Ads path on " OS_NAME,
					 Output::Type::Advertise);
		#undef OS_NAME
	}};
					   
	///Check if records has listAdsDir and equal
	if (var equal { true };
		in::listAdsDir.size() == records.size())
	{
		for (var& dir : in::listAdsDir)
			if (records.find(dir) == records.end()) {
				equal = false;
				break;
			}
		if (equal) /// Then consider there was no listAdsDir
			in::listAdsDir.clear();
	}
					   
	if (not in::listAdsDir.empty()) {
		const var list { std::move(in::listAdsDir) };
		
		for (var& dir : list)
			if (not dir.empty())
				if (const var found { records[dir] }; found)
				{
					std::move(found->begin(), found->end(),
							  std::back_inserter(in::listAdsDir));
					records.erase(records.find(dir));
				}
		distrib = std::uniform_int_distribution<>(0, int(in::listAdsDir.size() - 1));
			
		if (const var count_s { opt::valueOf[OPT_ADSCOUNT] };
			count_s.empty()) {
			adsCount[0] = in::regularDirs.size() + in::seasonDirs.size();
			adsCount[0] = in::listAdsDir.size() % adsCount[0] == 0
						? std::min(decltype(in::listAdsDir.size())(3), in::listAdsDir.size())
						: in::listAdsDir.size() % adsCount[0];
		} else {
			if (const var pos { count_s.find('-') };
				pos == std::string::npos)
				adsCount[0] = std::stoul(count_s);
			else {
				adsCount[0] = std::stoul(count_s.substr(0, pos));
				adsCount[1] = std::stoul(count_s.substr(pos + 1));
				distribCount = std::uniform_int_distribution<>(int(adsCount[0]),
															   int(adsCount[1]));
			}
		}
	}
					   
	if (const var mode { opt::valueOf[OPT_ARRANGEMENT] };
			mode == MODE_ARRANGEMENT_DEFAULT
		 or mode == MODE_ARRANGEMENT_UNORDERED
		 or mode == MODE_ARRANGEMENT_SHUFFLE_DEFAULT
		 or mode == MODE_ARRANGEMENT_DESCENDING_DEFAULT)
	{
		var indexFile { (unsigned long) 0 };

		while (true) {
			var finish { true };
			for (var i { 0 }; i < maxDirSize; ++i)
				for (var& indexPass : {1, 2})
				{
					if ((indexPass == 1 and i >= in::regularDirs.size())
						or (indexPass == 2 and i >= in::seasonDirs.size()))
						continue;
					
					if (const var dir { indexPass == 1 ? in::regularDirs[i]
						: in::seasonDirs[i] };
						dir.empty())
						continue;

					else if (const var found { records[dir] }; found) {
						if (mode == MODE_ARRANGEMENT_SHUFFLE_PERTITLE)
						{
							std::shuffle(found->begin(), found->end(),
										 mersenneTwisterEngine);
							for (var& f : *found)
								putIntoPlaylist(std::move(f));
							continue;
						}
						
						if (indexFile == 0) {
							if (mode == MODE_ARRANGEMENT_SHUFFLE_DEFAULT)
								std::shuffle(found->begin(), found->end(), mersenneTwisterEngine);
							
							else if (mode == MODE_ARRANGEMENT_DESCENDING_DEFAULT)
								std::sort(found->begin(), found->end(), descending);
						}
							
						for (var c { 0 }; c < in::fileCountPerTurn; ++c)
							if (indexFile + c < found->size()) {
								finish = false;

								putIntoPlaylist(std::move((*found)[indexFile + c]));
							}
					}
				} ///end pass loop
			
			if (mode == MODE_ARRANGEMENT_SHUFFLE_PERTITLE)
				break;
			
			if (indexFile < in::selectFiles.size())
				putIntoPlaylist(std::move(in::selectFiles[indexFile]));
			
			indexFile += in::fileCountPerTurn;
			
			if (finish and indexFile >= in::selectFiles.size())
				break;
		}
	}
	else {
		std::move(in::seasonDirs.begin(), in::seasonDirs.end(),
					   std::back_inserter(in::regularDirs));
		in::seasonDirs.clear();

		std::sort(in::regularDirs.begin(), in::regularDirs.end());

		for (var& dir : in::regularDirs) {
			if (dir.empty())
				continue;
			if (const var found { records[dir] }; found) {
				if (mode == MODE_ARRANGEMENT_PERTITLE)
					std::sort(found->begin(), found->end(), ascending);
				else if (mode == MODE_ARRANGEMENT_DESCENDING_PERTITLE)
					std::sort(found->begin(), found->end(), descending);

				for (var& f : *found)
				{
					if (	mode == MODE_ARRANGEMENT_ASCENDING
						or 	mode == MODE_ARRANGEMENT_DESCENDING
						or 	mode == MODE_ARRANGEMENT_SHUFFLE)
						in::selectFiles.emplace_back(std::move(f));
					else
						putIntoPlaylist(std::move(f));
				}
			}
		}

		if (mode == MODE_ARRANGEMENT_PERTITLE)
			sortFiles(&in::selectFiles);

		else if (mode == MODE_ARRANGEMENT_SHUFFLE)
			std::shuffle(in::selectFiles.begin(), in::selectFiles.end(), mersenneTwisterEngine);

		else if (	mode == MODE_ARRANGEMENT_DESCENDING
				 or mode == MODE_ARRANGEMENT_DESCENDING_PERTITLE)
			std::sort(in::selectFiles.begin(), in::selectFiles.end(), descending);

		else
			std::sort(in::selectFiles.begin(), in::selectFiles.end(), ascending);

		for (var& f : in::selectFiles)
			putIntoPlaylist(std::move(f));
	 }
	
	#ifndef DEBUG
	if (opt::valueOf[OPT_BENCHMARK] == "true" or opt::valueOf[OPT_DEBUG] == "true")
	#endif
		timeLapse(start,
				  groupNumber(std::to_string(output.playlistCount))
						+ " valid files took ");
					   
	if (isDontWrite)
		return RETURN_VALUE
					   
	output.generate(Output::Section::Footer);
}
#undef CString
#undef ReadOnlyCString
#undef property
#undef let
#undef var
#undef func
#undef RETURN_VALUE
#undef CONFIG_PATH
#undef INSTALL_PATH
#undef SEP_PATH
#undef PE_EXT
#undef INSTALL_FULLPATH
