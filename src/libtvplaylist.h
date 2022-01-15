//
//  libtvplaylist.h
//  Make TV Playlist
//
//  Created by Mr-Widiatmoko on 1/4/22.
//
//  This file is part of Make TV Playlist. It is subject to the license terms
//  in the LICENSE file found in the top-level directory of this
//  distribution and at https://github.com/Mr-Widiatmoko/MakeTVPlaylist/LICENSE.txt
//
#if _MSC_VER && !__INTEL_COMPILER
#pragma once
#define EXPORT_FUNC	__declspec(dllexport)
#else
#define EXPORT_FUNC
#endif

#ifndef LIBTVPLAYLIST_H_INCLUDED
#define LIBTVPLAYLIST_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_FUNC void process(int argc, char *argv[], /// Inputs
			 int *outc, char *outs[], unsigned long *maxLength);/// Outputs, you need to free 'outs' by your self.

/** Example Usage on C:
	
		#include <libtvplaylist.h>
 
		int main(int argc, char *argv[]) {
			{ /// BEGIN Just create playlist filename.
 
				/// This example will try to create playlist file under '/tmp' folder with file name 'my_playlist.m3u8'.
 
				char *inputs[] = {
					/// By default, --no-output-file=true for 'libtvplaylist', but false for 'tvplaylist'
					"--no-output-file=false", 			// or "-P=false"
					"--out-dir=/tmp", 					// or "-d=/tmp"
					"--fix-filename=my_playlist.m3u8",	// or "-f=my_playlist.m3u8"
					"/Users/dev/Movies",				 // Specify one input folder. If no input, then current directory will be used automatically.
					/// Another inputs, can be: files, folders or options
				};
 
				/// Output playlist file will be created on '/tmp/my_playlist.m3u8'.
				process(sizeof(inputs)/sizeof(inputs[0]), inputs, NULL, NULL, NULL);
 
			} /// END Get playlist filename.
 
			{ /// BEGIN Get filenames as array string.

				/// This example will try get all full path file names from playlist.

				int outc = 0;
				unsigned long maxLength = 0;
				process(argc, argv, &outc, NULL, &maxLength); /// Try to get numbers of filenames in 'outc' and maximum filename length.
				
				char *outs[outc]; /// Stack allocation for outs.
	 
				/// Allocate heap buffer with maximum 'maxLength' characters for full path filename.
				for (int i=0; i<outc; ++i) outs[i] = (char *)malloc(maxLength * sizeof(char));
	 
				process(argc, argv, NULL, outs, NULL); /// Copy full path filenames into outs elements.
				/// Yes this process runs twice and consume double times, if you seek for performance, please see first example above.
	 
				for (int i=0; i<outc; ++i) {
					char *element = outs[i];
					
					/// Do whatever you want with 'element'.
					
					/// Remember!: This is your responsibility!.
					free(element);
				}
			} /// END Get filenames as array string.
		}
 */
#ifdef __cplusplus
}
#endif

#undef EXPORT_FUNC

#endif /* LIBTVPLAYLIST_H_INCLUDED */
