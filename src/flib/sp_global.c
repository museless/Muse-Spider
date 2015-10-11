/* global data */
/* spnet.h */
char	*rPac =	"Accept-Language: zh-CN\r\n"
		"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; SVTencentTraveler ; .NET CLR .4322)\r\n"
		"Connection: Keep-Alive\r\n\r\n";

/* spdb.h */
char	*sqlOpt[] = {"db", "--default-file = my.cnf"};
int	nrOpt = (sizeof(sqlOpt) / sizeof(char *));

/* spinc.h */
char	kerNameBuf[] = {"Kernel"};
char	ubNameBuf[] = {"UrlBug"};
char	tbNameBuf[] = {"TextBug"};
char	ebNameBuf[] = {"ExtBug"};
