#include "stdafx.h"
#include "GameBackground.h"
#include "GameResources.h"
#include "Collidable.h"
#include "Game.h"
#include "Sounds.h"
#include "../MXLib/MXScript.h"

using namespace MX;

MX::Scene *_scene;
shared_ptr<MX::Spriter> _spriter;

class WarpRotationCommand : public WaitCommand
{
public:
	WarpRotationCommand(float rotation, unsigned mili) :  WaitCommand(mili), dRotation(rotation) { }

	bool operator () (Actor &actor) 
	{
		if (_stamp == 0)
			aRotation = actor.rotation;
		bool bRet = WaitCommand::operator()(actor);
		D3DXCOLOR t;
		//actor.x = aRotation + (dRotation - aRotation) * GetPercent();
		return bRet;
	}



	protected:
		float aRotation;
		float dRotation;

	};

shared_ptr<Command> warp_rotation(float nR, unsigned mili)
{
	return shared_ptr<Command>(new WarpRotationCommand(nR, mili));
}



class Stone1 : public Collidable
{
public:
	Stone1(const v2d& p)
	{
		pos = p;
		scaleX = 0.5f;
		scaleY = 0.5f;
		animation = make_shared<SpecificAnimation>(GraphicRes.stone1);
		animation->Start();
	}
	
	virtual void onEat(Player* player) override
	{
		v2d normal = normalized(player->pos - pos);
		player->bounce(normal);
		return;
	}
};


class GameBackground : public ActorSprite
{
public:
	GameBackground()
	{
		z = 1.0f;
		pos.x = 0.0f;
		pos.y = 0.0f;
		animation = make_shared<SpecificAnimation>(GraphicRes.background);
		animation->Start();
	}
};

class GameForeground : public ActorSprite
{
public:
	GameForeground()
	{
		color = 0xFFFFFFFF;
		z = 0.0f;
		pos.x = 0.0f;
		pos.y = 0.0f;
		animation = make_shared<SpecificAnimation>(GraphicRes.foreground);
		animation->Start();
	}
};


#if 0
class GameBackgroundGrass : public ActorSprite
{
public:
	GameBackgroundGrass()
	{
	//	scaleY = 0.25f;
		//scaleX = 0.25f;
		auto loop = make_shared<LoopCommand>();

		shear_factor = ((float)(rand()%100))/200.0f;

		custom_transform = &shear_transform;

		OnDo.connect(loop);

		pos.x = (float)(rand()%1280);
		pos.y = (float)(rand()%800);
		z = 1.0f-pos.y/800;

		animation = make_shared<SpecificAnimation>(GraphicRes.grass[rand()%3]);
		//animation = make_shared<SpecificAnimation>(GraphicRes.grass[3]);
		animation->Start();
	}


	void Do()
	{
		D3DXMatrixIdentity(&shear_transform);
		shear_transform._21 = cos((float)GetTickCount()/1000)*shear_factor;
		shear_transform._41 = - (pos.y+16.0f)*shear_transform._21 + shear_transform._21;

		ActorSprite::Do();
	}


	float                        shear_factor;
	D3DXMATRIX                   shear_transform;
	/*
	*/
};

#endif


class Berry : public Collidable
{
public:
	Berry()
	{
		z = 0.9f;

		pos.x = (float)(rand()%1280);
		pos.y = (float)(rand()%800);

		animation = make_shared<SpecificAnimation>(GraphicRes.berry);
		animation->Start();


		OnDo.connect(MX::q(wait(5000), lerp_color(0x00FFFFFF, 1000), die()));
	}

	void onEat(Player* player)
	{
		SoundBank::apple_bite.Play();
		player->AddBodypart();
		player->AddBodypart();
		player->AddBodypart();
		Die();
	}
};

template<typename ItemType>
class BonusItem : public Collidable
{
public:
	BonusItem(SoundSample *sound, const shared_ptr<Animation> &anim, int time = 5000, int vanish_time = 3000)
	{
		gsound = sound;
		z = 0.9f;
		r = 16.0f;

		pos.x = 50.0f + (float)(rand()%1180);
		pos.y = 50.0f + (float)(rand()%700);

		animation = make_shared<SpecificAnimation>(anim);
		animation->Start();

		shared_ptr<MX::Command> com = MX::q(wait(time), lerp_color(0x00FFFFFF, vanish_time), die());
		OnDo.connect(com);
	}

	void onEat(Player* player)
	{
		if (gsound)
			gsound->Play();
		player->Item = make_shared<ItemType>();
		Die();
	}

	SoundSample *gsound;
};

class PentagramBonus : public Collidable
{
public:
	PentagramBonus()
	{
		z = 0.9f;

		scaleX = 0.2f;
		scaleY = 0.2f;

		OnDo.connect(MX::q(wait(5000), lerp_color(0x00FFFFFF, 1000), die()));


		animation = make_shared<SpecificAnimation>(GraphicRes.pentagram);
		animation->Start();

		auto loop = make_shared<LoopCommand> ();
		loop->AddCommand(make_shared<WarpScaleCommand>(0.23f, 0.23f, 250));
		loop->AddCommand(make_shared<WarpScaleCommand>(0.2f, 0.2f, 250));
		OnDo.connect(loop);
	}
	void Do(){
		rotation +=  0.005 * World::GetElapsedTime();
		ActorSprite::Do();
		GraphicRes.pentagram_static->Animate(*_spriter, pos.x, pos.y, z , 0.0f, scaleX, scaleY);
	}

	void onEat(Player* player)
	{
		SoundBank::satan_hissing.Play();
		player->Item = make_shared<PentagramItem>();
		Die();
	}
};


class BerrySpawner : public Actor, public EffectWithCooldown
{
public:
	DWORD GetCooldownTime()
	{
		return 3000 * ((float)(rand()%100)/100.0f) * 2000;
	}


	void DoEffect()
	{
		_scene->AddActor(make_shared<Berry>());
	}

	void Do()
	{
		if (CooldownElapsed())
			DoEffect();
	}
};


class BonusSpawner : public Actor, public EffectWithCooldown
{
public:
	DWORD GetCooldownTime()
	{
		return 20000 * ((float)(rand()%100)/100.0f) * 5000;
	}


	void DoEffect()
	{
		float x_rand = 50.0f + (float)(rand()%1180);
		float y_rand = 50.0f + (float)(rand()%700);

		shared_ptr<Collidable> pItem;
		switch (rand() % 10)
		{
		case 0:
		case 1:
		case 2:
			pItem = make_shared<BonusItem<GoodBootleItem>>(&SoundBank::gulp, GraphicRes.bottle, 5000, 3000);
			break;
		case 3:
		case 4:
		case 5:
			pItem = make_shared<BonusItem<PoopItem>>(&SoundBank::apple_bite, GraphicRes.rotten_apple, 5000, 3000);
			break;
		case 6:
		case 7:
		case 8:			
			pItem = make_shared<BonusItem<ShieldItem>>(&SoundBank::apple_bite, GraphicRes.shield, 5000, 3000);
			break;
		case 9:
			pItem = make_shared<PentagramBonus>();
			break;
		}

		pItem->pos.x = x_rand;
		pItem->pos.y = y_rand;

		int nTries = 3;
		while (sw.doesCollide(pItem.get()))
		{
			x_rand = (float)(rand()%1280);
			y_rand = (float)(rand()%800);

			pItem->pos.x = x_rand;
			pItem->pos.y = y_rand;

			if (--nTries == 0)
				return;
		}

		_scene->AddActor(pItem);



	}

	void Do()
	{
		if (CooldownElapsed())
			DoEffect();
	}
};


void InitBackground(MX::Scene *scene, const shared_ptr<MX::Spriter> &spriter)
{
	_spriter = spriter;
	_scene = scene;


	scene->AddActor(make_shared<Stone1>(v2d(600.0f, 600.0f)));
	scene->AddActor(make_shared<GameBackground>());
	scene->AddActor(make_shared<BerrySpawner>());
	scene->AddActor(make_shared<BonusSpawner>());
	scene->AddActor(make_shared<GameForeground>());
	
	for (int i = 0; i < 5; i ++)
		scene->AddActor(make_shared<Stone1>(v2d((float)(rand()%1280), (float)(rand()%800))));

	//for (int i = 0; i < 10; i ++)
	//	scene->AddActor(make_shared<GameBackgroundGrass>());
		
}