#include <filesystem.h>
#include <fio.h>
#include <string.h>

/* Filesystem includes */
#include "romfs.h"

int filedump(const char * filename)
{
  char buf[128];
  size_t count=0;
  int fd = fs_open(filename, 0, O_RDONLY);
  if(fd == OPEN_FILE_FAIL)
     	return 0;
   
  while(!count) {
    //Read from filename to buffer
    count = fio_read(fd, buf, sizeof(buf));
    
    //Write buffer to stdout 
    printf("%s",buf);
  } 
}
