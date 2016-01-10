#pragma once

class ColorGenerator {
public:
	ColorGenerator();
	const float * getCurrentColor();
	const float * getNextColor();
private:
	int index;
};