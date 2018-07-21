// Box2D Car class taken from iforce2d.net

#include "Box2D/Box2D.h"
#include "b2dtire.h"
#include <vector>

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

class TDCar {
public:
	b2Body* m_body;
	std::vector<TDTire*> m_tires;
	b2RevoluteJoint *flJoint, *frJoint;
	b2Vec2 vertices[8];

	TDCar();
	TDCar(b2World* world);
	~TDCar();
	void update(int vertical_, int horizontal_);
};