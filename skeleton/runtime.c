<<<<<<< HEAD
/***************************************************************************
 *  Title: Runtime environment 
 * -------------------------------------------------------------------------
 *    Purpose: Runs commands
 *    Author: Stefan Birrer
 *    Version: $Revision: 1.1 $
 *    Last Modification: $Date: 2006/10/13 05:24:59 $
 *    File: $RCSfile: runtime.c,v $
 *    Copyright: (C) 2002 by Stefan Birrer
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    $Log: runtime.c,v $
 *    Revision 1.1  2005/10/13 05:25:59  sbirrer
 *    - added the skeleton files
 *
 *    Revision 1.6  2002/10/24 21:32:47  sempi
 *    final release
 *
 *    Revision 1.5  2002/10/23 21:54:27  sempi
 *    beta release
 *
 *    Revision 1.4  2002/10/21 04:49:35  sempi
 *    minor correction
 *
 *    Revision 1.3  2002/10/21 04:47:05  sempi
 *    Milestone 2 beta
 *
 *    Revision 1.2  2002/10/15 20:37:26  sempi
 *    Comments updated
 *
 *    Revision 1.1  2002/10/15 20:20:56  sempi
 *    Milestone 1
 *
 ***************************************************************************/
#define __RUNTIME_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/************Private include**********************************************/
#include "runtime.h"
#include "io.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

/************Global Variables*********************************************/

#define NBUILTINCOMMANDS (sizeof BuiltInCommands / sizeof(char*))

typedef struct bgjob_l {
  pid_t pid;
  pid_t parentpid;
  char* pname;
  int jobid;
  char* status;
  struct bgjob_l* next;
} bgjobL;

/* the pids of the background processes */
bgjobL *bgjobs = NULL;

/* the jobid got so far */
int the_jobid = 1;

/* the foreground pid right now */
int fgpid = 0;

/* whether this process is the shell (the one that won't exit even when got SIGINT) */
int is_shell = 1;

/************Function Prototypes******************************************/
/* run command */
static void RunCmdFork(commandT*, bool);
/* runs an external program command after some checks */
static void RunExternalCmd(commandT*, bool);
/* resolves the path and checks for exutable flag */
static bool ResolveExternalCmd(commandT*);
/* forks and runs a external program */
static void Exec(commandT*, bool);
/* runs a builtin command */
static void RunBuiltInCmd(commandT*);
/* checks whether a command is a builtin command */
static bool IsBuiltIn(char*);
/************External Declaration*****************************************/

/**************Implementation***********************************************/


void RunCmd(commandT** cmd, int n)
{
  int i;
  if(n == 1)
    RunCmdFork(cmd[0], TRUE);
  else{
    RunCmdPipe(cmd[0], cmd[1]);
    for(i = 0; i < n; i++)
      ReleaseCmdT(&cmd[i]);
  }
}

void RunCmdFork(commandT* cmd, bool fork)
{
  if (cmd->argc<=0)
    return;
  if (IsBuiltIn(cmd->argv[0]))
  {
    RunBuiltInCmd(cmd);
  }
  else
  {
    RunExternalCmd(cmd, fork);
  }
}

void RunCmdBg(commandT* cmd)
{
  
  
}

void RunCmdPipe(commandT* cmd1, commandT* cmd2)
{
}

void RunCmdRedirOut(commandT* cmd, char* file)
{
}

void RunCmdRedirIn(commandT* cmd, char* file)
{
}


/*Try to run an external command*/
static void RunExternalCmd(commandT* cmd, bool fork)
{
  if (ResolveExternalCmd(cmd)){
    Exec(cmd, fork);
  }
  else {
    printf("%s: command not found\n", cmd->argv[0]);
    fflush(stdout);
    ReleaseCmdT(&cmd);
  }
}

/*Find the executable based on search list provided by environment variable PATH*/
static bool ResolveExternalCmd(commandT* cmd)
{
  char *pathlist, *c;
  char buf[1024];
  int i, j;
  struct stat fs;

  if(strchr(cmd->argv[0],'/') != NULL){
    if(stat(cmd->argv[0], &fs) >= 0){
      if(S_ISDIR(fs.st_mode) == 0)
        if(access(cmd->argv[0],X_OK) == 0){/*Whether it's an executable or the user has required permisson to run it*/
          cmd->name = strdup(cmd->argv[0]);
          return TRUE;
        }
    }
    return FALSE;
  }
  pathlist = getenv("PATH");
  if(pathlist == NULL) return FALSE;
  i = 0;
  while(i<strlen(pathlist)){
    c = strchr(&(pathlist[i]),':');
    if(c != NULL){
      for(j = 0; c != &(pathlist[i]); i++, j++)
        buf[j] = pathlist[i];
      i++;
    }
    else{
      for(j = 0; i < strlen(pathlist); i++, j++)
        buf[j] = pathlist[i];
    }
    buf[j] = '\0';
    strcat(buf, "/");
    strcat(buf,cmd->argv[0]);
    //***********
    //DEBUG fprintf(stderr, "%s\n",buf);
    //***********
    if(stat(buf, &fs) >= 0){
      if(S_ISDIR(fs.st_mode) == 0)
        if(access(buf,X_OK) == 0){/*Whether it's an executable or the user has required permisson to run it*/
          cmd->name = strdup(buf); 
          return TRUE;
        }
    }
  }
  return FALSE; /*The command is not found or the user don't have enough priority to run.*/
}




//This function executes an outside command.
static void Exec(commandT* cmd, bool forceFork)
{
  // Creating fork. Parent will exit and child will continue to run the built-in command.
  int pid;
  if (forceFork)
    pid = fork();

  if (pid == 0 || !forceFork) {
    //child: put itself into a new process group and execute the command

    // when being created, put itslef to a new process group and label itself as not shell
    setpgid(0, 0);
    is_shell = 0;
    
    sleep(25);

    if (execv(cmd->name, cmd->argv) == -1) {
      perror("Command not executed. \n");
      return;
    }
  } else {
    //parent: wait for the child to terminate, or continue if '&' is supplied

    if (cmd->bg == 1) {
      // if this child is a background job
      printf("%ld: putting %s (id %ld) at background\n", (long)getpid(), cmd->name, (long)pid);

      // form the node to put into linked list
      bgjobL *x = (bgjobL*)malloc(sizeof(bgjobL));
      x->pid = pid;
      x->parentpid = getppid();
      x->pname = cmd->name;
      x->jobid = (the_jobid++);
      x->status = "Running";
      x->next = NULL;

      //put it into the linked list
      if (bgjobs == NULL)
        bgjobs = x;
      else {
        bgjobL* bgj = bgjobs;
        while (bgj->next != NULL) {
          bgj = bgj->next;
        }
        bgj->next = x;
      }

    } else {
      // if this child is a foreground job
      printf("setting fgpid %ld \n", (long)pid);
      fgpid = pid;

      int status;
      waitpid(pid, &status, 0);
      printf("%ld: %s (id %ld) exited with status %d\n", (long)getpid(), cmd->name, (long)pid, WEXITSTATUS(status));
      fgpid = 0;

    }
    return;
  }
}

static bool IsBuiltIn(char* cmd)
{
  if (strcmp(cmd, "bg") == 0 || strcmp(cmd, "jobs") == 0 || strcmp(cmd, "fg") == 0)
    return TRUE;
  return FALSE;
}


static void RunBuiltInCmd(commandT* cmd)
{
  int jobid;

  // implement the bg command 
  if (strcmp(cmd->argv[0],"bg") == 0) {
    //try to find the process entry
    jobid = atoi(cmd->argv[1]);
    bgjobL* thisbg = bgjobs;
    while (thisbg != NULL && thisbg->jobid != jobid)
      thisbg = thisbg->next;
    // mimic the bg behavior
    if (thisbg != NULL) {
      printf("sending SIGCONT to running job (id %ld, jid %d)\n", (long)thisbg->pid, thisbg->jobid);
      kill(thisbg->pid, SIGCONT);
      thisbg->status = "Running";
    }
  }

  // implement the jobs command
  if (strcmp(cmd->argv[0],"jobs") == 0) {
    bgjobL* thisbg = bgjobs;
    while (thisbg != NULL) {
      printf("%s job %s with id %ld, jobid %d \n", thisbg->status, thisbg->pname, (long)thisbg->pid, thisbg->jobid);
      thisbg = thisbg->next;
    }
  }

  // implement the fg command
  if (strcmp(cmd->argv[0],"fg") == 0) {
    //try to find the process entry
    jobid = atoi(cmd->argv[1]);
    bgjobL* thisbg = bgjobs;
    while (thisbg != NULL && thisbg->jobid != jobid)
      thisbg = thisbg->next;
    // mimic the fg behavior
    if (thisbg != NULL) {
      printf("bringing (id %ld, jid %d) to frontground \n", (long)thisbg->pid, thisbg->jobid);
      fgpid = thisbg->pid;
      kill(thisbg->pid, SIGCONT);
      thisbg->status = "Running";
      int status;
      waitpid(thisbg->pid, &status, 0);
      fgpid = 0;
      printf("(id %ld, jid %d) now safely terminates \n", (long)thisbg->pid, thisbg->jobid);
    }
  }
}


void CheckJobs()
{
  //check the status of every job recorded, and update the linked list
  // so that it only contains currently running jobs
  //running jobs are 0, and finished jobs are pid or -1
  if (bgjobs == NULL)
    return;

  int status, ret;
  // make sure the bgjobs is not a completed job
  while (bgjobs != NULL && (ret = waitpid(bgjobs->pid, &status, WNOHANG)) == bgjobs->pid) {
    printf("chopped off process %d with status %d and ret %d\n", bgjobs->pid, status, ret);
    bgjobs = bgjobs->next;
  }
  if (bgjobs == NULL)
    return;
  // go through the linked list and chop off finished processes
  bgjobL* this = bgjobs;
  bgjobL* thisnext = this->next;
  while (thisnext != NULL) {
    if ((ret = waitpid(thisnext->pid, &status, WNOHANG)) == thisnext->pid) {
      printf("chopped off process %d with status %d and ret %d\n", bgjobs->pid, status, ret);
      this->next = thisnext->next;
    }
    this = thisnext;
    thisnext = this->next;
  }

  return;
}


commandT* CreateCmdT(int n)
{
  int i;
  commandT * cd = malloc(sizeof(commandT) + sizeof(char *) * (n + 1));
  cd -> name = NULL;
  cd -> cmdline = NULL;
  cd -> is_redirect_in = cd -> is_redirect_out = 0;
  cd -> redirect_in = cd -> redirect_out = NULL;
  cd -> argc = n;
  for(i = 0; i <=n; i++)
    cd -> argv[i] = NULL;
  return cd;
}

/*Release and collect the space of a commandT struct*/
void ReleaseCmdT(commandT **cmd){
  int i;
  if((*cmd)->name != NULL) free((*cmd)->name);
  if((*cmd)->cmdline != NULL) free((*cmd)->cmdline);
  if((*cmd)->redirect_in != NULL) free((*cmd)->redirect_in);
  if((*cmd)->redirect_out != NULL) free((*cmd)->redirect_out);
  for(i = 0; i < (*cmd)->argc; i++)
    if((*cmd)->argv[i] != NULL) free((*cmd)->argv[i]);
  free(*cmd);
}
=======
/***************************************************************************
 *  Title: Runtime environment 
 * -------------------------------------------------------------------------
 *    Purpose: Runs commands
 *    Author: Stefan Birrer
 *    Version: $Revision: 1.1 $
 *    Last Modification: $Date: 2006/10/13 05:24:59 $
 *    File: $RCSfile: runtime.c,v $
 *    Copyright: (C) 2002 by Stefan Birrer
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    $Log: runtime.c,v $
 *    Revision 1.1  2005/10/13 05:25:59  sbirrer
 *    - added the skeleton files
 *
 *    Revision 1.6  2002/10/24 21:32:47  sempi
 *    final release
 *
 *    Revision 1.5  2002/10/23 21:54:27  sempi
 *    beta release
 *
 *    Revision 1.4  2002/10/21 04:49:35  sempi
 *    minor correction
 *
 *    Revision 1.3  2002/10/21 04:47:05  sempi
 *    Milestone 2 beta
 *
 *    Revision 1.2  2002/10/15 20:37:26  sempi
 *    Comments updated
 *
 *    Revision 1.1  2002/10/15 20:20:56  sempi
 *    Milestone 1
 *
 ***************************************************************************/
#define __RUNTIME_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/************Private include**********************************************/
#include "runtime.h"
#include "io.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

/************Global Variables*********************************************/

#define NBUILTINCOMMANDS (sizeof BuiltInCommands / sizeof(char*))

typedef struct bgjob_l {
  pid_t pid;
  struct bgjob_l* next;
} bgjobL;

/* the pids of the background processes */
bgjobL *bgjobs = NULL;

/************Function Prototypes******************************************/
/* run command */
static void RunCmdFork(commandT*, bool);
/* runs an external program command after some checks */
static void RunExternalCmd(commandT*, bool);
/* resolves the path and checks for exutable flag */
static bool ResolveExternalCmd(commandT*);
/* forks and runs a external program */
static void Exec(commandT*, bool);
/* runs a builtin command */
static void RunBuiltInCmd(commandT*);
/* checks whether a command is a builtin command */
static bool IsBuiltIn(char*);
/************External Declaration*****************************************/

/**************Implementation***********************************************/
int total_task;
void RunCmd(commandT** cmd, int n)
{
  int i;
  total_task = n;
  if(n == 1)
    RunCmdFork(cmd[0], TRUE);
  else{
    RunCmdPipe(cmd[0], cmd[1]);
    for(i = 0; i < n; i++)
      ReleaseCmdT(&cmd[i]);
  }
}

void RunCmdFork(commandT* cmd, bool fork)
{
  if (cmd->argc<=0)
    return;
  if (IsBuiltIn(cmd->argv[0]))
  {
    RunBuiltInCmd(cmd);
  }
  else
  {
    RunExternalCmd(cmd, fork);
  }
}

void RunCmdBg(commandT* cmd)
{
  // Added bool fork as a parameter.
  if (cmd->argc<=0)
    return;
  char buf[1024];
  strcpy(buf,""); //Initialize to empty string;
  int i;
  for(i = 0 ; i<=cmd->argc;i++)
    if ((cmd)->argv[i] != NULL){
      strcat(buf," ");
      strcat(buf,(cmd)->argv[i]);
    }
  // Creating fork. Parent will exit and child will continue to run the built-in command.
  int pid = fork();
  if (pid == 0) {
    /* child */
    if (IsBuiltIn(cmd->argv[0])){
      // copied from RunBuiltInCmd
      char *name[4];
      name[0] = "sh";
      name[1] = "-c";
      name[2] = buf;
      name[3] = NULL;
      if (execv("/bin/sh", name) == -1) {
        /* here errno is set.  You can retrieve a message with either
         * perror() or strerror()
         */
        perror(buf);
        return;
      }
    }
    else{
      // copied from Exec
      char *name[2];
      name[0] = buf;
      name[1] = NULL;
      if (execv(cmd->name, name) == -1) {
        /* here errno is set.  You can retrieve a message with either
         * perror() or strerror()
         */
        perror(buf);
        return;
      }
    }
  } else {
    // Main Thread
    // Do not wait for child to finish
    //int status;
    //waitpid(pid, &status, 0);//Might need to change this because user might want to exit when the child program is running.
    //printf("%s exited with status %d\n", buf, WEXITSTATUS(status));
    return;
  }
}

void RunCmdPipe(commandT* cmd1, commandT* cmd2)
{
}

void RunCmdRedirOut(commandT* cmd, char* file)
{
}

void RunCmdRedirIn(commandT* cmd, char* file)
{
}


/*Try to run an external command*/
static void RunExternalCmd(commandT* cmd, bool fork)
{
  if (ResolveExternalCmd(cmd)){
    Exec(cmd, fork);
  }
  else {
    printf("%s: command not found\n", cmd->argv[0]);
    fflush(stdout);
    ReleaseCmdT(&cmd);
  }
}

/*Find the executable based on search list provided by environment variable PATH*/
static bool ResolveExternalCmd(commandT* cmd)
{
  char *pathlist, *c;
  char buf[1024];
  int i, j;
  struct stat fs;

  if(strchr(cmd->argv[0],'/') != NULL){
    if(stat(cmd->argv[0], &fs) >= 0){
      if(S_ISDIR(fs.st_mode) == 0)
        if(access(cmd->argv[0],X_OK) == 0){/*Whether it's an executable or the user has required permisson to run it*/
          cmd->name = strdup(cmd->argv[0]);
          return TRUE;
        }
    }
    return FALSE;
  }
  pathlist = getenv("PATH");
  if(pathlist == NULL) return FALSE;
  i = 0;
  while(i<strlen(pathlist)){
    c = strchr(&(pathlist[i]),':');
    if(c != NULL){
      for(j = 0; c != &(pathlist[i]); i++, j++)
        buf[j] = pathlist[i];
      i++;
    }
    else{
      for(j = 0; i < strlen(pathlist); i++, j++)
        buf[j] = pathlist[i];
    }
    buf[j] = '\0';
    strcat(buf, "/");
    strcat(buf,cmd->argv[0]);
    //***********
    //DEBUG fprintf(stderr, "%s\n",buf);
    //***********
    if(stat(buf, &fs) >= 0){
      if(S_ISDIR(fs.st_mode) == 0)
        if(access(buf,X_OK) == 0){/*Whether it's an executable or the user has required permisson to run it*/
          cmd->name = strdup(buf); 
          return TRUE;
        }
    }
  }
  return FALSE; /*The command is not found or the user don't have enough priority to run.*/
}

//This function executes an outside command.
static void Exec(commandT* cmd, bool forceFork)
{
  //DEBUGfprintf(stderr, "cmd->name: %s\n", cmd->name);
  char buf[1024];
  strcpy(buf,""); //Initialize to empty string;
  int i;
  for(i = 0 ; i<=cmd->argc;i++)
    if ((cmd)->argv[i] != NULL){
      strcat(buf," ");
      strcat(buf,(cmd)->argv[i]);
    }
  //DEBUG fprintf(stderr, "buf: %s\n", buf);
  // Creating fork. Parent will exit and child will continue to run the built-in command.

  int pid;
  if (forceFork)
    pid = fork();
  if (pid == 0 || !forceFork) {
    /* child */
    char *name[2];
    name[0] = buf;
    name[1] = NULL;
    if (execv(cmd->name, name) == -1) {
      /* here errno is set.  You can retrieve a message with either
       * perror() or strerror()
       */
      perror(buf);
      return;
    }
  } else {
    int status;
    waitpid(pid, &status, 0);
    printf("%s exited with status %d\n", buf, WEXITSTATUS(status));
    return;
  }

}

static bool IsBuiltIn(char* cmd)
{
  char buf[1024];
  struct stat fs;
  strcpy(buf,"/bin/");
  strcat(buf,cmd);
  //***********
  fprintf(stderr, "%s\n",buf);
  //***********
  if(stat(buf, &fs) >= 0){
    if(S_ISDIR(fs.st_mode) == 0)
      if(access(buf,X_OK) == 0){/*Whether it's an executable or the user has required permisson to run it*/
        fprintf(stderr, "%s\n","Is BuiltIn");
        return TRUE;
      }
  }
  fprintf(stderr, "%s\n","Is not BuiltIn");
  return FALSE;     
}


static void RunBuiltInCmd(commandT* cmd)
{
    char buf[1024];
    strcpy(buf,""); //Initialize to empty string;
    int i;
    for(i = 0 ; i<=cmd->argc;i++)
      if ((cmd)->argv[i] != NULL){
        strcat(buf," ");
        strcat(buf,(cmd)->argv[i]);
      }
  // Creating fork. Parent will exit and child will continue to run the built-in command.
  int pid = fork();
  if (pid == 0) {
    /* child */
    char *name[4];
    name[0] = "sh";
    name[1] = "-c";
    name[2] = buf;
    name[3] = NULL;
    if (execv("/bin/sh", name) == -1) {
      /* here errno is set.  You can retrieve a message with either
       * perror() or strerror()
       */
      perror(buf);
      return;
    }
  } else {
    int status;
    waitpid(pid, &status, 0);//Might need to change this because user might want to exit when the child program is running.
    printf("%s exited with status %d\n", buf, WEXITSTATUS(status));
    return;
  }
}

void CheckJobs()
{
}


commandT* CreateCmdT(int n)
{
  int i;
  commandT * cd = malloc(sizeof(commandT) + sizeof(char *) * (n + 1));
  cd -> name = NULL;
  cd -> cmdline = NULL;
  cd -> is_redirect_in = cd -> is_redirect_out = 0;
  cd -> redirect_in = cd -> redirect_out = NULL;
  cd -> argc = n;
  for(i = 0; i <=n; i++)
    cd -> argv[i] = NULL;
  return cd;
}

/*Release and collect the space of a commandT struct*/
void ReleaseCmdT(commandT **cmd){
  int i;
  if((*cmd)->name != NULL) free((*cmd)->name);
  if((*cmd)->cmdline != NULL) free((*cmd)->cmdline);
  if((*cmd)->redirect_in != NULL) free((*cmd)->redirect_in);
  if((*cmd)->redirect_out != NULL) free((*cmd)->redirect_out);
  for(i = 0; i < (*cmd)->argc; i++)
    if((*cmd)->argv[i] != NULL) free((*cmd)->argv[i]);
  free(*cmd);
}
>>>>>>> origin/master
