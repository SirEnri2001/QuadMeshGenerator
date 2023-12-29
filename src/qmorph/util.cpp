#include"util.h"
#include <glm.hpp>
#include <cassert>

// solve a linear equation that using AC&AD to represent AB
// k*AB = m*AC+(1-m)*AD
using namespace glm;
float* solveKMEquation(vec3 AC, vec3 AD, vec3 AB) {
	//Point C & D mustn't be the same pos.
	assert(length(AC - AD) > FLT_EPSILON);
	vec3 normal = cross(AD, AC);
	normal = normal / length(normal);
	//projection on the patch
	vec3 projection = AB - normal * (AB * normal);
	AB = projection;
	float cNorm = length(AC);
	AD = AD / cNorm;
	AC = AC / cNorm;
	bool AB_normalized = false;
	if (length(AB) > 5 || length(AB) < 0.2) {
		AB = AB / cNorm;
		AB_normalized = true;
	}
	// solve the 2x2 linear system
	int ind1 = 1, ind2 = 2;
	if (abs((AD[ind1] - AC[ind1]) * AB[ind2] - (AD[ind2] - AC[ind2]) * AB[ind1]) < FLT_EPSILON) {
		ind1 = 0;
		ind2 = 1;
	}
	if (abs((AD[ind1] - AC[ind1]) * AB[ind2] - (AD[ind2] - AC[ind2]) * AB[ind1]) < FLT_EPSILON) {
		ind1 = 0;
		ind2 = 2;
	}
	if (abs((AD[ind1] - AC[ind1]) * AB[ind2] - (AD[ind2] - AC[ind2]) * AB[ind1]) < FLT_EPSILON) {
		assert(false);
	}
	//coefficientMat << projection[ind1], AC[ind1] - AD[ind1], projection[ind2], AC[ind2] - AD[ind2];
	//Eigen::Vector2d b(AC[ind1], AC[ind2]);
	//Eigen::Vector2d x = coefficientMat.fullPivLu().solve(b);
	//x[0] = x[0] / AC.norm();
	float l = ((AD[ind1] - AC[ind1]) * AB[ind2] - (AD[ind2] - AC[ind2]) * AB[ind1]);
	assert(abs(l) > FLT_EPSILON);
	float k = (AD[ind1] * AC[ind2] - AC[ind1] * AD[ind2])
		/ ((AD[ind1] - AC[ind1]) * AB[ind2] - (AD[ind2] - AC[ind2]) * AB[ind1]);
	float m = (AD[ind1] * AB[ind2] - AB[ind1] * AD[ind2])
		/ ((AD[ind1] - AC[ind1]) * AB[ind2] - (AD[ind2] - AC[ind2]) * AB[ind1]);
	if (AB_normalized) {
		return new float[2] {k, m};
	}
	else {
		return new float[2] {k* cNorm, m};

	}
}

//Point convertLaplacianOrigin(Point origin) {
//	return origin;
//}
//
//Point bisectorOfCpoints(Point left, Point right, Point normal) {
//	left = left / left.norm();
//	right = right / right.norm();
//	normal = normal / normal.norm();
//	Point sharpBisector = left + right;
//	sharpBisector = sharpBisector / sharpBisector.norm();
//	if ((right.cross(left)).dot(normal) > EPSILON) {
//		return sharpBisector;
//	}
//	else if ((right .cross(left)).dot(normal) < -EPSILON) {
//		return -sharpBisector;
//	}
//	else {
//		return (right.cross(normal)) / (right .cross(normal)).norm();
//	}
//}