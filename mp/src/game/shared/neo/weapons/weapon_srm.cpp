#include "cbase.h"
#include "weapon_srm.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSRM, DT_WeaponSRM)

BEGIN_NETWORK_TABLE(CWeaponSRM, DT_WeaponSRM)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponSRM)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponSRM)

LINK_ENTITY_TO_CLASS(weapon_srm, CWeaponSRM);

PRECACHE_WEAPON_REGISTER(weapon_srm);

CWeaponSRM::CWeaponSRM()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0;

	m_nNumShotsFired = 0;
}

void CWeaponSRM::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponSRM::Spawn()
{
	BaseClass::Spawn();
}

bool CWeaponSRM::Deploy(void)
{
	return BaseClass::Deploy();
}

void CWeaponSRM::UpdatePenaltyTime()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	if (((owner->m_nButtons & IN_ATTACK) == false) &&
		(m_flSoonestAttack < gpGlobals->curtime))
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp(m_flAccuracyPenalty, 0.0f, GetMaxAccuracyPenalty());
	}
}

void CWeaponSRM::ItemPreFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

void CWeaponSRM::ItemBusyFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

void CWeaponSRM::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if (m_bInReload)
	{
		return;
	}

	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	if (owner->m_nButtons & IN_ATTACK)
	{
		if (m_flSoonestAttack < gpGlobals->curtime)
		{
			if (m_iClip1 <= 0)
			{
				DryFire();

				m_flSoonestAttack = gpGlobals->curtime + GetFastestDryRefireTime();
			}
			else
			{
				m_flSoonestAttack = gpGlobals->curtime + GetFireRate();
			}
		}
	}
}

Activity CWeaponSRM::GetPrimaryAttackActivity()
{
	if (m_nNumShotsFired < 1)
	{
		return ACT_VM_PRIMARYATTACK;
	}

	if (m_nNumShotsFired < 2)
	{
		return ACT_VM_RECOIL1;
	}

	if (m_nNumShotsFired < 3)
	{
		return ACT_VM_RECOIL2;
	}

	return ACT_VM_RECOIL3;
}

void CWeaponSRM::AddViewKick()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	QAngle viewPunch;

	viewPunch.x = SharedRandomFloat("srmpx", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("srmpy", -0.6f, 0.6f);
	viewPunch.z = 0;

	owner->ViewPunch(viewPunch);
}
