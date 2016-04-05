#include "RhythMIR_application.h"

RhythMIR::RhythMIR(sf::RenderWindow& _window) :
	ImGuiApplication(_window),
	machine_(window_, clock_, font_, display_hud_)
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