#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

// create a shared variable for bank account
// create two process --> parent and child process
// allow parent to deposit money to bank account
// allow child to withdraw money to bank account
// both child and parent can check balance in bank account randomly
// parent and child process should loop indefinitely

// in each loop:
  // sleep for 0-5 second
  // print Dear Old Dad: Attempting to check balance
  // generate random number if even  and local balance is < 100:
      // deposit money
    // else:
      // print Dear Old Dad: Thinks studen has enough Cash, localBalance
  // else if odd print Dear Old Dad: :Last Checking blanace

  // deposit money
  // copy value of bank account into a local non shared variable localBalance
  // randomly generate a amount between 0 -100
  // if even: deposit in bank account and print Dear old dad: Deposists
  // copy contents of non-shared local variable localBalance into shared variable bank account
  // if odd: print Dear old Dad: Doesn't have any money to give
 
// create process for child parent and mom
void ParentProcess(int [], sem_t *);
void ChildProcess(int [], sem_t *);
void MomProcess(int [], sem_t *);
// void  ClientProcess(int []);

int  main(int  argc, char *argv[])
{
    int ShmID;
    int *ShmPTR;
    // pid_t  pid;
    int status;
    //  creating variables as needed 
    int parents = atoi(argv[1]);
    int childs = atoi(argv[2]);
    int total = atoi(argv[1])+atoi(argv[2]);
    pid_t pid[total];
    sem_t  *mutex;

     if (argc != 3) {
          // printf("Use: %s #1 #2 #3 #4\n", argv[0]);
          printf("Use: %s #1 #2\n", argv[0]);
          exit(1);
     }

     ShmID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of four integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     ShmPTR[0] = 0;
     printf("Server has attached the shared memory...\n");

    if ((mutex = sem_open("banksemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
      perror("semaphore initilization");
      exit(1);
    }

    for (int i=0; i<total; i++) {
      pid[i] = fork();
      if (pid[i] < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
      } else if (pid[i] == 0) {
        if (i == 0) {
          ParentProcess(ShmPTR, mutex);
        } else if (parents == 2 && i == 2) {
          MomProcess(ShmPTR, mutex);
        } else {
          ChildProcess(ShmPTR, mutex);
        }
        exit(0);
      }
    }

    // wait(&status);
    for (int i=0; i<total; i++) {
      wait(NULL);
    }
    printf("Server has detected the completion of process...\n");
    shmdt((void *) ShmPTR);
    printf("Server has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Server has removed its shared memory...\n");
    printf("Server exits...\n");
    exit(0);
}

void ParentProcess(int sharedMem[], sem_t* mutex) {
  int bankAccount;
  int randAmount;
  srand(getpid());

  for (int i=0; i<5; i++) {
    // sleep for 5 second
    sleep(rand()%6);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    sem_wait(mutex);
    bankAccount = sharedMem[0];
    // generate random number between 0-100
    randAmount = rand()%101;
    if (randAmount % 2) {
      //even
      if (bankAccount < 100){
        bankAccount += randAmount;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", randAmount, bankAccount);
        sharedMem[0] = bankAccount;
      } else {
        printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", bankAccount);
      }
    } else {
      //odd
      printf("Dear Old Dad: Last Checking Balance = $%d\n", bankAccount);
    }
    sem_post(mutex);
  }
}

// child process
void ChildProcess(int sharedMem[], sem_t* mutex) {
  int bankAccount;
  int randNeedAmount;
  srand(getpid());

  for (int i=0; i<5; i++) {
    // sleep for 5 second
    sleep(rand()%6);
    printf("Poor Student: Attempting to Check Balance\n");
    sem_wait(mutex);
    bankAccount = sharedMem[0];
    // generate random number between 0-50
    randNeedAmount = rand()%51;
    if (randNeedAmount % 2) {
      //even
      if (randNeedAmount <= bankAccount){
        bankAccount -= randNeedAmount;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", randNeedAmount, bankAccount);
        sharedMem[0] = bankAccount;
      } else {
        printf("Poor Student: Not Enough Cash ($%d)\n", bankAccount);
      }
    } else {
      //odd
      printf("Poor Student: Last Checking Balance = $%d\n", bankAccount);
    }
    sem_post(mutex);
  }
}

// mom process
void MomProcess(int sharedMem[], sem_t* mutex) {
  int bankAccount;
  int randAmount;
  srand(getpid());

  for (int i=0; i<5; i++) {
    // sleep for 10 second
    sleep(rand()%11);
    printf("Loveable Mom: Attempting to Check Balance\n");
    sem_wait(mutex);
    bankAccount = sharedMem[0];
    // generate random number between 0-125
    randAmount = rand()%126;
    if (bankAccount <= 100){
      bankAccount += randAmount;
      printf("Lovable Mom: Deposits $%d / Balance = $%d\n", randAmount, bankAccount);
      sharedMem[0] = bankAccount;
    } 
    sem_post(mutex);
  }
}