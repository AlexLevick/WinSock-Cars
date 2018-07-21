#include "b2dcar.h"

TDCar::TDCar()
{
	// do nothing
}

TDCar::TDCar(b2World* world)
{
	//create car body
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	m_body = world->CreateBody(&bodyDef);

	vertices[0].Set(1.5, 0);
	vertices[1].Set(3, 2.5);
	vertices[2].Set(2.8, 5.5);
	vertices[3].Set(1, 10);
	vertices[4].Set(-1, 10);
	vertices[5].Set(-2.8, 5.5);
	vertices[6].Set(-3, 2.5);
	vertices[7].Set(-1.5, 0);
	b2PolygonShape polygonShape;
	polygonShape.Set(vertices, 8);
	b2Fixture* fixture = m_body->CreateFixture(&polygonShape, 0.1f);//shape, density

	b2RevoluteJointDef jointDef;
	jointDef.bodyA = m_body;
	jointDef.enableLimit = true;
	jointDef.lowerAngle = 0;//with both these at zero...
	jointDef.upperAngle = 0;//...the joint will not move
	jointDef.localAnchorB.SetZero();//joint anchor in tire is always center

	float maxForwardSpeed = 50;
	float maxBackwardSpeed = -40;
	float backTireMaxDriveForce = 300;
	float frontTireMaxDriveForce = 500;
	float backTireMaxLateralImpulse = 8.5;
	float frontTireMaxLateralImpulse = 7.5;

	//back left tire
	TDTire* tire = new TDTire(world);
	tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
	jointDef.bodyB = tire->m_body;
	jointDef.localAnchorA.Set(-3, 0.75f);
	world->CreateJoint(&jointDef);
	m_tires.push_back(tire);

	//back right tire
	tire = new TDTire(world);
	tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
	jointDef.bodyB = tire->m_body;
	jointDef.localAnchorA.Set(3, 0.75f);
	world->CreateJoint(&jointDef);
	m_tires.push_back(tire);

	//front left tire
	tire = new TDTire(world);
	tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
	jointDef.bodyB = tire->m_body;
	jointDef.localAnchorA.Set(-3, 8.5f);
	flJoint = (b2RevoluteJoint*)world->CreateJoint(&jointDef);
	m_tires.push_back(tire);

	//front right tire
	tire = new TDTire(world);
	tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
	jointDef.bodyB = tire->m_body;
	jointDef.localAnchorA.Set(3, 8.5f);
	frJoint = (b2RevoluteJoint*)world->CreateJoint(&jointDef);
	m_tires.push_back(tire);
}

TDCar::~TDCar()
{
	m_tires.clear();
	m_body->GetWorld()->DestroyBody(m_body);
}

void TDCar::update(int vertical_, int horizontal_) {
	for (int i = 0; i < m_tires.size(); i++)
		m_tires[i]->updateFriction();
	for (int i = 0; i < m_tires.size(); i++)
		m_tires[i]->updateDrive(vertical_);

	float lockAngle = 40 * DEGTORAD;
	float turnSpeedPerSec = 320 * DEGTORAD;//from lock to lock in 0.25 sec
	float turnPerTimeStep = turnSpeedPerSec / 60.0f;
	float desiredAngle = 0;
	switch (horizontal_){
	case 1:  desiredAngle = lockAngle;  break;
	case 2: desiredAngle = -lockAngle; break;
	default:;//nothing
	}
	float angleNow = flJoint->GetJointAngle();
	float angleToTurn = desiredAngle - angleNow;
	angleToTurn = b2Clamp(angleToTurn, -turnPerTimeStep, turnPerTimeStep);
	float newAngle = angleNow + angleToTurn;
	flJoint->SetLimits(newAngle, newAngle);
	frJoint->SetLimits(newAngle, newAngle);
}