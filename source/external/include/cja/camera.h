#pragma once
#include "maths/matrix44.h"
#include "maths/vector2.h"

namespace abfw
{
	class SpriteRenderer;
	class Platform;
}

//Craig
class Camera
{
public:
	Camera(abfw::SpriteRenderer* renderer, abfw::Platform& platform);
	~Camera(void);

	// Camera tends towards the target location by the velocity amount each frame
	void TendTowards(abfw::Vector2 target, float velocity);

	// Move function that assumes the movement is relative to current position (relative transform)
	void MoveBy(abfw::Vector2 translation);
	// Move function that assumes the movement disregards current position (absolute transform)
	void MoveTo(abfw::Vector2 position);

	// Scale function that sets scale relative to original scale (absolute transform)
	void Scale(abfw::Vector2 scalefactor);

	// Rotate function that adds to the current rotation (relative transform)
	// Rotation currently does not work correctly
	//void Rotate(float deg);

	// Intensity of screen shake and duration (in seconds)
	void ScreenShake(int intensity, float duration);

	void UpdateCamera(const float& ticks);
	void SetActiveCamera();
	void ResetCamera();

private:

	int shaking_intensity_;
	float shaking_remaining_time_;
	abfw::Vector2 target_;
	float velocity_;
	bool tracking_;

	bool changed_; // Tracks if camera has been changed to stop unnecessary updates
	abfw::SpriteRenderer* renderer_; // pointer to sprite renderer for updating its projection matrix
	abfw::Vector2 translation_;
	abfw::Vector2 scale_;
	float rotation_;
	abfw::Matrix44 result_;
	const abfw::Platform& platform_;
};