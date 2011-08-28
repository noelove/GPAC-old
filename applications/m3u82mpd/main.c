/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Telecom ParisTech 2010 - 
 *					All rights reserved
 *
 *  This file is part of GPAC / m3u82mpd application
 *
 *  GPAC is gf_free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include "../modules/mpd_in/m3u8.h"
#include "../../include/gpac/network.h"
#include "../../include/gpac/download.h"

int main(int argc, char **argv)
{
    GF_Err e;
    VariantPlaylist * pl = NULL;
    char *url = argv[1];
    char *cache_m3u8_file;
	u32 i, count;
    FILE *fmpd;
    Bool verbose = 0;
    u32 update_interval = 0;
    char *m3u8_local_name = "file.m3u8";
    Bool is_local = 0;

    gf_sys_init(0);

	gf_log_set_tool_level(GF_LOG_NETWORK, verbose ? GF_LOG_DEBUG : GF_LOG_INFO);

    while (1) {

        if (gf_url_is_local(url)) {
            m3u8_local_name = url;
            is_local = 1;
        } else {
            e = gf_dm_wget(url, m3u8_local_name);
            if (e != GF_OK) return -1;
        }

        e = parse_root_playlist(m3u8_local_name, &pl, ".");
        if (e != GF_OK) return -1;
        
        fmpd = fopen(argv[2], "wt");
        
        fprintf(fmpd, "<MPD type=\"Live\" xmlns=\"urn:3GPP:ns:PSS:AdaptiveHTTPStreamingMPD:2009\">\n");
        fprintf(fmpd, " <ProgramInformation moreInformationURL=\"http://gpac.sourceforge.net\">\n");
        fprintf(fmpd, "  <Title>Media Presentation Description for file %s</Title>\n", url);
        fprintf(fmpd, "  <Source>Generated by GPAC %s</Source>\n", GPAC_FULL_VERSION);

        fprintf(fmpd, " </ProgramInformation>\n");
        fprintf(fmpd, " <Period start=\"PT0S\">\n");	

        count = gf_list_count(pl->programs);
        for (i=0; i<count; i++) {
	        u32 j, count2;
            Program *prog = gf_list_get(pl->programs, i);
            count2 = gf_list_count(prog->bitrates);
            for (j = 0; j<count2; j++) {
                PlaylistElement *pe = gf_list_get(prog->bitrates, j);
                fprintf(stdout, "%d, %d, %s, %s, %d\n", pe->durationInfo, pe->bandwidth, pe->title, pe->url, pe->elementType);
                if (pe->elementType == TYPE_PLAYLIST) {
                    u32 k, count3;
                    char *tmp;
                    char c;
                    char baseURL[GF_MAX_PATH]; 
                    tmp = strrchr(url, '/');
                    tmp++;
                    c = tmp[0];
                    tmp[0] = 0;
                    strcpy(baseURL, url);
                    tmp[0] = c;
	                fprintf(fmpd, "  <Representation mimeType=\"video/mp2t\">\n");	
	                fprintf(fmpd, "   <SegmentInfo duration=\"PT%dS\" baseURL=\"%s\">\n", pe->durationInfo, baseURL);	
                    count3 = gf_list_count(pe->element.playlist.elements);
                    update_interval = (count3 - 1) * pe->durationInfo * 1000;
                    for (k=0; k<count3; k++) {
                        PlaylistElement *elt = gf_list_get(pe->element.playlist.elements, k);
                        if (k) fprintf(fmpd, "    <Url sourceURL=\"%s\"/>\n", elt->url);	
                        else fprintf(fmpd, "    <InitialisationSegmentURL sourceURL=\"%s\"/>\n", elt->url);	
                    }
                    fprintf(fmpd, "   </SegmentInfo>\n");
                    fprintf(fmpd, "  </Representation>\n");
                } else if (pe->elementType == TYPE_STREAM) {
                    fprintf(stdout, "Stream\n");
                }
            }
        }
        fprintf(fmpd, " </Period>\n");
        fprintf(fmpd, "</MPD>");
        fclose(fmpd);
        variant_playlist_del(pl);
        if (is_local) break;
        gf_sleep(update_interval);
        
    }

    gf_sys_close();
    return 0;
}
