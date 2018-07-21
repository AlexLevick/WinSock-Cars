#include "networkcar.h"

NetworkCar::NetworkCar()
{}

NetworkCar::NetworkCar(GameCar player_, sf::Color colour_)
{
	// Since gfx is created from box2d car, copy the player gfx and change the colour
	car_gfx_ = player_.car_gfx_;
	car_gfx_.setOutlineColor(colour_);

	tire_gfx_ = player_.tire_gfx_;
	for (int i = 0; i < tire_gfx_.size(); i++)
	{
		tire_gfx_[i].setOutlineColor(colour_);
	}
}

NetworkCar::~NetworkCar()
{

}

void NetworkCar::Update(float time_)
{	
	if (timed_draw_)
	{
		// "Timed draw"
		car_gfx_.setPosition(sf::Vector2f(previous_[0].x + (diff_[0].x * time_), previous_[0].y + (diff_[0].y * time_)));

		// Used to fix issue from crossing 0 degrees and rotating wrong direction
		float limit_ = 150;
		float car_rotation_; 
		if (diff_[0].r > limit_)
		{
			diff_[0].r = -(diff_[0].r - 360);
			car_rotation_ = previous_[0].r - (diff_[0].r * time_);
		}
		else if (diff_[0].r < -limit_)
		{
			diff_[0].r = (diff_[0].r + 360);
			car_rotation_ = previous_[0].r + (diff_[0].r * time_);
		}
		else
			car_rotation_ = previous_[0].r + (diff_[0].r * time_);

		car_gfx_.setRotation(car_rotation_);

		for (int i = 0; i < tire_gfx_.size(); i++)
		{
			tire_gfx_[i].setPosition(sf::Vector2f(previous_[i + 1].x + (diff_[i + 1].x * time_), previous_[i + 1].y + (diff_[i + 1].y * time_)));

			// Used to fix issue from crossing 0 degrees and rotating wrong direction
			float tire_rotation_;
			if (diff_[i + 1].r > limit_)
			{
				diff_[i + 1].r = -(diff_[i + 1].r - 360);
				tire_rotation_ = previous_[i + 1].r - (diff_[i + 1].r * time_);
			}
			else if (diff_[i + 1].r < -limit_)
			{
				diff_[i + 1].r = (diff_[i + 1].r + 360);
				tire_rotation_ = previous_[i + 1].r + (diff_[i + 1].r * time_);
			}
			else
				tire_rotation_ = previous_[i + 1].r + (diff_[i + 1].r * time_);

			tire_gfx_[i].setRotation(tire_rotation_);
		}
	}
	else
	{
		// "Untimed draw"
		car_gfx_.setPosition(sf::Vector2f(previous_[0].x, previous_[0].y));
		car_gfx_.setRotation(previous_[0].r);

		for (int i = 0; i < tire_gfx_.size(); i++)
		{
			tire_gfx_[i].setPosition(sf::Vector2f(previous_[i + 1].x, previous_[i + 1].y));
			tire_gfx_[i].setRotation(previous_[i + 1].r);
		}
	}
}

void NetworkCar::CalculateDiff()
{
	for (int i = 0; i < 5; i++)
	{
		diff_[i].x = next_[i].x - previous_[i].x;
		diff_[i].y = next_[i].y - previous_[i].y;
		diff_[i].r = next_[i].r - previous_[i].r;
	}
}