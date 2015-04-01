/*
 * utility.h
 *
 *  Created on: Mar 28, 2015
 *      Author: newuser
 */

#ifndef UTILITY_H_
#define UTILITY_H_


typedef struct {
	float x, y, w;
} coord3;

typedef struct {
	float m0, m1, m2;
} motorVel3;

typedef struct {
	float x, y;
} coord2;

const coord2 P_GOAL = {130, 0};
const float P_GOAL_WIDTH = 75;
const float P_CONTROL_K_XY = 2;
const float P_MAX_VELOCITY = 30;

const float P_CONTROL_K_W = 3;
const float P_MAX_SPIN = 1;

void rotate(coord3* v, float theta);

coord2 utility_rotate(coord2 v, float theta);

coord2 utility_3to2(coord3 v);

coord3 utility_2to3(coord2 v);

float utility_dist1(coord2 v);

coord2 utility_addVector(coord2 p1, coord2 p2);

coord2 utility_subVector(coord2 p1, coord2 p2);

coord2 utility_multVector(coord2 v, float m);

coord3 utility_multVector3(coord3 v, float m);

coord3 utility_divVector3(coord3 v, float m);

coord3 utility_subVector3(coord3 v1, coord3 v2);

coord2 utility_getVector(coord2 p1, coord2 p2);

float utility_dist(coord2 p1, coord2 p2);

float utility_dist3(coord3 r1, coord3 r2);

coord2 utility_unitVector(coord2 v);

coord2 utility_vectorWithLength(coord2 v, float l);

float utility_angleMod(float angle);

float utility_getAngle1(coord2 v);

float utility_getAngle(coord2 p1, coord2 p2);

float utility_dotProduct(coord2 v1, coord2 v2);

void sleep_ms(long ms);

double getTime_ms();

double getTime_s();

void startTimer(int timerId);

double getTimerTime_ms(int timerId);

double getTimerTime_s(int timerId);

void utility_setTime_s(double timeInSeconds);

int sign(int x);

float fsign(float x);

#endif /* UTILITY_H_ */
