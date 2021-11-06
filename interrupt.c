#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define PIN_A 8
#define PIN_B 9

#define EVENT_END_WAIT_TIME 0.5
#define SLEEP_TIME 500000

double lastInterruptTime = 0;

int secondsMultiplier = 1000000;

// the total movement for this event
int totalMovement = 0;

// track if an event is in progress
int moving = 0;

double startTime = 0;
double endTime = 0;

int getAmountUsed(int ticks) {
    int ticksPerRotation = 24;
    double rollRadiusInInches = 2;
    double circumferenceInInches = M_PI * 2 * rollRadiusInInches;

    double numberOfTurns = (double) ticks / (double) ticksPerRotation; 

    return circumferenceInInches * numberOfTurns;
}

double getTime() {
    // long int start_time;
	struct timespec gettime_now;

	clock_gettime(CLOCK_REALTIME, &gettime_now);

    double seconds = gettime_now.tv_sec;
    double billion = 1000000000;

    return seconds + (gettime_now.tv_nsec / billion);
}

double timeSinceEnd() {
    return getTime() - endTime;
}

void setStartTime() {
    startTime = getTime();
}

void update() {
    double interruptTime = getTime();

    if (interruptTime - lastInterruptTime < 0.001) {
        printf("debounced!\n");
    } else {
        moving = 1;
        totalMovement++;
        endTime = getTime();
        lastInterruptTime = interruptTime;
    }
}

void runEvent() {
    double totalDistance = getAmountUsed(totalMovement);

    printf("%.0f inches used \n", (double) totalDistance);
    printf("This is when it would say the thing\n");
}

void resetState() {
    moving = 0;
    totalMovement = 0;
}

void triggerEvent() {
    if (moving == 1) {
        if (timeSinceEnd() >= EVENT_END_WAIT_TIME) {
            // reset state and trigger event
            runEvent();
            resetState();
        } else {
            printf("not long enough since last trigger\n");
        }
    } else {
        printf("wheel hasn't moved since last trigger\n");
    }
}

int main() {
    wiringPiSetup();
    pinMode(PIN_A, INPUT);

    startTime = getTime();
    lastInterruptTime = getTime();

    wiringPiISR (PIN_A, INT_EDGE_FALLING, update) ;

    // check for event every x seconds
    for (;;) {
        triggerEvent();
        usleep(SLEEP_TIME);
    }

    return 0;
}
