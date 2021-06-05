//*******************************************************************************************************
//Author: Jack Moon
//Program Name: 3D Rod Intersection Point
//Program Description: Given two points in 3D space along with corresponding lengths, return true if 
//there exists a common endpoint between the two segments, and store said endpoint in a given variable.
//If there is no common endpoint, return false. A hint vector is supplied for cases with multiple
//common endpoints.
//Program Use Cases: Collision detection in 3D could be used to detect when a player is close enough
//to interact with another player or object. Another use could be to calculate where to trigger an
//audio or visual effect when two objects, like a grenade and a wall, collide. It could also be used
//in physics collisions, for everything from armor cloth interactions to player-boundary collisions.
//Last Updated: 04/11/21
//*******************************************************************************************************

#include <math.h>
#include <stdio.h>
#include <iostream>
using namespace std;

struct vector3d 
{
	float x;
	float y;
	float z;
};

typedef vector3d point3d;

//subtracts vector v2 from v1
vector3d subtractVectors(vector3d &v1, vector3d &v2)

{
    vector3d tempVector;
    tempVector.x = v1.x - v2.x;
    tempVector.y = v1.y - v2.y;
    tempVector.z = v1.z - v2.z;
    return tempVector;
}

//adds two vectors together
vector3d addVectors(vector3d &v1, vector3d &v2)

{
    vector3d tempVector;
    tempVector.x = v2.x + v1.x;
    tempVector.y = v2.y + v1.y;
    tempVector.z = v2.z + v1.z;
    return tempVector;
}

//multiplies a vector by a scalar
vector3d scalarMultiply (float scalar, vector3d &v)

{
	vector3d tempVector;
	tempVector.x = v.x * scalar;
	tempVector.y = v.y * scalar;
	tempVector.z = v.z * scalar;
	return tempVector;
}

//returns the magnitude of a given vector
float vectorMagnitude(vector3d &v)

{
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

//normalizes a given vector to have a magnitude of 1
vector3d normalizeVector(vector3d &v)

{
	vector3d tempVector;
	float mag = vectorMagnitude(v);
	tempVector.x = v.x/mag;
	tempVector.y = v.y/mag;
	tempVector.z = v.z/mag;
	return tempVector;
}

//returns the dot product of v1 and v1
float dotProduct(vector3d &v1, vector3d &v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}

//returns the cross product of v1 X v2
vector3d crossProduct(vector3d &v1, vector3d &v2)

{
	vector3d tempVector;
	tempVector.x = (v1.y * v2.z) - (v1.z * v2.y);
	tempVector.y = (v1.z * v2.x) - (v1.x * v2.z);
	tempVector.z = (v1.x * v2.y) - (v1.y * v2.x);
	return tempVector;
}

bool intersect_line_segments(
	point3d position_0,			// origin of first line segment.
	float length_0,			// length of first line segment.
	point3d position_1,			// origin of second line segment.
	float length_1,			// length of second line segment.
	vector3d hint_direction,		// in the event there are multiple solutions, return the
						// one furthest in this direction.
	point3d *out_common_end_position)	// if result is true, point where both line segments can be
						// oriented to end. otherwise uninitialized.
{
	
	float epsilon = 0.001f; //Value used for floating point equality checks; can be changed for higher or lower precision
    vector3d differenceVector = subtractVectors(position_1, position_0); // Distance between the two origin points
	float differenceMagnitude = vectorMagnitude(differenceVector); //Length of the difference between origin points
	vector3d vectorToAdd = {0,0,0}; //Vector to be reused throughout program to perform operations on a vector before adding it; this allows all the helper functions to pass by reference and save memory

	//A sphere can be drawn by rotating a rod of any length around a point; there is a common endpoint only if
	//two given spheres intersect at any points. They will either intersect at exactly one point, a circle of points,
	//or all points (if they are exactly the same sphere)

	//First, check if they intersect at all
	if((length_0 + length_1) - differenceMagnitude <= -epsilon)
	{
		return false;
	}

	//Check if one "sphere" fully surrounds the other, creating no common endpoints; this comes from the added magnitudes
	//off the difference vector and smaller length being smaller than the longer length
	if(length_1 - length_0 < -epsilon && (differenceMagnitude + length_1) - length_0 < -epsilon) //If the length_0 sphere is the bigger one
	{
		return false;
	}

	if(length_0 - length_1 < -epsilon && (differenceMagnitude + length_0) - length_1 < -epsilon) //If the length_1 sphere is the bigger one
	{
		return false;
	}

	//Next, check if they are the exact same "sphere", and therefore have infinite common endpoints on the sphere's surface.
	//The point to return will be the one furthest in the direction of the hint vector
	if(abs(length_1-length_0) <= epsilon && abs(position_1.x-position_0.x) <= epsilon 
		&& abs(position_1.y-position_0.y) <= epsilon && abs(position_1.z-position_0.z) <= epsilon)
	{
		vector3d hintNormalized = normalizeVector(hint_direction); //Normalize the hint vector
		vectorToAdd = scalarMultiply(length_0, hintNormalized);
		*out_common_end_position = addVectors(position_0, vectorToAdd); //Scale the normalized vector by the common length, and add to the common point
		return true;
	}

	//Now, see if they meet at exactly one point; first case happens if the combined lengths are the same as the distance between points
	if((length_0 + length_1) - differenceMagnitude <= epsilon)
	{
		//Common endpoint is in the direction of the difference between the origin points, at a distance equal to the fraction
		//of the difference vector inside the position_0 "sphere" (if taking the difference vector to be position_1 - position_0)
		vectorToAdd = scalarMultiply(length_0/differenceMagnitude, differenceVector);
		*out_common_end_position = addVectors(position_0, vectorToAdd);
		return true;
	}

	//Second case of exactly one common endpoint happens if one of the "spheres" is inside the other, but the distance between both points
	//added to the length of the smaller rod equals the length of the longer rod; "spheres" of equal radius cannot be completely enclosed
	//by each other, so if the length's are equal this case cannot happen.
	if(length_1 - length_0 < -epsilon && abs((length_1 + differenceMagnitude) - length_0) <= epsilon) //length_0 is the longer
	{
		//Add the origin position of the larger "sphere" to the normalized difference vector scaled by length_0
		vectorToAdd = scalarMultiply(length_0/differenceMagnitude, differenceVector);
		*out_common_end_position = addVectors(position_0, vectorToAdd);
		return true;
	} 
	if(length_0 - length_1 < -epsilon && abs((length_0 + differenceMagnitude) - length_1) <= epsilon) //length_1 is the longer
	{
		//Add the origin position of the larger "sphere" to the normalized difference vector scaled by length_0;
		//the difference vector is also flipped in direction to get position_0 - position_1 for this calculation
		vectorToAdd =  scalarMultiply(-1 * length_1/differenceMagnitude, differenceVector);
		*out_common_end_position = addVectors(position_1, vectorToAdd);
		return true;
	}

	//If we make it this far, the spheres intersect normally! As a result they have a circle of common endpoints for which
	//we need to find the center, radius, and vector normal to the plain it lies in. Then find the point furthest in the hint direction

	//Finding the center of the circle, using position_0 as a reference:

	//Use some trig to calculate the length of the difference between position_0 and the circle center
	float distanceRatio = 0.5f + ((length_0 * length_0 - length_1 * length_1)/(2 * differenceMagnitude * differenceMagnitude));

	vectorToAdd = scalarMultiply(distanceRatio, differenceVector);
	point3d circleCenter = addVectors(position_0, vectorToAdd); //Add position 0 to the difference vector scaled by the distance ratio

	//Radius is easily calculated using Pythagorean theorem
	float circleRadius = sqrt((length_0 * length_0) - (distanceRatio * differenceMagnitude * distanceRatio * differenceMagnitude));

	//Normal of the circle is simply the difference vector normalized
	vector3d circleNormal = normalizeVector(differenceVector);

	//Check if hint vector is the same or exact negative of the circle normal; if it is, every point on the circle is a valid solution
	//so we add an arbitrary vector to the hint vector to produce a valid result. This vector can be anything, even randomized if wanted
	vector3d normalHintCrossProduct = crossProduct(circleNormal, hint_direction);
	if(abs(normalHintCrossProduct.x) <= epsilon && abs(normalHintCrossProduct.y) <= epsilon && 
		abs(normalHintCrossProduct.z) <= epsilon) //If cross product is {0,0,0}, vectors are equal or exact opposite
	{
		vector3d hintVectorAddition = {1,0,0}; //Arbitrary vector to be added to the hint direction if it points in the same direction as the normal of the intersection circle
		hint_direction = addVectors(hint_direction, hintVectorAddition);
	}

	//Project the hint vector on to the same plane as the circle to get the target direction for our solution
	vector3d scaledNormal = scalarMultiply(dotProduct(circleNormal, hint_direction), circleNormal); //Scale the circle's normal by the 
	vectorToAdd = subtractVectors(hint_direction, scaledNormal); //Subtract the normal scaled by their dot product to get the point on the circle's plane closest to the target point
	vectorToAdd = normalizeVector(vectorToAdd); //Normalize the vector; results in a unit vector pointing from the circle center to the solution point
	vectorToAdd = scalarMultiply(circleRadius, vectorToAdd); //Scale the vector by the intersection circle's radius
	*out_common_end_position = addVectors(circleCenter, vectorToAdd); //Finally add the computed vector to the circle center to get the solution point
	return true;
}

int main()
{
	point3d commonEndPosition;
	point3d point0 = {0,0,0};
	float length0 = 2.0f;
	point3d point1 = {2,0,0};
	float length1 = 2.0f;
	vector3d hintVector = {0,0,1};
	bool hasCommonEnd = intersect_line_segments(point0, length0, point1, length1, hintVector, &commonEndPosition);

	cout << hasCommonEnd << endl;
	cout << commonEndPosition.x << endl;
    cout << commonEndPosition.y << endl;
	cout << commonEndPosition.z << endl;
}
