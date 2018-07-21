// Box2D Tire class taken from iforce2d.net

#include "Box2D\Box2D.h"

class TDTire {
public:
	b2Body* m_body;
	float m_maxForwardSpeed;
	float m_maxBackwardSpeed;
	float m_maxDriveForce;
	float m_maxLateralImpulse;

	TDTire(b2World* world);
	~TDTire();

	b2Vec2 getLateralVelocity();
	b2Vec2 getForwardVelocity();
	void updateFriction();
	void updateDrive(int vertical_);
	void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse);
};

