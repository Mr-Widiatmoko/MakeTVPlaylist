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

#ifndef LIBPTVSHOW_H_INCLUDED
#define LIBPTVSHOW_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void process(int argc, char *argv[], /// Inputs
			 int *outc, char **outs);/// Outputs, you need to free 'outs' by your self.

/** Example Usage on C:
	
		#include <libtvplaylist.h>
 
		int main(int argc, char *argv[]) {
			{ /// BEGIN Just create playlist filename.
				char *inputs[] = {
					/// By default, --no-output-file=true for 'libtvplaylist', but false for 'tvplaylist'
					"--no-output-file=false", // or "-P=false"
					"/Users/dev/Movies", // Specify one input folder. If no input, then current directory will be automatically used.
					/// Another inputs, can be: files, folders or options
				};
				process(sizeof(inputs)/sizeof(inputs[0]), inputs, NULL, NULL);
			} /// END Get playlist filename.
 
			{ /// BEGIN Get filenames as array string.
				int outc = 0;
				process(argc, argv, &outc, NULL); /// Try to get numbers of filenames in 'outc'
				
				char *outs[outc]; /// Stack allocation for outs.
	 
				/// Allocate heap buffer 1000 characters for full path filename.
				for (int i=0; i<outc; ++i) outs[i] = (char *)malloc(1000 * sizeof(char));
	 
				process(argc, argv, &outc, outs); /// Copy full path filenames into outs elements.
	 
				if (outs)
					for (int i=0; i<outc; ++i) {
						char *element = out[i];
						
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

#endif /* LIBPTVSHOW_H_INCLUDED */
