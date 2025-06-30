#include "Application.h"

using namespace Wave;

class Sandbox : public Wave::IApplication
{
public:

	Sandbox() {}

	virtual void Startup() override;
	virtual void Cleanup() override;

	virtual void Update(float deltaT) override;
	virtual void RenderScene(void) override;

private:
};

CREATE_APPLICATION(Sandbox)

void Sandbox::Startup()
{

}

void Sandbox::Cleanup()
{

}

void Sandbox::Update(float deltaT)
{

}

void Sandbox::RenderScene()
{

}