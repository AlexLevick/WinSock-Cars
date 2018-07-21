#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include "utils.h"
#include <vector>

// The IP address for the server
#define SERVERIP "127.0.0.1"

// The UDP port number for the server
#define SERVERPORT 4444

#define NUMPLAYERS 2

enum _message_type{
	CONNECT,
	PLAYERID,
	RACER,
	READY,
	UPDATE,
	FINISH,
};

struct _position{

	float x, y, r;
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

struct _connected{
	int player_id_;
	sockaddr_in address_;
	int last_id_;
	bool read_;
	// Position for car and 4 tires
	_position position_[5];
	bool updated_;
};

bool MessageSend(_message_type, int ID_);
void MessageRead();

SOCKET sock;
sockaddr_in serverAddr;
// As client checks every message for if it is old
int messages_sent_ = 1;
std::vector<_connected> players_;
bool send_updates_ = false;

int main()
{
	printf("Network Race Server\n");

	startWinSock();

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("Socket failed\n");
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);

	if (bind(sock, (const sockaddr *)&serverAddr, sizeof(serverAddr)) != 0)
	{
		printf("Bind failed\n");
	}

	while (true)
	{
		// if 100ms has passed
		{
			MessageRead();
			if (send_updates_)
			{
				for (int i = 0; i < NUMPLAYERS; i++)
				{
					MessageSend(UPDATE, i);
				}
			}
		}
	}
}

bool MessageSend(_message_type mes_type_, int ID_)
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
		my_msg_.player_id_ = ID_;
		my_msg_.type_ = mes_type_;

		switch (mes_type_)
		{
		case PLAYERID:{
			break;
		}
		case RACER:{
			break;
		}
		case READY:{
			break;
		}
		case UPDATE:{
			for (int i = 0; i < NUMPLAYERS; i++)
			{
				// Do not to send player their own position
				// Do not send an update for a player if we have not received one
				if (ID_ != i)
				{
					if (players_[i].read_)
					{
						memcpy(my_msg_.position_, players_[i].position_, sizeof(players_[i].position_));
						players_[i].read_ = false;
					}
					else
						return false;
				}
			}
			break;
		}
		case FINISH:{
			break;
		}
		}

		if (sendto(sock, (char*)&my_msg_, sizeof(my_msg_), 0,
			(const sockaddr *)&players_[ID_].address_, sizeof(players_[ID_].address_)) != sizeof(my_msg_))
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
	int count = select(0, &readable, NULL, NULL, &timeout);
	if (count == SOCKET_ERROR)
	{
		printf("\nSelect failed\n");
	}
	else
	{
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
		else
		{
			switch (my_msg_.type_)
			{
			case CONNECT:{
				_connected temp_;
				temp_.address_ = fromAddr;
				temp_.player_id_ = players_.size();
				temp_.last_id_ = my_msg_.message_id_;
				temp_.read_ = false;
				temp_.updated_ = false;
				players_.push_back(temp_);
				if (!MessageSend(PLAYERID, players_.size() - 1))
				{
					// If unable to send player messages, remove them from connected list
					printf("\nPlayer ID message not sent correctly\n");
					players_.pop_back();
				}
				else
					printf("\nPlayer ID message successful\n");
				break;
			}
			case PLAYERID:{
				players_[my_msg_.player_id_].read_ = true;

				// If enough players have connected and all players have confirmed player ID
				// then tell all players that players have connected and client start sending updates
				if (players_.size() == NUMPLAYERS)
				{
					bool all_read_ = true;

					for (int i = 0; i < NUMPLAYERS; i++)
					{
						if (!players_[i].read_)
							all_read_ = false;
					}

					if (all_read_)
					{
						for (int i = 0; i < NUMPLAYERS; i++)
						{
							MessageSend(RACER, i);
						}
						send_updates_ = true;
					}
				}

				// Note message ID as last message received
				players_[my_msg_.player_id_].last_id_ = my_msg_.message_id_;
				break;
			}
			case READY:{
				//Begin countdown maybe?
				// Note message ID as last message received
				players_[my_msg_.player_id_].last_id_ = my_msg_.message_id_;
				break;
			}
			case UPDATE:{
				// Use memcpy as unable to assign whole arrays
				// Check the message isn't old
				if (players_[my_msg_.player_id_].last_id_ < my_msg_.message_id_)
				{
					memcpy(players_[my_msg_.player_id_].position_, my_msg_.position_, sizeof(my_msg_.position_));
					// Note message ID as last message received
					players_[my_msg_.player_id_].last_id_ = my_msg_.message_id_;
					players_[my_msg_.player_id_].read_ = true;
				}
				
			}
			case FINISH:{
				// Note timestamp and wait for other racer to finish, compare timestamps and announce winner
				// Note message ID as last message received
				players_[my_msg_.player_id_].last_id_ = my_msg_.message_id_;
				break;
			}
			}
		}
	}
}