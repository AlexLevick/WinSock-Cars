#ifndef NETWORKCAR_H
#define NETWORKCAR_H

#include <SFML/Graphics.hpp>
#include "gamecar.h"

struct _position{

	float x, y, r;
};

// Class to hold the gfx and functions for other cars, copies the gfx from player car
class NetworkCar{
public:
	sf::ConvexShape car_gfx_;
	std::vector<sf::RectangleShape> tire_gfx_;
	_position previous_[5];
	_position next_[5];
	_position diff_[5];
	bool timed_draw_;

	NetworkCar();
	NetworkCar(GameCar player_, sf::Color colour_);
	~NetworkCar();
	void Update(float time_);
	void CalculateDiff();
};

#endif // !NETWORKCAR_H
