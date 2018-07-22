#include "gamecar.h"

#define PI 3.14159265
#define ratio 10
#define BtoP(a) (ratio * a)
#define PtoB(a) (a / ratio)
#define tireWidth 0.5
#define tireHeight 1.25

GameCar::GameCar()
{}

GameCar::GameCar(b2World* world_, sf::Color colour_)
{
	car_b2d_ = new TDCar(world_);
	car_b2d_->m_body->SetTransform(b2Vec2(40, 25), 0);

	car_gfx_.setFillColor(sf::Color::Black);
	car_gfx_.setOutlineColor(colour_);
	car_gfx_.setOutlineThickness(-1.4);

	for (b2Fixture* fixture = car_b2d_->m_body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		b2Shape::Type shapeType = fixture->GetType();
		if (shapeType == b2Shape::e_polygon)
		{
			b2PolygonShape* polygonShape = (b2PolygonShape*)fixture->GetShape();

			car_gfx_.setPointCount(polygonShape->m_count);
			for (int i = 0; i < polygonShape->m_count; i++)
			{
				car_gfx_.setPoint(i, sf::Vector2f(BtoP(polygonShape->m_vertices[i].x), BtoP(polygonShape->m_vertices[i].y)));
			}
		}
	}
	car_gfx_.setRotation(car_b2d_->m_body->GetAngle() * 180 / PI);

	for (int i = 0; i < 4; i++)
	{
		float x, y, r, width, height;
		x = car_b2d_->m_tires[i]->m_body->GetPosition().x;
		y = car_b2d_->m_tires[i]->m_body->GetPosition().y;
		r = car_b2d_->m_tires[i]->m_body->GetAngle() * PI / 180;

		sf::RectangleShape temp_tire_(sf::Vector2f(BtoP(tireWidth), BtoP(tireHeight)));
		temp_tire_.setOrigin(sf::Vector2f(BtoP(tireWidth) / 2, BtoP(tireHeight) / 2));
		temp_tire_.setPosition(sf::Vector2f(BtoP(x), BtoP(y)));
		temp_tire_.setRotation(r);
		temp_tire_.setFillColor(sf::Color::Black);
		temp_tire_.setOutlineColor(colour_);
		temp_tire_.setOutlineThickness(-1.4);

		tire_gfx_.push_back(temp_tire_);
	}
}

GameCar::~GameCar()
{

}

void GameCar::Update()
{
	car_gfx_.setPosition(sf::Vector2f(BtoP(car_b2d_->m_body->GetPosition().x), BtoP(car_b2d_->m_body->GetPosition().y)));
	car_gfx_.setRotation(car_b2d_->m_body->GetAngle() * 180 / PI);

	for (int i = 0; i < tire_gfx_.size(); i++)
	{
		tire_gfx_[i].setPosition(sf::Vector2f(BtoP(car_b2d_->m_tires[i]->m_body->GetPosition().x), BtoP(car_b2d_->m_tires[i]->m_body->GetPosition().y)));
		tire_gfx_[i].setRotation(car_b2d_->m_tires[i]->m_body->GetAngle() * 180 / PI);
	}
}