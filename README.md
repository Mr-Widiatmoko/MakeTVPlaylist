#  Make TV Playlist
As the name suggests, this is a small utility to create playlist from various serial TV Title that ordered one episode per Title (by default). To be clear see the screen shot, and learn from it.

![Select root folder](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/README/ScreenShot1.png "Screen shot of TVPlaylist selecting the root of videos folder")
![Select from multiple folders and files](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/README/ScreenShot2.png "Screen shot of TVPlaylist selecting from various folders and files")

## Overview:
This is a utility to create playlist file with extension .m3u8 (by default) that contains sequence per episode per Title (by default) of multiple Title TV program or Folder. To do that, you have two option:
1. By selecting folders or files, then Right-Click choose "Quick Actions" -> "Make TV Playlist". 
	
	Then "mpv" (by default, if you have one) or your default video player will be automatically used to play the playlist file.
	
	This workflow implemented using zsh script, so the performace to create playlist is very, very, very ... slugish. To upgrade performace for more than 1000% and have bunch of bells and whistles you need to download [tvplaylist](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/Release/tvplaylist)(for macOS Intel!) or For others platform, you need to compile by your self, see Installation).
		
2. By using Terminal.app and type "tvplaylist" press SPACE then drag folders or files from Finder to Terminal, then press ENTER. the program will display full path of playlist file.

![tvplaylist on Terminal](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/README/ScreenShot3.png "Screen shot of tvplayliston Terminal.app")

For Apple Silicon, Windows, Linux, and POSIX users, you just need to compile [main.cpp](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/src/main.cpp) using modern c++ compiler or by using [CMakeLists.txt](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/CMakeLists.txt), please see Installation. 

## Contents:
- [Release](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/Release) folder contains two release binary files, that is [Make TV Playlist.workflow](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/Release/Make%20TV%20Playlist.workflow.zip) and [tvplaylist](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/Release/tvplaylist)(Only for macOS Intel!, others platform need to compile, see Installation).
- [src](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/src) folder contains [tvplaylist]() source code in C++20 dialect.
- [CMakeLists.txt](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/CMakeLists.txt) file to build "tvplaylist" using cmake.  
- [README](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/README) folder contains files needed by this [README.md](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/README.md)
- [README.md](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/README.md) file displaying What you looking right now.
- [LICENSE.txt](https://github.com/Mr-Widiatmoko/MakeTVPlaylist/LICENSE.txt) license file for this software.

## Installation:
At least you need to download [Make TV Playlist.workflow](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/Release/Make%20TV%20Playlist.workflow.zip) (Only for macOS), then Double-Click it on Finder to install as Finder Quick Action Extension, that's it, your done!.

But, if you need more features and blasting super speed for processing, you may want to download  [tvplaylist](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/Release/tvplaylist) and put this file into: 
- Inside "/usr/local/bin" folder (PREFERED) or
- Home folder (eg: /Users/[YOU]) or 
- Download folder (eg: /Users/[YOU]/Downloads).

Thats it, and "Make TV Playlist" will automatically use "tvplaylist" for processing.

To build "tvplaylist" on macOS Apple Silicon, Windows, Linux, or another platforms, first you must have modern C++ compiler (and/or cmake) installed. 

To build using cmake, inside "MakeTVPlaylist" folder (or folder containing CMakeLists.txt) copy below and paste on the Terminal:

	mkdir Release
	cd Release
	cmake ../
	make
	make install
	make clean
	
Don't forget to [Press ENTER]. Thats it, you will get "tvplaylist" on "/usr/local/bin" directory.

If you don't have cmake installed then, from 'src' directory on Terminal, paste following:

    c++ main.cpp -std=c++2b -o /usr/local/bin/tvplaylist

Don't forget to [Press ENTER]. Thats it, you will get "tvplaylist" on "/usr/local/bin/" directory.

If you need to build this as library file, then copy below and paste on the Terminal:

	mkdir WHATEVER
	cd WHATEVER
	cmake ../ -DMAKE_LIB=1
	make
	make install
	make clean
 
Don't forget to [Press ENTER]. Thats it, you will get "libtvplaylist.dylib" on "/usr/local/lib" directory and "libtvplaylist.h" on "/usr/local/include" directory. 

## Usage fo Dummies:
1. For [Make TV Playlist.workflow](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/Release/Make%20TV%20Playlist.workflow.zip):
	
	- Right-Click on folders or files, 
	- Select "Quick Actions"
	- Select "Make TV Playlist"
	
2. For [tvplaylist](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/Release/tvplaylist):

	- On Terminal type "tvplaylist"
	- Press SPACEBAR
	- Drag folders or files from Finder to Terminal
	- Press ENTER

## Known issues:
- none

## DoNation:
![Brave Wallet](https://raw.githubusercontent.com/Mr-Widiatmoko/MakeTVPlaylist/master/README/BW.png "0x7023122b749ac0AD65fc60E64525aC3de36D7677")
	
	0x7023122b749ac0AD65fc60E64525aC3de36D7677
