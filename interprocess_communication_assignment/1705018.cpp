#include <pthread.h>
#include <cstdio>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <random>

#define TIME_UNIT 1000

using namespace std;

sem_t *kioskSem, boardingGateSem, specialKioskSem;
sem_t vipSem, channelSem, leftRightSem, rightLeftSem;
sem_t printingSem;
sem_t *securityCheckSem;

ifstream inputFile;
ofstream outFile;

int globalTime = 0;
int M, N, P, w, x, y, z;
int leftRightUsing = 0, rightLeftUsing = 0, vipChance, passLostChance;

void *TimeThreadFunction(void *arg)
{
    while (true)
    {
        usleep(TIME_UNIT);

        globalTime++;
    }
}

void *BoardingPassFunction(int uid)
{

    int currentTime = 0, startTime;
    int idx = 0, tempValue;

    sem_wait(&printingSem);
    for (int i = 0; i < N; i++)
    {
        sem_getvalue(&kioskSem[i], &tempValue);

        if (tempValue > 0)
        {
            idx = i;
            break;
        }
    }

    outFile << "Passenger " << uid << " has started waiting in kiosk " << idx + 1 << " at " << globalTime << endl;
    sem_post(&printingSem);

    startTime = globalTime;

    sem_wait(&kioskSem[idx]);

    currentTime = globalTime;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started self-check in at kiosk " << idx + 1 << " at time " << globalTime << " [waited " << (globalTime - startTime) << " time unit]" << endl;
    sem_post(&printingSem);

    usleep(TIME_UNIT * w);

    sem_post(&kioskSem[idx]);

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has finished check in at time " << globalTime << " [finished self check in " << (currentTime) << " + w(" << w << ") = " << globalTime << endl;
    sem_post(&printingSem);
}

void *SpecialKiosk(int uid)
{

    int currentTime = 0, startTime = globalTime;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started waiting in the special kiosk at " << globalTime << endl;
    sem_post(&printingSem);

    sem_wait(&specialKioskSem);

    currentTime = globalTime;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started self-check in at special kiosk at time " << globalTime << " [waited " << (globalTime - startTime) << " time unit]" << endl;
    sem_post(&printingSem);

    usleep(TIME_UNIT * w);

    sem_post(&specialKioskSem);

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has finished checking in special kisok at time " << globalTime << " [finished self check in " << (currentTime) << " + w(" << w << ") = " << globalTime << endl;
    sem_post(&printingSem);
}

void *SecurityCheckFunction(int uid)
{
    int idx = 0;
    int tempValue, semValue;
    // *semValue = 100000000;

    for (int i = 0; i < N; i++)
    {
        sem_getvalue(&securityCheckSem[i], &tempValue);

        if (tempValue > 0)
        {
            idx = i;
            break;
        }
    }

    int startTime = globalTime, currentTime = 0;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started waiting for security check in belt " << (idx + 1) << " from time " << globalTime << endl;
    sem_post(&printingSem);

    sem_wait(&securityCheckSem[idx]);
    currentTime = globalTime;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started the security check at time " << globalTime << " [waited for " << globalTime - startTime << " time unit for the belt to be available]" << endl;
    sem_post(&printingSem);

    usleep(TIME_UNIT * x);

    sem_post(&securityCheckSem[idx]);

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has crossed the security check at time " << globalTime << " [ " << currentTime << " + x(" << x << ") = " << globalTime << "]" << endl;
    sem_post(&printingSem);
}

void *BoardingGateFunction(int uid)
{

    int startTime = globalTime, currentTime;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started waiting to be boarded at time " << globalTime << endl;
    sem_post(&printingSem);

    sem_wait(&boardingGateSem);
    currentTime = globalTime;

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has started boarding the plane at time " << globalTime << " [waited for " << globalTime - startTime << " time unit in the queue]" << endl;
    sem_post(&printingSem);

    usleep(TIME_UNIT * y);

    sem_post(&boardingGateSem);

    sem_wait(&printingSem);
    outFile << "Passenger " << uid << " has boarded the plane at time " << globalTime << " [ " << currentTime << " + y(" << y << ") = " << globalTime << "]" << endl;
    sem_post(&printingSem);
}

void *VipChannelFunction(int uid, bool dir)
{

    bool mydir = dir;

    //mydir = true  --> Left  -> Right
    //mydir = false --> Right -> Left

    int startTime = globalTime, currentTime;

    if (mydir)
    {

        sem_wait(&printingSem);
        outFile << "Passenger " << uid << " has started waiting in VIP channel for Left to Right at time " << globalTime << endl;
        sem_post(&printingSem);

        sem_wait(&leftRightSem);
        leftRightUsing++;

        if (leftRightUsing == 1)
        {
            sem_wait(&vipSem);
            sem_wait(&channelSem);
        }

        sem_post(&leftRightSem);

        currentTime = globalTime;

        sem_wait(&printingSem);
        outFile << "Passenger " << uid << " has started using the VIP channel for Left to Right at time " << globalTime << " [waited for " << globalTime - startTime << " time unit in the queue]" << endl;
        sem_post(&printingSem);

        usleep(TIME_UNIT * z);

        sem_wait(&leftRightSem);
        leftRightUsing--;

        if (leftRightUsing == 0)
        {
            sem_post(&vipSem);
            sem_post(&channelSem);
        }
        sem_post(&leftRightSem);

        sem_wait(&printingSem);
        outFile << "Passenger " << uid << " has finished using the VIP channel for Left to Right at time " << globalTime << " [ " << currentTime << " + z(" << z << ") = " << globalTime << "]" << endl;
        sem_post(&printingSem);
    }
    else
    {

        sem_wait(&printingSem);
        outFile << "Passenger " << uid << " has started waiting in VIP channel for Right to Left at time " << globalTime << endl;
        sem_post(&printingSem);

        sem_wait(&rightLeftSem);

        rightLeftUsing++;
        sem_wait(&vipSem);
        sem_post(&vipSem);

        if (rightLeftUsing == 1)
        {
            sem_wait(&channelSem);
        }

        sem_post(&rightLeftSem);

        sem_wait(&printingSem);
        outFile << "Passenger " << uid << " has started using the VIP channel for Right to Left at time " << globalTime << " [waited for " << globalTime - startTime << " time unit in the queue]" << endl;
        sem_post(&printingSem);

        currentTime = globalTime;

        usleep(TIME_UNIT * z);

        sem_wait(&rightLeftSem);
        rightLeftUsing--;

        if (rightLeftUsing == 0)
        {
            sem_post(&channelSem);
        }

        sem_post(&rightLeftSem);

        sem_wait(&printingSem);
        outFile << "Passenger " << uid << " has finished using the VIP channel for Right to Left at time " << globalTime << " [ " << currentTime << " + z(" << z << ") = " << globalTime << "]" << endl;
        sem_post(&printingSem);
    }
}

void *AllOperations(void *arg)
{

    int uid = *(int *)arg;

    bool ifVip = false, ifPassLost = false;

    int randomVip = rand() % 100, randomPass = rand() % 100;

    if (randomVip < vipChance)
    {
        ifVip = true;
    }
    if (randomPass < passLostChance)
    {
        ifPassLost = true;
    }

    if (ifVip)
    {
        printf("%d is Vip\n", uid);
    }

    if (ifPassLost)
    {
        printf("%d has lost pass\n", uid);
    }

    BoardingPassFunction(uid);
    if (ifVip)
    {
        VipChannelFunction(uid, true);
    }
    else
    {
        SecurityCheckFunction(uid);
    }

    if (ifPassLost)
    {
        VipChannelFunction(uid, false);
        SpecialKiosk(uid);
        VipChannelFunction(uid, true);
    }

    BoardingGateFunction(uid);

    pthread_exit(NULL);
}

int main()
{

    inputFile.open("input.txt");
    outFile.open("output.txt");

    if (inputFile.is_open())
    {
        inputFile >> M >> N >> P;
        inputFile >> w >> x >> y >> z;
        //while(getline(myfile, line))
    }
    else
    {
        printf("unable to open file\n");
    }

    inputFile.close();

    printf("M = %d, N = %d, P = %d\n", M, N, P);
    printf("w = %d, x = %d, y = %d, z = %d\n", w, x, y, z);

    // securityCheckSem = new sem_t[N];

    // for(int i = 0; i < P; i++)
    // {
    //     sem_init(&securityCheckSem[i], 0, P);
    // }

    securityCheckSem = new sem_t[N];
    kioskSem = new sem_t[M];

    for (int i = 0; i < N; i++)
    {
        sem_init(&securityCheckSem[i], 0, P);
    }

    for (int i = 0; i < M; i++)
    {
        sem_init(&kioskSem[i], 0, 1);
    }

    sem_init(&boardingGateSem, 0, 1);
    sem_init(&specialKioskSem, 0, 1);
    sem_init(&vipSem, 0, 1);
    sem_init(&channelSem, 0, 1);
    sem_init(&printingSem, 0, 1);
    sem_init(&leftRightSem, 0, 1);
    sem_init(&rightLeftSem, 0, 1);

    srand(time(0));

    int passengers, returnVal;
    printf("Enter num of passengers: \n");
    scanf("%d", &passengers);

    printf("Enter Vip Probability: \n");
    scanf("%d", &vipChance);

    printf("Enter Pass loss probability: \n");
    scanf("%d", &passLostChance);

    pthread_t *threads, timeThread;
    pthread_attr_t ta;

    threads = new pthread_t[passengers];

    pthread_attr_init(&ta);
    pthread_attr_setschedpolicy(&ta, SCHED_RR);

    pthread_create(&timeThread, &ta, TimeThreadFunction, NULL);

    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(3);

    for (int i = 0; i < passengers; i++)
    {
        int number = distribution(generator);

        int id2 = i + 1;
        int *passengerId = &id2;
        returnVal = pthread_create(&threads[i], &ta, AllOperations, (void *)passengerId);

        usleep(number * TIME_UNIT);
    }

    for (int i = 0; i < passengers; i++)
    {
        pthread_join(threads[i], NULL);
        printf("thread %d finished\n", i + 1);
    }

    return 0;
}