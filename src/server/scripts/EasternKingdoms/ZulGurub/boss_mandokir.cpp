 /*
  * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
  * Copyright (C) 2010-2013 Oregon <http://www.oregoncore.com/>
  * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
  * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
  *
  * This program is free software; you can redistribute it and/or modify it
  * under the terms of the GNU General Public License as published by the
  * Free Software Foundation; either version 2 of the License, or (at your
  * option) any later version.
  *
  * This program is distributed in the hope that it will be useful, but WITHOUT
  * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
  * more details.
  *
  * You should have received a copy of the GNU General Public License along
  * with this program. If not, see <http://www.gnu.org/licenses/>.
  */

/* ScriptData
SDName: Boss_Mandokir
SDScripter: Celania
SD%Complete: 100
SDComment: Needs to be verified on a larger scale but testing has resulted in no errors.
SDCategory: Zul'Gurub
EndScriptData */

#include "ScriptPCH.h"
#include "zulgurub.h"

enum Texts
{
    SAY_AGGRO           =   -1309015,
    SAY_DING_KILL       =   -1309016,
    SAY_GRATS_JINDO     =   -1309017,
    SAY_GAZE            =   -1309018,
    SAY_GAZE_WHISPER    =   -1309019,
	SAY_REVIVE          =   -1309024
};

enum Spells
{
    SPELL_CHARGE          = 24408,
	SPELL_CHARGE_GAZE     = 24315,
    SPELL_CLEAVE          = 7160,
    SPELL_FEAR            = 29321,
    SPELL_WHIRLWIND       = 15589,
    SPELL_MORTAL_STRIKE   = 16856,
    SPELL_ENRAGE          = 24318,
    SPELL_GAZE            = 24314,
    SPELL_LEVEL_UP        = 24312,
    SPELL_MOUNT           = 23243,
	SPELL_REVIVE          = 24341,
    SPELL_GUILLOTINE      = 24316,

    // Ohgan's Spells
    SPELL_SUNDERARMOR     = 24317,
	SPELL_THRASH          = 3391,
    SPELL_EXECUTE         = 7160
};

enum Summons
{
    OHGAN                 = 14988,
	CHAINED_SPIRIT        = 15117
};

float CHAINED_SPIRIT_LOC [15][4] =
{
    {-12272.1f, -1942.250f, 135.231f, 0.56704f},
    {-12277.2f, -1935.940f, 136.778f, 0.26859f},
    {-12282.2f, -1920.320f, 131.593f, 6.03892f},
    {-12283.4f, -1928.490f, 135.307f, 0.09580f},
    {-12246.1f, -1893.090f, 134.157f, 4.84119f},
    {-12254.5f, -1895.100f, 133.670f, 4.79799f},
    {-12262.5f, -1899.160f, 131.824f, 5.24567f},
    {-12263.5f, -1945.960f, 132.431f, 0.64166f},
    {-12258.0f, -1951.820f, 131.170f, 0.77125f},
    {-12250.6f, -1958.700f, 132.913f, 0.85371f},
    {-12241.2f, -1966.670f, 134.090f, 1.17652f},
    {-12232.5f, -1972.890f, 132.928f, 1.10191f},
    {-12220.4f, -1973.710f, 132.390f, 1.52367f},
    {-12209.5f, -1973.580f, 132.171f, 1.78677f},
    {-12197.1f, -1974.770f, 131.102f, 1.45926f}
};

// Mandokir
struct boss_mandokirAI : public ScriptedAI
{
    boss_mandokirAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    uint32 KillCount;
    uint32 Gaze_Timer;
    uint32 TargetInRange;
    uint32 Cleave_Timer;
    uint32 Whirlwind_Timer;
    uint32 Fear_Timer;
    uint32 MortalStrike_Timer;
    uint32 Check_Timer;
	uint32 Charge_Timer; 
    float targetX;
    float targetY;
    float targetZ;

	float Threat_Count;
    bool  Threat_stored;

    ScriptedInstance *instance;

    bool endGaze;
    bool someGazed;
    bool RaptorDead;
    bool CombatStart;

    uint64 GazeTarget;

    void Reset()
    {
        KillCount = 0;
        Gaze_Timer = 33000;
        Cleave_Timer = 7000;
        Whirlwind_Timer = 20000;
        Fear_Timer = 1000;
        MortalStrike_Timer = 1000;
        Check_Timer = 1000;
		Charge_Timer = 20000;

		Threat_Count = 0;
        Threat_stored = false;

		if(instance)
			instance->SetData(TYPE_MANDOKIR,NOT_STARTED);

        targetX = 0.0f;
        targetY = 0.0f;
        targetZ = 0.0f;
        TargetInRange = 0;

        GazeTarget = 0;

        someGazed = false;
        endGaze = false;
        RaptorDead = false;
        CombatStart = false;

		if (instance)
			instance->SetData64(DATA_SPIRIT_CLEAR,0);

        DoCast(me, SPELL_MOUNT);
    }

	void JustDied(Unit* /*killer*/)
	{
		if (instance)
		{
			instance->SetData64(DATA_SPIRIT_CLEAR,0);
			instance->SetData(TYPE_MANDOKIR,DONE);
		}
	}

    void KilledUnit(Unit* victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
        {
            ++KillCount;

			instance->SetData64(DATA_SPIRIT_REVIVE,victim->GetGUID());

            if (KillCount == 3)
            {
                DoScriptText(SAY_DING_KILL, me);

                if (instance)
                {
                    if (uint64 JindoGUID = instance->GetData64(DATA_JINDO))
                    {
                        if (Unit* jTemp = Unit::GetUnit(*me, JindoGUID))
                        {
                            if (jTemp->isAlive())
                                jTemp->MonsterYellToZone(SAY_GRATS_JINDO,LANG_UNIVERSAL,me->GetGUID());
                                //DoScriptText(SAY_GRATS_JINDO, jTemp);
                        }
                    }
                }
                DoCast(me, SPELL_LEVEL_UP, true);
                KillCount = 0;
            }
        }
    }

    void EnterCombat(Unit * /*who*/)
    {
        DoScriptText(SAY_AGGRO, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->getVictim() && me->isAlive())
        {
            if (!CombatStart)
            {
                // At combat Start Mandokir is mounted so we must unmount it first
                me->Unmount();

                // And summon his raptor
                me->SummonCreature(OHGAN, me->getVictim()->GetPositionX(), me->getVictim()->GetPositionY(), me->getVictim()->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                CombatStart = true;

				if (instance)
					instance->SetData(TYPE_MANDOKIR,IN_PROGRESS);

				// Summon Spirits
				uint32 i;
                for (i = 0; i < 15; ++i)
					me->SummonCreature(CHAINED_SPIRIT, CHAINED_SPIRIT_LOC[i][0], CHAINED_SPIRIT_LOC[i][1], CHAINED_SPIRIT_LOC[i][2], CHAINED_SPIRIT_LOC[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN , 5000);
            }

			// Random Charge
            if (Charge_Timer <= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                {
                    if (DoGetThreat(me->getVictim()))
                        DoModifyThreatPercent(me->getVictim(), -50);

                    DoCast(pTarget, SPELL_CHARGE);
                    AttackStart(pTarget);
                }
                Charge_Timer = 20000;
            }
            else
                Charge_Timer -= diff;


            if (Gaze_Timer <= diff)                         // Every 20 seconds Mandokir will check this
            {
                if (GazeTarget)
                {
                    Unit* pUnit = Unit::GetUnit(*me, GazeTarget);
                    if (pUnit && pUnit->isInCombat() && DoGetThreat(pUnit) > Threat_Count)
                    {
                        DoCast(pUnit, SPELL_CHARGE_GAZE);
                        AttackStart(pUnit);
                        DoCast(pUnit, SPELL_GUILLOTINE);
                        Threat_Count = 0.0f;
                    }
                }
                someGazed = false;
                Gaze_Timer = 20000;
            }
            else
                Gaze_Timer -= diff;

            if (Gaze_Timer < 8000 && !someGazed)            // 8 second(cast time + expire time) before the check for the gaze effect Mandokir will cast gaze debuff on a random target
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                {
                    DoScriptText(SAY_GAZE, me, pTarget);
                    DoCast(pTarget, SPELL_GAZE);
                    me->MonsterWhisper(SAY_GAZE_WHISPER, pTarget->GetGUID());
                    GazeTarget = pTarget->GetGUID();
                    someGazed = true;
                    endGaze = true;
                    Threat_stored = false;
                }
            }

            if (Gaze_Timer < 6000 && !Threat_stored)
            {
                Unit* pThreat = Unit::GetUnit(*me, GazeTarget);
                Threat_Count = DoGetThreat(pThreat);                // store threat when you're gazed
                Threat_stored = true;
            }

            if (Gaze_Timer < 1000 && endGaze)               // 1 second before the debuff expires, check whether the GazeTarget is in LoS
            {
                Unit* pSight = Unit::GetUnit(*me, GazeTarget);
                Player* pPlayer = Player::GetPlayer(*me, pSight->GetGUID());
                if (pSight && !me->IsWithinLOSInMap(pSight))  // Is the target in our LOS? If not, teleport to me:
                    pPlayer->TeleportTo(pPlayer->GetMapId(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

                endGaze = false;
            }

            if (!someGazed)
            {
                // Cleave
                if (Cleave_Timer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_CLEAVE);
                    Cleave_Timer = 7000;
                }
                else
                    Cleave_Timer -= diff;

                // Whirlwind
                if (Whirlwind_Timer <= diff)
                {
                    DoCast(me, SPELL_WHIRLWIND);
                    Whirlwind_Timer = 18000;
                }
                else
                    Whirlwind_Timer -= diff;

                // If more than 3 targets in melee range Mandokir will cast fear
                if (Fear_Timer <= diff)
                {
                    TargetInRange = 0;

                    std::list<HostileReference*>::const_iterator i = me->getThreatManager().getThreatList().begin();
                    for (; i != me->getThreatManager().getThreatList().end(); ++i)
                    {
                        Unit* pUnit = Unit::GetUnit(*me, (*i)->getUnitGuid());
                        if (pUnit && me->IsWithinMeleeRange(pUnit))
                            ++TargetInRange;
                    }

                    if (TargetInRange > 3)
                        DoCast(me->getVictim(), SPELL_FEAR);

                    Fear_Timer = 4000;
                }
                else
                    Fear_Timer -=diff;

                // Mortal Strike if target is below 50% hp
                if (me->getVictim() && me->getVictim()->GetHealth() < me->getVictim()->GetMaxHealth() * 0.5f)
                {
                    if (MortalStrike_Timer <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_MORTAL_STRIKE);
                        MortalStrike_Timer = 15000;
                    }
                    else
                        MortalStrike_Timer -= diff;
                }
            }

            // Checking if Ohgan is dead. If yes Mandokir will enrage.
            if (Check_Timer <= diff)
            {
                if (instance)
                {
                    if (instance->GetData(TYPE_OHGAN) == DONE)
                    {
                        if (!RaptorDead)
                        {
                            DoCast(me, SPELL_ENRAGE);
                            RaptorDead = true;
                        }
                    }
                }

                Check_Timer = 1000;
            }
            else
                Check_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    }
};

// Ohgan
struct mob_ohganAI : public ScriptedAI
{
    mob_ohganAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    uint32 SunderArmor_Timer;
	uint32 Thrash_Timer;
    uint32 Execute_Timer;
    ScriptedInstance *instance;

    void Reset()
    {
        SunderArmor_Timer = 5000;
        Thrash_Timer = urand(5000, 9000);
        Execute_Timer = 1000;
    }

	void KilledUnit(Unit* victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
			instance->SetData64(DATA_SPIRIT_REVIVE,victim->GetGUID());
	}
		
	void EnterCombat(Unit * /*who*/) {}

    void JustDied(Unit* /*Killer*/)
    {
        if (instance)
            instance->SetData(TYPE_OHGAN, DONE);
    }

    void UpdateAI (const uint32 diff)
    {
        // Return since we have no target
        if (!UpdateVictim())
            return;

        // SunderArmor_Timer
        if (SunderArmor_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_SUNDERARMOR);
            SunderArmor_Timer = 10000 + rand()%5000;
        }
        else
            SunderArmor_Timer -= diff;

        // Thrash_Timer
        if (Thrash_Timer <= diff)
        {
            DoCast(me, SPELL_THRASH);
            Thrash_Timer = urand(5000, 9000);
        }
        else
            Thrash_Timer -= diff;

        // Execute_Timer
        if (me->getVictim()->GetHealth() <= me->getVictim()->GetMaxHealth() * 0.2f)  // check health first
        {
            if (Execute_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_EXECUTE);
                Execute_Timer = 10000;
            }
            else
                Execute_Timer -= diff;
        }
        else
            Execute_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

// Chained Spirit
struct mob_chained_spiritAI: public ScriptedAI{

	mob_chained_spiritAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }
	
    ScriptedInstance *instance;

	Unit* target;
	bool hasRezzed;
	uint32 Move_Timer;

	void Reset()
    {
        target = NULL;
		hasRezzed = false;
		Move_Timer = 2000;
    }
	
	void UpdateAI(const uint32 diff)
    {
		if (instance && instance->GetData(TYPE_MANDOKIR) != IN_PROGRESS)
		{
			me->RemoveFromWorld();
			return;
		}
				
		if (Move_Timer <= diff){

			if (target == NULL)
				//if ( !Dead_Player.empty())
				if (Unit *player = Unit::GetUnit(*me,instance->GetData64(DATA_SPIRIT_REVIVE)))
				{
					target = player;
					me->GetMotionMaster()->MovePoint(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());						
				}
				else
					return;
			else
			{
				if (target->isAlive()){
					me->RemoveFromWorld();
					return;
				}
				else
				{
					if (me->IsWithinDist3d(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 10.0f) && !hasRezzed)
					{
						me->StopMoving();
						me->MonsterWhisper(SAY_REVIVE, target->GetGUID());
						Player* pPlayer = Player::GetPlayer(*me, target->GetGUID());
						DoCast(pPlayer, SPELL_REVIVE);
						hasRezzed = true;
					}
				}
			}
			Move_Timer = 2000;
		}
		else
			Move_Timer -= diff;


	}
};


CreatureAI* GetAI_boss_mandokir(Creature* creature)
{
    return new boss_mandokirAI (creature);
}

CreatureAI* GetAI_mob_ohgan(Creature* creature)
{
    return new mob_ohganAI (creature);
}

CreatureAI* GetAI_mob_chained_spirit(Creature* creature)
{
    return new mob_chained_spiritAI (creature);
}

void AddSC_boss_mandokir()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_mandokir";
    newscript->GetAI = &GetAI_boss_mandokir;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ohgan";
    newscript->GetAI = &GetAI_mob_ohgan;
    newscript->RegisterSelf();

	newscript = new Script;
    newscript->Name = "mob_chained_spirit";
    newscript->GetAI = &GetAI_mob_chained_spirit;
    newscript->RegisterSelf();


}

