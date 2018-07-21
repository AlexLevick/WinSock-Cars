#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SFML/Graphics.hpp>
#include "Box2D/Box2D.h"
#include "utils.h"
#include <WinSock2.h>
#include "gamecar.h"
#include "networkcar.h"
#include <vector>
#include <stdlib.h> 

#define PI 3.14159265

// The IP address of the server to connect to
#define SERVERIP "127.0.0.1"
// The TCP port number on the server to connect to
#define SERVERPORT 4444

#define ratio 10
#define BtoP(a) (ratio * a)
#define PtoB(a) (a / ratio)

/*
-Let client decide if they crossed the finish line
	Send with timestamp synchronised across all clients with server
	If anyone else says they crossed the finish line compare timestamps for who actually won
*/

enum _announcement{
	WAITING,
	CONFIRMATION,
	COUNTDOWN,
	RACING,
	FINISHED,
};

enum _message_type{
	CONNECT,
	PLAYERID,
	RACER,
	READY,
	UPDATE,
	FINISH,
};

// message struct sent to server
struct _message_server{

	_message_type type_;
	int player_id_;
	int message_id_;
	float time_stamp_;
	// Position for car and 4 tires
	_position position_[5];
};

// message struct sent to client
struct _message_client{

	_message_type type_;
	int player_id_;
	int message_id_;
	float time_stamp_;
	// Position for car and 4 tires
	_position position_[5];
};

void Physics();
bool Connect();
bool MessageSend(_message_type mes_type_);
void MessageRead();
b2Body *CreateDynamicBody(float xPos, float yPos, float32 width, float32 height);
b2Body *CreateStaticBody(float xPos, float yPos, float32 width, float32 height);
//void CleanUp();

b2World* world_;
GameCar player_;
NetworkCar racer_;

bool racer_conn_ = false;

SOCKET sock;
sockaddr_in toAddr;
int messages_sent_;
int ID_;
int last_ID_;

_announcement announcement_state_ = WAITING;

sf::Clock clock_, last_;
sf::Time time_elapsed_, last_update_;

bool use_last_;
int miss_update_;

int main()
{
	sf::RenderWindow window(sf::VideoMode(1200, 900), "Network Race", sf::Style::Close);
	window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(60);

	world_ = new b2World(b2Vec2(0, 0));
	player_ = GameCar(world_, sf::Color::Red);
	racer_ = NetworkCar(player_, sf::Color::Yellow);

	// Create central obstacle
	float obst_width = 50;
	float obst_height = 30;
	b2Body* obstacle_b2d_ = CreateStaticBody(PtoB(window.getSize().x) / 2, PtoB(window.getSize().y) / 2, obst_width, obst_height);
	sf::RectangleShape obstacle_gfx_(sf::Vector2f(BtoP(obst_width), BtoP(obst_height)));
	obstacle_gfx_.setOrigin(sf::Vector2f(BtoP(obst_width/2), BtoP(obst_height/2)));
	obstacle_gfx_.setFillColor(sf::Color::Transparent);
	obstacle_gfx_.setOutlineColor(sf::Color::Green);
	obstacle_gfx_.setOutlineThickness(-1.4);

	// Create window boundaries// Create starting line boundary and graphic
	float bound_width = PtoB(window.getSize().x);
	float bound_height = PtoB(window.getSize().y);
	b2Body* left_bound_b2d_ = CreateStaticBody(0, PtoB(window.getSize().y) / 2, 1, bound_height);
	b2Body* right_bound_b2d_ = CreateStaticBody(PtoB(window.getSize().x), PtoB(window.getSize().y) / 2, 1, bound_height);
	b2Body* up_bound_b2d_ = CreateStaticBody(PtoB(window.getSize().x) / 2, 0, bound_width, 1);
	b2Body* down_bound_b2d_ = CreateStaticBody(PtoB(window.getSize().x) / 2, PtoB(window.getSize().y), bound_width, 1);

	// Variables to show various methods
	// Toggles whether to use timed draw or not // Control with T and F (on/off)
	racer_.timed_draw_ = true;
	// Toggles whether to use timed draw with a time reset every 100ms (can't miss an update)
	// or since last updated time allowing for linear prediction when an update is missed // Control with L and M
	use_last_ = false;
	// Determines how often we don't miss an update packet, control with 1 - 0, from 10% to 100%
	miss_update_ = 10;

	int vertical_ = 0, horizontal_ = 0;

	sf::Font font_;
	if (!font_.loadFromFile("digital.ttf"))
	{
		printf("font did not load\n");
	}
	sf::Text announce_gfx_;
	announce_gfx_.setFont(font_);

	startWinSock();
	if (!Connect())
		window.close();

	// Game loop
	while (window.isOpen())
	{
		time_elapsed_ = clock_.getElapsedTime();
		last_update_ = last_.getElapsedTime();
		// Check if 100ms has elapsed since last update send
		if (time_elapsed_.asMilliseconds() >= 100)
		{
			MessageRead();
			// Only send update messages if there is someone to read them
			if (racer_conn_)
				MessageSend(UPDATE);
			// Restart clock
			clock_.restart();
			time_elapsed_ = clock_.getElapsedTime();
		}

		
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				// window closed
			case sf::Event::Closed:
				//CleanUp();
				window.close();
				break;

				// key released
			case sf::Event::KeyReleased:
				if (event.key.code == sf::Keyboard::Up)
					vertical_ = 0;

				if (event.key.code == sf::Keyboard::Down)
					vertical_ = 0;

				if (event.key.code == sf::Keyboard::Left)
					horizontal_ = 0;

				if (event.key.code == sf::Keyboard::Right)
					horizontal_ = 0;
				break;

				// key pressed
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape)
				{
					//CleanUp();
					window.close();
				}

				if (event.key.code == sf::Keyboard::Up)
					vertical_ = 1;

				if (event.key.code == sf::Keyboard::Down)
					vertical_ = 2;

				if (event.key.code == sf::Keyboard::Left)
					horizontal_ = 2;

				if (event.key.code == sf::Keyboard::Right)
					horizontal_ = 1;

				if (event.key.code == sf::Keyboard::T)
					racer_.timed_draw_ = true;

				if (event.key.code == sf::Keyboard::F)
					racer_.timed_draw_ = false;

				if (event.key.code == sf::Keyboard::L)
					use_last_ = true;

				if (event.key.code == sf::Keyboard::M)
					use_last_ = false;

				if (event.key.code == sf::Keyboard::Num0)
					miss_update_ = 10;
				if (event.key.code == sf::Keyboard::Num1)
					miss_update_ = 1;
				if (event.key.code == sf::Keyboard::Num2)
					miss_update_ = 2;
				if (event.key.code == sf::Keyboard::Num3)
					miss_update_ = 3;
				if (event.key.code == sf::Keyboard::Num4)
					miss_update_ = 4;
				if (event.key.code == sf::Keyboard::Num5)
					miss_update_ = 5;
				if (event.key.code == sf::Keyboard::Num6)
					miss_update_ = 6;
				if (event.key.code == sf::Keyboard::Num7)
					miss_update_ = 7;
				if (event.key.code == sf::Keyboard::Num8)
					miss_update_ = 8;
				if (event.key.code == sf::Keyboard::Num9)
					miss_update_ = 9;
				break;
			}
		}

		// Update player's movement
		player_.car_b2d_->update(vertical_, horizontal_);

		// Run physics simulation
		Physics();

		// Set player's car and tires
		player_.Update();

		// Set racer's car and tires if connected
		if (racer_conn_)
		{
			// Milliseconds divided by 100 so that 100ms = to multiplier of 1
			if (use_last_)
				racer_.Update(last_update_.asMilliseconds() / 100.0f);
			else
				racer_.Update(time_elapsed_.asMilliseconds() / 100.0f);
		}

		obstacle_gfx_.setPosition(sf::Vector2f(BtoP(obstacle_b2d_->GetPosition().x), BtoP(obstacle_b2d_->GetPosition().y)));

		switch (announcement_state_)
		{
		case WAITING:{
			announce_gfx_.setString("Waiting for player to connect");
			sf::FloatRect textRect = announce_gfx_.getLocalBounds();
			announce_gfx_.setOrigin(textRect.left + textRect.width / 2, textRect.top + textRect.height / 2);
			announce_gfx_.setPosition(sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2));
			break;
		}
		case CONFIRMATION:{
			announce_gfx_.setString("Player connected");
			sf::FloatRect textRect = announce_gfx_.getLocalBounds();
			announce_gfx_.setOrigin(textRect.left + textRect.width / 2, textRect.top + textRect.height / 2);
			announce_gfx_.setPosition(sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2));
			break;
		}
		case COUNTDOWN:{
			break;
		}
		case RACING:{
			break;
		}
		case FINISH:{
			break;
		}
		}

		// Clear window for drawing
		window.clear();

		// Draw all objects
		window.draw(obstacle_gfx_);
		window.draw(announce_gfx_);
		//window.draw(line_gfx_);

		// Only draw racer car if it is connected
		if (racer_conn_)
		{
			for (int i = 0; i < racer_.tire_gfx_.size(); i++)
			{
				window.draw(racer_.tire_gfx_[i]);
			}
			window.draw(racer_.car_gfx_);
		}

		for (int i = 0; i < player_.tire_gfx_.size(); i++)
		{
			window.draw(player_.tire_gfx_[i]);
		}
		window.draw(player_.car_gfx_);

		window.display();
	}

	return 0;
}

void Physics()
{
	float timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	// UPDATE PHYSICS
	world_->Step(
		timeStep,
		velocityIterations,
		positionIterations);
}

bool Connect()
{
	// Create UDP socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("Client socket failed \n");
		return false;
	}

	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons(SERVERPORT);
	toAddr.sin_addr.s_addr = inet_addr(SERVERIP);

	if (!MessageSend(_message_type::CONNECT))
	{
		printf("Unable to send connect message");
		return false;
	}

	printf("Connect message sent successfully");
	return true;
}

bool MessageSend(_message_type mes_type_)
{
	fd_set writeable;
	FD_ZERO(&writeable);
	FD_SET(sock, &writeable);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;
	int count = select(0, NULL, &writeable, NULL, &timeout);
	if (count == SOCKET_ERROR)
	{
		printf("\nSelect failed\n");
	}
	else if (count == 0)
		printf("\nSelect returned no writeable\n");
	else
	{
		_message_server my_msg_;
		my_msg_.message_id_ = messages_sent_;
		my_msg_.type_ = mes_type_;

		switch (mes_type_)
		{
		case PLAYERID:{
			my_msg_.player_id_ = ID_;
			break;
		}
		case READY:{
			my_msg_.player_id_ = ID_;
			break;
		}
		case UPDATE:{
			my_msg_.player_id_ = ID_;
			my_msg_.position_[0].x = player_.car_gfx_.getPosition().x;
			my_msg_.position_[0].y = player_.car_gfx_.getPosition().y;
			my_msg_.position_[0].r = player_.car_gfx_.getRotation();
			for (int i = 0; i < player_.tire_gfx_.size(); i++)
			{
				my_msg_.position_[i + 1].x = player_.tire_gfx_[i].getPosition().x;
				my_msg_.position_[i + 1].y = player_.tire_gfx_[i].getPosition().y;
				my_msg_.position_[i + 1].r = player_.tire_gfx_[i].getRotation();
			}
			break;
		}
		case FINISH:{
			my_msg_.player_id_ = ID_;
			break;
		}
		}

		if (sendto(sock, (char*)&my_msg_, sizeof(my_msg_), 0,
			(const sockaddr *)&toAddr, sizeof(toAddr)) != sizeof(my_msg_))
		{
			return false;
		}
		else
		{
			messages_sent_++;
			return true;
		}
	}
}

void MessageRead()
{
	fd_set readable;
	FD_ZERO(&readable);
	FD_SET(sock, &readable);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;

	// Boolean used in do loop to read all available messages
	bool read_message_;

	do{
		read_message_ = false;

		int count = select(0, &readable, NULL, NULL, &timeout);
		if (count == SOCKET_ERROR)
		{
			printf("\nSelect failed\n");
		}
		else if (count == 1)
		{
			read_message_ = true;

			_message_server my_msg_;
			sockaddr_in fromAddr;
			int fromAddrSize = sizeof(fromAddr);
			int count = recvfrom(sock, (char*)&my_msg_, sizeof(my_msg_), 0,
				(sockaddr *)&fromAddr, &fromAddrSize);
			if (count < 0)
			{
				printf("\nMessage not received correctly\n");
			}
			else if (count != sizeof(my_msg_))
			{
				printf("\nMessage not correct size\n");
			}
			else if (last_ID_ < my_msg_.message_id_)
			{
				switch (my_msg_.type_)
				{
				case PLAYERID:{
					ID_ = my_msg_.player_id_;
					if (!MessageSend(PLAYERID))
						printf("\nUnable to confirm player ID\n");
					else
						printf("\nPlayer ID confirmation successful\n");
					break;
				}
				case RACER:{
					racer_conn_ = true;
					announcement_state_ = CONFIRMATION;
					break;
				}
				case READY:{
					//Begin countdown maybe?
					announcement_state_ = COUNTDOWN;
					break;
				}
				case UPDATE:{
					if ((rand() % 10) < miss_update_)
					{
						memcpy(racer_.previous_, racer_.next_, sizeof(racer_.next_));
						memcpy(racer_.next_, my_msg_.position_, sizeof(my_msg_.position_));
						racer_.CalculateDiff();
						last_.restart();
						last_update_ = last_.getElapsedTime();
					}
					break;
				}
				case FINISH:{
					announcement_state_ = FINISHED;
					break;
				}
				}
			}
			else
				printf("\nReceived old message\n");
		}
	} while (read_message_);
}

b2Body *CreateDynamicBody(float xPos, float yPos, float32 width, float32 height)
{
	b2Body* dynamic_body_;
	b2BodyDef bodyDef;
	b2PolygonShape dynamicBox;
	b2FixtureDef fixtureDef;
	b2Vec2 BodyInitPosition(xPos, yPos);

	bodyDef.type = b2_dynamicBody;
	bodyDef.position = BodyInitPosition;
	dynamic_body_ = world_->CreateBody(&bodyDef);

	dynamicBox.SetAsBox(width / 2, height / 2);

	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0.0f;
	dynamic_body_->CreateFixture(&fixtureDef);

	return dynamic_body_;
}

b2Body *CreateStaticBody(float xPos, float yPos, float32 width, float32 height)
{
	// setup the body definition
	b2Body* static_body_;
	b2BodyDef bodyDef;
	b2Vec2 StaticPosition(xPos, yPos);
	b2PolygonShape ground_shape;

	bodyDef.position = StaticPosition;

	static_body_ = world_->CreateBody(&bodyDef);

	ground_shape.SetAsBox(width / 2, height / 2);

	static_body_->CreateFixture(&ground_shape, 0.0f);

	return static_body_;
}

/*void CleanUp()
{
	delete world_;
	world_ = NULL;
}*/