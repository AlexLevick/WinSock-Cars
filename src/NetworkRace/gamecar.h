// class to store SFML and Box2D car

#ifndef GAMECAR_H
#define GAMECAR_H

#include <Box2D/Box2D.h>
#include "b2dcar.h"
#include <SFML/Graphics.hpp>

class GameCar {
public:
	TDCar* car_b2d_;
	sf::ConvexShape car_gfx_;
	std::vector<sf::RectangleShape> tire_gfx_;

	GameCar();
	GameCar(b2World* world_, sf::Color colour_);
	~GameCar();

	void Update();

};

#endif