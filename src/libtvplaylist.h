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
			int outc = 0;
			char **outs;
			process(argc, argv, &outc, outs);
			if (outs) {
				for (int i=0; i<outc; ++i) {
					char *element = out[i];
					/// Do whatever you want with 'element'.
				}
				
				/// Remember!: This is your responsibility!.
				free(outs);
			}
		}
 */
#ifdef __cplusplus
}
#endif

#endif /* LIBPTVSHOW_H_INCLUDED */
