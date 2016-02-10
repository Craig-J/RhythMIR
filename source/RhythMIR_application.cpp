#include "RhythMIR_application.h"

RhythMIR::RhythMIR(sf::VideoMode _video_mode = sf::VideoMode().getDesktopMode(), std::string _application_title = "RhythMIR") :
	Application(_video_mode, _application_title),
	machine_(*this)
{}

RhythMIR::~RhythMIR()
{}

bool RhythMIR::Initialize()
{
	machine_.Start();
	return false;
}

bool RhythMIR::CleanUp()
{
	machine_.Exit();
	return false;
}

bool RhythMIR::Update()
{
	machine_.Update(clock_.getLastFrameTime().asSeconds());
	return false;
}

void RhythMIR::Render()
{
	machine_.Render(clock_.getLastFrameTime().asSeconds());
}