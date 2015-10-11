/*******************************************
 * this tool use for formatting sort file
 *******************************************/

/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Format

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

/* typedef */
typedef	struct	formatsave	FSV;

/* struct */
struct	formatsave {
	char	*fsv_str;
	int	fsv_len;

	int	fsv_cnt;
};


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	char	*pFileSaver;
static	FSV	*formatSaveContainer;
static	int	nReadLine = 128;


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Four */
static void fsv_perror(char *errStr);


/*------------------------------------------
	Part Four: Format

	1. main
	2. fsv_perror

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char **argv)
{
	char	*pBegin, *pMov;
	FSV	*pFsv;
	int	nCir;

	if(argc != 2) {
		printf("FSV---> usage: <file path>\n");
		exit(FUN_RUN_FAIL);
	}

	if(read_all_file(&pFileSaver, argv[1], 0xA) == FUN_RUN_FAIL)
		fsv_perror("main - read_all_file");

	if(!(pFsv = formatSaveContainer = malloc(nReadLine * sizeof(FSV))))
		fsv_perror("main - malloc");

	for(pMov = pBegin = pFileSaver, nCir = 0; nCir < nReadLine; nCir++, pFsv++) {
		pMov = strchr(pBegin, ' ') + 1;
		pFsv->fsv_str = pMov;

		pBegin = strchr(pMov, ' ');
		pFsv->fsv_len = pBegin - pMov;

		pFsv->fsv_cnt = atoi(++pBegin);

		pBegin = strchr(pBegin, '\n') + 1;
	}

	for(pFsv = formatSaveContainer; pFsv < formatSaveContainer + nReadLine; pFsv++)
		printf("%.*s\n", pFsv->fsv_len, pFsv->fsv_str);

	for(pFsv = formatSaveContainer; pFsv < formatSaveContainer + nReadLine; pFsv++)
		printf("%d\n", pFsv->fsv_cnt);

	free(pFileSaver);
	free(formatSaveContainer);

	exit(FUN_RUN_OK);
}


/*-----fsv_perror-----*/
static void fsv_perror(char *errStr)
{
	printf("FSV---> %s: %s\n", errStr, strerror(errno));
	exit(FUN_RUN_FAIL);
}
