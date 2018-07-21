#include "b2dtire.h"

TDTire::TDTire(b2World* world)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	m_body = world->CreateBody(&bodyDef);

	b2PolygonShape polygonShape;
	polygonShape.SetAsBox(0.5f, 1.25f);
	m_body->CreateFixture(&polygonShape, 1);//shape, density

	m_body->SetUserData(this);
}

TDTire::~TDTire()
{
	m_body->GetWorld()->DestroyBody(m_body);
}

b2Vec2 TDTire::getLateralVelocity() 
{
	b2Vec2 currentRightNormal = m_body->GetWorldVector(b2Vec2(1, 0));
	return b2Dot(currentRightNormal, m_body->GetLinearVelocity()) * currentRightNormal;
}

b2Vec2 TDTire::getForwardVelocity() 
{
	b2Vec2 currentForwardNormal = m_body->GetWorldVector(b2Vec2(0, 1));
	return b2Dot(currentForwardNormal, m_body->GetLinearVelocity()) * currentForwardNormal;
}

void TDTire::updateFriction() 
{
	b2Vec2 impulse = m_body->GetMass() * -getLateralVelocity();
	m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);
	m_body->ApplyAngularImpulse(0.1f * m_body->GetInertia() * -m_body->GetAngularVelocity(), true);

	b2Vec2 currentForwardNormal = getForwardVelocity();
	float currentForwardSpeed = currentForwardNormal.Normalize();
	float dragForceMagnitude = -2 * currentForwardSpeed;
	m_body->ApplyForce(dragForceMagnitude * currentForwardNormal, m_body->GetWorldCenter(), true);
}

void TDTire::updateDrive(int vertical_) {
	//find desired speed
	float desiredSpeed = 0;
	switch (vertical_) {
	case 1:   desiredSpeed = m_maxForwardSpeed;  break;
	case 2: desiredSpeed = m_maxBackwardSpeed; break;
	default: return;//do nothing
	}

	//find current speed in forward direction
	b2Vec2 currentForwardNormal = m_body->GetWorldVector(b2Vec2(0, 1));
	float currentSpeed = b2Dot(getForwardVelocity(), currentForwardNormal);

	//apply necessary force
	float force = 0;
	if (desiredSpeed > currentSpeed)
		force = m_maxDriveForce;
	else if (desiredSpeed < currentSpeed)
		force = -m_maxDriveForce;
	else
		return;
	m_body->ApplyForce(force * currentForwardNormal, m_body->GetWorldCenter(), true);
}

void TDTire::setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse) {
	m_maxForwardSpeed = maxForwardSpeed;
	m_maxBackwardSpeed = maxBackwardSpeed;
	m_maxDriveForce = maxDriveForce;
	m_maxLateralImpulse = maxLateralImpulse;
}