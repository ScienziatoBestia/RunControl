#include "CheckRunningProcesses.h"

/*************************************************************************************************************/
int CheckRunningProcesses(process_t caller) {
/*************************************************************************************************************/
#define LINE_BUFSIZE 128
  
    char line[LINE_BUFSIZE];
    FILE *pipe;

    if(caller==RunControl) pipe = popen("./bin/CheckRC", "r");
    else if(caller==SlowControl) pipe = popen("./bin/CheckSC", "r");
    else if(caller==HPGeSlowControl) pipe = popen("./bin/CheckHSC", "r");
    else if(caller==Producer) pipe = popen("./bin/CheckP", "r");
    else if(caller==Consumer) pipe = popen("./bin/CheckC", "r");
  
    // Check for errors
    if(pipe == NULL) perror("***CheckRunningProcesses: No pipe ");
  
    // Read the script output from the pipe line by line
    int linenr=0;
    while (fgets(line, LINE_BUFSIZE, pipe) != NULL) {
      printf("%s", line);
      ++linenr;
    }
  
    pclose(pipe); /* Close the pipe */

    return linenr;
}
  

