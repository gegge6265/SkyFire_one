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
SDName: Instance_ZulGurub
SD%Complete: 80
SDComment: Missing reset function after killing a boss for Ohgan, Thekal.
SDCategory: Zul'Gurub
EndScriptData */

#include "ScriptPCH.h"
#include "zulgurub.h"

enum areatrigger
{
    AT_ALTAR_OF_BLOOD = 1,
    AT_ZULGURUB_ENTRANCE = 2
};

enum triggered
{
    NOT_TRIGGERED = 0,
    TRIGGERED = 1
};

enum Texts
{
    SAY_MINION_DESTROY       =  -1309022,                //where does it belong?    upon entering the altar of blood (Areatrigger ID 3960)
    SAY_PROTECT_ALTAR        =  -1309023                //where does it belong?     upon entering ZG (Areatrigger ID 3930)
};

struct instance_zulgurub : public ScriptedInstance
{
    instance_zulgurub(Map* pMap) : ScriptedInstance(pMap) {Initialize();};

    //If all High Priest bosses were killed. Lorkhan, Zath and Ohgan are added too.
    uint32 m_auiEncounter[MAX_ENCOUNTERS];

    //Storing Lorkhan, Zath and Thekal because we need to cast on them later. Jindo is needed for healfunction too.
    uint64 m_uiLorKhanGUID;
    uint64 m_uiZathGUID;
    uint64 m_uiThekalGUID;
    uint64 m_uiJindoGUID;

	uint32 b_at_altar_of_blood;
    uint32 b_at_zulgurub_entrance;
	std::vector<uint64> spirit_revive;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

        m_uiLorKhanGUID = 0;
        m_uiZathGUID = 0;
        m_uiThekalGUID = 0;
        m_uiJindoGUID = 0;

		spirit_revive.clear();

		b_at_altar_of_blood = NOT_TRIGGERED;
        b_at_zulgurub_entrance = NOT_TRIGGERED;
    }

    bool IsEncounterInProgress() const
    {
        //not active in Zul'Gurub
        return false;
    }

    void OnCreatureCreate(Creature* creature, bool /*add*/)
    {
        switch (creature->GetEntry())
        {
            case 11347: m_uiLorKhanGUID = creature->GetGUID(); break;
            case 11348: m_uiZathGUID = creature->GetGUID(); break;
            case 14509: m_uiThekalGUID = creature->GetGUID(); break;
            case 11380: m_uiJindoGUID = creature->GetGUID(); break;
            case 14515:
            if (m_auiEncounter[0] >= IN_PROGRESS)
                creature->DisappearAndDie();
            else
                m_auiEncounter[0] = IN_PROGRESS;
            break;
        }
    }

    void SetData(uint32 uiType, uint32 uiData)
    {
        switch (uiType)
        {
            case TYPE_ARLOKK:
                m_auiEncounter[0] = uiData;
                break;

            case TYPE_JEKLIK:
                m_auiEncounter[1] = uiData;
                break;

            case TYPE_VENOXIS:
                m_auiEncounter[2] = uiData;
                break;

            case TYPE_MARLI:
                m_auiEncounter[3] = uiData;
                break;

            case TYPE_THEKAL:
                m_auiEncounter[4] = uiData;
                break;

            case TYPE_LORKHAN:
                m_auiEncounter[5] = uiData;
                break;

            case TYPE_ZATH:
                m_auiEncounter[6] = uiData;
                break;

            case TYPE_OHGAN:
                m_auiEncounter[7] = uiData;
                break;

			case TYPE_MANDOKIR:
				m_auiEncounter[8] = uiData;
				break;

			case TYPE_AT_ALTAR:
                b_at_altar_of_blood = uiData;
                break;

            case TYPE_AT_ENTRANCE:
                b_at_zulgurub_entrance = uiData;
                break;
        }
    }

	void SetData64(uint32 uiType, uint64 uiData)
    {
        switch (uiType)
        {
			case DATA_SPIRIT_REVIVE:
				spirit_revive.push_back(uiData);
				break;
			case DATA_SPIRIT_CLEAR:
				spirit_revive.clear();
				break;
		}
    }

    uint32 GetData(uint32 uiType)
    {
        switch (uiType)
        {
            case TYPE_ARLOKK:
                return m_auiEncounter[0];
            case TYPE_JEKLIK:
                return m_auiEncounter[1];
            case TYPE_VENOXIS:
                return m_auiEncounter[2];
            case TYPE_MARLI:
                return m_auiEncounter[3];
            case TYPE_THEKAL:
                return m_auiEncounter[4];
            case TYPE_LORKHAN:
                return m_auiEncounter[5];
            case TYPE_ZATH:
                return m_auiEncounter[6];
            case TYPE_OHGAN:
                return m_auiEncounter[7];
			case TYPE_MANDOKIR:
				return m_auiEncounter[8];
			case TYPE_AT_ALTAR:
                return b_at_altar_of_blood;
            case TYPE_AT_ENTRANCE:
                return b_at_zulgurub_entrance;
        }
        return 0;
    }

    uint64 GetData64(uint32 uiData)
    {
        switch (uiData)
        {
            case DATA_LORKHAN:
                return m_uiLorKhanGUID;
            case DATA_ZATH:
                return m_uiZathGUID;
            case DATA_THEKAL:
                return m_uiThekalGUID;
            case DATA_JINDO:
                return m_uiJindoGUID;
			case DATA_SPIRIT_REVIVE:
				{
					if (!spirit_revive.empty())
					{
					uint64 temp = spirit_revive.front();
					spirit_revive.pop_back();
					return temp;
					}
					else return 0;
				}
        }
        return 0;
    }
};

bool AreaTrigger_at_altar_of_blood(Player* player, const AreaTriggerEntry* /*pAt*/)
{
    ScriptedInstance *instance;

    instance = player->GetInstanceScript();

    if (instance->GetData(TYPE_AT_ALTAR) == NOT_TRIGGERED)
    {
        if (instance)
        {
            if(uint64 HakkarGUID = instance->GetData64(DATA_HAKKAR))
                if (Unit* hTemp = Unit::GetUnit(*player, HakkarGUID))                
                    if (hTemp->isAlive())
                    {
                        hTemp->MonsterYellToZone(SAY_MINION_DESTROY,LANG_UNIVERSAL,player->GetGUID());
                        instance->SetData(TYPE_AT_ALTAR,TRIGGERED);
                        return true;
                    }
        }
    }
    return false;
}

bool AreaTrigger_at_zulgurub_entrance(Player* player, const AreaTriggerEntry* /*pAt*/)
{
    ScriptedInstance *instance;

    instance = player->GetInstanceScript();

    if (instance->GetData(TYPE_AT_ENTRANCE) == NOT_TRIGGERED)
    {
        if (instance)
        {
            if(uint64 HakkarGUID = instance->GetData64(DATA_HAKKAR))
                if (Unit* hTemp = Unit::GetUnit(*player, HakkarGUID))                
                    if (hTemp->isAlive())
                    {
                        hTemp->MonsterYellToZone(SAY_PROTECT_ALTAR,LANG_UNIVERSAL,player->GetGUID());
                        instance->SetData(TYPE_AT_ENTRANCE,TRIGGERED);
                        return true;
                    }
        }
    }
    return false;
}

InstanceScript* GetInstanceData_instance_zulgurub(Map* pMap)
{
    return new instance_zulgurub(pMap);
}

void AddSC_instance_zulgurub()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_zulgurub";
    newscript->GetInstanceScript = &GetInstanceData_instance_zulgurub;
    newscript->RegisterSelf();

	newscript = new Script;
    newscript->Name = "at_altar_of_blood";
    newscript->pAreaTrigger = &AreaTrigger_at_altar_of_blood;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_zulgurub_entrance";
    newscript->pAreaTrigger = &AreaTrigger_at_zulgurub_entrance;
    newscript->RegisterSelf();
}

