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

constexpr auto TRUE_FALSE = {"true", "false"};
std::unordered_map<std::string, std::string> state;

constexpr auto OPT_SHOWCONFIG				{"show-config"};
constexpr auto OPT_SHOWCONFIG_ALTERNATIVE = {"show-defaults",
	"display-config", "display-defaults", "print-config", "print-defaults",
};
constexpr auto OPT_WRITEDEFAULTS				{"write-defaults"};	// W
constexpr auto OPT_WRITEDEFAULTS_ALTERNATIVE = 	{"write-config"};
constexpr auto OPT_WRITEDEFAULTS_ARGS = 		{
	"new", "edit", "remove", "reset", "add"
};

constexpr auto OPT_LOADCONFIG		{"load-config"};			// L
constexpr auto OPT_ARRANGEMENT 		{"arrangement"};			// w
constexpr auto OPT_ARRANGEMENT_DEFAULT		{"default"};
constexpr auto OPT_ARRANGEMENT_UNORDERED	{"unordered"};
constexpr auto OPT_ARRANGEMENT_ASCENDING	{"ascending"};
constexpr auto OPT_ARRANGEMENT_PERTITLE		{"per-title"};
constexpr auto OPT_ARRANGEMENT_SHUFFLE		{"shuffle"};
constexpr auto OPT_ARRANGEMENT_SHUFFLE_DEFAULT	{"default-shuffle"};
constexpr auto OPT_ARRANGEMENT_SHUFFLE_PERTITLE	{"per-title-shuffle"};
constexpr auto OPT_ARRANGEMENT_DESCENDING			{"descending"};
constexpr auto OPT_ARRANGEMENT_DESCENDING_DEFAULT	{"default-descending"};
constexpr auto OPT_ARRANGEMENT_DESCENDING_PERTITLE	{"per-title-descending"};
constexpr auto OPT_HELP 			{"help"};					// h
constexpr auto OPT_VERSION 			{"version"};				// v
constexpr auto OPT_VERBOSE 			{"verbose"};				// V
constexpr auto OPT_BENCHMARK 		{"benchmark"};				// b
constexpr auto OPT_OVERWRITE 		{"overwrite"};				// O
constexpr auto OPT_SKIPSUBTITLE 	{"skip-subtitle"};			// x
constexpr auto OPT_OUTDIR 			{"out-dir"};				// d
constexpr auto OPT_ADSDIR 			{"ads-dir"};				// D
constexpr auto OPT_ADSCOUNT			{"ads-count"};				// C
constexpr auto OPT_EXECUTION		{"execution"};				// c
constexpr auto OPT_EXECUTION_THREAD			{"thread"};
constexpr auto OPT_EXECUTION_ASYNC			{"async"};
constexpr auto OPT_EXCLHIDDEN		{"exclude-hidden"};			// n
constexpr auto OPT_REGEXSYNTAX			{"regex-syntax"};		// X
constexpr auto OPT_REGEXSYNTAX_ARGS = 	{"ecma", "basic", "extended", "awk",
	"grep", "egrep"
};
constexpr auto OPT_CASEINSENSITIVE				 {"make-case-insensitive"};	// N
constexpr auto OPT_CASEINSENSITIVE_ALTERNATIVE = {"case-insensitive",
	"caseinsensitive", "ignore-case", "ignorecase"
};

constexpr auto OPT_SEARCH			{"search"};					// q

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

constexpr auto OPT_DATE				{"date"};					// z
constexpr auto OPT_EXCLDATE			{"exclude-date"};			// Z

constexpr auto OPT_DCREATED			{"created"};				// t
constexpr auto OPT_DMODIFIED		{"modified"};				// m
constexpr auto OPT_DACCESSED		{"accessed"};				// a
constexpr auto OPT_DCHANGED			{"changed"};				// g

constexpr auto OPT_DEXCLCREATED		{"exclude-created"};		// T
constexpr auto OPT_DEXCLMODIFIED	{"exclude-modified"};		// M
constexpr auto OPT_DEXCLACCESSED	{"exclude-accessed"};		// A
constexpr auto OPT_DEXCLCHANGED		{"exclude-changed"};		// G

constexpr auto OPT_DEBUG			{"debug"};                  // B
constexpr auto OPT_DEBUG_ARGS = 	{"date", "args", "id3"};


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

constexpr auto OPT_INSTALL			{"install"};
constexpr auto INSTALL=\
"--install\n\
        Install tvplaylist into \"" INSTALL_PATH "\".\n\
";
constexpr auto OPT_UNINSTALL		{"uninstall"};
constexpr auto UNINSTALL=\
"--uninstall\n\
        Uninstall tvplaylist from \"" INSTALL_PATH "\".\n\
";
constexpr auto OPT_UPDATE			{"update"};
constexpr auto UPDATE=\
"--update\n\
--upgrade\n\
        Update tvplaylist.\n\
";

constexpr auto SHOW=\
"--show-defaults\n\
--show-config\n\
--display-defaults\n\
--display-config\n\
--print-defaults\n\
--print-config\n\
        Display options summary and configuration file contents, then quit.\n\
        See also --verbose or --verbose=info or --debug=args or --benchmark.\n\
        Example: --print-defaults\n\
";
constexpr auto WRITE=\
"-W, --write-defaults [new | reset | edit | add | remove]\n\
     --write-config [new | reset | edit | add | remove]\n\
        Write anything you defined to \""\
        CONFIG_PATH\
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
constexpr auto LOAD=\
"-L, --load-config 'custom file'\n\
        Load configuration from custom file.\n\
        Example:\n\
            --load-config tuesday_night.txt\n\
        Default configuration file located in \""\
			CONFIG_PATH\
"\".\n\
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
constexpr auto VERSION=\
"tvplaylist version 1.1 (Early 2022)\nTM and (C) 2022 Widiatmoko. \
All Rights Reserved. License and Warranty\n";

constexpr auto HELP=\
"Create playlist file from vary directories and files, \
then by default, arranged one episode per Title.\nHosted in https://github.com/Mr-Widiatmoko/MakeTVPlaylist\n\n\
Usage:\n    tvplaylist [Option or Dir or File] ...\n\n\
If no argument was specified, the current directory will be use.\n\n\
Option:\n\
-h, --help ['keyword']\n\
        Display options description.\n\
-v, --version\n\
        Display version.\n\
";
constexpr auto ADSDIR=\
"-D, --ads-dir 'directory constains advertise'\n\
        Add advertise directory. Ads file will be inserted between each ordered files.\n\
        You can specifying this multiple times.\n\
        To set the numbers how many ads shown use --ads-count.\n\
";
constexpr auto ADSCOUNT=\
"-C, --ads-count 'fixed count'\n\
                 'min count' .. 'max count'\n\
                 'min count' - 'max count'\n\
        Set the number of how many ads will be shown per insertion. Set 0 to disable ads.\n\
        If you set using range, then ads will be shown randomly between 'min count' to 'max count'.\n\
        To set where the advertise directories location path, use --ads-dir.\n\
";
constexpr auto VERBOSE=\
"-V, --verbose [all | info]\n\
        Display playlist content.\n\
        Define as 'all' will also show fail messages.\n\
        Define as 'info' will also display options info summary.\n\
";
constexpr auto BENCHMARK=\
"-b, --benchmark\n\
        Benchmarking execution.\n\
";
constexpr auto ARRANGEMENT=\
"-w, --arrangement [default | unordered | per-title | ascending | shuffle] \n\
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
constexpr auto SEARCH=\
"--search 'keywords'\n\
        tvplaylist search engine.\n\
        --search is alias from --no-output-file:verbose:arrangement=ascending:ignore-case\n\
          Example:\n\
            --search \"ext=* medalion or title=medalion exclude=horse\"\n\
        You can specifying this multiple times.\n\
";
constexpr auto OVERWRITE=\
"-O, --overwrite\n\
        Overwrite output playlist file.\n\
";
constexpr auto SKIPSUBTITLE=\
"-x, --skip-subtitle\n\
        Dont include subtitle file.\n\
";
constexpr auto OUTDIR=\
"-d, --out-dir \"directory path\"\n\
        Override output directory for playlist file.\n\
";
constexpr auto EXECUTION=\
"-c, --execution [thread | async | linear]\n\
        Specify execution, \"async\" is selected by default.\n\
";
constexpr auto EXCLHIDDEN=\
"-n, --exclude-hidden\n\
        Exclude hidden folders or files (not for Windows).\n\
";
[[deprecated("Recognized in --find or --exlude-find.")]] [[maybe_unused]]
constexpr auto CASEINSENSITIVE=\
"-N, --make-case-insensitive\n\
     --case-insensitive\n\
     --caseinsensitive\n\
     --ignore-case\n\
     --ignorecase\n\
        Make --find and --exclude-find case-insensitive.\n\
        To make case-sensitive again, pass --ignorecase=false.\n\
";
constexpr auto FIND=\
"-i, --find 'keyword'\n\
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
constexpr auto REGEXSYNTAX=\
"-X, --regex-syntax [ecma | awk | grep | egrep | basic | extended]\n\
        Specify regular expression syntax type to use.\n\
        Available value are: 'ecma'(Default), 'awk', 'grep', 'egrep', 'basic', 'extended'.\n\
            'basic' use the basic POSIX regex grammar and\n\
            'extended' use the extended POSIX regex grammar.\n\
        You can also specifying this inside --regex or --exclude-regex, for example\n\
            --regex type=basic  or  --exclude-regex type=extended\n\
";
constexpr auto REGEX=\
"-r, --regex 'syntax'\n\
-R, --exclude-regex 'syntax'\n\
        Filter only files with (or exclude) filename match regular expression.\n\
        To set regex syntax type, pass type=[ecma | awk | grep | egrep | basic | extended]\n\
            Example: --regex type=grep  or  --regex=type=grep\n\
        You can specifying this multiple times.\n\
";
constexpr auto EXT=\
"-e, --ext \"'extension', 'extension', ...\"\n\
-E, --exclude-ext \"'extension', 'extension' ...\"\n\
        Filter only (or exclude) files that match specific extensions, separated by comma.\n\
            Example: --ext \"pdf, docx\" or --ext=pdf,docx\n\
        To process all files use character *.\n\
            Example: --ext=* \n\
        To enable get contents from other playlists, you must include the playlist extensions.\n\
            Example: --ext=mp4,mkv,mp3,m3u,m3u8,pls,wpl,xspf\n\
";
constexpr auto SIZE=\
"-s, --size < | > 'size'\n\
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
constexpr auto DATE=\
"-z, --date = | < | > 'date and/or time'\n\
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
constexpr auto CREATED=\
"-t, --created = | < | > 'date and/or time'\n\
               'min' .. 'max'\n\
               'min' - 'max'\n\
-T, --exclude-created = | < | > 'date and/or time'\n\
                      'min' .. 'max'\n\
                      'min' - 'max'\n\
        Filter only (or exclude) files that was created on specified date[s] and/or time[s].\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
";
constexpr auto ACCESSED=\
"-a, --accessed = | < | > 'date and/or time'\n\
                'min' .. 'max'\n\
                'min' - 'max'\n\
-A, --exclude-accessed = | < | > 'date and/or time'\n\
                        'min' .. 'max'\n\
                        'min' - 'max'\n\
        Filter only (or exclude) files that was accessed on specified date[s] and/or time[s].\n\
        You can specifying this multiple times, for both single value or range values.\n\
        For more information about 'date and/or time' possible values, see below.\n\
";
constexpr auto MODIFIED=\
"-m, --modified = | < | > 'date and/or time'\n\
                'min' .. 'max'\n\
                'min' - 'max'\n\
-M, --exclude-modified = | < | > 'date and/or time'\n\
                       'min' .. 'max'\n\
                       'min' - 'max'\n\
        Filter only (or exclude) files that was modified on specified date[s] and/or time[s].\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
";
constexpr auto CHANGED=\
"-g, --changed = | < | > 'date and/or time'\n\
               'min' .. 'max'\n\
               'min' - 'max'\n\
-G, --exclude-changed = | < | > 'date and/or time'\n\
                       'min' .. 'max'\n\
                       'min' - 'max'\n\
        Filter only (or exclude) files that was changed on specified date[s] and/or time[s].\n\
        For more information about 'date and/or time' possible values, see below.\n\
        You can specifying this multiple times, for both single value or range values.\n\
";
constexpr auto FIXFILENAME=\
"-f, --out-filename 'filename'\n\
        Override output playlist filename, by default it will crreate \".m3u8\" playlist.\n\
        To create \".pls\", \".wpl\", or others type of playlist, pass it as extension filename.\n\
            Example: --out-filename=my_playlist.xspf.\n\
        To convert or grab files inside another playlist file, \n\
        first add playlist ext[s] and media file ext[s] you desired into --ext.\n\
            Example: --ext=mp4,pls,wpl,xspf another.pls another.wpl another.xspf\n\
        Here the example to convert from different type of playlist to another type:\n\
            Example: --ext=mp3,m3u old_musics.m3u --out-filename=old_music.xspf\n\
";
constexpr auto NOOUTPUTFILE=\
"-F, --no-ouput-file [yes | no]\n\
        Choose to create playlist file or no. Default 'yes' if option was declared or if was build as library.\n\
";
constexpr auto HELP_REST=\
"\n\
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
constexpr auto HELP_DATE_REST=\
"Posible value for 'date and/or time' are: 'Year', 'Month Name' (mnemonic or full), 'Month Number', 'Day', 'WeekDay Name' (mnemonic or full), 'Hour' (if AM/PM defined then it is 12 hours, otherwise 24 hours), 'Minute', 'Second', AM or PM.\n\
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


constexpr const char* const* OPTS[] = { &OPT_VERSION, &OPT_HELP, &OPT_ARRANGEMENT,
	&OPT_SEARCH, &OPT_VERBOSE, &OPT_BENCHMARK, & OPT_OVERWRITE,
	&OPT_SKIPSUBTITLE, &OPT_OUTDIR, &OPT_ADSDIR, &OPT_ADSCOUNT,
	&OPT_EXECUTION, &OPT_LOADCONFIG, &OPT_WRITEDEFAULTS, &OPT_SHOWCONFIG, &OPT_FIXFILENAME,
	&OPT_NOOUTPUTFILE, &OPT_SIZE, &OPT_EXCLSIZE, &OPT_EXT, &OPT_EXCLEXT,
	&OPT_FIND, &OPT_EXCLFIND, &OPT_REGEX, &OPT_EXCLREGEX, &OPT_EXCLHIDDEN,
	&OPT_INSTALL, &OPT_UNINSTALL, &OPT_UPDATE,
	&OPT_DATE, &OPT_EXCLDATE,
	&OPT_DCREATED, &OPT_DMODIFIED, &OPT_DACCESSED, &OPT_DCHANGED,
	&OPT_DEXCLCREATED, &OPT_DEXCLMODIFIED, &OPT_DEXCLACCESSED, &OPT_DEXCLCHANGED,
	
	/// Ignored
	&OPT_DEBUG
};

/// Conjunction with OPTS, to enable access OPTS[index] == HELPS[index]
constexpr const char* const* HELPS[] = { &VERSION, &HELP, &ARRANGEMENT,
	&SEARCH, &VERBOSE, &BENCHMARK, & OVERWRITE,
	&SKIPSUBTITLE, &OUTDIR, &ADSDIR, &ADSCOUNT,
	&EXECUTION, &LOAD, &WRITE, &SHOW, &FIXFILENAME,
	&NOOUTPUTFILE, &SIZE, &SIZE, &EXT, &EXT,
	&FIND, &FIND, &REGEX, &REGEX, &EXCLHIDDEN,
	&INSTALL, &UNINSTALL, &UPDATE,
	&DATE, &DATE,
	&CREATED, &MODIFIED, &ACCESSED, &CHANGED,
	&CREATED, &MODIFIED, &ACCESSED, &CHANGED,
	
	/// Ignored
	&HELP_REST, &HELP_DATE_REST
};

static_assert((sizeof(OPTS)/sizeof(OPTS[0])) - 1 == (sizeof(HELPS)/sizeof(HELPS[0])) - 2,
			  "Size need to be equal!, to be able accessed by index");

constexpr const char* const* SINGLE_VALUE_OPT[] = {&OPT_LOADCONFIG, &OPT_SHOWCONFIG,
	&OPT_ARRANGEMENT,
	&OPT_VERBOSE, &OPT_BENCHMARK, &OPT_OVERWRITE, &OPT_SKIPSUBTITLE,
	&OPT_OUTDIR, &OPT_ADSCOUNT, &OPT_EXECUTION, &OPT_FIXFILENAME, &OPT_EXCLHIDDEN,
	&OPT_EXT, &OPT_EXCLEXT};

[[maybe_unused]]
constexpr const char* const* MULTI_VALUE_OPT[] = {
	&OPT_ADSDIR,
	&OPT_SIZE, &OPT_EXCLSIZE,
	&OPT_FIND, &OPT_EXCLFIND, &OPT_REGEX, &OPT_EXCLREGEX,
	&OPT_DATE, &OPT_EXCLDATE,
	&OPT_DCREATED, &OPT_DMODIFIED, &OPT_DACCESSED, &OPT_DCHANGED,
	&OPT_DEXCLCREATED, &OPT_DEXCLMODIFIED, &OPT_DEXCLACCESSED, &OPT_DEXCLCHANGED,};

constexpr const char* const* ALL_HELPS[] = {
	&VERSION, &HELP, &LOAD, &WRITE, &SHOW, &ARRANGEMENT, &SEARCH, &VERBOSE, &BENCHMARK,
	&OVERWRITE, &SKIPSUBTITLE, &OUTDIR, &ADSDIR, &ADSCOUNT, &EXECUTION, &FIXFILENAME,
	&NOOUTPUTFILE, &EXCLHIDDEN,
	
	&SIZE, &EXT, &FIND, &REGEX, &DATE, &CREATED, &MODIFIED, &ACCESSED, &CHANGED,
	
	&INSTALL, &UNINSTALL, &UPDATE,

	&HELP_REST, &HELP_DATE_REST
};

//#include <format>

#if MAKE_LIB
#include "libtvplaylist.h"
#endif

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

func transformWhiteSpace(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return std::isspace(c) ? ' ' : c;
	});
	return s;
}

func groupNumber(std::string number)
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

func trim(const std::string& s)
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

enum class IgnoreCase { None, Both, Left, Right };

/// Is left contains right
/// Parameters:
/// - ignoreChar usage, for example for {'.', ' '} -> "La.la.Li" equal "La La Li"
func isContains(const std::string& l, const std::string& r,
				const IgnoreCase ic = IgnoreCase::None,
				const std::pair<char, char>* const ignoreChar = nullptr)
{
	if (r.size() > l.size()) return std::string::npos;
	
	unsigned k{ 0 };
	unsigned i{ 0 };
	if (l.size() > r.size())
		for (; i<l.size(); ++i) {
			auto match { ignoreChar and
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

func isEqual(const char* const l, const char* const r,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long max = 0)
{
	if (not l or not r) return false;
	auto sz1 { std::strlen(l) };
	auto sz2 { std::strlen(r) };
	if (max == 0 and sz1 != sz2) return false;
	auto total { max > 0 ? max : sz1 };
	
	const char* _l = l;
	const char*_r = r;
	while (total-- > 0) {
		switch (ic){
			case IgnoreCase::Both:
				if (std::tolower(*_l) != std::tolower(*_r))
					return false;
				break;
			case IgnoreCase::Left:
				if (std::tolower(*_l) != *_r)
					return false;
				break;
			case IgnoreCase::Right:
				if (*_l != std::tolower(*_r))
					return false;
				break;
			default:
				if (*_l != *_r)
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
	if (startLeft == 0 and startRight == 0 and max == 0
		and l.size() != r.size()) return false;
	
	auto minimum { std::min(l.size(), r.size()) };
	for (auto i{ 0 }; i<(max > 0 ? std::min(max, minimum) : minimum); ++i)
		switch (ic){
			case IgnoreCase::Both:
				if (std::tolower(l[i + startLeft]) != std::tolower(r[i + startRight]))
					return false;
				break;
			case IgnoreCase::Left:
				if (std::tolower(l[i + startLeft]) != r[i + startRight])
					return false;
				break;
			case IgnoreCase::Right:
				if (l[i + startLeft] != std::tolower(r[i + startRight]))
					return false;
				break;
			default:
				if (l[i + startLeft] != r[i + startRight])
					return false;
		}
	return true;
}

inline
func isEqual(const char* const source,
			 const std::initializer_list<const char*>& list,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long startLeft = 0,
			 const unsigned long startRight = 0,
			 const unsigned long max = 0)
{
	for (auto& s : list) {
		if (isEqual(source, s, ic, startLeft, startRight, max))
			return true;
	}
	return false;
}

template <template <class ...> class Container, class ... Args>
func isEqual(const std::string& source,
			 const Container<std::string, Args ...>* args,
			 const IgnoreCase ic = IgnoreCase::None,
			 const unsigned long startLeft = 0,
			 const unsigned long startRight = 0,
			 const unsigned long max = 0)
{
	for (auto& check : *args)
		if (isEqual(source, check, ic, startLeft, startRight, max))
			return true;

	return false;
}

func isInt(const std::string& s, int* const value = nullptr)
{
	int result;
	if (auto [p, ec] = std::from_chars(s.c_str(), s.c_str()+s.size(), result);
		ec == std::errc()) {
		if (value)
			*value = result;
		return true;
	}
	
	return false;
}

inline
func getLikely(const std::string_view& src, const std::string_view& with)
{
	auto first { src.begin() }, last { src.end() };
	auto d_first { with.begin() };
	float score = 0, scoreOpenent = 0;
	if (first != last) {
		decltype (d_first) d_last = std::next(d_first, std::distance(first, last));
		for (decltype (first) i = first; i != last; ++i) {
			if (i != std::find(first, i, *i))
				continue;
			scoreOpenent += std::count(d_first, d_last, *i);
			score += std::count(i, last, *i);
		}
	}
	
	return scoreOpenent / score * 100;
}

func printHelp(const char* const arg = nullptr)
{
	func print{[](const int i) {
		std::cout << *HELPS[i];
		if (isEqual(*OPTS[i], { OPT_DATE, OPT_EXCLDATE, OPT_DCREATED,
			OPT_DEXCLCREATED, OPT_DCHANGED, OPT_DEXCLCHANGED, OPT_DMODIFIED,
			OPT_DEXCLMODIFIED, OPT_DACCESSED, OPT_DEXCLACCESSED}))
			std::cout << HELP_DATE_REST;
	}};
	if (auto i{0}; arg) {
		for (auto& opt : OPTS) {
			if (isEqual(arg, *opt)) {
				if (isEqual(arg, OPT_HELP))
					for (auto& help : ALL_HELPS)
						std::cout << *help;
				else
					print(i);
				break;
			}
			i++;
			if (i >= (sizeof(OPTS)/sizeof(OPTS[0])) - 1) {
				std::vector<std::string> found;
				auto k { 0 };
				auto indexFound { k };
				for (auto& opt : OPTS) {
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
					std::vector<int> foundIndexes;
					if (found.empty()) {
						for (auto h { 0 }; ; ++h) {
							if (h >= (sizeof(ALL_HELPS)/sizeof(ALL_HELPS[0])) - 2)
								break;
							if (isContains(*ALL_HELPS[h], arg, IgnoreCase::Both)
								not_eq std::string::npos)
								foundIndexes.emplace_back(h);
						}
					}
					
					if (foundIndexes.empty())
						std::cout << "⚠️ No option named \"" << arg << '\"';
					else {
						std::cout << "Found " << foundIndexes.size()
						<< " result containing \"" << arg << "\":\n\n";
						for (auto& id : foundIndexes) {
							print(id);
							std::cout << '\n';
						}
					}
						
					if (foundIndexes.empty() and not found.empty()) {
						std::cout << ", do you mean ";
						for (auto k{0}; k<found.size(); ++k)
							std::cout << '\"' << found[k] << '\"'
							<< (k + 1 == found.size() ? "." : " or ");
					}
				}
				std::cout << '\n';
				break;
			}
		}
	}
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

#if 0
#define directory_iterator(x, y)	fs::directory_iterator(x)
#else

#include <dirent.h>
func directory_iterator(const fs::path& path, const unsigned char type)
{
	std::vector<fs::directory_entry> result;
	
	DIR* folder = opendir(path.string().c_str());
	if(folder) {
		auto parentPath { path.string() + fs::path::preferred_separator };
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
					std::string name = parentPath;
					name += entry->d_name;
					fs::path path_name { std::move(name) };
					auto d { fs::directory_entry(std::move(path_name)) };
					d.refresh();
					if (((d.status().permissions() & ( fs::perms::owner_read
													| fs::perms::group_read
													| fs::perms::others_read))
							== fs::perms::none)
						or (state[OPT_EXCLHIDDEN] == "true"
							and entry->d_name[0] == '.'))
						continue;
					if (d.is_symlink()) {
						const auto ori { fs::directory_entry(std::move(fs::read_symlink(d.path()))) };
						if (ori.path().empty() or not ori.exists()) {
							if (
								((type & DT_DIR) == DT_DIR and not ori.is_directory())
								or
								((type & DT_REG) == DT_REG and not ori.is_regular_file())
								)
							continue;
						}
						d.assign(ori);
						d.refresh();
					}
					
					result.emplace_back(std::move(d));
				}
			}
		}
	}
	closedir(folder);
	return result;
}
#endif


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
	std::string buffer;
	for (auto& c : literal) {
		if (std::isspace(c))
			continue;
		else if (c == ',' and buffer.size() > 2) {
			if (buffer[0] == '*' and buffer[1] == '.')
				buffer.erase(buffer.begin());
			else if (buffer[0] not_eq '.')
				buffer.insert(buffer.begin(), '.');
			//result->emplace_back(buffer);
			std::fill_n(std::inserter(*result, result->end()), 1, std::move(buffer));
			buffer = "";
		} else
			buffer += std::tolower(c);
	}
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
		auto ull { Date::get_ull(l, r) };
		return ull.first < ull.second;
	}
	
	friend
	func operator > (const Date& l, const Date& r) -> bool {
		auto ull { Date::get_ull(l, r) };
		return ull.first > ull.second;
	}
	
	friend
	func operator <= (const Date& l, const Date& r) -> bool {
		auto ull { Date::get_ull(l, r) };
		return ull.first <= ull.second;
	}
	
	friend
	func operator >= (const Date& l, const Date& r) -> bool {
		auto ull { Date::get_ull(l, r) };
		return ull.first >= ull.second;
	}
	
	friend
	func operator == (const Date& l, const Date& r) -> bool {
		#define is_equal(x, y) (x != 0 and y != 0 ? x == y : true)
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
		std::tm tm{};
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

	func string(const char* const format = nullptr, const bool UTC = false) const {
		std::string s;
		if (format and std::strlen(format) > 0) {
			std::tm tm{};
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
			std::time_t t =  std::mktime(&tm);
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
private:
	static constexpr auto WEEKDAYS = {
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
	
	static constexpr auto MONTHS = {
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
public:
	static
	func getWeekDayName(const unsigned short index, const char* const locale = nullptr)
		-> std::string
	{
		if (index >= 1 or index <= 7) {
			std::tm tm{};
			tm.tm_year = 2020-1900; // 2020
			tm.tm_mon = 2-1; // February
			tm.tm_mday = 8 + index; // 15th
			std::time_t t = std::mktime(&tm); //std::time(nullptr);
			
			char mbstr[20];
			std::setlocale(LC_ALL, locale ? locale : std::locale().name().c_str());
			if (std::strftime(mbstr, sizeof(mbstr), "%A", std::localtime(&t)))
				return std::string(mbstr);
		}
		return std::string();
	}
	
	static
	func getWeekDayNames(const char* result[7], const char* const locale = nullptr) {
		for (auto i{9}, k{0}; i<16; ++i, ++k) {
			std::tm tm{};
			tm.tm_year = 2020-1900; // 2020
			tm.tm_mon = 2-1; // February
			tm.tm_mday = i; // 15th
			std::time_t t = std::mktime(&tm); //std::time(nullptr);
			
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
		for (int Y{ aYear > 0 ? aYear : 1970 };
			 Y <= aYear > 0 ? aYear : 1970; ++Y)
		{
			auto curr_year { std::chrono::year(Y) };
			
			for (int cur_month{ aMonth > 0 ? aMonth : 1 };
				 cur_month != (aYear > 0 ? aMonth + 1 : 13); ++cur_month)
			
				for (auto d { aDay > 0 ? aDay : 1 };
					 d<(aDay > 0 ? aDay + 1 : 32); ++d)
				{
					const std::chrono::year_month_day 	ymd
						{ curr_year / cur_month / d };
					const std::chrono::weekday 			cur_weekday
						{ std::chrono::sys_days{ ymd } };
					
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
	static
	func get_ull(const Date& l, const Date& r)
					-> std::pair<unsigned long long, unsigned long long>
	{
		std::string sleft, sright;
		func go{[&sleft, &sright](const unsigned short& x, const unsigned short& y)
		{
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
		go(l.weekday, r.weekday);
		
		return std::make_pair(std::stoull(sleft), std::stoull(sright));
	}
	
	func fold() const
	{
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
	
	func setWeekday()
	{
		const std::chrono::year_month_day 	ymd
			{ std::chrono::year(year) / month / day };
		const std::chrono::weekday 			aweekday
			{ std::chrono::sys_days{ ymd } };
		
		if (weekday != aweekday.iso_encoding())
			weekday = aweekday.iso_encoding();
	}
public:
	static
	func now()
	{
		const std::chrono::time_point<std::chrono::system_clock> now =
				std::chrono::system_clock::now();
		const std::time_t t = std::chrono::system_clock::to_time_t(now);

		Date date(&t);
		return date;
	}
	
	func set(const std::time_t* t, const bool UTC = false)
	{
		char a[5];
		unsigned short * unit[] = {&year, &month, &day, &hour, &minute, &second};
		for (unsigned short i{ 0 }; auto& f : {"%Y", "%m", "%d", "%H", "%M", "%S"}) {
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
			const std::chrono::time_point<std::chrono::system_clock> now =
					std::chrono::system_clock::now();
			const std::time_t t = std::chrono::system_clock::to_time_t(now);
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
			for (auto i{0}; i<s.size(); ++i)
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
					else if (not get.empty())
						others.emplace_back(get);
				} else if (std::ispunct(s[i])) {
					auto get{ s.substr(k, i - k) };
					if (isDigit(get)) {
						if (last == ':')
							time.emplace_back(std::stoi(get));
						else
							date.emplace_back(std::stoi(get));
					} else if (not get.empty())
						others.emplace_back(get);
				} else if (std::isspace(s[i])) {
					if (k != i) {
						auto get{ s.substr(k, i - k) };
						auto isNumber{ isDigit(get) };
						if (std::isalpha(last)) {
							if (auto pm{ isPM(get)}; pm)
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
						auto get = s.substr(k);
						if (auto pm{ isPM(get)}; pm)
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
				auto get{ s.substr(k) };
				auto isNumber{ isDigit(get) };
				if (last == ':' and isNumber)
					time.emplace_back(std::stoi(get));
				else if ((std::ispunct(last) or last == '\0') and isNumber)
					date.emplace_back(std::stoi(get));
				else if (not get.empty())
					others.emplace_back(get);
			}
			
			
			
			
		auto weekDay = -1;
		for (auto found{ 0 };
			auto& s : others)
		{
			s[0] = std::tolower(s[0]);
			
			if (weekDay == -1) {
				for (auto i{ 0 }; auto& m : WEEKDAYS) {
					if (s == m)
					{
						weekDay = (i % 7) + 1;
						break;
					}
					i++;
				}
				
				if (weekDay != -1) {
					assert(weekday == 0);
					weekday = weekDay;
					continue;
				}
			}
			
			for (auto i{ 0 };
				auto& m : MONTHS) {
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
				std::cout << "⚠️ Cannot understand literal \"" << s << "\"!.\n";
		}
		
		
		for (auto& i : date)
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
			auto Y { std::to_string(year) };
			auto nowy { std::to_string(int(ymd.year())) };
			year = std::stoi(nowy.substr(0, nowy.size() - Y.size()) + Y);
		}
			
		if (year > 0 and month > 0 and day > 0 and weekday == 0) {
			weekday = std::chrono::weekday{ std::chrono::sys_days{
				std::chrono::year_month_day{std::chrono::year(year)/month/day}
			}}.iso_encoding();
			
			if (weekDay > -1 and weekDay != weekday)
				std::cout << "⚠️ Invalid Weekday was defined for " << string("%B %d %Y") << "!\n";
		}
			
		unsigned short* unit[] = {&hour, &minute, &second };
		auto hasPM{ false };
		for (auto i { 0 }; auto& t : time)
			if (t < 0) {
				hasPM = t == -2;
			} else if (i < sizeof(unit)/sizeof(unit[0]))
				*unit[i++] = t;
			else
				std::cout << "⚠️ Invalid unit time on "
					<< hour << ':'<< minute << ':' << second << '\n';
		if (hasPM)
			hour += 12;
		
		
#if DEBUG
		std::cout << "input: \"" << s << "\" -> " << weekday << ' ' <<
		year << '/' << month << '/' << day << ' ' << hour << ':' << minute << ':' << second << '\n';
#endif

	}

	func isValid() -> bool {
		return not (year == 0 and month == 0 and day == 0 and hour == 0 and minute == 0 and second == 0 and weekday == 0)
		and (weekday <= 7 or year >= 10 or month <= 12 or day <= 33 or hour <= 24 or minute <= 60 or second <= 60);
	}

};

struct ID3
{
	
	property std::unordered_multimap<std::string_view, std::string> tags;

	static constexpr auto TAGS = {
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
	};
	
	static constexpr const char* GENRES[] = {
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
		auto pos { keyword.find('=') };
		auto found{ pos > 2 and pos != std::string::npos };
		
		if (found) {
			auto selectedTag { keyword.substr(0, pos) };
			found = internal_SetOfTags.find(selectedTag) != internal_SetOfTags.end();
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
		auto found { tags.find(tag) };
		if (found != tags.end())
			return found->second;
		else
			return std::string();
	}
	
	func set(const std::string& tag, const std::string& value)
	{
		auto found { tags.find(tag) };
		if (found != tags.end())
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
//		auto found { tags.find(tag) };
//		if (found != tags.end())
//			return found->second;
//		else {
//			auto newItem { std::make_pair(tag, "") };
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
		auto keyVal { l.parseInput(keyword) };
		if (keyVal) {
			if (keyVal->first == "id3") {
				for (auto& tag : l.tags)
					if (isContains(	tag.second, keyVal->second,
									l.isCaseInsensitive ? IgnoreCase::Both
									: IgnoreCase::None)
						!= std::string::npos)
						return true;
			} else
				return isContains(l[keyVal->first], keyVal->second,
								  l.isCaseInsensitive ? IgnoreCase::Both
								  : IgnoreCase::None)
						!= std::string::npos;
		}
		
		return false;
	}
	
private:
	const char* path;
	bool isCaseInsensitive;
	std::unordered_set<std::string> internal_SetOfTags;
	
	static
	func btoi(const char* bytes, const int size, const int offset)
	{
		unsigned int result = 0x00;
		int i = 0;
		for(i = 0; i < size; ++i)
			{
				result = result << 8;
				result = result | (unsigned char) bytes[offset + i];
			}
		
		return result;
	}
	
//	static
//	func itob(const int integer)
//	{
//		int i;
//		int size = 4;
//		char* result = (char*) malloc(sizeof(char) * size);
//
//		// We need to reverse the bytes because Intel uses little endian.
//		char* aux = (char*) &integer;
//		for(i = size - 1; i >= 0; i--)
//			{
//				result[size - 1 - i] = aux[i];
//			}
//
//		return result;
//	}
	
//	static
//	func syncint_encode(const int value)
//	{
//		int out, mask = 0x7F;
//
//		while (mask ^ 0x7FFFFFFF) {
//			out = value & ~mask;
//			out <<= 1;
//			out |= value & mask;
//			mask = ((mask + 1) << 8) - 1;
//			value = out;
//		}
//
//		return out;
//	}
	
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
		static constexpr auto SZ_HEADER 				= 10;
		static constexpr auto SZ_TAG 					= 3;
		static constexpr auto SZ_VERSION 				= 1;
		static constexpr auto SZ_REVISION				= 1;
		static constexpr auto SZ_FLAGS 					= 1;
		static constexpr auto SZ_HEADER_SIZE 			= 4;
		static constexpr auto SZ_EXTENDED_HEADER_SIZE 	= 4;
		
		
		property char 	tag[SZ_TAG];
		property char 	major_version,
						minor_version,
						flags;
		property int 	tag_size,
						extended_header_size;
		
		V2(const char* const buffer) :
			major_version{0x0}, minor_version{0x0}, flags{0x0},
			tag_size{0}, extended_header_size{0}
		{
			auto position{ 0 };
			std::memcpy(tag, buffer, SZ_TAG);
			major_version = buffer[position += SZ_TAG];
			minor_version = buffer[position += SZ_VERSION];
			flags = buffer[position += SZ_REVISION];
			tag_size = syncint_decode(
				btoi(buffer, SZ_HEADER_SIZE, position += SZ_FLAGS));
			
			if ((flags & (1 << 6)) == (1 << 6))
				extended_header_size = syncint_decode(
					btoi(buffer, SZ_EXTENDED_HEADER_SIZE, position += SZ_HEADER_SIZE));
		}
		
		struct Frame {
			static constexpr auto SZ_HEADER 					= 10;
			static constexpr auto SZ_ID 						= 4;
			static constexpr auto SZ_SIZE 						= 4;
			static constexpr auto SZ_FLAGS 						= 2;
			static constexpr auto SZ_ENCODING 					= 1;
			static constexpr auto SZ_LANGUAGE 					= 3;
			static constexpr auto SZ_SHORT_DESCRIPTION 			= 1;
			
			static constexpr auto INVALID_FRAME 	= 0;
			static constexpr auto TEXT_FRAME 		= 1;
			static constexpr auto COMMENT_FRAME 	= 2;
			static constexpr auto APIC_FRAME 		= 3;
			
			static constexpr auto ISO_ENCODING 		= 0;
			static constexpr auto UTF_16_ENCODING 	= 1;
			
			property char id[SZ_ID];
			property char flags[SZ_FLAGS];
			property int size;
			property std::string data;
			
			func isValid() const
			{
				return size != 0;
			}
			
			Frame(const char* const raw, int offset, const int major_version) :
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
				auto tmp = raw;
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
	
	func initSet()
	{
		if (internal_SetOfTags.empty())
			for (auto& tag : TAGS)
				internal_SetOfTags.emplace(tag);
	}
public:
	
	func write(const char* const a_path = nullptr)
	{
		if (not path and not a_path)
			return;
		std::fstream file;
		file.open(a_path ? a_path : path);
		
		file.seekp(0, std::ios_base::end);
		const int end = int(file.tellp());
		
		file.seekp(end - 128);
		int pos = int(file.tellp());
		
		func set{[&file, &pos](ID3* const id3,
							   const char* const key,
							   const int size,
							   const char* const value = nullptr)
		{
			const char* currVal { nullptr };
			if (key and id3->tags.find(key) == id3->tags.end())
				id3->add(key, value ? value : "");
			else if (key)
				currVal = id3->get(key).c_str();
			
			auto val { key ? (value ? value : (currVal ? currVal
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
		
		if (auto genre { get("genre") };
			not genre.empty())
			for (auto i{0}; auto& g : GENRES) {
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
		std::string s;
		
		for (auto& tag : tags)
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
	
	ID3() { initSet(); }
	
	ID3(const char* const path, const bool case_insensitive = false)
	{
		initSet();
		this->path = path;
		isCaseInsensitive = case_insensitive;
		std::ifstream file;
		file.open(path, std::ios::in | std::ios::ate | std::ios::binary);
		if (not file.good()) {
			this->path = nullptr;
			return;
		}
		
		file.seekg(0);

		if (char s_tag[V2::SZ_HEADER];
			file.read(s_tag, V2::SZ_HEADER)
			and 0 == std::strncmp(s_tag, "ID3", 3))
		{
			V2 tag(s_tag);
			if (char bytes[tag.tag_size + 10];
				
				tag.major_version != 0
				and file.read(bytes, tag.tag_size + 10))
			{
				
				add("version", "2." + std::to_string(tag.major_version)
				+ (tag.minor_version > 0
					? "." + std::to_string(tag.minor_version)
					: ""));
				
				auto isv24 { tag.major_version == 4 };
				auto isv23 { tag.major_version == 3 };
				
				auto offset{ tag.extended_header_size };
				
				while (offset < tag.tag_size)
				{
					V2::Frame frame(bytes, offset, tag.major_version);
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
							int index = -1;
							
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
						std::cout << "Frame: " << frame.frame_id << " = " << frame.data << '\n';
						
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
				for(int i = 0; i < size; ++i)
					buffer[i] = file.get();
				
				if (size == 1) {
					int result = buffer[0];
					if (auto genreIndex{0}; isGenre)
						for (auto& g : GENRES) {
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
			const int end = int(file.tellg());
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
				auto genre { get(1, true) };
				int index;
				if (auto [p, ec] = std::from_chars(genre.c_str(),
								genre.c_str()+genre.size(), index);
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
#undef property

std::unordered_set<std::string> EXCLUDE_EXT;
std::unordered_set<std::string> DEFAULT_EXT {
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

std::vector<std::string> listFindDir, listExclFindDir;

func isValidFile(const fs::path& path)
{
	if (not fs::exists(path))
		return false;

	if (not state[OPT_EXCLEXT].empty() and state[OPT_EXCLEXT] not_eq "*")
		if (not EXCLUDE_EXT_REPLACED) {
			parseExtCommaDelimited(state[OPT_EXCLEXT], &EXCLUDE_EXT);
			EXCLUDE_EXT_REPLACED = true;
		}
	if (not state[OPT_EXT].empty() and state[OPT_EXT] not_eq "*")
		if (not DEFAULT_EXT_REPLACED) {
			parseExtCommaDelimited(state[OPT_EXT], &DEFAULT_EXT);
			DEFAULT_EXT_REPLACED = true;
		}
	
	
	
	fs::path tmp{ path };
	if (fs::is_symlink(tmp)) { //TODO: is_symlink() cannot detect macOS alias file!
		tmp = fs::read_symlink(path);
		
		if (not fs::exists(tmp) or not fs::is_regular_file(tmp))
			return false;
	}
	
	std::string fileExt { tolower(tmp.extension().string()) };
	if (state[OPT_EXT] not_eq "*"
		and DEFAULT_EXT.find(fileExt)
		== DEFAULT_EXT.end())
		return false;
	
	if (state[OPT_EXCLEXT] not_eq "*"
		and not state[OPT_EXCLEXT].empty())
		if (EXCLUDE_EXT.find(fileExt)
			not_eq EXCLUDE_EXT.end())
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
		
		const char* const* st[2][4] = {
			{ &OPT_DCREATED, &OPT_DACCESSED, &OPT_DMODIFIED, &OPT_DCHANGED },
			{ &OPT_DEXCLCREATED, &OPT_DEXCLACCESSED, &OPT_DEXCLMODIFIED, &OPT_DEXCLCHANGED },
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
			if (state[*st[z][i]] not_eq "") {
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
	
	if (not listFind.empty() or not listExclFind.empty()) // MARK: Find statement
	{
		auto ismp3 { isEqual(fileExt.c_str(), {".mp3", ".aac", ".m4a"}) };
		auto filename { excludeExtension(tmp.filename()) };
		auto isCaseInsensitive { state[OPT_CASEINSENSITIVE] == "true" };
		
		ID3 id3;
		if (ismp3)
			id3 = ID3(tmp.string().c_str(), isCaseInsensitive);
	
		if (bool found{ false }; not listFind.empty())
		{
			for (auto& keyword : listFind)
			{
				auto handled { keyword[0] == char(1) };
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

		for (auto& keyword : listExclFind)
		{
			auto handled { keyword[0] == char(1) };
			if (auto found { false }; not handled and ismp3 and (handled = true)) {
				found = id3 % keyword;
				if (handled and found)
					return false;
			}
			
			if (not handled and isContains(filename, keyword, isCaseInsensitive
				? IgnoreCase::Both : IgnoreCase::None) not_eq std::string::npos)
				return false;
		}
	}


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
		
	return true;
}

func getAvailableFilename(const fs::path& original,
						  const std::string& prefix = " #",
						  const std::string& suffix = "")
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

func sort(const fs::path& a, const fs::path& b, bool ascending)
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
			return ascending ? av[i] < bv[i] : av[i] > bv[i];
		}

	return afn < bfn;
}

inline
func ascending(const fs::path& a, const fs::path& b)
{
	return sort(a, b, true);
}

inline
func descending(const fs::path& a, const fs::path& b)
{
	return sort(a, b, false);
}


func sortFiles(std::vector<fs::path>* const selectFiles)
{
	if (selectFiles->size() > 1) {
		std::unordered_map<std::string, std::shared_ptr<std::vector<fs::path>>> selectFilesDirs;
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

func isDirNameValid(const fs::path& dir)
{
	if (listFindDir.empty() and listExclFindDir.empty())
		return true;
	
	const auto filename { dir.filename().string() };
	const auto ignoreCase { state[OPT_CASEINSENSITIVE] == "true" };
	for (auto& keyword : listFindDir)
		if (isContains(filename, keyword, ignoreCase ? IgnoreCase::Left
					   : IgnoreCase::None) not_eq std::string::npos)
			return true;
	
	auto result { true };
	for (auto& keyword : listExclFindDir)
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
	or (state[OPT_EXCLHIDDEN] == "true" and path.filename().string()[0] == '.')
	);
}

func listDir(const fs::path& ori, std::vector<fs::directory_entry>* const out,
			 const bool sorted=true, const bool includeRegularFiles = false)
{
	if (not out)
		return;
	
	auto path { std::move(ori) };
	if (fs::is_symlink(ori)) {
		std::error_code ec;
		path = fs::read_symlink(ori, ec);
		if (ec or not fs::is_directory(path))
			return;
	}

	for (auto& child : directory_iterator(path, (includeRegularFiles
				? (DT_DIR | DT_REG) : DT_DIR)))
		out->emplace_back(child);
	
	if (sorted and out->size() > 1)
		std::sort(out->begin(), out->end(), [](const fs::directory_entry& a,
											   const fs::directory_entry& b) {
			return ascending(a.path(), b.path());
		});
}

template <template <class ...> class Container, class ... Args>
func listDirRecursively(const fs::path& path,
						Container<fs::path, Args...>* const out,
						const bool includeRegularFiles)
{
	if (not out or path.empty())
		return;
	
	fs::path head = path;
	std::fill_n(std::inserter(*out, out->end()), 1, std::move(path));
	
	/// Try to expand single dir and put into list
	std::vector<fs::directory_entry> list;
	std::vector<fs::path> dirs;
	std::vector<std::thread> threads;
	std::vector<std::future<void>> asyncs;
	
	
	func emplace{[&list, &out]()
	{
		for (auto& d : list)
			std::fill_n(std::inserter(*out, out->end()), 1, std::move(d.path()));
	}};
	
	do {
		do {
			list.clear();
			dirs.clear();
			listDir(head, &list, false, includeRegularFiles);
			if (list.size() == 1)
				std::fill_n(std::inserter(*out, out->end()), 1,
							std::move(list[0].path()));
			if (not list.empty()) {
				for (auto& child : list)
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
		
		for (auto& d : dirs)
			if (d.empty())
				continue;
			else if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD)
				threads.emplace_back(listDir, d, &list,
									 false, includeRegularFiles);
			else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC)
				asyncs.emplace_back(std::async(
						std::launch::async, listDir, d, &list,
											   false, includeRegularFiles));
			else
				listDir(d, &list, false, includeRegularFiles);
		
		
		if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD) {
			for (auto& t : threads)
				t.join();
			threads.clear();
		}
		else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC) {
			for (auto& a : asyncs)
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

std::vector<fs::path> regularDirs;
std::vector<fs::path> seasonDirs;
std::vector<fs::path> selectFiles;

const std::vector<std::string> SUBTITLES_EXT { ".srt", ".ass", ".vtt" };
const std::pair<char, char> FILENAME_IGNORE_CHAR {'.', ' '};

func findSubtitleFile(const fs::path& original,
					  std::vector<fs::path>* const result)
{
	if (auto parentPath{original.parent_path()}; not parentPath.empty()) {
		auto noext{excludeExtension(original.filename())};
		for (auto& f : directory_iterator(parentPath, DT_REG))
			if (isValid(f)
				and f.path().string().size() >= original.string().size()
				and isEqual(f.path().extension().string(), &SUBTITLES_EXT,
							IgnoreCase::Left)
				and isContains(f.path().filename().string(), noext,
				IgnoreCase::Both, &FILENAME_IGNORE_CHAR) not_eq std::string::npos)

				result->emplace_back(std::move(f));
	}
}



template <typename Container, typename T>
func insertTo(Container* const out, const T& path)
{
	auto isFound { isValid(path) };
	if (isFound)
		std::fill_n(std::inserter(*out, out->end()), 1, std::move(path));

	return isFound;
}

func checkForSeasonDir(const fs::path& path) -> void
{
	if (not path.empty()) {
		auto hasDir{false};
		
		bool isNum{true};
		std::vector<MAXNUM> lastNum;
		std::vector<fs::path> bufferNum;
		
		func pullFromBufferNum{ [&bufferNum, &isNum]()
		{
			isNum = false;
			for (auto& child : bufferNum) {
				insertTo(&regularDirs, child);
				checkForSeasonDir(child);
			}
		}};
		
		std::vector<fs::directory_entry> sortedDir;
		listDir(path, &sortedDir);
		
		if (sortedDir.size() == 1)
			checkForSeasonDir(sortedDir[0].path());
		else if (sortedDir.size() > 1)
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
			for (auto it = regularDirs.begin(); it != regularDirs.end(); ) {
				if (*it == path) {
					it = regularDirs.erase(it);
				} else {
					++it;
				}
			}
			
			insertTo(&seasonDirs, path);
		} else
			insertTo(&regularDirs, path);
	}
}

func getBytes(const std::string& s)
{
	std::string unit{"mb"};
	std::string value{s};
	if (s.size() > 2 and std::isalpha(s[s.size() - 2])) {
		unit = s.substr(s.size() - 2);
		value = s.substr(0, s.size() - 2);
	}
	
	uintmax_t result{0};
	
	if (not isInt(value))
		return result;
	
	float v { std::stof(value) };
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
	auto pos{argv.find(separator)};
	if (pos == std::string::npos)
		return nullptr;
	
	auto first{argv.substr(0, pos)};
	auto second{argv.substr(pos + separator.size())};

	uintmax_t from{getBytes(std::move(first))};
	from = std::max(from, static_cast<uintmax_t>(0));
	uintmax_t to{getBytes(std::move(second))};
	to = std::max(to, static_cast<uintmax_t>(0));
	return std::make_shared<std::pair<uintmax_t, uintmax_t>>(from, to);
}

constexpr auto ARG_SEP { ':' };
constexpr auto ARG_ASSIGN { '=' };

func expandArgs(const int argc, char* const argv[],
				const int startAt, std::vector<std::string>* const args)
{
	bool newFull{ false };
	unsigned lastOptCode{ 0 };
	func push{ [&newFull, &args](const char* const arg,
								 const unsigned index,
								 const int last)
	{
		if (last < 0 or index - last <= 0) return;
				
		unsigned size = index - last;
		std::string newArg;

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
	
	const bool isMultiLine{
		startAt < argc
		and std::string(argv[startAt]).find("\x0a") not_eq std::string::npos
	};
	for (int i{startAt}; i<argc; ++i) {
		auto arg { argv[i] };
		
		if (isMultiLine)
			while (arg[0] == 0x0a or arg[0] == 0x0d)
				arg++;
		
		auto len { std::strlen(arg) };
		auto isMnemonic{ len > 1 and arg[0] == '-'
			and (std::isalpha(arg[1]) or arg[1] == ARG_SEP or arg[1] == ';' ) };
		auto isFull {false};
		if (not isMnemonic) {
			isFull = len > 2 and arg[0] == '-' and arg[1] == '-'
				and (std::isalpha(arg[2]) or arg[2] == ARG_SEP or arg[1] == ';');
			if (isFull)
				newFull = true;
			else
			{
				if (lastOptCode > 0 and arg[0] == ARG_ASSIGN) {
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
		
		unsigned equalOpCount{ 0 };
		unsigned fallthrough { 0 };
		unsigned index = lastOptCode == 3 ? -1 : (isFull and lastOptCode != 2 ? 1 : 0);
		int last{ -1 };
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
			if (last < 0 and std::isalpha(arg[index])) {
				if (isMnemonic) {
					lastOptCode = 1;
					std::string new_s {'-'};
					new_s += arg[index];
					args->emplace_back(new_s);
				} else if (isFull) {
					//lastOptCode = 2;
					last = index;
				}
			} else {
				if (equalOpCount == 0 and (arg[index] == ARG_ASSIGN or std::isspace(arg[index]))) {
					if (isFull) {
						lastOptCode = 2;
						push(arg, index, last);
						last = -1;
					} else {
						lastOptCode = 1;
						last = index + 1;
					}
					equalOpCount++;
				} else if (auto foundEnter { arg[index] == 0x0a };
						   ((arg[index] == ARG_SEP or arg[index] == ';')
						   and (index + 1 == std::strlen(arg) or
								(index + 1 < std::strlen(arg) and std::isalpha(arg[index + 1])))
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
						index + 1 < std::strlen(arg) and std::isalpha(arg[index + 1]);
					}
					equalOpCount = 0;
				} else if (isFull and last > 0 and (arg[index] == '<' or arg[index] == '>')) {
					if (arg[last - 1] == ARG_ASSIGN or arg[last - 1] == '<' or arg[last - 1] == '>')
						continue;
					
					lastOptCode = 0;
					push(arg, index, last);
					last = index;
				} else {
					if (arg[index] == '"') {
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

func parseKeyValue(std::string* const s, const bool isExclude)
{
	if (not s or s->empty())
		return;
		
	const char* keyword = "";
	std::string value;
	bool isKeyValue { true };
	for (auto& key : {"dir", OPT_EXT, OPT_EXCLEXT,
		OPT_CASEINSENSITIVE, OPT_EXCLHIDDEN, OPT_SIZE, OPT_EXCLSIZE,
		"case-insensitive", "caseinsensitive",
		"ignore-case", "ignorecase"})
	{
		const auto sz { std::strlen(key) };
		
		if (s->size() > sz + 2)
			if (const auto c{ s->at(sz) };
				
				s->starts_with(key) and (c == '=' or c == '<' or c == '>'))
			{
				keyword = key;
				value = s->substr(sz + (c == '=' ? 1 : 0));
				break;
			}
	}
	if (isEqual(keyword, { OPT_SIZE,OPT_EXCLSIZE}))
	{
		auto pos { value.find("..") };
		auto next { 2 };
		if (pos == std::string::npos) {
			pos =value.find('-');
			next = 1;
		}
		if (pos != std::string::npos) {
			auto lower { getBytes(value.substr(0, pos))};
			auto upper { getBytes(value.substr(pos + next)) };
			if (lower > upper)
				isKeyValue = false;
			else {
				state[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] = '\0';
				state[keyword] = std::to_string(lower);
				
				(isExclude ? listExclSize : listSize).emplace_back(std::make_pair(
							   lower,
							   upper));
			}
		}
		else if (const auto havePrefix { value[0] == '<' or value[0] == '>'};
				 havePrefix) {
			if (auto number { getBytes(value.substr(havePrefix ? 1 : 0)) };
				number < 0)
				isKeyValue = false;
			else {
				state[isExclude ? OPT_EXCLSIZEOPGT : OPT_SIZEOPGT] =
				state[keyword] = std::to_string(number);
				(isExclude ? listExclSize : listSize).clear();
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
			if (state[OPT_CASEINSENSITIVE] == "true")
				value = tolower(value);
			(isExclude ? listExclFindDir : listFindDir).emplace_back(value);
		}
	}
	else if (isEqual(keyword, {OPT_EXCLHIDDEN, OPT_EXT, OPT_EXCLEXT})) {
		state[keyword] = value;
	}
	else if (isEqual(keyword, {OPT_DATE, OPT_EXCLDATE,
		OPT_DCREATED, OPT_DCHANGED, OPT_DACCESSED, OPT_DMODIFIED,
   OPT_DEXCLCREATED, OPT_DEXCLCHANGED, OPT_DEXCLMODIFIED, OPT_DEXCLACCESSED}))
   {
	   const auto havePrefix { value[0] == '<' or value[0] == '>'};
	   if (auto opGt{ value[0] };
		   
		   havePrefix or opGt == '=')
	   {
		   if (Date date(value);
			   not date.isValid())
			   isKeyValue = false;
		   else {
			   if (isEqual(keyword, OPT_DATE)) {
				   listDCreated.emplace_back(std::make_pair(opGt, date));
				   listDChanged.emplace_back(std::make_pair(opGt, date));
				   listDModified.emplace_back(std::make_pair(opGt, date));
				   listDAccessed.emplace_back(std::make_pair(opGt, date));
				   state[OPT_DCREATED]  = "1";
				   state[OPT_DMODIFIED] = "1";
				   state[OPT_DACCESSED] = "1";
				   state[OPT_DCHANGED]  = "1";
			   } else if (isEqual(keyword, OPT_EXCLDATE)) {
				   listDExclCreated.emplace_back(std::make_pair(opGt, date));
				   listDExclChanged.emplace_back(std::make_pair(opGt, date));
				   listDExclModified.emplace_back(std::make_pair(opGt, date));
				   listDExclAccessed.emplace_back(std::make_pair(opGt, date));
				   state[OPT_DEXCLCREATED]  = "1";
				   state[OPT_DEXCLMODIFIED] = "1";
				   state[OPT_DEXCLACCESSED] = "1";
				   state[OPT_DEXCLCHANGED]  = "1";
			   } else {
				 (isEqual(keyword, OPT_DCREATED) ? listDCreated
				: isEqual(keyword, OPT_DCHANGED) ? listDChanged
				: isEqual(keyword, OPT_DMODIFIED) ? listDModified
				: isEqual(keyword, OPT_DACCESSED) ? listDAccessed
				: isEqual(keyword, OPT_DEXCLCREATED) ? listDExclCreated
				: isEqual(keyword, OPT_DEXCLCHANGED) ? listDExclChanged
				: isEqual(keyword, OPT_DEXCLMODIFIED) ? listDExclModified
				: listDExclAccessed
				).emplace_back(std::make_pair(opGt, date));

				   state[keyword] = "1";
			   }
		   }
	   }
	   else {
		   auto pos { value.find("..") };
		   auto next { 2 };
		   
		   if (pos == std::string::npos) {
			   next = 1;
			   unsigned strip { 0 };
			   pos = 0;
			   for (auto m{ 0 }; m < value.size(); ++m)
				   if (value[m] == '-') {
					   strip++;
					   pos = m;
				   }
			   if (strip != 1)
				   pos = std::string::npos;
		   }
		   
		   if (pos == std::string::npos)
			   isKeyValue = false;
		   else {
			   if (Date lower(value.substr(0, pos));
				   not lower.isValid())
				   isKeyValue = false;
			   else {
				   if (Date upper(value.substr(pos + next));
					   not upper.isValid() or lower > upper)
					   isKeyValue = false;
				   else {
					   if (isEqual(keyword, OPT_DATE)) {
						   listDCreatedR.emplace_back(std::make_pair(lower, upper));
						   listDChangedR.emplace_back(std::make_pair(lower, upper));
						   listDModifiedR.emplace_back(std::make_pair(lower, upper));
						   listDAccessedR.emplace_back(std::make_pair(lower, upper));
						   state[OPT_DCREATED]  = "1";
						   state[OPT_DMODIFIED] = "1";
						   state[OPT_DACCESSED] = "1";
						   state[OPT_DCHANGED]  = "1";
					   } else if (isEqual(keyword, OPT_EXCLDATE)) {
						   listDExclCreatedR.emplace_back(std::make_pair(lower, upper));
						   listDExclChangedR.emplace_back(std::make_pair(lower, upper));
						   listDExclModifiedR.emplace_back(std::make_pair(lower, upper));
						   listDExclAccessedR.emplace_back(std::make_pair(lower, upper));
						   state[OPT_DEXCLCREATED]  = "1";
						   state[OPT_DEXCLMODIFIED] = "1";
						   state[OPT_DEXCLACCESSED] = "1";
						   state[OPT_DEXCLCHANGED]  = "1";
					   } else {
							 (isEqual(keyword, OPT_DCREATED) ? listDCreatedR
							: isEqual(keyword, OPT_DCHANGED) ? listDChangedR
							: isEqual(keyword, OPT_DMODIFIED) ? listDModifiedR
							: isEqual(keyword, OPT_DACCESSED) ? listDAccessedR
							: isEqual(keyword, OPT_DEXCLCREATED) ? listDExclCreatedR
							: isEqual(keyword, OPT_DEXCLCHANGED) ? listDExclChangedR
							: isEqual(keyword, OPT_DEXCLMODIFIED) ? listDExclModifiedR
							: listDExclAccessedR
							).emplace_back(std::make_pair(lower, upper));
						   state[keyword] = "1";
					   }
				   }
			   }
		   }
	   }
   }
   else if (0 != std::strlen(keyword)) {
		state[OPT_CASEINSENSITIVE] = value;
		if (isEqual(value, "true", IgnoreCase::Left)) {
			for (auto& k : listFindDir) k = tolower(k);
			for (auto& k : listExclFindDir) k = tolower(k);
			for (auto& k : listFind) k = tolower(k);
			for (auto& k : listExclFind) k = tolower(k);
		}
	}
	
	if (isKeyValue and 0 != std::strlen(keyword))
		s->insert(0, 1, char(1));
}

template <typename  T>
func getLines(std::basic_istream<T>* const inputStream,
			  std::vector<std::string>* const lines,
			  const std::initializer_list<std::string>&& excludePrefix)
{
	int commentCount { 0 };
	bool isComment { false };
	std::string buff;
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
		unsigned i { 0 };
		for (const auto& s : excludePrefix) {
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
		else if (auto peek { inputStream->peek() };
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

constexpr auto ARGS_SEPARATOR {"\x0a\x0aARGS-SEPARATOR\x0a\x0a"};

enum class WriteConfigMode { New, Edit, Add, Remove };

func writeConfig(const std::vector<std::string>* const args,
				 const WriteConfigMode mode)
{
	enum class ReadDirection { All, Old, Current };
	func argsToLines{[&args](std::vector<std::string>* const lines,
							 const ReadDirection direction,
							 const bool removeOptPrefix = false)
	{
		std::string buffer;
		auto foundSep { false };
		for (unsigned i { 0 }; i<args->size(); ++i) {
			const auto s = (*args)[i];
			if (s == ARGS_SEPARATOR)
			{
				foundSep = true;
				if (direction == ReadDirection::Old)
					break;
				else
					continue;
			} else if (direction == ReadDirection::Current and not foundSep)
				continue;
			
			unsigned offset = 0;
			auto isOpt { s.starts_with("--") };
			if (isOpt)
				offset = 2;
			else if (isOpt = s.size() >= 2 and s[0] == '-' and std::isalpha(s[1])
					 and (s.size() == 2 or (s[2] == '=' or s[2] == ' '));
					 isOpt)
				offset = 1;
			if (isOpt) {
				if ((offset == 2 and isEqual(s.c_str() + offset,
						{OPT_WRITEDEFAULTS, OPT_SHOWCONFIG,
						OPT_INSTALL, OPT_UNINSTALL, OPT_UPDATE}))
					or (offset == 1 and (s[1] == 'W')))
				{
					if (auto next { i + 1 < args->size() ? (*args)[i + 1] : "" };
						OPT_WRITEDEFAULTS
						and not isEqual(next.c_str(), OPT_WRITEDEFAULTS_ARGS,
										IgnoreCase::Left))
						;
					else
						i++;
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
	
	std::unordered_set<std::string> definedList;
	std::vector<std::string> lines;
	if (	mode == WriteConfigMode::Edit
		or 	mode == WriteConfigMode::Remove)
	{
		std::vector<std::string> newLines;
		argsToLines(&lines, ReadDirection::Old, false);
		argsToLines(&newLines, ReadDirection::Current, false);
	
		if (mode == WriteConfigMode::Remove) {
			for (auto line = lines.begin(); line<lines.end();) {
				auto found { false };
				for (auto& opt : newLines)
					if (found = *line == opt; found) {
						lines.erase(line);
						break;
					}
				if (not found)
					line++;
			}
		} else {
			for (auto line = newLines.begin(); line<newLines.end(); ++line)
				if (bool isOpt {
					(line->size() > 3 and line->starts_with("--")
						and std::isalpha((*line)[2]) and std::isalpha((*line)[3]))
					or
					(line->size() >= 2 and (*line)[0] == '-' and std::isalpha((*line)[1])
						and (line->size() == 2 or ((*line)[2] == '=' or (*line)[2] == ' ')))
				};
					
					isOpt)
				{
					auto pos { line->find('=') };
					if (pos == std::string::npos)
						pos = line->find(' ');
					
					if (pos not_eq std::string::npos)
						definedList.emplace(line->substr(0, pos));
				}
			
			for (auto line = lines.begin(); line<lines.end();) {
				auto found { false };
				for (auto& opt : definedList)
					if (found = line->starts_with(opt); found) {
						lines.erase(line);
						break;
					}
				if (not found)
					line++;
			}
			
			
			for (auto& line : newLines)
				lines.emplace_back(std::move(line));
		}
		
		definedList.clear();
	}
	else if (mode == WriteConfigMode::New)
		argsToLines(&lines, ReadDirection::Current, true);
	else
		argsToLines(&lines, ReadDirection::All, true);
	
	/// Remove duplication
	for (auto line = lines.end() - 1; line>=lines.begin(); --line) {
		auto found { false };
		for (auto& opt : SINGLE_VALUE_OPT)
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
	
	std::fstream file(CONFIG_PATH, std::ios::out);
	#define DATE_FORMAT	"%A, %d %B %Y at %I:%M:%S %p"
	if (mode == WriteConfigMode::New )
		file << "# Configuration made in " << Date::now().string(DATE_FORMAT) << '\n';
	else
		file << "# Edited on " << Date::now().string(DATE_FORMAT) << '\n';
	#undef DATE_FORMAT
	
	for (auto& line : lines)
		file << line << '\n';
	
	file.flush();
	
	file.close();
}

func loadConfig(std::vector<std::string>* const args)
{
	if (not args
		or state[OPT_LOADCONFIG].empty()
		or not fs::exists(state[OPT_LOADCONFIG])
		or not fs::is_regular_file(state[OPT_LOADCONFIG]))
		return;
	
	std::ifstream file(state[OPT_LOADCONFIG], std::ios::in);
	file.seekg(0);

	std::vector<std::string> lines;
	getLines(&file, &lines, {"--"});
	
	file.close();
	
	for (auto found{ false };
		 const auto& line : lines) {
		for (const char* const *const opt : OPTS)
			if (auto isMnemonic {
				(line.size() >= 2 and line[0] == '-' and std::isalpha(line[1])
				 and (line.size() == 2 or (line[3] == '=' or line[3] == ' ')))
				or (line.size() == 1 and std::isalpha(line[0]))
				or (line.size() > 2 and std::isalpha(line[0])
					and (line[1] == '=' or line[1] == ' '))
			};
				isMnemonic
				or (line.size() > 3 and line[0] == '-' and line[1] == '-'
					and 0 == std::strncmp(line.c_str() + 2, *opt, std::strlen(*opt)))
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
				
				if (auto col{ line.find('=') };
					col != std::string::npos
					or line.find(' ') != std::string::npos)
				{
					if (col == std::string::npos)
						col = line.find(' ');
					else
						col++;
					
					while (++col < line.size() and std::isspace(line[col]))
						;
					
					if (col < line.size()) {
						auto value { line.substr(col) };
						
						std::vector<unsigned> puncs;
						for (unsigned i{ 0 }; i<value.size(); ++i)
							if (value[i] == '"')
								puncs.emplace_back(i);
						
						if (value[0] == '"' and puncs.size() % 2 == 0 and puncs.size() > 2) {
							if (args)
								for (unsigned k{ 0 }; k<puncs.size(); k += 2)
								{
									std::string sub;
									for (auto j{puncs[k] + 1}; j<puncs[k + 1]; j++)
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

const std::string XML_CHARS_ALIAS[] = {"&quot", "&apos", "&lt", "&gt", "&amp"};
const std::string XML_CHARS_NORMAL[] = {"\"", "\\", "<", "<", "&"};
constexpr auto NETWORK_PROTOCOLS = {"http:", "https:", "ftp:", "ftps:", "rtsp:", "mms:"};

func replace_all(std::string& inout,
				 const std::string_view what, const std::string_view with)
{
	std::size_t count{};
	for (std::string::size_type pos{};
		 inout.npos != (pos = inout.find(what.data(), pos, what.length()));
		 pos += with.length(), ++count) {
		inout.replace(pos, what.length(), with.data(), with.length());
	}
}
 

func loadPlaylist(const fs::path& path, std::vector<fs::path>* const outPaths)
{
	if (not outPaths or path.empty() or not fs::exists(path))
		return;
	
	fs::path lastParent;
	func prolog{[&path, &lastParent](std::ifstream* const file) {
		lastParent = fs::current_path();
		fs::current_path(path.parent_path());
		*file = std::ifstream(path.string(), std::ios::in);
		file->seekg(0);
	}};
	
	func epilogue{[&lastParent](std::ifstream* const file) {
		file->close();
		fs::current_path(lastParent);
	}};
	
	func push{[&outPaths](std::string& buff) {
		if (buff.empty())
			return;
		auto found { false };
		for (const char* const protocol : NETWORK_PROTOCOLS)
			if (buff.find('%') != std::string::npos
				and buff.starts_with(protocol)) {
				replace_all(buff, "%20", " "); // TODO: Decoding percent-encoded triplets of unreserved characters. Percent-encoded triplets of the URI in the ranges of ALPHA (%41–%5A and %61–%7A), DIGIT (%30–%39), hyphen (%2D), period (%2E), underscore (%5F), or tilde (%7E) do not require percent-encoding and should be decoded to their corresponding unreserved characters.
				replace_all(buff, "%3D", "=");
				replace_all(buff, "%2B", "+");
				replace_all(buff, "%2D", "-");
				replace_all(buff, "%3F", "?");
				replace_all(buff, "%3B", ";");
				replace_all(buff, "%25", "%");
				outPaths->emplace_back(fs::path(buff));
				found = true;
				break;
			}
		
		if (not found) {
			if (buff.starts_with("file://"))
				buff = buff.substr(7);
			outPaths->emplace_back(fs::absolute(fs::path(buff)));
		}
		
		buff.clear();
	}};
	
	func after{[](std::ifstream* const file,
				  const char* const keyword,
				  std::string* const before = nullptr)
	{
		char c;
		unsigned long index = 0;
		unsigned long indexMax = std::strlen(keyword);
		std::string buff;
		const auto lastPos { file->tellg() };
		unsigned commentCount { 0 };
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
					 std::vector<std::string>* const lines)
	{
		std::string buff;
		bool isComment{ false };
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
		 std::initializer_list<const char* const>* const prefixes,
		 std::initializer_list<const char* const>* const suffixes)
	{
		for (const char* const prefix : *prefixes)
			if (after(file, prefix)) {
				for (const char* const suffix : *suffixes)
					if (std::string value;
						after(file, suffix, &value))
					{
//						" to  &quot;
//						' to  &apos;
//						< to  &lt;
//						> to  &gt;
//						& to  &amp;
						if (value.find('&') != std::string::npos)
							for (auto w { 0 }; w<sizeof(XML_CHARS_ALIAS) /sizeof(XML_CHARS_ALIAS[0]); ++w)
								if (isContains(value, XML_CHARS_ALIAS[w],
									IgnoreCase::Left) not_eq std::string::npos)
								{
									replace_all(value, XML_CHARS_ALIAS[w], XML_CHARS_NORMAL[w]);
								}
						push(value);
						return true;
					}
				break;
			}
		return false;
	}};
	
	func xml{[&prolog, &epilogue, &xml_tag, &push, &after]
		(std::initializer_list<const char* const>&& prefixes,
		 std::initializer_list<const char* const>&& suffixes)
	{
		std::ifstream file;
		prolog(&file);
		while (xml_tag(&file, &prefixes, &suffixes))
				;
		epilogue(&file);
	}};
	
	func pls{[&prolog, &epilogue, &getLines, &push]() {
		std::ifstream file;
		prolog(&file);
		std::vector<std::string> lines;
		getLines(&file, &lines);
		for (auto& line : lines)
			if (unsigned long col;
				line.starts_with("File")
				and (col = line.find('=')) != std::string::npos)
			{
				std::string path { line.substr(col + 1) };
				push(path);
			}
		epilogue(&file);
	}};
	
	func m3u{[&prolog, &epilogue, &getLines, &push]() {
		std::ifstream file;
		prolog(&file);
		std::vector<std::string> lines;
		getLines(&file, &lines);
		for (auto& line : lines)
			push(line);
		epilogue(&file);
	}};
	
	const auto ext { path.extension().string() };
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
#undef func

#if MAKE_LIB
#define RETURN_VALUE	;
#define ARGS_START_INDEX	0
void process(int argc, char *argv[], int *outc, char *outs[], unsigned long *maxLength)
{
	state[OPT_NOOUTPUTFILE] = "true";
#else
#define RETURN_VALUE	EXIT_SUCCESS;
#define ARGS_START_INDEX	1
auto main(const int argc, char* const argv[]) -> int
{
#endif

	std::vector<fs::path> bufferDirs;
	std::vector<fs::path> listAdsDir;

	state[OPT_SIZEOPGT] = '\0';
	state[OPT_SIZE] 	= "0";
	state[OPT_EXCLSIZEOPGT] = '\0';
	state[OPT_EXCLSIZE] 	= "0";

	state[OPT_EXECUTION]= OPT_EXECUTION_ASYNC;
	state[OPT_REGEXSYNTAX] = "ecma";
	state[OPT_ARRANGEMENT] = OPT_ARRANGEMENT_DEFAULT;
		
	std::vector<std::string> args;
	
	state[OPT_LOADCONFIG] = CONFIG_PATH ;
	
	loadConfig(&args);
	args.emplace_back(ARGS_SEPARATOR);
	expandArgs(argc, argv, ARGS_START_INDEX, &args);
	
	unsigned long fileCountPerTurn{ 1 };

	std::unordered_set<std::string_view> invalidArgs;
	
	for (int i{0}; i<args.size(); ++i) {
		if (auto isMatch{ [&args, &i](const char* const with,
							  const char mnemonic,
							  bool writeBoolean=false,
							  const std::initializer_list<const char*>& others = {}) {
			auto result { false };
			const auto isWithStartWithDoubleStrip {
				std::strlen(with) > 2 and with[0] == '-' and with[1] == '-' };
			if (args[i].length() > 3
				and args[i][0] == '-'
				and args[i][1] == '-')
			{
				result = isEqual(args[i], with, IgnoreCase::Left,
								 isWithStartWithDoubleStrip ? 0 : 2);
				
				if (not result)
					for (auto& other : others)
						if (const auto isOtherStartWithDoubleStrip {
							std::strlen(other) > 2 and
							other[0] == '-' and other[1] == '-' };
							isEqual(args[i], other, IgnoreCase::Left,
									isOtherStartWithDoubleStrip ? 0 : 2))
						{
							if (isOtherStartWithDoubleStrip)
								args[i] = with;
							else {
								args[i] = "--";
								args[i] += with;
							}
							result = true;
							break;
						}
			} else if (mnemonic != '\0'
					   and args[i].length() == 2
					   and args[i][0] == '-')
			{
				result = args[i][1] == mnemonic;
			}
			if (result) {
				if (args[i].length() == 2) { // convert mnemonic to full
					args[i] = with;
					if (not isWithStartWithDoubleStrip)
						args[i].insert(0, "--");
				}
				if (writeBoolean)
					state[with] = "true";
			}
			return result;
		} }; args[i] == ARGS_SEPARATOR)
				continue;
			else if (isMatch(OPT_UPDATE, '\0', false, {"upgrade"})) {
				#if defined(MAKE_LIB)
				continue;
				#endif
				
				const auto path { fs::path(INSTALL_FULLPATH) };
				#define REPO_URI "https://github.com/Mr-Widiatmoko/MakeTVPlaylist.git"
				#if defined(_WIN32) || defined(_WIN64)
				// TODO: Windows
				std::cout << "📢 Under construction.\n";
				#define CACHE_PATH "%userprofile%\\AppData\\Local"
				if (0 != std::system("git.exe --version")
					or (0 != std::system("cmake.exe --version"))) {
					std::cout << "\"GIT\" and \"CMAKE\" required!.\n";
					continue;
				}
				fs::current_path(fs::path(CACHE_PATH));
				auto dir { fs::path("tvplaylist") };
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
					std::cout << "\"GIT\" or \"CMAKE\" required!.\nTo install:\n\t\"brew install git\"\n\t\"brew install cmake\"\n";
					continue;
				}
				
				fs::current_path(fs::path("~"));
				auto dir { fs::path(".tvplaylist") };
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
				std::cout << (fs::exists(path) ? "Updated" :
							  "For some reason, update fail!") << ".\n";
				return RETURN_VALUE
			}
			else if (isMatch(OPT_INSTALL, '\0')) {
				#if defined(MAKE_LIB)
				continue;
				#endif
					
				const auto path { fs::path(INSTALL_FULLPATH) };
				
				if (not isEqual(argv[0], path)) {
					#if defined(_WIN32) || defined(_WIN64)
					if (fs::exists(path))
						fs::remove(path);
					auto cmd { std::string("copy \"") };
					#else
					auto cmd { std::string("cp -f \"") };
					#endif
					cmd += argv[0];
					cmd += "\" ";
					cmd += path;
					std::system(cmd.c_str());
					std::cout << (fs::exists(path) ? "Installed" :
								  "For some reason, install fail!") << ".\n";
				}
			}
			else if (isMatch(OPT_UNINSTALL, '\0')) {
				#if defined(MAKE_LIB)
				continue;
				#endif
					
				const auto path { fs::path(INSTALL_FULLPATH) };
				if (fs::exists(path))
				#if defined(_WIN32) || defined(_WIN64)
					std::system("del " INSTALL_FULLPATH);
				#else
					std::system("rm -f " INSTALL_FULLPATH);
				#endif
				std::cout << (not fs::exists(path) ? "Uninstalled" :
							  "For some reason, uninstall fail!") << ".\n";
			}
			else if (isMatch(OPT_HELP, 'h') or isMatch(OPT_VERSION, 'v')) // MARK: Option Matching
			{
				printHelp(i + 1 < args.size()
						  ? (args[i + 1][0] == '-' and args[i + 1][1] == '-'
							 ? tolower(args[++i].substr(2)).c_str()
							 : tolower(args[++i]).c_str())
						  : args[i].substr(2).c_str());
				
				if (i + 1 == args.size())
					return RETURN_VALUE
			}
			else if (isMatch(OPT_DEBUG, 		'B', true)) {
				if (i + 1 < args.size() and
					isEqual(args[i + 1].c_str(), OPT_DEBUG_ARGS))
				{
					i++;
					state[OPT_DEBUG] = args[i];
					if (args[i] == "id3")
					{
						if (i + 1 == args.size())
							std::cout << "Usage: --debug=id3 [key[=]value ...] file [file ...]\n";
						std::unordered_map<std::string, std::string> keyVals;
						std::vector<fs::path> files;
						while (++i < args.size()) {
							if (auto path { fs::path(args[i]) };
								fs::exists(path)
								and isEqual(path.extension().string().c_str(),
											".mp3", IgnoreCase::Left))
								files.emplace_back(std::move(path));
							else {
								if (auto pos { args[i].find('=') };
									pos not_eq std::string::npos)
									keyVals.emplace(std::make_pair(
													args[i].substr(0, pos),
													args[i].substr(pos + 1)));
								else {
									keyVals.emplace(std::make_pair(args[i], args[i + 1]));
									i++;
								}
							}
						}
						
						for (auto& file : files) {
							auto mp3{ ID3(file.string().c_str()) };
							for (auto& keyVal : keyVals)
								mp3.set(keyVal.first, keyVal.second);
							
							if (not keyVals.empty())
								mp3.write();
							std::cout << "File " << file.filename() << ":\n"
							<< mp3.string() << '\n';
						}
						
						return RETURN_VALUE
					}
					if (args[i] == "date") {
						if (i + 1 == args.size())
							std::cout << "Usage: --debug=date 'date or/and time' ['format']\n";
						else {
							i++;
							auto input { args[i] };
							
							Date date(input);
							
							std::cout << '\"' << input << '\"' << "  -> \"";
							
							std::cout << date.string(i + 1 < args.size()
													 ? args[++i].c_str()
													 : nullptr);
							std::cout << "\" "
								<< (date.isValid() ? "✅" : "❌") << '\n';
						}
						
						return RETURN_VALUE
					}
				}
			}
			else if (isMatch(OPT_SHOWCONFIG, '\0', true, OPT_SHOWCONFIG_ALTERNATIVE));
			else if (isMatch(OPT_WRITEDEFAULTS, 'W', true, OPT_WRITEDEFAULTS_ALTERNATIVE)) {
				if (i + 1 < args.size()) {
					if (isEqual(args[i + 1], "reset", IgnoreCase::Left)) {
						fs::remove(CONFIG_PATH);
						state[OPT_WRITEDEFAULTS].clear();
						i++;
					}
					else if (isEqual(args[i + 1].c_str(),
									 OPT_WRITEDEFAULTS_ARGS, IgnoreCase::Left)) {
						i++;
						state[OPT_WRITEDEFAULTS] = args[i];
					}
				} else state[OPT_WRITEDEFAULTS] = "new";
			}
			else if (isMatch(OPT_LOADCONFIG, 'L')) {
				if (i + 1 < args.size() and fs::exists(args[i + 1])) {
					i++;
					state[OPT_LOADCONFIG] = args[i];
					loadConfig(&args);
					continue;
				}
				
				std::cout << "⚠️ Expecting config file path. Please see --help "
				<< args[i].substr(2) << '\n';
			}
			else if (isMatch(OPT_ADSDIR, 'D')) {
				if (i + 1 < args.size() and fs::exists(args[i + 1])) {
					i++;
					listAdsDir.emplace_back(fs::path(args[i]));
					continue;
				}
				
				std::cout << "⚠️ Expecting directory path. Please see --help "
				<< args[i].substr(2) << '\n';
			}
			else if (isMatch(OPT_ADSCOUNT, 'C')) {
				if (std::pair<int, int> range;
					i + 3 < args.size()
					and (args[i + 2] == ".." or args[i + 2] == "-")
					and isInt(args[i + 1], &(range.first))
					and isInt(args[i + 3], &(range.second)))
				{
					if (range.first < range.second) {
						state[OPT_ADSCOUNT] = range.first + '-' + range.second;
						i += 3;
						continue;
					}
					std::cout << "⚠️ " << args[i] << " value range is up side down!. "
					<< range.first << " greater than " << range.second << '\n';
				}
				else if (auto push{[&args, &i] (unsigned long pos, unsigned long offset) {
					auto lower { args[i + 1].substr(0, pos) };
					   auto upper { args[i + 1].substr(pos + offset) };
					   
					   int count[2] { 0, 0 };
					   if (isInt(lower, &count[0]) and isInt(upper, &count[1])
						   and count[0] < count[1]) {
						   state[OPT_ADSCOUNT] = lower + '-' + upper;
						   i++;
						   return true;
					   }
					   return false;
				   }};
					i + 1 < args.size())
				{
					if (auto pos { args[i + 1].find('-') };
							pos != std::string::npos
						and pos != 0 and pos != args[i + 1].size() - 1)
					{
						if (push(pos, 1))
							continue;
					}
					else if (pos = args[i + 1].find("..");
							 pos != std::string::npos)
					{
						if (push(pos, 2))
							continue;
					}
					else if (int value{};
						isInt(args[i + 1], &value)) {
						i++;
						state[OPT_ADSCOUNT] = std::to_string(value);
						continue;
					}
				}
				std::cout << "⚠️ Expecting number of advertise. Please see --help "
				<< args[i].substr(2) << '\n';
			}
			else if (isMatch(OPT_NOOUTPUTFILE, 	'F', true)) {
				if (i + 1 < args.size())
				{
					i++;
					if (isEqual(args[i + 1].c_str(), {"true", "yes"},
								IgnoreCase::Left))
						state[OPT_NOOUTPUTFILE] = "true";
					else if (isEqual(args[i + 1].c_str(), {"false", "no"},
									 IgnoreCase::Left))
						state[OPT_NOOUTPUTFILE] = "false";
					else
						i--;
				}
			}
			else if (isMatch(OPT_ARRANGEMENT, 'w')) {
				if (i + 1 < args.size()
					   and (args[i + 1].starts_with(OPT_ARRANGEMENT_DEFAULT)
						 or args[i + 1] == OPT_ARRANGEMENT_UNORDERED
						 or args[i + 1] == OPT_ARRANGEMENT_PERTITLE
						 or args[i + 1] == OPT_ARRANGEMENT_ASCENDING
						 or args[i + 1] == OPT_ARRANGEMENT_SHUFFLE
						 or args[i + 1] == OPT_ARRANGEMENT_SHUFFLE_PERTITLE
						 or args[i + 1].starts_with(OPT_ARRANGEMENT_SHUFFLE_DEFAULT)
						 or args[i + 1] == OPT_ARRANGEMENT_DESCENDING
						 or args[i + 1] == OPT_ARRANGEMENT_DESCENDING_PERTITLE
						 or args[i + 1].starts_with(OPT_ARRANGEMENT_DESCENDING_DEFAULT)))
				{
					i++;
					std::string value = args[i];
					if (args[i].starts_with(OPT_ARRANGEMENT_DEFAULT)
						or args[i].starts_with(OPT_ARRANGEMENT_DEFAULT)
						or args[i].starts_with(OPT_ARRANGEMENT_DESCENDING_DEFAULT))
					{
						auto pos = args[i].find('=');
						value = args[i].substr(0, pos - 1);
						auto count_s { args[i].substr(pos + 1) };
						if (int count; isInt(count_s, &count) and count > 1)
							fileCountPerTurn = count;
					}
					
					state[OPT_ARRANGEMENT] = value;
				} else
					std::cout << "⚠️ Expecting arrangement type. Please see --help "
					<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_CASEINSENSITIVE, 'N', true, OPT_CASEINSENSITIVE_ALTERNATIVE))
			{
				if (i + 1 < args.size() and isEqual(args[i + 1].c_str(),
													TRUE_FALSE,
													IgnoreCase::Left))
					state[OPT_CASEINSENSITIVE] = args[++i];
				
				if (state[OPT_CASEINSENSITIVE] == "true") {
					for (auto& k : listFindDir) k = tolower(k);
					for (auto& k : listExclFindDir) k = tolower(k);
					for (auto& k : listFind) k = tolower(k);
					for (auto& k : listExclFind) k = tolower(k);
				}
			}
			else if (isMatch(OPT_SEARCH, 'q')) {
				if (i + 1 < args.size()) {
					state[OPT_NOOUTPUTFILE] = "true";
					state[OPT_VERBOSE] = "true";
					state[OPT_ARRANGEMENT] = OPT_ARRANGEMENT_ASCENDING;
					state[OPT_CASEINSENSITIVE] = "true";
					
					i++;
					
					auto index{ -1 };
					auto last{ 0 };
					auto push{[&]() {
						auto keyVal { args[i].substr(last, index) };
						if (keyVal != "or" and keyVal != "and") {
							parseKeyValue(&keyVal, keyVal.starts_with("exclude-"));
							
							if (constexpr auto EXCL {"exclude="};
								keyVal.starts_with(EXCL))
							{
								auto value { keyVal.substr(std::strlen(EXCL)) };
								if (value.find('=') != std::string::npos)
									parseKeyValue(&value, true);
								if (not value.empty())
									listExclFind.emplace_back(value);
							} else if (not keyVal.empty())
								listFind.emplace_back(keyVal);
						}
					}};
					while (++index < args[i].size()) {
						if (std::isspace(args[i][index])) {
							if (last != -1 and index - last > 0) {
								push();
								last = -1;
							}
							continue;
						}
						if (last == -1)
							last = index;
					}
					if (last != -1)
						push();
				} else
					std::cout << "⚠️ Expecting search keyword!\n";
			}
			else if (isMatch(OPT_FIND, 			'i')
					 or isMatch(OPT_EXCLFIND, 	'I')) {
				if (i + 1 < args.size())
				{
					const auto opt { args[i].substr(2) };
					const auto isExclude { opt == OPT_EXCLFIND };
					i++;
					#if 0
					parseKeyValue(&args[i], isExclude);
					(isExclude ? listExclFind : listFind)
						.emplace_back(state[OPT_CASEINSENSITIVE] == "true"
									  ? tolower(args[i]) : args[i]);
					state[opt] = "1";
					#else
					auto index{ -1 };
					auto last{ 0 };
					auto push{[&]() {
						auto keyVal { args[i].substr(last, index) };
						
						parseKeyValue(&keyVal, keyVal.starts_with("exclude-"));
						
						if (constexpr auto EXCL {"exclude="};
							keyVal.starts_with(EXCL))
						{
							auto value { keyVal.substr(std::strlen(EXCL)) };
							if (value.find('=') != std::string::npos)
								parseKeyValue(&value, true);
							if (not value.empty()) {
								(isExclude ? listExclFind : listFind)
									.emplace_back(value);
								state[opt] = "1";
							}
						} else if (not keyVal.empty()) {
							(isExclude ? listExclFind : listFind)
								.emplace_back(keyVal);
							state[opt] = "1";
						}
					}};
					while (++index < args[i].size()) {
						if (std::isspace(args[i][index])) {
							if (last != -1 and index - last > 0) {
								push();
								last = -1;
							}
							continue;
						}
						if (last == -1)
							last = index;
					}
					if (last != -1)
						push();
					#endif
				} else
					std::cout << "⚠️ Expecting keyword after \""
					<< args[i] << "\" option. Please see --help "
					<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_REGEXSYNTAX, 	'X')) {
				if (auto found{ false }; i + 1 < args.size()) {
					for (auto& s : OPT_REGEXSYNTAX_ARGS)
						if (isEqual(args[i + 1].c_str(), s, IgnoreCase::Left)) {
							state[OPT_REGEXSYNTAX] = s;
							i++;
							found = true;
							break;
						}
					if (found)
						continue;
				}
				std::cout << "⚠️ Expecting regular expression syntax after \""
							<< args[i] << "\" option. Please see --help "
							<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_REGEX, 		'r')
					 or isMatch(OPT_EXCLREGEX, 	'R')) {
				if (auto found{ false }; i + 1 < args.size())
				{
					if (auto pos { args[i + 1].find('=') };
						pos != std::string::npos)
					{
						if (args[i + 1].substr(0, pos) == "type") {
							auto value { args[i + 1].substr(pos + 1) };
							for (auto& keyword : OPT_REGEXSYNTAX_ARGS)
								if (keyword == value) {
									state[OPT_REGEXSYNTAX] = keyword;
									i++;
									found = true;
									break;
								}
						}
					}
					
					if (not found) {
						auto getRegexSyntaxType{[](const std::string& s)
							-> std::regex_constants::syntax_option_type {
							if (s == "basic") return std::regex_constants::syntax_option_type::basic;
							if (s == "extended") return std::regex_constants::syntax_option_type::extended;
							if (s == "awk") return std::regex_constants::syntax_option_type::awk;
							if (s == "grep") return std::regex_constants::syntax_option_type::grep;
							if (s == "egrep") return std::regex_constants::syntax_option_type::egrep;
							else return std::regex_constants::syntax_option_type::ECMAScript;
						}};
						const auto opt {args[i].substr(2)};
						(opt == OPT_REGEX ? listRegex : listExclRegex)
							.emplace_back(std::regex(args[i + 1],
										getRegexSyntaxType(state[OPT_REGEXSYNTAX])));
						state[opt] = "1";
						i++;
					}
				} else
					std::cout << "⚠️ Expecting regular expression after \""
								<< args[i] << "\" option. Please see --help "
								<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_DATE, 				'z')
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
				const auto opt { args[i].substr(2) };
				auto as_single{[&](const char opGt) -> bool {
					Date date(args[i + 1]);
					if (date.isValid()) {
						if (opt == OPT_DATE) {
							listDCreated.emplace_back(std::make_pair(opGt, date));
							listDChanged.emplace_back(std::make_pair(opGt, date));
							listDModified.emplace_back(std::make_pair(opGt, date));
							listDAccessed.emplace_back(std::make_pair(opGt, date));
							state[OPT_DCREATED]  = "1";
							state[OPT_DMODIFIED] = "1";
							state[OPT_DACCESSED] = "1";
							state[OPT_DCHANGED]  = "1";
						} else if (opt == OPT_EXCLDATE) {
							listDExclCreated.emplace_back(std::make_pair(opGt, date));
							listDExclChanged.emplace_back(std::make_pair(opGt, date));
							listDExclModified.emplace_back(std::make_pair(opGt, date));
							listDExclAccessed.emplace_back(std::make_pair(opGt, date));
							state[OPT_DEXCLCREATED]  = "1";
							state[OPT_DEXCLMODIFIED] = "1";
							state[OPT_DEXCLACCESSED] = "1";
							state[OPT_DEXCLCHANGED]  = "1";
						} else {
						(opt == OPT_DCREATED ? listDCreated
						 : opt == OPT_DCHANGED ? listDChanged
						 : opt == OPT_DMODIFIED ? listDModified
						 : opt == OPT_DACCESSED ? listDAccessed
						 : opt == OPT_DEXCLCREATED ? listDExclCreated
						 : opt == OPT_DEXCLCHANGED ? listDExclChanged
						 : opt == OPT_DEXCLMODIFIED ? listDExclModified
						 : listDExclAccessed
						 ).emplace_back(std::make_pair(opGt, date));

							state[opt] = "1";
						}
						i++;
						return true;
					}
					return false;
				}};
				if (i + 1 < args.size()
					and (args[i + 1][0] == '<'
							or args[i + 1][0] == '>'
							or args[i + 1][0] == '='))
				{
					if (args[i + 1] == "<" or args[i + 1] == ">"
						or args[i + 1] == "=")
					{
						if (i + 2 < args.size())
							i++;
						else
							goto DATE_NEEDED;
					}

					if (as_single(args[i + 1][0]))
						continue;
				}
				else
				{
					auto push{[&](const Date& lower, const Date& upper) {
						if (lower > upper) {
							std::cout << "⚠️ Date range up side down!, "
							<< lower.string() << " greater than " << upper.string() << '\n';
							return false;
						}
						if (opt == OPT_DATE) {
							listDCreatedR.emplace_back(std::make_pair(lower, upper));
							listDChangedR.emplace_back(std::make_pair(lower, upper));
							listDModifiedR.emplace_back(std::make_pair(lower, upper));
							listDAccessedR.emplace_back(std::make_pair(lower, upper));
							state[OPT_DCREATED]  = "1";
							state[OPT_DMODIFIED] = "1";
							state[OPT_DACCESSED] = "1";
							state[OPT_DCHANGED]  = "1";
						} else if (opt == OPT_EXCLDATE) {
							listDExclCreatedR.emplace_back(std::make_pair(lower, upper));
							listDExclChangedR.emplace_back(std::make_pair(lower, upper));
							listDExclModifiedR.emplace_back(std::make_pair(lower, upper));
							listDExclAccessedR.emplace_back(std::make_pair(lower, upper));
							state[OPT_DEXCLCREATED]  = "1";
							state[OPT_DEXCLMODIFIED] = "1";
							state[OPT_DEXCLACCESSED] = "1";
							state[OPT_DEXCLCHANGED]  = "1";
						} else {
						(opt == OPT_DCREATED ? listDCreatedR
						 : opt == OPT_DCHANGED ? listDChangedR
						 : opt == OPT_DMODIFIED ? listDModifiedR
						 : opt == OPT_DACCESSED ? listDAccessedR
						 : opt == OPT_DEXCLCREATED ? listDExclCreatedR
						 : opt == OPT_DEXCLCHANGED ? listDExclChangedR
						 : opt == OPT_DEXCLMODIFIED ? listDExclModifiedR
						 : listDExclAccessedR
						 ).emplace_back(std::make_pair(lower, upper));
						state[opt] = "1";
						}
						return true;
					}};
					if (i + 3 < args.size() and
						(args[i + 2] == "-" or args[i + 2] == "..")) {
						if (Date lower(args[i + 1]); lower.isValid())
							if (Date upper(args[i + 3]); upper.isValid())
							{
								if (push(lower, upper)) {
									i += 3;
									continue;
								}
							}
					} else if (i + 1 < args.size()) {
						auto pos = args[i + 1].find("..");
						auto next { 2 };
						if (pos == std::string::npos) {
							next = 1;
							unsigned strip { 0 };
							pos = 0;
							for (auto m{ 0 }; m < args[i + 1].size(); ++m)
								if (args[i + 1][m] == '-') {
									strip++;
									pos = m;
								}
							if (strip != 1)
								pos = std::string::npos;
						}
							
						if (pos not_eq std::string::npos) {
							if (Date lower(args[i + 1].substr(0, pos));
								lower.isValid())
								if (Date upper(args[i + 1].substr(pos + next));
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
DATE_NEEDED:	std::cout << "⚠️ Expecting date and/or time after \""
							<< args[i] << "\" option. Please see --help "
							<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_OVERWRITE, 	'O', true)) {
				if (i + 1 < args.size() and isEqual(args[i + 1].c_str(),
													TRUE_FALSE, IgnoreCase::Left))
					state[OPT_OVERWRITE] = args[++i];
			}
			else if (isMatch(OPT_BENCHMARK, 	'b', true)) {
				if (i + 1 < args.size() and isEqual(args[i + 1].c_str(),
													TRUE_FALSE, IgnoreCase::Left))
					state[OPT_OVERWRITE] = args[++i];
			}
			else if (isMatch(OPT_SKIPSUBTITLE, 	'x', true)) {
				if (i + 1 < args.size() and isEqual(args[i + 1].c_str(),
													TRUE_FALSE, IgnoreCase::Left))
					state[OPT_OVERWRITE] = args[++i];
			}
			else if (isMatch(OPT_EXCLHIDDEN, 	'n', true)) {
				if (i + 1 < args.size() and isEqual(args[i + 1].c_str(),
													TRUE_FALSE, IgnoreCase::Left))
					state[OPT_OVERWRITE] = args[++i];
			}
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
					if (args[i] == OPT_EXECUTION_ASYNC or args[i] == OPT_EXECUTION_THREAD)
						state[OPT_EXECUTION] = args[i];
					else
						state[OPT_EXECUTION] = "Linear";
				} else
					std::cout << "⚠️ Expecting 'thread', 'async', or 'none' after \""
					<< args[i] << "\" option. Please see --help "
					<< args[i].substr(2) << "\n";
			} else if (isMatch(OPT_EXT, 	'e')
					   or isMatch(OPT_EXCLEXT, 'E')) {
				if (i + 1 < args.size()) {
					i++;
					state[args[i - 1].substr(2)] = args[i] == "*.*" ? "*" : args[i];
				} else
					std::cout << "Expecting extension after \""
					<< args[i] << "\" option (eg: \"mp4, mkv\"). Please see --help "
					<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_FIXFILENAME, 	'f', false, {"fix-filename"})) {
				if (i + 1 < args.size()) {
					i++;
					state[OPT_FIXFILENAME] = args[i];
					if (not fs::path(args[i]).parent_path().string().empty())
					{
						state[OPT_FIXFILENAME] = fs::path(args[i]).filename().string();
						state[OPT_OUTDIR] = fs::path(args[i]).parent_path().string();
					}
				} else
					std::cout << "⚠️ Expecting file name after \""
					<< args[i] << "\" option (eg: \"my_playlist.m3u8\"). Please see --help "
					<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_OUTDIR, 		'd')) {
				if (i + 1 < args.size()) {
					i++;
					state[OPT_OUTDIR] = fs::absolute(args[i]);
					if (auto tmp{args[i]};
						tmp[tmp.size() - 1] not_eq fs::path::preferred_separator)
						state[OPT_OUTDIR] += fs::path::preferred_separator;
				} else
					std::cout << "⚠️ Expecting directory after \""
					<< args[i] << "\" option (eg: \"Downloads/\"). Please see --help "
					<< args[i].substr(2) << "\n";
			}
			else if (isMatch(OPT_SIZE, 			's')
					 or isMatch(OPT_EXCLSIZE, 	'S')) {
				if (bool isExclude{ args[i].substr(2) == OPT_EXCLSIZE };
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
							std::cout << "⚠️ Range is up side down! \""
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
SIZE_NEEDED:		std::cout << "⚠️ Expecting operator '<' or '>' followed\
by size in KB, MB, or GB.\nOr use value in range using form 'from-to' OR 'from..to'\
 Please see --help " << OPT_SIZE << "\n";
		} else if (fs::is_directory(args[i]))
			insertTo(&bufferDirs, fs::path(args[i]));
		else if (fs::is_regular_file(std::move(fs::path(args[i])))) {
			if (auto path{ fs::absolute(args[i]) };
				
				isValid(path) and isValidFile(path))
			{
				std::vector<fs::path> list;
				loadPlaylist(path, &list);
				if (list.empty())
					insertTo(&selectFiles, path);
				else
					for (auto& f : list)
						if (isValid(f) and isValidFile(f))
							insertTo(&selectFiles, std::move(f));
			}
		} else
			invalidArgs.emplace(args[i]);
	}
	// MARK: End option matching
	
	if (not invalidArgs.empty()) {
		std::string_view invalid_args[invalidArgs.size()];
		std::move(invalidArgs.begin(), invalidArgs.end(), invalid_args);
		std::cout << "\n⚠️ What " << (invalidArgs.size() > 1 ? "are these" : "is this") << "? :\n";
		for (auto i{ 0 }; i<invalidArgs.size(); ++i) {
			auto item { &invalid_args[i] };
			std::string others;
			if (item->size() >= 4 and item->starts_with("--")) {
				std::vector<std::pair<float, std::string>> possible;
				auto substr_item = std::move(item->substr(2));
				for (auto& opt : OPTS)
					if (auto percentage{ getLikely(substr_item, *opt) };
						percentage > 80)
						possible.emplace_back(std::make_pair(percentage, *opt));
				
				if (not possible.empty()) {
					std::sort(possible.begin(), possible.end(), [](
						const std::pair<float, std::string>& a,
						const std::pair<float, std::string>& b)
					{
						return a.first > b.first;
					});
					others = " do you mean: ";
					for (auto k{0}; auto&& s : possible)
						others.append("--" + s.second + (++k == possible.size() ? "." : " or "));
				}
			}
				
			std::cout << '"' << *item << '"' << others << '\n';
		}
		std::cout << "\nFor more information, please try to type \""
			<< fs::path(argv[0]).filename().string() << " --help ['keyword']\"\n\n";
	}
	
	if (bufferDirs.empty() and selectFiles.empty())
		insertTo(&bufferDirs, fs::current_path());
	
	const auto inputDirsCount = bufferDirs.size();
	fs::path inputDirs[inputDirsCount + 1];
	std::copy(bufferDirs.begin(), bufferDirs.end(), inputDirs);

	while (bufferDirs.size() == 1) {
		state[OPT_OUTDIR] = fs::absolute(bufferDirs[0]).string();
		insertTo(&regularDirs, bufferDirs[0]);/// Assume single input dir is regularDir
		if (isContainsSeasonDirs(state[OPT_OUTDIR])) {
			break;
		}
		bufferDirs.clear();
		std::vector<fs::directory_entry> sortedDirs;
		listDir(fs::path(state[OPT_OUTDIR]), &sortedDirs);
				
		for (auto& child : sortedDirs)
			insertTo(&bufferDirs, child.path());
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
	if (not state[OPT_SHOWCONFIG].empty() or
		not invalidArgs.empty() or state[OPT_VERBOSE] == "all"
		or state[OPT_VERBOSE] == "info" or state[OPT_BENCHMARK] == "true"
		or state[OPT_DEBUG] == "true" or state[OPT_DEBUG] == "args")
	#endif
	{ // MARK: Options Summary
		constexpr auto WIDTH{ 20 };
		#ifndef DEBUG
		if (state[OPT_DEBUG] == "true" or state[OPT_DEBUG] == "args")
		#endif
		{
			std::cout << "Original Arguments: ";
			for (auto i{1}; i<argc; ++i)
				std::cout << '"' << argv[i] << '"' << (i+1>=argc ? "" : ", ");
			std::cout << '\n';
			std::cout << "Parsed Arguments  : ";
			for (auto i{0}; i<args.size(); ++i) {
				if (args[i] == ARGS_SEPARATOR)
					continue;
				std::cout << '"' << args[i] << '"' << (i+1>=args.size() ? "" : ", ");
			}
			std::cout << '\n';
		}
	#define PRINT_OPT(x)	(x.empty() ? "false" : x)
	#define LABEL(x)	x << std::setw(unsigned(WIDTH - std::strlen(x))) << ": "
	std::cout
		<< LABEL(OPT_ARRANGEMENT) << state[OPT_ARRANGEMENT] << '\n'
		<< LABEL(OPT_EXECUTION) << state[OPT_EXECUTION] << '\n'
		<< LABEL(OPT_VERBOSE) << PRINT_OPT(state[OPT_VERBOSE]) << '\n'
		<< LABEL(OPT_BENCHMARK) << PRINT_OPT(state[OPT_BENCHMARK]) << '\n'
		<< LABEL(OPT_OVERWRITE) << PRINT_OPT(state[OPT_OVERWRITE]) << '\n'
		<< LABEL(OPT_NOOUTPUTFILE) << PRINT_OPT(state[OPT_NOOUTPUTFILE]) << '\n'
		<< LABEL(OPT_FIXFILENAME) << state[OPT_FIXFILENAME] << '\n'
		<< LABEL("Current Directory") << fs::current_path().string() << '\n'
		<< LABEL(OPT_OUTDIR) << state[OPT_OUTDIR] << '\n'
		<< LABEL(OPT_EXCLHIDDEN) << PRINT_OPT(state[OPT_EXCLHIDDEN]) << '\n'
		<< LABEL(OPT_SKIPSUBTITLE) << PRINT_OPT(state[OPT_SKIPSUBTITLE]) << '\n';
	{
		std::cout << LABEL(OPT_EXT) << state[OPT_EXT];
		if (auto i{0};
			state[OPT_EXT].empty())
			for (auto& ext : DEFAULT_EXT)
				std::cout << ext << (++i < DEFAULT_EXT.size() - 1 ? ", " : "");
		std::cout << '\n';
	}
	std::cout << LABEL("case-insensitive") << PRINT_OPT(state[OPT_CASEINSENSITIVE]) << '\n';
	
	#undef PRINT_OPT
			
	for (auto& S : {OPT_FIND, OPT_EXCLFIND}) {
		std::cout << LABEL(S);
		for (auto i{0}; i<(S == OPT_FIND ? listFind : listExclFind).size(); ++i)
			std::cout << (S == OPT_FIND ? listFind : listExclFind)[i]
			<< (i < (S == OPT_FIND ? listFind : listExclFind).size() - 1 ? ", " : "");
		std::cout << '\n';
	}

	for (auto& S : {OPT_REGEX, OPT_EXCLREGEX}) {
		std::cout << LABEL(S);
		for (auto i{0}; i<(S == OPT_REGEX ? listRegex : listExclRegex).size(); ++i)
			std::cout << (S == OPT_REGEX ? listRegex : listExclRegex)[i].mark_count()
			<< " expression"
			<< (i < (S == OPT_REGEX ? listRegex : listExclRegex).size() - 1 ? ", " : "");
		std::cout << '\n';
	}

	for (auto& S : {OPT_SIZE, OPT_EXCLSIZE}) {
		std::cout << LABEL(S);
		if (state[OPT_SIZEOPGT][0] != '\0' or state[OPT_EXCLSIZEOPGT][0] != '\0')
			std::cout << ((S == OPT_SIZE ? listSize : listExclSize).empty()
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
		const char * const* st[8] = { &OPT_DCREATED, &OPT_DCHANGED, &OPT_DACCESSED, &OPT_DMODIFIED,
			&OPT_DEXCLCREATED, &OPT_DEXCLCHANGED, &OPT_DEXCLACCESSED, &OPT_DEXCLMODIFIED };
		
		std::vector<std::pair<char, Date>>* ot[8] = { &listDCreated, &listDAccessed, &listDModified, &listDChanged,
			  &listDExclCreated, &listDExclAccessed, &listDExclModified, &listDExclChanged };

		
		std::vector<std::pair<Date, Date>>* rt[8] = { &listDCreatedR, &listDAccessedR, &listDModifiedR, &listDChangedR,
			  &listDExclCreatedR, &listDExclAccessedR, &listDExclModifiedR, &listDExclChangedR };
		
		for (auto i {0}; i<8; ++i) {
			std::cout << LABEL(*st[i]);
			for (auto k{0}; k<ot[i]->size(); ++k) {
				std::cout << '\"' << ot[i]->at(k).first << ' ' << ot[i]->at(k).second.string() << "\", ";
			}
			
			for (auto k{0}; k<rt[i]->size(); ++k) {
				std::cout << '\"' << rt[i]->at(k).first.string() << "\" .. \"" << rt[i]->at(k).second.string() << "\", ";
			}
			std::cout << '\n';
		}
	}
	
	std::cout << LABEL("Inputs");
		for (auto i{0}; i<inputDirsCount + selectFiles.size(); ++i) {
			if (i < inputDirsCount) {
				std::cout << inputDirs[i];
			} else {
				std::cout << selectFiles[i - inputDirsCount];
			}
			
			std::cout  << (i < (inputDirsCount + selectFiles.size()) - 1 ? ", " : "");
		}
	std::cout << "\n";
	
	std::cout << LABEL(OPT_ADSCOUNT) << state[OPT_ADSCOUNT] << '\n';
	std::cout << LABEL(OPT_ADSDIR);
	auto i { -1 };
	for (i++; auto& d : listAdsDir)
		std::cout << d.string() << (i < listAdsDir.size() - 1 ? ", " : "\n");
			
	std::cout << "\n";
	#undef LABEL
	} // END Info
					   
	if (not invalidArgs.empty())
		return RETURN_VALUE
					   
	if (const auto mode{ state[OPT_WRITEDEFAULTS] };
		not mode.empty())
		writeConfig(&args,
					  mode == "edit" 	? WriteConfigMode::Edit
					: mode == "add" 	? WriteConfigMode::Add
					: mode == "remove" 	? WriteConfigMode::Remove
					: WriteConfigMode::New);

	if (not state[OPT_SHOWCONFIG].empty()) {
		auto displayFile{[](const char* const path) {
			if (not fs::exists(fs::path(path)))
				return;
			
			std::cout << "\nContent of file \"" << path << "\":\n";
			std::ifstream file(path);
			std::cout << file.rdbuf() << '\n';
			file.close();
		}};
			
		displayFile(CONFIG_PATH);
		auto otherFile { state[OPT_LOADCONFIG].c_str() };
		if (not isEqual(CONFIG_PATH, otherFile))
			displayFile(otherFile);
	}
					   
    if (	not state[OPT_SHOWCONFIG].empty()
		or 	not state[OPT_WRITEDEFAULTS].empty())
        return RETURN_VALUE

	{///Clean up <key=val> dir=??? in listFind and listExclFind
		auto cleanUp{[](std::vector<std::string>* list) -> void {
			std::vector<std::string> tmp;
			for (auto i{0}; i<list->size(); ++i)
				if (list->at(i)[0] != char(1))
					tmp.emplace_back(std::move(list->at(i)));
			*list = std::move(tmp);
		}};
		cleanUp(&listFind);
		cleanUp(&listExclFind);
	}
					   
	if (state[OPT_OUTDIR].empty()) {
		if (selectFiles.empty())
			state[OPT_OUTDIR] = fs::current_path().string();
		else
			state[OPT_OUTDIR] = fs::path(selectFiles[0]).parent_path().string();
	}
	
	auto start{std::chrono::system_clock::now()};

	std::vector<std::thread> threads;
	std::vector<std::future<void>> asyncs;
					   
	{
		std::vector<fs::path> list = std::move(listAdsDir);
		
		for (auto& child : list)
			if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD)
				threads.emplace_back([&, child]() {
					listDirRecursively(child, &listAdsDir, false); });
			else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC)
				asyncs.emplace_back(std::async(std::launch::async, [&, child]() {
					listDirRecursively(child, &listAdsDir, false); }));
			else
				listDirRecursively(child, &listAdsDir, false);
	}
	
	for (bool isByPass {state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_ASCENDING};
		 auto& child : bufferDirs)
		if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD) {
			if (isByPass)
				threads.emplace_back([&, child]() {
					listDirRecursively(child, &regularDirs, false);
				});
			else
				threads.emplace_back(checkForSeasonDir, child);
		}
		else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC)
			if (isByPass)
				asyncs.emplace_back(std::async(std::launch::async, [&, child]() {
					listDirRecursively(child, &regularDirs, false);
				}));
			else
				asyncs.emplace_back(std::async(std::launch::async,
							checkForSeasonDir, child));
		else
		{
			if (isByPass)
				listDirRecursively(child, &regularDirs, false);
			else
				checkForSeasonDir(child);
		}

	if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD) {
		for (auto& t : threads)
			t.join();
		threads.clear();
	} else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC) {
		for (auto& a : asyncs)
			a.wait();
		asyncs.clear();
	}

	
	auto maxDirSize{std::max(regularDirs.size(), seasonDirs.size())};

	const auto BY_PASS {
			state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_PERTITLE
			or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE_PERTITLE
			or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_ASCENDING
			or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE
			or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING
			or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING_PERTITLE
		};
					   
	if (	state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DEFAULT
		or 	state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE_DEFAULT
		or 	state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING_DEFAULT) {
		std::sort(regularDirs.begin(), regularDirs.end());
		std::sort(seasonDirs.begin(), seasonDirs.end());
			
		sortFiles(&selectFiles);
	}

	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true" or state[OPT_DEBUG] == "true")
	#endif
	{
		if (inputDirsCount > 0)
			timeLapse(start, groupNumber(
						std::to_string(regularDirs.size() + seasonDirs.size()))
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
			if ((select == 1 and i >= regularDirs.size())
				or (select == 2 and i >= seasonDirs.size()))
				continue;
			std::cout
				<< (i == 0 and select == 1 ? "BEGIN valid dirs-----\n" : "")
				<< (select == 1 ? 'R' : 'S') << ':'
				<< (select == 1 ? regularDirs[i] : seasonDirs[i])
				<< (i + 1 == maxDirSize ? "\nEND valid dirs-----\n\n" : "\n");
		}
	
	
	std::unordered_map<std::string, std::shared_ptr<std::vector<fs::path>>> records;
	
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
	std::string outExt;
	#ifndef DEBUG
	auto isVerbose { 	state[OPT_VERBOSE] == "all"
					or 	state[OPT_VERBOSE] == "true"
					or 	state[OPT_DEBUG] == "true" };
	#endif
	auto dontWrite { state[OPT_NOOUTPUTFILE] == "true" };
					   
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 mersenneTwisterEngine(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distrib;
	std::uniform_int_distribution<> distribCount;
	unsigned long adsCount[2] {0, 0};
					   
	if (state[OPT_NOOUTPUTFILE] != "true") {
		outExt = tolower(outputName.extension().string());
		
		if (outExt == ".pls") {
			outputFile << "[playlist]\n";
		}
		else if (outExt == ".m3u") {
			outputFile << "#EXTM3U\n";
		}
		else if (outExt == ".xspf") {
			outputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
			"<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n"\
			"\t<trackList>\n";
		}
		else if (outExt == ".xml") {
			outputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
			"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "\
			"\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"\
			"<plist version=\"1.0\">\n"\
			"<dict>\n"\
			"\t<key>Major Version</key><integer>1</integer>\n"\
			"\t<key>Minor Version</key><integer>1</integer>\n"\
			"\t<key>Date</key><date>" << Date::now().string("%FT%T") << "</date>\n"\
			"\t<key>Application Version</key><string>1.1</string>\n"\
			"\t<key>Tracks</key>\n"\
			"\t<dict>"
			;
		}
		else if (outExt == ".wpl")
		{
			outputFile << "<?wpl version=\"1.0\"?>\n"\
				"<smil>\n"\
				"\t<head>\n"\
				"\t\t<meta name=\"Generator\" content=\"tvplaylist -- 1.1\"/>\n"\
				"\t\t<title>" << outputName.filename().string() << "</title>\n"\
				"\t</head>\n"\
				"\t<body>\n"\
				"\t\t<seq>\n";
		}
		else if (outExt == ".smil") {
			outputFile << "<?wpl version=\"1.0\"?>\n"\
				"<smil>\n"\
				"\t<body>\n"\
				"\t\t<seq>\n";
		}
		else if (outExt == ".b4s") {
			outputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"\
			"<WindampXML>\n"\
			"\t<playlist>\n";
		}
		else if (isEqual(outExt.c_str(), {".asx", ".wax", ".wvx"})) {
			outputFile << "<asx version=\"3.0\"?>\n"\
				"\t\t<title>" << outputName.filename().string() << "</title>\n";
		}
	}
	
	auto putIntoPlaylist{ [&](const fs::path& file) {
		auto putIt{ [&](const fs::path& file, const char* title = nullptr) {
			playlistCount++;
			if (dontWrite) {
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
			}
			
			{
				auto fullPath { file.string() };
				auto needAboslute { true };
				for (const char* const protocol : NETWORK_PROTOCOLS)
					if (fullPath.starts_with(protocol)) {
						replace_all(fullPath, " ", "%20");
						replace_all(fullPath, "=", "%3D");
						replace_all(fullPath, "+", "%2B");
						replace_all(fullPath, "-", "%2D");
						replace_all(fullPath, "?", "%3F");
						replace_all(fullPath, ";", "%3B");
						replace_all(fullPath, "%", "%25");
						needAboslute = false;
						break;
					}
				
				if (needAboslute)
					fullPath = fs::absolute(file).string();
				
				if (not dontWrite) {
					std::string prefix;
					std::string suffix;
					std::string name;
					if (isEqual(outExt.c_str(), {".wpl", ".b4s", ".smil",
												 ".asx", ".wax", ".wvx"}))
					{
						for (auto w { 0 }; w<sizeof(XML_CHARS_ALIAS) /sizeof(XML_CHARS_ALIAS[0]); ++w)
							if (isContains(fullPath, XML_CHARS_NORMAL[w],
									IgnoreCase::Left) not_eq std::string::npos)
							{
								replace_all(fullPath, XML_CHARS_NORMAL[w], XML_CHARS_ALIAS[w]);
								break;
							}
					
						if (not isEqual(outExt.c_str(), {".wpl", ".smil"}))
						{
							fs::path tmp = fs::path(fullPath).filename();
							name = tmp.string().substr(0,
								tmp.string().size()
								- tmp.extension().string().size());
						}
					}
					
					if (outExt == ".pls") {
						auto indexString{ std::to_string(playlistCount) };
						prefix = "File" + indexString + '=';
						suffix = "\nTitle" + indexString + '=';
						suffix += title;
					}
					else if (outExt == ".xspf") {
						prefix = "\t\t<track>\n\t\t\t<title>";
						prefix += title;
						prefix += "</title>\n"\
									"\t\t\t<location>file://";
						suffix = "</location>\n\t\t</track>";
						
					}
					else if (outExt == ".wpl") {
						prefix = "\t\t\t<media src=\"";
						suffix = "\"/>";
					}
					else if (outExt == ".b4s") {
						prefix = "\t\t<entry Playstring=\"file:";
						suffix = "\">\n\t\t\t<Name>"
									+ name
									+ "</Name>\n\t\t</entry>";
					}
					else if (outExt == ".smil") {
						prefix = "\t\t\t<audio src=\"";
						suffix = "\"/>";
					}
					else if (isEqual(outExt.c_str(), {".asx", ".wax", ".wvx"})) {
						prefix = "\t<entry>\n\t\t<title>"
									+ name
									+ "</title>\t\t<ref href=\"";
						suffix = "\"/>\n\t</entry>";
					}
					else if (outExt == ".xml") {
						auto key = std::to_string(1000 - playlistCount);
						prefix = "\t\t<key>" + key + "</key>\n"\
								"\t\t<dict>\n"\
								"\t\t\t<key>Track ID</key><integer>" + key + "</integer>\n"\
								"\t\t\t<key>Name</key><string>" + name + "</string>\n"\
								"\t\t\t<key>Location</key><string>file://";
						suffix = "</string>\n\t\t</dict>";
					}
					
					outputFile 	<< prefix << fullPath << suffix << '\n';
				}
				
				#ifndef DEBUG
				if (isVerbose)
				#endif
					std::cout << fullPath << '\n';

			}
		}};
		#if defined(_WIN32) || defined(_WIN64)
		#define OS_NAME	"Windows"
		#elif defined(__APPLE__)
		#define OS_NAME	"macOS"
		#else
		#define OS_NAME	"Linux"
		#endif
		
		putIt(file, OS_NAME\
					" Path");
			

		if (state[OPT_SKIPSUBTITLE] != "true") {
			std::vector<fs::path> subtitleFiles;
			findSubtitleFile(file, &subtitleFiles);
			for (auto& sf : subtitleFiles)
				putIt(std::move(sf), 	"Subtitle Path on "\
										OS_NAME);
		}
			
		if (not listAdsDir.empty())
			for (auto i{0}; i<(adsCount[1] == 0
							   ? adsCount[0]
							   : distribCount(mersenneTwisterEngine));
				++i, ++playlistCount)
				putIt(fs::absolute(
							listAdsDir[distrib(mersenneTwisterEngine)]).string(),
							"Ads path on "\
							OS_NAME);
		#undef OS_NAME

	}};
	
	auto filterChildFiles{ [&records](const fs::path& dir, bool recurive=false) {
		std::vector<fs::path> bufferFiles;
		
		auto putToRecord{[&bufferFiles, &dir, &records](bool wantToSort) {
			if (bufferFiles.empty())
				return;
			
			if (wantToSort and bufferFiles.size() > 1)
				std::sort(bufferFiles.begin(), bufferFiles.end(), ascending);
			
			records.emplace(std::make_pair(dir.string(),
										   std::make_shared<std::vector<fs::path>>(std::move(bufferFiles))
										   ));
		}};
		
		if (recurive) {
			std::vector<fs::path> dirs;
			listDirRecursively(dir, &dirs, false);
			std::sort(dirs.begin(), dirs.end());
			
			for (auto& d : dirs) {
				std::vector<fs::path> tmp;
				
				for (auto& f : directory_iterator(d, DT_REG))
					if (isValidFile(f.path())) {
						std::vector<fs::path> list;
						loadPlaylist(f.path().string(), &list);
						if (list.empty())
							tmp.emplace_back(std::move(f));
						else
							for (auto& p : list)
								if (fs::is_regular_file(p) and isValid(p) and isValidFile(p))
									tmp.emplace_back(std::move(p));
					}
				
				sortFiles(&tmp);
				
				for (auto& f : tmp)
					bufferFiles.emplace_back(std::move(f));
			}
			
			putToRecord(false);
		}
		
		for (auto& f : directory_iterator(dir, DT_REG))
			if (isValidFile(f.path())) {
				std::vector<fs::path> list;
				loadPlaylist(f.path().string(), &list);
				if (list.empty())
					bufferFiles.emplace_back(std::move(f));
				else
					for (auto& p : list)
						if (fs::is_regular_file(p) and isValid(p) and isValidFile(p))
							bufferFiles.emplace_back(std::move(p));
			}
		
		putToRecord(true);
	}};
					   
					   
	start = std::chrono::system_clock::now();
					   
	for (auto i{0}; i<std::max(maxDirSize, listAdsDir.size()); ++i)
		for (auto& x : {0, 1, 2})
			if (i < (x == 1 ? regularDirs.size() : (x == 2 ? seasonDirs.size()
													: listAdsDir.size())) )
				if (auto dir { x == 1 ? regularDirs[i] : (x == 2 ? seasonDirs[i]
														  : listAdsDir[i]) };
					not dir.empty() and isDirNameValid(dir)) {
					if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD)
						threads.emplace_back([&, dir]() {
							filterChildFiles(dir, x == 2);
						});
					else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC)
						asyncs.emplace_back(std::async(std::launch::async, [&, dir]() {
							filterChildFiles(dir, x == 2);
						}));
					else
						filterChildFiles(dir, x == 2);
				}
					   
	if (state[OPT_EXECUTION] == OPT_EXECUTION_THREAD)
		for (auto& t : threads)
			t.join();
	else if (state[OPT_EXECUTION] == OPT_EXECUTION_ASYNC)
		for (auto& a : asyncs)
			a.wait();

	if (not listAdsDir.empty()) {
		std::vector<fs::path> list = std::move(listAdsDir);
		for (auto& dir : list)
			if (not dir.empty())
				if (const auto found { records[dir] }; found)
					std::move(found->begin(), found->end(),
							  std::back_inserter(listAdsDir));
		distrib = std::uniform_int_distribution<>(0, int(listAdsDir.size() - 1));
			
		if (auto count_s { state[OPT_ADSCOUNT] };
			count_s.empty()) {
			adsCount[0] = regularDirs.size() + seasonDirs.size();
			adsCount[0] = listAdsDir.size() % adsCount[0] == 0
						? std::min(decltype(listAdsDir.size())(3), listAdsDir.size())
						: listAdsDir.size() % adsCount[0];
		} else {
			if (auto pos { count_s.find('-') };
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
					   
	if (BY_PASS)
	{
		for (auto& d : seasonDirs)
			regularDirs.emplace_back(std::move(d));
		seasonDirs.clear();
		
		maxDirSize = regularDirs.size();
		
		std::sort(regularDirs.begin(), regularDirs.end());
			
		playlistCount += selectFiles.size();
			
		for (auto& dir : regularDirs) {
			if (dir.empty())
				continue;
			if (const auto found { records[dir] }; found) {
				if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_PERTITLE)
					std::sort(found->begin(), found->end(), ascending);
				else if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING_PERTITLE)
					std::sort(found->begin(), found->end(), descending);
					
				for (auto& f : *found)
				{
					playlistCount++;
					if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_ASCENDING
						or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING
						or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE)
						selectFiles.emplace_back(std::move(f));
					else
						putIntoPlaylist(std::move(f));
				}
			}
		}
	
		if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_PERTITLE)
			sortFiles(&selectFiles);
			
		else if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE)
			std::shuffle(selectFiles.begin(), selectFiles.end(), mersenneTwisterEngine);
			
		else if (	state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING
				 or state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING_PERTITLE)
			std::sort(selectFiles.begin(), selectFiles.end(), descending);
			
		else
			std::sort(selectFiles.begin(), selectFiles.end(), ascending);
			
		for (auto& f : selectFiles)
			putIntoPlaylist(std::move(f));
	}
	else {
		std::vector<fs::path> bufferSort;
		while (true) {
			auto finish{true};
			bufferSort.clear();
			for (auto i{0}; i < maxDirSize; ++i)
				for (auto& indexPass : {1, 2})
					///pass 1 for regularDirs, pass 2 for seasonDirs
				{
					if ((indexPass == 1 and i >= regularDirs.size())
						or (indexPass == 2 and i >= seasonDirs.size()))
						continue;
					
					if (const auto dir{indexPass == 1 ? regularDirs[i] : seasonDirs[i]};
						dir.empty())
						continue;

					else if (auto found { records[dir] }; found) {
						if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE_PERTITLE)
						{
							std::shuffle(found->begin(), found->end(), mersenneTwisterEngine);
							for (auto& f : *found)
								putIntoPlaylist(std::move(f));
							continue;
						}
						
						if (indexFile ==0) {
							if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE_PERTITLE)
								std::shuffle(found->begin(), found->end(), mersenneTwisterEngine);
							
							else if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_DESCENDING_DEFAULT)
								std::sort(found->begin(), found->end(), descending);
						}
							
						for (auto c{ 0 }; c < fileCountPerTurn; ++c)
							if (indexFile + c < found->size()) {
								finish = false;

								bufferSort.emplace_back((*found)[indexFile + c]);
							}
					}
				} //end pass loop
			
			if (state[OPT_ARRANGEMENT] == OPT_ARRANGEMENT_SHUFFLE_PERTITLE)
				break;
			
			if (indexFile < selectFiles.size())
				bufferSort.emplace_back(std::move(selectFiles[indexFile]));
			
			indexFile += fileCountPerTurn;
			//if (bufferSort.size() > 1) std::sort(bufferSort.begin(), bufferSort.end());
			for (auto& ok : bufferSort)
				putIntoPlaylist(std::move(ok));
			
			if (finish and indexFile >= selectFiles.size())
				break;
		}
	}
	
	#ifndef DEBUG
	if (state[OPT_BENCHMARK] == "true" or state[OPT_DEBUG] == "true")
	#endif
		timeLapse(start, groupNumber(std::to_string(playlistCount)) + " valid files took ");
					   
	if (state[OPT_NOOUTPUTFILE] != "true") {
		if (outExt == ".pls") {
			outputFile << "\nNumberOfEntries=" << playlistCount
				<< "\nVersion=2\n";
		}
		else if (outExt == ".b4s") {
			outputFile << "\t</playlist>\n</WinampXML>\n";
		}
		else if (outExt == ".xspf") {
			outputFile << "\t</trackList>\n</playlist>\n";
		}
		else if (isEqual(outExt.c_str(), {".wpl", ".smil"})) {
			outputFile << "\t\t</seq>\n\t</body>\n</smil>\n";
		}
		else if (isEqual(outExt.c_str(), {".asx", ".wax", ".wvx"})) {
			outputFile << "</asx>\n";
		}
		else if (outExt == ".xml") {
			outputFile << "\t</dict>\n</dict>\n</plist>";
		}
			
		outputFile.flags();
		if (outputFile.is_open())
			outputFile.close();
			
		if (playlistCount == 0)
			fs::remove(outputName);
		else
			std::cout << fs::absolute(outputName).string() << '\n';
	}
}
#undef CONFIG_PATH
#undef INSTALL_PATH
#undef SEP_PATH
#undef PE_EXT
#undef INSTALL_FULLPATH
