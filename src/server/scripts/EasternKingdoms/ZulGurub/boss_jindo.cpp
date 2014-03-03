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
SDName: Boss_Jin'do the Hexxer
SD%Complete: 85
SDComment: Mind Control not working because of core bug. Shades visible for all.
SDCategory: Zul'Gurub
EndScriptData */

#include "ScriptPCH.h"
#include "zulgurub.h"


enum Texts{
	SAY_AGGRO = -1309014
};

enum Spells{

	SPELL_HEX = 24053,
	SPELL_DELUSIONSOFJINDO = 24306,
	SPELL_SUMMONBRAINWASHTOTEM = 24262,
	SPELL_SUMMONHEALINGWARD = 24309,

	//Shade
	SPELL_SHADOWSHOCK = 24458,
	SPELL_INVISIBLE = 24307,
	SPELL_SHADESHADOW = 24313,

	//Totems
	SPELL_BRAINWASH = 24261,
	SPELL_HEALWARD = 24311
};


struct boss_jindoAI : public ScriptedAI
{
    boss_jindoAI(Creature *c) : ScriptedAI(c) {}

    uint32 BrainWashTotem_Timer;
    uint32 HealingWard_Timer;
    uint32 Hex_Timer;
    uint32 Delusions_Timer;
    uint32 Teleport_Timer;

	std::vector<Creature*> Shades;

    void Reset()
    {
        BrainWashTotem_Timer = 20000;
        HealingWard_Timer = 16000;
        Hex_Timer = 8000;
        Delusions_Timer = 10000;
        Teleport_Timer = 5000;

		for (auto& i: Shades)
			i->DisappearAndDie();
    }

    void EnterCombat(Unit * /*who*/)
    {
        DoScriptText(SAY_AGGRO, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;


		//BrainWashTotem_Timer
        if (BrainWashTotem_Timer <= diff)
        {
        	DoCast(SPELL_SUMMONBRAINWASHTOTEM);
            BrainWashTotem_Timer = 18000 + rand()%8000;
        } else BrainWashTotem_Timer -= diff;
		
		
		//HealingWard_Timer
        if (HealingWard_Timer <= diff)
        {
            DoCast(SPELL_SUMMONHEALINGWARD);
            HealingWard_Timer = 14000 + rand()%6000;
        } else HealingWard_Timer -= diff;
		
        //Hex_Timer
        if (Hex_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_HEX);

            if (DoGetThreat(me->getVictim()))
                DoModifyThreatPercent(me->getVictim(),-80);

            Hex_Timer = 12000 + rand()%8000;
        } else Hex_Timer -= diff;
		
        //Casting the delusion curse with a shade. So shade will attack the same target with the curse.
        if (Delusions_Timer <= diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
            {
                DoCast(pTarget, SPELL_DELUSIONSOFJINDO);

                Creature *Shade = me->SummonCreature(14986, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 5000);
                if (Shade){
					Shade->AI()->AttackStart(pTarget);
					Shades.emplace_back(Shade);
				}
                    
            }

            Delusions_Timer = 4000 + rand()%8000;
        } else Delusions_Timer -= diff;
		
		
        //Teleporting a random player into the pit
        if (Teleport_Timer <= diff)
        {
            Unit *pTarget = NULL;
            pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if (pTarget && pTarget->GetTypeId() == TYPEID_PLAYER)
            {
                DoTeleportPlayer(pTarget, -11583.7783f,-1249.4278f, 77.5471f, 4.745f);

                if (DoGetThreat(me->getVictim()))
                    DoModifyThreatPercent(pTarget,-100);
            }

            Teleport_Timer = 15000 + rand()%8000;
        } else Teleport_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};


//BrainWash Totem
struct mob_brain_wash_totemAI : public ScriptedAI
{
	mob_brain_wash_totemAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    bool channel;

    ScriptedInstance *instance;

    void Reset()
	{
		channel = false;
	}

    void UpdateAI (const uint32 diff)
    {
        // Initiate Channel
        if (!channel)
        {
			// we need to select a unit within 100yards even though it is not on the threat list yet
			// is there an easier way for this?

			// get its owners threat table
			std::list<HostileReference*>& m_threatlist = me->GetCharmerOrOwner()->getThreatManager().getThreatList();

            // add players to its own list
            for (std::list<HostileReference*>::iterator itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
            {
				Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid());
					if (pTarget && pTarget->GetTypeId() == TYPEID_PLAYER)
						me->AddThreat(pTarget,0);
            }

			if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
               DoCast(pTarget, SPELL_BRAINWASH);
            channel = true;
        }
    }
};

//Healing Ward
struct mob_healing_wardAI : public ScriptedAI
{
    mob_healing_wardAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    uint32 Heal_Timer;

    ScriptedInstance *instance;

    void Reset()
    {
        Heal_Timer = 2000;
    }

    void UpdateAI (const uint32 diff)
    {
        //Heal_Timer
        if (Heal_Timer <= diff)
		{
			me->CastSpell((Unit*)NULL,SPELL_HEALWARD,false);
			//DoCast(SPELL_HEALWARD); //DoCast needs to support non targeted spells 
			Heal_Timer = 3000;
		} else Heal_Timer -= diff;
    }
};

//Shade of Jindo
struct mob_shade_of_jindoAI : public ScriptedAI
{
    mob_shade_of_jindoAI(Creature *c) : ScriptedAI(c) {}

    uint32 ShadowShock_Timer;

    void Reset()
    {
        ShadowShock_Timer = 1000;
        DoCast(me, SPELL_INVISIBLE, true);
		DoCast(me, SPELL_SHADESHADOW, true);
    }

    void UpdateAI (const uint32 diff)
    {

		//ShadowShock_Timer
		if (ShadowShock_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_SHADOWSHOCK);
            ShadowShock_Timer = 2000;
        } else ShadowShock_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};


CreatureAI* GetAI_boss_jindo(Creature* creature)
{
    return new boss_jindoAI (creature);
}

CreatureAI* GetAI_mob_brain_wash_totem(Creature* creature)
{
    return new mob_brain_wash_totemAI (creature);
}

CreatureAI* GetAI_mob_healing_ward(Creature* creature)
{
    return new mob_healing_wardAI (creature);
}

CreatureAI* GetAI_mob_shade_of_jindo(Creature* creature)
{
    return new mob_shade_of_jindoAI (creature);
}

void AddSC_boss_jindo()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_jindo";
    newscript->GetAI = &GetAI_boss_jindo;
    newscript->RegisterSelf();

	newscript = new Script;
    newscript->Name = "mob_brain_wash_totem";
    newscript->GetAI = &GetAI_mob_brain_wash_totem;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_healing_ward";
    newscript->GetAI = &GetAI_mob_healing_ward;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shade_of_jindo";
    newscript->GetAI = &GetAI_mob_shade_of_jindo;
    newscript->RegisterSelf();

}

