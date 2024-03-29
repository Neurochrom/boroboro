#pragma once
#include <memory>
#include "../MXLib/MXAnimation.h"
#include "../MXLib/MXWorld.h"

class EffectWithCooldown
{
public:
	EffectWithCooldown()
	{
		last_stamp = 0;
	}

	virtual void DoEffect()
	{

	}

	virtual DWORD GetCooldownTime()
	{
		return 100;
	}

	virtual bool CooldownElapsed()
	{
		if (last_stamp + GetCooldownTime() < MX::World::GetCurrentTime())
		{
			last_stamp = MX::World::GetCurrentTime();
			return true;
		}
		return false;
	}

	virtual bool DoThis()
	{
		if (CooldownElapsed())
		{
			DoEffect();
			return true;
		}
		return false;
	}

protected:
	DWORD last_stamp;
};


class EffectWithGivenCooldown : public EffectWithCooldown
{
public:
	EffectWithGivenCooldown(DWORD cool) : cooldown(cool)
	{
	}

	DWORD GetCooldownTime()
	{
		return cooldown;
	}

protected:
	DWORD cooldown;
};

class GraphicResources
{
public:
	GraphicResources();

	void Init(MX::Draw &draw);


	std::shared_ptr<MX::Animation> snake_head;
	std::shared_ptr<MX::Animation> snake_tongue;
	std::shared_ptr<MX::Animation> snake_body;

	std::shared_ptr<MX::Animation> snake_head2;
	std::shared_ptr<MX::Animation> snake_body2;

	
	std::shared_ptr<MX::Animation> stone1;
	
	
	std::shared_ptr<MX::Animation> fire;

	std::shared_ptr<MX::Animation> background;
	std::shared_ptr<MX::Animation> foreground;
#if 0
	std::shared_ptr<MX::Animation> grass[4];
#endif

	std::shared_ptr<MX::Animation> blood;
	std::shared_ptr<MX::Animation> berry;

	std::shared_ptr<MX::Animation> bottle;
	std::shared_ptr<MX::Animation> shield;

	std::shared_ptr<MX::Animation> pentagram;
	std::shared_ptr<MX::Animation> pentagram_static;
	std::shared_ptr<MX::Animation> pentagram_small;

	std::shared_ptr<MX::Animation> rotten_apple;
	std::shared_ptr<MX::Animation> poop;


	std::shared_ptr<MX::Animation> white_screen;

};

extern GraphicResources GraphicRes;