#include <unistd.h>
#define _close close
#define _open open
#define _read read
#define _write write
#define _filelength filelength
#define _O_CREAT O_CREAT
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_TRUNC O_TRUNC
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
static int filelength(int a){return -1;}
