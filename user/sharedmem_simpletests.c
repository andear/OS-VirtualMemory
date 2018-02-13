#include "types.h"
#include "stat.h"
#include "user.h"

#define USERTOP 0xA0000
#define PGSIZE 4096

void
testPassed(void)
{
  printf(1, "....Passed\n");
}

void
testFailed(void)
{
  printf(1, "....FAILED\n");
}

void expectedVersusActualNumeric(char* name, int expected, int actual)
{
  printf(1, "      %s expected: %d, Actual: %d\n", name, expected, actual);
}

void
whenRequestingSharedMemory_ValidAddressIsReturned(void)
{
  printf(1, "Test: whenRequestingSharedMemory_ValidAddressIsReturned...");
  char* sharedPage = shmem_access(0);
  char* highestPage =       (char*)(USERTOP - PGSIZE);
  char* secondHighestPage = (char*)(USERTOP - 2*PGSIZE);
  char* thirdHighestPage =  (char*)(USERTOP - 3*PGSIZE);
  char* fourthHighestPage = (char*)(USERTOP - 4*PGSIZE);
  
  if(sharedPage == highestPage ||
     sharedPage == secondHighestPage ||
     sharedPage == thirdHighestPage ||
     sharedPage == fourthHighestPage) {
    testPassed();
  } else {
    testFailed(); 
  }
}

void
afterRequestingSharedMemory_countReturns1()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1...");
  char* sharedPage = shmem_access(0);
  int count = shmem_count(0);

  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
whenSharingAPage_ParentSeesChangesMadeByChild()
{
  printf(1, "Test: whenSharingAPage_ParentSeesChangesMadeByChild...");
  char* sharedPage = shmem_access(0);
  sharedPage[0] = 42;

  int pid = fork();
  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(0);
    childsSharedPage[0] = childsSharedPage[0] + 1;
    exit();
  } else {
    // in parent
    wait(); // wait for child to terminate
    if(sharedPage[0] == 43){
      testPassed();
    } else {
      testFailed();
      expectedVersusActualNumeric("'sharedPage[0]'", 43, sharedPage[0]);
    }
  }
}

void
whenProcessExits_SharedPageIsFreed()
{
  printf(1, "Test: whenProcessExits_SharedPageIsFreed...");
  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(0);
    sharedPage[0] = 42;
    exit();
  } else {
    // in parent
    wait();
    char* parentsSharedPage = shmem_access(0);
    if(parentsSharedPage[0] != 42){
      testPassed();
    } else {
      // should be garbage value after being freed, but it's still 42
      testFailed();
      expectedVersusActualNumeric("'parentsSharedPage[0]'", 1, parentsSharedPage[0]);
    }
  }
}

void
whenSharingAPageBetween2Processes_countReturns2()
{
  printf(1, "Test: whenSharingAPageBetween2Processes_countReturns2...");

  char* sharedPage = shmem_access(0);
  sharedPage = sharedPage + 0;  // silence unused variable error

  int pid = fork();

  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(0);
    childsSharedPage = childsSharedPage + 0;  // silence unused variable error

    int count = shmem_count(0);
    if(count != 2){
      testFailed();
      expectedVersusActualNumeric("'count'", 2, count);
    }

    exit();
  } else{
    // in parent
    wait(); // wait for child to exit
    int parentsCount = shmem_count(0);
    if(parentsCount != 1){
      testFailed();
      expectedVersusActualNumeric("'parentsCount'", 1, parentsCount);
    }
  }

  testPassed();
}

void
whenProcessExists_countReturns0()
{
  printf(1, "Test: whenProcessExists_countReturns0...");

  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(0);
    sharedPage = sharedPage + 0;  // silence unused variable error
    exit();
  } else {
    // in parent
    wait();
    int count = shmem_count(0);

    if(count != 0){
      testFailed();
      expectedVersusActualNumeric("'count'", 0, count);
    } else {
      testPassed();
    }

  }
}

void
beforeRequestingSharedMemory_countReturns0()
{
  printf(1, "Test: beforeRequestingSharedMemory_countReturns0...");

  int count = shmem_count(0);

  if(count != 0){
    testFailed();
    expectedVersusActualNumeric("'count'", 0, count);
  } else {
    testPassed();
  }
}

int
main(void)
{
  int pid;

  // we fork then run each test in a child process to keep the main process
  // free of any shared memory
  pid = fork();
  if(pid == 0){
    whenRequestingSharedMemory_ValidAddressIsReturned();
    exit();
  }
  wait();
  
  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPage_ParentSeesChangesMadeByChild();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPageBetween2Processes_countReturns2();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExits_SharedPageIsFreed();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExists_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    beforeRequestingSharedMemory_countReturns0();
    exit();
  }
  wait();


  exit();
}
