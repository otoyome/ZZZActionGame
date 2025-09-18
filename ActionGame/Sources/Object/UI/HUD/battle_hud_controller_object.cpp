#include "object/ui/hud/battle_hud_controller_object.h"

namespace
{
	constexpr std::string_view DAMAGE_CALLBACK = "DAMAGE_CALLBACK";
	constexpr std::string_view STAGGER_END_CALLBACK = "STAGGER_END_CALLBACK";
	constexpr std::string_view ENTER_SPECIAL_CALLBACK = "ENTER_SPECIAL_CALLBACK";
}

battle_hud_controller_object::battle_hud_controller_object(object_descs desc)
: game_object(desc.name)
, setup_player_datas(false)
, setup_enemy_datas(false)
, player_hp_bar(desc.player_hp_bar)
, player_sp_bar(desc.player_sp_bar)
, player_face_image(desc.player_face_image)

, attack_icon(desc.attack_icon)
, special_icon(desc.special_icon)
, dodge_icon(desc.dodge_icon)
, parry_icon(desc.parry_icon)

, boss_hp_bar(desc.boss_hp_bar)
, boss_trunk_bar(desc.boss_trunk_bar)
, boss_face_image(desc.boss_face_image)

, timer(desc.timer)

, direction(static_cast<fade::directions>(-1))
, fade_state(static_cast<fade::states>(-1))
, fade_animation_time(0)
, fade_animation_max_time(0)
{
}

battle_hud_controller_object::~battle_hud_controller_object()
{
}

void battle_hud_controller_object::post_update(float elapsed_time)
{
	if (fade_state == fade::states::processing)
	{
		fade_animation_time += elapsed_time;
		float rate = 0.0f;
		switch (direction)
		{
			case fade::directions::in:
			{
				math::compute_fade::in_calculator calculator({ 0.0f, fade_animation_max_time });
				rate = calculator.compute(fade_animation_time);
				break;
			}
			case fade::directions::out:
			{
				math::compute_fade::out_calculator calculator({ 0.0f, fade_animation_max_time });
				rate = calculator.compute(fade_animation_time);
				break;
			}
		}

		player_hp_bar->set_alpha(rate);
		player_sp_bar->set_alpha(rate);
		player_face_image->set_alpha(rate);

		attack_icon->set_alpha(rate);
		special_icon->set_alpha(rate);
		dodge_icon->set_alpha(rate);
		parry_icon->set_alpha(rate);

		boss_hp_bar->set_alpha(rate);
		boss_trunk_bar->set_alpha(rate);
		boss_face_image->set_alpha(rate);

		timer->set_alpha(rate);

		if (fade_animation_max_time < fade_animation_time)
			fade_state = fade::states::idle;
	}

	{
		if (!setup_player_datas)
		{
			auto player = object_manager::instance()->search_object<player_controller_object>();
			if (player)
			{
				player_object = player;
			//	player_hp_bar->set_value(player->get_controll_object()->get_health());
			//	player_hp_bar->set_max_value(player->get_controll_object()->get_max_health());
				player_hp_bar->set_character(player->get_controll_object());
				player_sp_bar->set_character(player->get_controll_object());

				player->get_controll_object()->register_damage_callback(DAMAGE_CALLBACK,
					[this](const character_object::damage_datas& data)
					{
						on_damage_player(data);
					});

				//	受付
				player->get_controll_object()->register_damage_callback(DAMAGE_CALLBACK,
					[this](const character_object::damage_datas& data)
					{
						on_damage_player(data);
					});
				player->get_controll_object()->register_change_state_callback(character_object::state_names[character_object::state_special],
					ENTER_SPECIAL_CALLBACK,
					[this](const character_object* data)
					{
						special_icon->press();
					});

				player_face_image->set_image(player->get_controll_object()->get_face_texture(), player->get_controll_object()->get_face_uv());
				setup_player_datas = true;
			}
		}
		else
		{
			if (auto player = player_object.lock())
			{
				auto character = player->get_controll_object();

				//attack_icon->set_alpha(rate);

				//	特殊使用可能判定
				bool enabled = character->get_special_power() - character->get_use_special_power() >= 0;
				special_icon->set_enabled(enabled);

				//dodge_icon->set_alpha(rate);

				//parry_icon->set_alpha(rate);
			}
			else
			{
				setup_player_datas = false;
			}
		}
	}

	{
		enemy_controller_object::enemy_type_data boss_data = { enemy_controller_object::enemy_type::enemy, INT_MAX };
		std::shared_ptr<enemy_controller_object> boss;
		auto enemy_objects = object_manager::instance()->search_objects<enemy_controller_object>();
		for (auto& enemy_object : enemy_objects)
		{
			if (!enemy_object->is_active())
				continue;

			auto enemy_type_data = enemy_object->get_enemy_type();
			if (enemy_type_data.first == enemy_controller_object::enemy_type::boss)
			{
				bool insert = true;
				if (boss_data.first == enemy_controller_object::enemy_type::boss && enemy_type_data.second > boss_data.second)
				{
					insert = false;
				}
				if (insert)
				{
					boss_data = enemy_type_data;
					boss = enemy_object;
				}
			}
		}
		if (!setup_enemy_datas && boss)
		{
			boss_object = boss;
			//boss_hp_bar->set_value(boss->get_controll_object()->get_health());
			//boss_hp_bar->set_max_value(boss->get_controll_object()->get_max_health());
			boss_hp_bar->set_character(boss->get_controll_object());
			boss_trunk_bar->set_character(boss->get_controll_object());

			boss->get_controll_object()->register_damage_callback(DAMAGE_CALLBACK,
				[this](const character_object::damage_datas& data)
				{
					on_damage_boss(data);
				});

			boss->get_controll_object()->register_change_state_callback(character_object::state_names[character_object::state_stagger_complete], STAGGER_END_CALLBACK,
				[this](character_object*)
				{
					on_damage_boss_stagger_end();
				});

			boss_face_image->set_image(boss->get_controll_object()->get_face_texture(), boss->get_controll_object()->get_face_uv());
			setup_enemy_datas = true;
		}
	}
}

void battle_hud_controller_object::play_fade(fade::directions direction, float time)
{
	if (this->direction == direction)
		return;

	this->direction = direction;
	this->fade_state = fade::states::processing;
	this->fade_animation_time = 0;
	this->fade_animation_max_time = time;
}

void battle_hud_controller_object::on_finalize()
{
	if (auto player = player_object.lock())
	{
		player->get_controll_object()->unregister_damage_callback(DAMAGE_CALLBACK);
	}
	if (auto boss = boss_object.lock())
	{
		boss->get_controll_object()->unregister_damage_callback(DAMAGE_CALLBACK);
	}
}

void battle_hud_controller_object::on_damage_player(const character_object::damage_datas& data)
{
	auto player = player_object.lock();
	if (!player)
	{
		setup_player_datas = false;
		return;
	}

	//player_hp_bar->change_value(player->get_controll_object()->get_health());
	player_hp_bar->call_shake({ data.desc.camera_shake_time, data.desc.camera_shake_power });
	player_sp_bar->call_shake({ data.desc.camera_shake_time, data.desc.camera_shake_power });

	player_face_image->call_shake({ data.desc.camera_shake_time, data.desc.camera_shake_power });
}

void battle_hud_controller_object::on_damage_boss(const character_object::damage_datas& data)
{
	auto boss = boss_object.lock();
	if (!boss)
	{
		setup_enemy_datas = false;
		return;
	}

	//boss_hp_bar->change_value(boss->get_controll_object()->get_health());
	boss_hp_bar->call_shake({ data.desc.camera_shake_time, data.desc.camera_shake_power });

	boss_trunk_bar->call_shake({ data.desc.camera_shake_time, data.desc.camera_shake_power });
	boss_face_image->call_shake({ data.desc.camera_shake_time, data.desc.camera_shake_power });
}

void battle_hud_controller_object::on_damage_boss_stagger_end()
{
	auto boss = boss_object.lock();
	if (!boss)
	{
		setup_enemy_datas = false;
		return;
	}

//	boss_trunk_bar->change_value(boss->get_controll_object()->get_max_trunk() - boss->get_controll_object()->get_trunk());
}
