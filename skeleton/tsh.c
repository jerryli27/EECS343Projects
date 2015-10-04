// Net IDs: 
//
//
/***************************************************************************
 *  Title: MySimpleShell 
 * -------------------------------------------------------------------------
 *    Purpose: A simple shell implementation 
 *    Author: Stefan Birrer
 *    Version: $Revision: 1.1 $
 *    Last Modification: $Date: 2006/10/13 05:25:59 $
 *    File: $RCSfile: tsh.c,v $
 *    Copyright: (C) 2002 by Stefan Birrer
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    $Log: tsh.c,v $
 *    Revision 1.1  2005/10/13 05:25:59  sbirrer
 *    - added the skeleton files
 *
 *    Revision 1.4  2002/10/24 21:32:47  sempi
 *    final release
 *
 *    Revision 1.3  2002/10/23 21:54:27  sempi
 *    beta release
 *
 *    Revision 1.2  2002/10/15 20:37:26  sempi
 *    Comments updated
 *
 *    Revision 1.1  2002/10/15 20:20:56  sempi
 *    Milestone 1
 *
 ***************************************************************************/
#define __MYSS_IMPL__

/************System include***********************************************/
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/************Private include**********************************************/
#include "tsh.h"
#include "io.h"
#include "interpreter.h"
#include "runtime.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

#define BUFSIZE 80

/************Global Variables*********************************************/

/************Function Prototypes******************************************/
/* handles SIGINT and SIGSTOP signals */	
static void sig_handler(int);

/************External Declaration*****************************************/

/**************Implementation***********************************************/

typedef struct bgjob_l {
  pid_t pid;
  pid_t parentpid;
  char* pname;
  int jobid;
  char* status;
  struct bgjob_l* next;
} bgjobL;

int fgpid;
int is_shell;
bgjobL *bgjobs;

int main (int argc, char *argv[])
{
  /* Initialize command buffer */
  char* cmdLine = malloc(sizeof(char*)*BUFSIZE);

  /* shell initialization */
  if (signal(SIGINT, sig_handler) == SIG_ERR) PrintPError("SIGINT");
  if (signal(SIGTSTP, sig_handler) == SIG_ERR) PrintPError("SIGTSTP");

  while (TRUE) /* repeat forever */
  {
    /* print prompt */
    printf("%%");
    //printf("my status: %d", waitpid(getpid(), &status, WNOHANG));
    //printf("%%");
    /* read command line */
    getCommandLine(&cmdLine, BUFSIZE);

    if(strcmp(cmdLine, "exit") == 0 || feof(stdin))
    {
      forceExit=TRUE;
      break;
    }else {
      // If we juse printf(cmdLine), compiler gives a warning because the cmdLine can contain things like  %s 
      // and the program will try to find some string that does not exist. This could be exploited to run virus.
      //fprintf(stderr, "%s", cmdLine);
    }

    /* checks the status of background jobs */
    CheckJobs();

    /* interpret command and line
     * includes executing of commands */
    Interpret(cmdLine);
  }

  // make sure all the shell's child processes are terminated
  if (forceExit == TRUE) {
    while (bgjobs != NULL) {
      kill(bgjobs->pid, SIGINT);
      bgjobs = bgjobs->next;
    }
  }

  /* shell termination */
  free(cmdLine);
  return 0;
} /* end main */


// signals SIGTSTP, SIGINT are handled here
static void sig_handler(int signo)
{
  int my_pid = getpid();

  //handle singal SIGINT (ctrl-c)
  if (signo == SIGINT) {
    printf("pid %d got SIGINT \n", (int)my_pid);
    // send signal to the foreground job
    if (fgpid != 0) {
      printf("sending SIGINT to process %d\n", (int)fgpid);
      kill(fgpid, SIGINT);
      fgpid = 0;
    }
    // all processes receiving this, if not the shell, must handle the signal normally
    //   and send the signal to all its children
    if (is_shell != 1) {
      //go through the linked list to send signal to all its children
      bgjobL* thisbg = bgjobs;
      while (thisbg != NULL) {
        if (thisbg->parentpid == my_pid) {
          printf("sending SIGINT to process %d\n", (int)thisbg->pid);
          kill(thisbg->pid, SIGINT);
        }
        thisbg = thisbg->next;
      }
      // handle the signal for itself
      printf("process %d going to exit\n", (int)getpid());
      SIG_DFL(SIGINT);
    }
  }

  // handle signal SIGTSTP (ctrl-z), basically same like SIGINT
  if (signo == SIGTSTP) {
    printf("pid %d got SIGTSTP \n", (int)my_pid);
    // send the signal to its fg job
    if (fgpid != 0) {
      printf("sending SIGTSTP to process %d\n", (int)fgpid);
      kill(fgpid, SIGTSTP);
      fgpid = 0;
    }
    // all processes receiving this, if not the shell, must handle the signal normally
    //   and send the signal to all its children
    if (is_shell != 1) {
      //go through the linked list to send signal to all its children
      bgjobL* thisbg = bgjobs;
      while (thisbg != NULL) {
        if (thisbg->parentpid == my_pid) {
          printf("sending SIGTSTP to process %d\n", (int)thisbg->pid);
          kill(thisbg->pid, SIGTSTP);
          thisbg->status = "Stopped";
        }
        thisbg = thisbg->next;
      }
      // handle the signal for itself
      printf("process %d going to stop\n", (int)getpid());
      //SIG_DFL(SIGTSTP);
    }
  }

  return;

}

