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
SDName: Boss_Hakkar
SD%Complete: 100
SDComment: Removal of Poison (Spell 24321) on Blood Siphon (Spell 24323) handled through DB
SDCategory: Zul'Gurub
EndScriptData */


/*
ToDo:
1) Have Son of Hakkar patrol
2) Verify BloodSiphon code
*/

#include "ScriptPCH.h"
#include "zulgurub.h"
#include "Debugging/Errors.h"
#include "Logging/Log.h"

enum Texts
{
    SAY_AGGRO                =  -1309020,
    SAY_FLEEING              =  -1309021,
};

enum Spells
{
    // Blood Siphon
    SPELL_BLOODSIPHON        =   24322,
    SPELL_BLOODSIPHON_POISON =   24323,
    SPELL_BLOODSIPHON_SELF   =   24324,

    // Aspects
    SPELL_ASPECT_OF_JEKLIK   =   24687,
    SPELL_ASPECT_OF_VENOXIS  =   24688,
    SPELL_ASPECT_OF_MARLI    =   24686,
    SPELL_ASPECT_OF_THEKAL   =   24689,
    SPELL_ASPECT_OF_ARLOKK   =   24690,

	// Self
    SPELL_CORRUPTEDBLOOD     =   24328,
    SPELL_CAUSEINSANITY      =   24327,
    SPELL_WILLOFHAKKAR       =   24178,                  // Used by Voodoo Piles in ZG, not Hakkar?
    SPELL_ENRAGE             =   24318,
    SPELL_CLEAVE             =   7160,

    // Son of Hakkar
    SPELL_POISONOUSBLOOD_TRIGGER    =   24320,
    SPELL_POISONOUSBLOOD     =   24321,
    SPELL_SUMMONPOISONCLOUD  =   24319
};

enum Summons
{
    SON_OF_HAKKAR            =   11357,
    POISON_CLOUD             =   14989,
    SIPHON_TRIGGER           =   40000
};

float SON_OF_HAKKAR_LOC [2][4] =
{
    {-11738.400f, -1690.800f, 40.748f, 1.62497f},
    {-11833.454f, -1694.459f, 40.748f, 1.58962f}
};

struct boss_hakkarAI : public ScriptedAI
{
    boss_hakkarAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    ScriptedInstance *instance;

    uint32 BloodSiphon_Timer;
    uint32 CorruptedBlood_Timer;
    uint32 CauseInsanity_Timer;
    uint32 WillOfHakkar_Timer;
    uint32 Cleave_Timer;
    uint32 Enrage_Timer;
    uint32 Son_Respawn_Timer;
    uint32 Channel_Timer;

    uint32 CheckJeklik_Timer;
    uint32 CheckVenoxis_Timer;
    uint32 CheckMarli_Timer;
    uint32 CheckThekal_Timer;
    uint32 CheckArlokk_Timer;

    uint32 AspectOfJeklik_Timer;
    uint32 AspectOfVenoxis_Timer;
    uint32 AspectOfMarli_Timer;
    uint32 AspectOfThekal_Timer;
    uint32 AspectOfArlokk_Timer;

    bool Enraged;
    
    Unit* son_of_hakkar[2];

    void Reset()
    {
        BloodSiphon_Timer = 15000; //90000
        CorruptedBlood_Timer = 25000;
        CauseInsanity_Timer = 17000;
        WillOfHakkar_Timer = 17000;
        Cleave_Timer = 7000;
        Enrage_Timer = 600000;
        Son_Respawn_Timer = 20000;
        Channel_Timer = 0;

        CheckJeklik_Timer = 1000;
        CheckVenoxis_Timer = 2000;
        CheckMarli_Timer = 3000;
        CheckThekal_Timer = 4000;
        CheckArlokk_Timer = 5000;

        AspectOfJeklik_Timer = 4000;
        AspectOfVenoxis_Timer = 7000;
        AspectOfMarli_Timer = 12000;
        AspectOfThekal_Timer = 8000;
        AspectOfArlokk_Timer = 18000;

        Enraged = false;
    }

    void EnterCombat(Unit * /*who*/)
    {
        DoScriptText(SAY_AGGRO, me);
        short i = rand() % 2;
        son_of_hakkar[i] = me->SummonCreature(SON_OF_HAKKAR,SON_OF_HAKKAR_LOC[i][0],SON_OF_HAKKAR_LOC[i][1],SON_OF_HAKKAR_LOC[i][2],SON_OF_HAKKAR_LOC[i][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,10000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Son_Respawn_Timer <= diff)
        {
            if ( !son_of_hakkar[0])
                son_of_hakkar[0] = me->SummonCreature(SON_OF_HAKKAR,SON_OF_HAKKAR_LOC[0][0],SON_OF_HAKKAR_LOC[0][1],SON_OF_HAKKAR_LOC[0][2],SON_OF_HAKKAR_LOC[0][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,10000);
            else
            if( !son_of_hakkar[1])
                son_of_hakkar[1] = me->SummonCreature(SON_OF_HAKKAR,SON_OF_HAKKAR_LOC[1][0],SON_OF_HAKKAR_LOC[1][1],SON_OF_HAKKAR_LOC[1][2],SON_OF_HAKKAR_LOC[1][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,10000);
            Son_Respawn_Timer = 20000;
			//son_of_hakkar[1]->GetMotionMaster()->MovePath
        }
        else
            Son_Respawn_Timer -= diff;

        
        if (BloodSiphon_Timer <= diff)
        {
            DoCast(SPELL_BLOODSIPHON_SELF);
            Channel_Timer = 8000;
            
            BloodSiphon_Timer = 90000;
        }
        else
            BloodSiphon_Timer -= diff;

        //CorruptedBlood_Timer
        if (CorruptedBlood_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_CORRUPTEDBLOOD);
            CorruptedBlood_Timer = 30000 + rand()%15000;
        } else
			if (Channel_Timer <= diff)
				CorruptedBlood_Timer -= diff;

        //CauseInsanity_Timer
		if (CauseInsanity_Timer <= diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(pTarget, SPELL_CAUSEINSANITY);
                CauseInsanity_Timer = 35000 + rand()%8000;
        }
        else
			if (Channel_Timer <= diff)
				CauseInsanity_Timer -= diff;

        // Cleave
        if (Cleave_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_CLEAVE);
            Cleave_Timer = 7000;
        }
        else
			if (Channel_Timer <= diff)
				Cleave_Timer -= diff;

        if (!Enraged && Enrage_Timer <= diff)
        {
            DoCast(me, SPELL_ENRAGE);
            Enraged = true;
        } else 
			if (Channel_Timer <= diff) 
				Enrage_Timer -= diff;

        //Checking if Jeklik is dead. If not we cast her Aspect
        if (CheckJeklik_Timer <= diff)
        {
            if (instance)
            {
                if (instance->GetData(TYPE_JEKLIK) != DONE)
                {
                    if (AspectOfJeklik_Timer <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_ASPECT_OF_JEKLIK);
                        AspectOfJeklik_Timer = 10000 + rand()%4000;
                    } else AspectOfJeklik_Timer -= diff;
                }
            }
            CheckJeklik_Timer = 1000;
        } else CheckJeklik_Timer -= diff;

        //Checking if Venoxis is dead. If not we cast his Aspect
        if (CheckVenoxis_Timer <= diff)
        {
            if (instance)
            {
                if (instance->GetData(TYPE_VENOXIS) != DONE)
                {
                    if (AspectOfVenoxis_Timer <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_ASPECT_OF_VENOXIS);
                        AspectOfVenoxis_Timer = 8000;
                    } else AspectOfVenoxis_Timer -= diff;
                }
            }
            CheckVenoxis_Timer = 1000;
        } else CheckVenoxis_Timer -= diff;

        //Checking if Marli is dead. If not we cast her Aspect
        if (CheckMarli_Timer <= diff)
        {
            if (instance)
            {
                if (instance->GetData(TYPE_MARLI) != DONE)
                {
                    if (AspectOfMarli_Timer <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_ASPECT_OF_MARLI);
                        AspectOfMarli_Timer = 10000;
                    } else AspectOfMarli_Timer -= diff;
                }
            }
            CheckMarli_Timer = 1000;
        } else CheckMarli_Timer -= diff;

        //Checking if Thekal is dead. If not we cast his Aspect
        if (CheckThekal_Timer <= diff)
        {
            if (instance)
            {
                if (instance->GetData(TYPE_THEKAL) != DONE)
                {
                    if (AspectOfThekal_Timer <= diff)
                    {
                        DoCast(me, SPELL_ASPECT_OF_THEKAL);
                        AspectOfThekal_Timer = 15000;
                    } else AspectOfThekal_Timer -= diff;
                }
            }
            CheckThekal_Timer = 1000;
        } else CheckThekal_Timer -= diff;

        //Checking if Arlokk is dead. If yes we cast her Aspect
        if (CheckArlokk_Timer <= diff)
        {
            if (instance)
            {
                if (instance->GetData(TYPE_ARLOKK) != DONE)
                {
                    if (AspectOfArlokk_Timer <= diff)
                    {
                        DoCast(me, SPELL_ASPECT_OF_ARLOKK);
                        DoResetThreat();

                        AspectOfArlokk_Timer = 10000 + rand()%5000;
                    } else AspectOfArlokk_Timer -= diff;
                }
            }
            CheckArlokk_Timer = 1000;
        } else CheckArlokk_Timer -= diff;

        if (Channel_Timer <= diff)
            DoMeleeAttackIfReady();
        else
            Channel_Timer -= diff;
    }
};

// Son of Hakkar
// needs patroling
struct mob_son_of_hakkarAI: public ScriptedAI
{
	mob_son_of_hakkarAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }
	
    ScriptedInstance *instance;

	void Reset()
    {
        //me->GetMotionMaster()->MovePath(10,true);
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32 &uiDamage)
    {
        if (uiDamage >= me->GetHealth())
        {
            me->CastSpell(me,SPELL_SUMMONPOISONCLOUD,false);     // see SummonProperties 
        }        
    }
	
	void UpdateAI(const uint32 diff)
    {
         DoMeleeAttackIfReady();
	}
};

struct mob_poison_cloudAI: public ScriptedAI{
    mob_poison_cloudAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }
	
    ScriptedInstance *instance;

    uint32 Poison_Timer;

    bool test;
    
	void Reset()
    {
        Poison_Timer = 1000;
        test = false;
    }
	
    void UpdateAI(const uint32 diff)
    {            
        if (!test)
        {
            me->CastSpell(me, SPELL_POISONOUSBLOOD_TRIGGER, false);
            test = true;
        }
    }

};



CreatureAI* GetAI_boss_hakkar(Creature* creature)
{
    return new boss_hakkarAI (creature);
}

CreatureAI* GetAI_mob_son_of_hakkar(Creature* creature)
{
    return new mob_son_of_hakkarAI (creature);
}

CreatureAI* GetAI_mob_poison_cloud(Creature* creature)
{
    return new mob_poison_cloudAI (creature);
}


void AddSC_boss_hakkar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_hakkar";
    newscript->GetAI = &GetAI_boss_hakkar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_son_of_hakkar";
    newscript->GetAI = &GetAI_mob_son_of_hakkar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_poison_cloud";
    newscript->GetAI = &GetAI_mob_poison_cloud;
    newscript->RegisterSelf();
}