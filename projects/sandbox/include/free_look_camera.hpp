#ifndef free_look_camera_hpp__
#define free_look_camera_hpp__

#include <ryujin/entities/registry.hpp>
#include <ryujin/math/vec3.hpp>

using namespace ryujin;

class free_look_camera
{
public:
	free_look_camera() = default;
	free_look_camera(vec3<float> position, registry& reg);

	void on_update(const double delta);

	void set_x_sensitivity(f32 sensitivity);
	void set_y_sensitivity(f32 sensitivity);

	void set_speed(f32 speed);

	void constrain_pitch(bool constrain);

private:
	u64 _padding;

	f32 _yaw;
	f32 _pitch;

	bool _constrainPitch = true;
	f32 _sensitivityX = 1.0f;
	f32 _sensitivityY = 1.0f;

	f32 _speed = 0.2f;

	entity_handle<registry::entity_type> _entity;
};

#endif // free_look_camera_h__