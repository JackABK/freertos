#include <filesystem.h>
#include <fio.h>
#include <string.h>
#include "filedump.h"
/* Filesystem includes */
#include "romfs.h"

int filedump(const char * filename)
{
  char buf[128];
  size_t count=0;
  int fd = fs_open(filename, 0, O_RDONLY);
  if(fd == OPEN_FILE_FAIL)
     	return FILE_DUMP_FAIL;
   
  while(!count) {
    //Read from filename to buffer
    count = fio_read(fd, buf, sizeof(buf));
    
    //Write buffer to stdout 
    printf("%s",buf);
  } 
		return FILE_DUMP_SUCCESS;
}
