#include "RhythMIR_application.h"

RhythMIR::RhythMIR(sf::VideoMode _video_mode, std::string _application_title) :
	Application(_video_mode, _application_title),
	machine_(window_, clock_, font_)
{}

RhythMIR::~RhythMIR()
{}

bool RhythMIR::Initialize()
{
	return machine_.Start();
}

void RhythMIR::CleanUp()
{
	machine_.Exit();
}

void RhythMIR::ProcessEvent(sf::Event& _event)
{
	machine_.ProcessEvent(_event);
}

bool RhythMIR::Update()
{
	return machine_.Update(clock_.getLastFrameTime().asSeconds());
}

void RhythMIR::Render()
{
	machine_.Render(clock_.getLastFrameTime().asSeconds());
}