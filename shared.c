#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

void  parent(int  ShmPTR[]) {
  sleep(rand() % 6);
  if (ShmPTR[0] > 100) {
    printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", ShmPTR[0]);
  } else {
    int amountToGive = rand() % 101;
    if (amountToGive % 2 == 0) {
      ShmPTR[0] = ShmPTR[0] + amountToGive;
      printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amountToGive, ShmPTR[0]);
    }
    else printf("Dear old Dad: Doesn't have any money to give\n");        
  }
}

void child(int ShmPTR[]) {
  sleep(rand() % 6);
  int balance = rand() % 51;
  printf("Poor Student needs $%d\n", balance);

  if (balance <= ShmPTR[0]) {
    ShmPTR[0] -= balance;
    printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, ShmPTR[0]);
  } else {
    printf("Poor Student: Not Enough Cash ($%d)\n", ShmPTR[0]);
  }
}

int  main(int  argc, char *argv[]) {
  // Init shared memory section
  // Create space for integers in shared memory
  int ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
  // Check for error
  if (ShmID < 0) {
      printf("*** shmget error (server) ***\n");
      exit(1);
  }
  printf("Process has received a shared memory of two integers...\n");
 
  // Attach shared memory
  int* ShmPTR = (int *) shmat(ShmID, NULL, 0);
  // Check for error
  if (*ShmPTR == -1) {
      printf("*** shmat error (server) ***\n");
      exit(1);
  }
  printf("Process has attached the shared memory...\n");

  ShmPTR[0] = 0;  // BankAccount
  ShmPTR[1] = 0;  // Turn

  // Fork child
  pid_t pid = fork();

  if (pid < 0) {
    printf("*** fork error (server) ***\n");
    exit(1);
  }
  else if (pid == 0) {
    for (int i = 0; i < 25; i++) {
      // Wait until it's my turn
      while (ShmPTR[1] != 0) {;}
      // Run student code
      child(ShmPTR);
      // Your turn dad!
      ShmPTR[1] = 1; 
    }
    exit(0);
  } else {
    // Parent
    for (int i = 0; i < 25; i++) {
      // Wait until it's my turn
      while (ShmPTR[1] != 1) {;}
      // Run dad code
      parent(ShmPTR);
      // Your turn student!
      ShmPTR[1] = 0;
    }
    int status;
    wait(&status);
    printf("Process has detected the completion of its child...\n");
  }

  // Detach memory
  shmdt((void *) ShmPTR);
  printf("Process has detached its shared memory...\n");

  // Remove shared memory
  shmctl(ShmID, IPC_RMID, NULL);
  printf("Process has removed its shared memory...\n");

  printf("Server exits...\n");
  exit(0);
}