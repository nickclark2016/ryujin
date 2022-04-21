#include <free_look_camera.hpp>

#include <ryujin/graphics/camera_component.hpp>
#include <ryujin/input/input.hpp>

#include <iostream>

free_look_camera::free_look_camera(const vec3<float> position, registry& reg)
{
	_yaw = _pitch = 0.0f;

	_entity = reg.allocate();
	_entity.assign(camera_component{0.01f, 1000.0f, 70.0f, invalid_slot_map_key, 0, true});

	set_position(_entity.get<transform_component>(), position);
}

void free_look_camera::on_update(double delta)
{
	const auto in = input::get_input();
	if (!in)
		return;

	transform_component& transform = _entity.get<transform_component>();

	// Movement
	if(in->get_keys().get_state(keyboard::key::W) != keyboard::state::RELEASED)
	{
		set_position(transform, transform.position + (_speed * extract_forward(transform.rotation)));
	}

	if (in->get_keys().get_state(keyboard::key::S) != keyboard::state::RELEASED)
	{
		set_position(transform, transform.position - (_speed * extract_forward(transform.rotation)));
	}

	if (in->get_keys().get_state(keyboard::key::A) != keyboard::state::RELEASED)
	{
		set_position(transform, transform.position + (_speed * cross(extract_forward(transform.rotation), extract_up(transform.rotation))));
	}

	if (in->get_keys().get_state(keyboard::key::D) != keyboard::state::RELEASED)
	{
		set_position(transform, transform.position - (_speed * cross(extract_forward(transform.rotation), extract_up(transform.rotation))));
	}

	// Rotation
	const auto deltaMouse = in->get_mouse().cursor_position_delta();

	_yaw += as<f32>(deltaMouse.x) * _sensitivityX * as<f32>(delta);	
	_yaw = fmod(_yaw, 360.0f);

	_pitch += as<f32>(deltaMouse.y) * _sensitivityY * as<f32>(delta);

	if (_constrainPitch)
	{
		_pitch = clamp(_pitch, -89.0f, 89.0f);
	}

	set_rotation(transform, as_radians(vec3(_pitch, _yaw, 0.0f)));

	if (std::isnan(_yaw))
	{
		std::cout << "YAW NAN 1" << std::endl;
	}
}

void free_look_camera::set_x_sensitivity(const f32 sensitivity)
{
	_sensitivityX = sensitivity;
}

void free_look_camera::set_y_sensitivity(const f32 sensitivity)
{
	_sensitivityY = sensitivity;
}

void free_look_camera::set_speed(const f32 speed)
{
	_speed = speed;
}

void free_look_camera::constrain_pitch(const bool constrain)
{
	_constrainPitch = constrain;
}
