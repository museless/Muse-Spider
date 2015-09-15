/*Copyright (c) 2015, William Muse
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */


#ifndef	_SPHTML_H
#define	_SPHTML_H

/* define */

/* match mode */
#define	MHIGH_LEN	2
#define	MATCH_HIGH	"<h"
#define	MENDHG_LEN	3
#define	MATCH_ENDHG	"</h"
#define	MBEG_LEN	3
#define	MATCH_LIBEG	"<li"
#define	MLEDN_LEN	5
#define	MATCH_LIEND	"</li>"
#define	MHTTP_LEN	7
#define	MATCH_HTTP	"http://"
#define	MHTP_LEN	4
#define	MATCH_HTP	"http"
#define	MHTML_LEN	5
#define	MATCH_HTML	".html"
#define	MSHTML_LEN	6
#define	MATCH_SHTML	".shtml"
#define	MHTM_LEN	4
#define	MATCH_HTM	".htm"
#define	MHREF_LEN	6
#define	MATCH_HREF	"href=\""
#define	MEHTML_LEN	7
#define	MATCH_ENDHTML	"</html>"
#define	MLINK_LEN	2
#define	MATCH_LINKBRK	"\r\n"
#define	MCHARSET_LEN	8
#define	MATCH_CHARSET	"charset="
#define	MTITLE_LEN	7
#define	MATCH_TITLE	"<title>"
#define	METITLE_LEN	8
#define	MATCH_ETITLE	"</title>"
#define	MENDBR_LEN	5
#define	MATCH_BR	"</br>"
#define	MLBR_LEN	6
#define	MATCH_LBR	"<br />"
#define	MSRCIPT_LEN	8
#define	MATCH_SRCIPT	"<script>"
#define	Ma_LEN		2
#define	MATCH_a		"<a"
#define	MA_LEN		2
#define	MATCH_A		"<A"
#define	MSPAN_LEN	5
#define	MATCH_SPAN	"<span"
#define	MSTRONG_LEN	7
#define	MATCH_STRONG	"<strong"
#define	MFONT_LEN	5
#define	MATCH_FONT	"<font"
#define	MB_LEN		2
#define	MATCH_B		"<b"
#define	MEND_LEN	2
#define	MATCH_END	"</"
#define	MNOTE_LEN	2
#define	MATCH_NOTE	"<!"

#define	MLAMD_LEN	14
#define	MATCH_LAMD	"Last-Modified:"
#define	MDATE_LEN	5
#define	MATCH_DATE	"Date:"
#define	MLOCA_LEN	10
#define	MATCH_LOCA	"Location: "

#define	MNOBR_LEN	7
#define	MATCH_NOBR	"</nobr>"

#endif
