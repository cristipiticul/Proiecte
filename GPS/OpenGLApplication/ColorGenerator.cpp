#include "ColorGenerator.h"

const float colors[] = {
	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f
};
const int noColors = 6;

ColorGenerator::ColorGenerator()
{
	index = 0;
}

const float * ColorGenerator::getCurrentColor() {
	const float* p = colors + index * 3;
	return p;
}

const float * ColorGenerator::getNextColor()
{
	index++;
	if (index >= noColors) {
		index = 0;
	}
	return getCurrentColor();
}
