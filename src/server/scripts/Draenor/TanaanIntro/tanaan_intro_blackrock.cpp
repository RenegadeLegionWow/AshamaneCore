/*
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "MotionMaster.h"
#include "Player.h"
#include "tanaan_intro.h"
#include "WorldSession.h"

/// Passive Scene Object
class playerScript_from_cave_to_ridge : public PlayerScript
{
public:
    playerScript_from_cave_to_ridge() : PlayerScript("playerScript_from_cave_to_ridge") { }

    void OnSceneStart(Player* player, uint32 scenePackageId, uint32 /*sceneInstanceId*/) override
    {
        if (scenePackageId != TanaanSceneObjects::SceneFromCaveToRidge)
            return;

        player->RemoveAurasDueToSpell(TanaanPhases::PhaseBlackrockSlaves);
        player->RemoveAurasDueToSpell(TanaanPhases::PhaseBlackrockMainNpcs);
    }

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneFromCaveToRidge))
            return;

        player->AddAura(TanaanPhases::PhaseBlackrockSlaves, player);
        player->AddAura(TanaanPhases::PhaseBlackrockMainNpcs, player);
    }
};

/// Passive Scene Object
class playerScript_blackhand_reveal : public PlayerScript
{
public:
    playerScript_blackhand_reveal() : PlayerScript("playerScript_blackhand_reveal") { }

    void OnSceneCancel(Player* player, uint32 sceneInstanceId) override
    {
        StartBattle(player, sceneInstanceId);
    }

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        StartBattle(player, sceneInstanceId);
    }

    void StartBattle(Player* player, uint32 sceneInstanceId)
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneBlackHandReveal))
            return;

        player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneKhadgarGoesToDam);
    }
};

/// Passive Scene Object
class playerScript_khadgar_goes_to_dam : public PlayerScript
{
public:
    playerScript_khadgar_goes_to_dam() : PlayerScript("playerScript_khadgar_goes_to_dam") { }

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneKhadgarGoesToDam))
            return;

        player->AddAura(TanaanPhases::PhaseBlackrockKhadgarRock, player);
        player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneKhadgarAtDam);
    }
};

/// Passive Scene Object
class playerScript_dam_explosion : public PlayerScript
{
public:
    playerScript_dam_explosion() : PlayerScript("playerScript_dam_explosion") { }

    void OnQuestReward(Player* player, const Quest* p_Quest) override
    {
        if (player && p_Quest && p_Quest->GetQuestId() == TanaanQuests::QuestTheBattleOfTheForge)
        {
            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneKhadgarAtDam);
            player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneDamExplosion);
        }
    }

    void OnSceneCancel(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneDamExplosion))
            return;

        SwitchPhaseMapAfterDamExplosion(player);
        player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneWaterPortal);
    }

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneDamExplosion))
            return;

        player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneWaterPortal);
    }

    void OnSceneTriggerEvent(Player* player, uint32 sceneInstanceId, std::string p_Event) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneDamExplosion))
            return;

        if (p_Event == "Force Phase")
            SwitchPhaseMapAfterDamExplosion(player);
    }

    void SwitchPhaseMapAfterDamExplosion(Player* player)
    {
        std::set<uint32> l_PhaseId, l_Terrainswap, l_InactiveTerrainSwap;
        l_Terrainswap.insert((uint32)TanaanZones::TerrainSwapID);
        player->GetSession()->SendSetPhaseShift(l_PhaseId, l_Terrainswap, l_InactiveTerrainSwap);
    }
};

/// Passive Scene Object
class playerScript_khadgar_water_portal : public PlayerScript
{
public:
    playerScript_khadgar_water_portal() : PlayerScript("playerScript_khadgar_water_portal") { }

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneWaterPortal))
            return;

        player->AddAura(TanaanPhases::PhaseBlackrockKhadgarUpper, player);
    }
};

/// Passive Scene Object
class playerScript_gunpowder_plot : public PlayerScript
{
public:
    playerScript_gunpowder_plot() : PlayerScript("playerScript_gunpowder_plot") { }

    void OnSceneTriggerEvent(Player* player, uint32 sceneInstanceID, std::string /*event*/) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceID, TanaanSceneObjects::SceneBuildingExplosion))
            return;

        Creature* speaker = player->FindNearestCreature(TanaanCreatures::NpcHanselHeavyHands, 5.0f);

        if (!speaker || !speaker->AI())
            return;

        /*if (p_Event == "TalkA")
        /// TALK
        else if (p_Event == "TalkB")
        /// TALK
        else if (p_Event == "TalkC")
        /// TALK ///< player->PlayScene(893, player);
        else if (p_Event == "TalkD")
        /// TALK*/
    }
};

/// 78553 - Thral
/// 78554 - Maladaar
class npc_thrall_maladaar_blackrock : public CreatureScript
{
public:
    npc_thrall_maladaar_blackrock() : CreatureScript("npc_thrall_maladaar_blackrock")
    {
    }

    bool OnQuestReward(Player* player, Creature* /*p_Creature*/, const Quest* p_Quest, uint32 /*p_Option*/) override
    {
        if (p_Quest->GetQuestId() == TanaanQuests::QuestABattleToPrepareAlly)
            player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneArmingPrisonersAlly);
        else if (p_Quest->GetQuestId() == TanaanQuests::QuestABattleToPrepareHorde)
            player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneArmingPrisonersHorde);

        return false;
    }
};

/// 78430 - Cordana Felsong
class npc_blackrock_follower : public CreatureScript
{
public:
    npc_blackrock_follower() : CreatureScript("npc_blackrock_follower") { }

    bool OnQuestAccept(Player* player, Creature* /*p_Creature*/, const Quest* p_Quest) override
    {
        if (p_Quest->GetQuestId() == TanaanQuests::QuestTheBattleOfTheForge)
        {
            Position playerPosition = player->GetPosition();

            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneArmingPrisonersAlly);
            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneArmingPrisonersHorde);
            player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneBlackHandReveal);

            player->SummonCreature(TanaanCreatures::NpcLadyLiadrinBlackrockSummon, playerPosition);
            player->SummonCreature(TanaanCreatures::NpcCordanaFelsong, playerPosition);

            if (player->GetTeamId() == TEAM_ALLIANCE)
                player->SummonCreature(TanaanCreatures::NpcQiana, playerPosition);
            else
                player->SummonCreature(TanaanCreatures::NpcOlin, playerPosition);
        }

        return false;
    }

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_cordana_felsong_blackrockAI(p_Creature);
    }

    struct npc_cordana_felsong_blackrockAI : public ScriptedAI
    {
        npc_cordana_felsong_blackrockAI(Creature* p_Creature) : ScriptedAI(p_Creature)
        {
            m_checkVictimTimer = 2000;
            m_Summoned = false;
        }

        ObjectGuid m_PlayerGuid;
        uint32 m_checkVictimTimer;
        bool m_Summoned;

        void Reset() override
        {
            //me->AddAura(TanaanSpells::SpellCoverOfElune, me);
        }

        void IsSummonedBy(Unit* p_Summoner) override
        {
            m_Summoned = true;

            if (p_Summoner->GetTypeId() != TYPEID_PLAYER)
                return;

            m_PlayerGuid = p_Summoner->GetGUID();

            FollowSummoner(p_Summoner);
        }

        void FollowSummoner(Unit* p_Summoner)
        {
            float followAngle = 0.0f;

            switch (me->GetEntry())
            {
                case TanaanCreatures::NpcOlin:
                case TanaanCreatures::NpcQiana:
                    followAngle = 1.57f; // pi
                    break;
                case TanaanCreatures::NpcCordanaFelsong:
                    followAngle = 2.355f; // 3 pi / 4
                    break;
                case TanaanCreatures::NpcLadyLiadrinBlackrockSummon:
                    followAngle = 3.925f; // 7 pi / 4
                    break;
            }

            if (Player* l_Player = p_Summoner->ToPlayer())
                me->GetMotionMaster()->MoveFollow(l_Player, 0.05f, followAngle);
        }

        void DamageTaken(Unit* /*p_Attacker*/, uint32& p_Damage) override
        {
            if (p_Damage >= me->GetHealth())
                me->SetFullHealth();
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!m_Summoned)
                return;

            if (m_checkVictimTimer > diff)
            {
                m_checkVictimTimer -= diff;
                return;
            }
            else
                m_checkVictimTimer = 2000;

            Player* l_EscortedPlayer = ObjectAccessor::FindPlayer(m_PlayerGuid);

            if (!l_EscortedPlayer)
                return;

            if (   l_EscortedPlayer->GetQuestStatus(TanaanQuests::QuestTheBattleOfTheForge) != QUEST_STATUS_INCOMPLETE
                && l_EscortedPlayer->GetQuestStatus(TanaanQuests::QuestTheBattleOfTheForge) != QUEST_STATUS_COMPLETE)
            {
                me->DespawnOrUnsummon();
                return;
            }

            Unit* escortedPlayerVictim = l_EscortedPlayer->GetVictim();

            if (!escortedPlayerVictim)
            {
                me->CombatStop();
                FollowSummoner(l_EscortedPlayer);
                return;
            }

            if (!me->IsWithinMeleeRange(escortedPlayerVictim))
            {
                Position l_Pos = l_EscortedPlayer->GetPosition();
                me->GetMotionMaster()->MoveCharge(l_Pos.m_positionX, l_Pos.m_positionY, l_Pos.m_positionZ, me->GetSpeed(MOVE_RUN));
                return;
            }

            me->Attack(escortedPlayerVictim, true);
            DoMeleeAttackIfReady();
        }
    };
};

/// 80775/81294 - Ogron Warcrusher
class npc_ogron_warcrusher : public CreatureScript
{
public:
    npc_ogron_warcrusher() : CreatureScript("npc_ogron_warcrusher")
    {
    }

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_ogron_warcrusherAI(p_Creature);
    }

    struct npc_ogron_warcrusherAI : public ScriptedAI
    {
        npc_ogron_warcrusherAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

        EventMap m_Events;

        enum eEvents
        {
            EventAddaura = 1
        };

        void Reset() override
        {
            m_Events.Reset();
            m_Events.ScheduleEvent(EventAddaura, 3000);
        }

        void JustDied(Unit* p_Killer) override
        {
            std::list<Player*> l_PlayerList;
            GetPlayerListInGrid(l_PlayerList, me, 42.0f);

            for (Player* l_Player : l_PlayerList)
            {
                if (p_Killer == l_Player)
                    continue;

                if (l_Player->GetQuestStatus(TanaanQuests::QuestTheBattleOfTheForge) == QUEST_STATUS_INCOMPLETE)
                    l_Player->KilledMonsterCredit(TanaanKillCredits::CreditOgronWarcrusher);
            }
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            m_Events.Update(p_Diff);

            switch (m_Events.ExecuteEvent())
            {
                case eEvents::EventAddaura:
                    me->AddAura(TanaanSpells::SpellCrushingStomp, me);
                    m_Events.ScheduleEvent(eEvents::EventAddaura, 12000);
                    break;
            }
        }
    };
};

/// 80786 - Blackrock Grunt
class npc_blackrock_grunt : public CreatureScript
{
public:
    npc_blackrock_grunt() : CreatureScript("npc_blackrock_grunt")
    {
    }

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_blackrock_gruntAI(p_Creature);
    }

    struct npc_blackrock_gruntAI : public ScriptedAI
    {
        npc_blackrock_gruntAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

        void JustDied(Unit* p_Killer) override
        {
            std::list<Player*> l_PlayerList;
            GetPlayerListInGrid(l_PlayerList, me, 3.0f);

            for (Player* l_Player : l_PlayerList)
            {
                if (p_Killer == l_Player)
                    continue;

                if (l_Player->GetQuestStatus(TanaanQuests::QuestTheBattleOfTheForge) == QUEST_STATUS_INCOMPLETE)
                    l_Player->KilledMonsterCredit(TanaanKillCredits::CreditBlackrockGrunt);
            }
        }
    };
};

/// 79917 - Ganar
class npc_tanaan_ganar : public CreatureScript
{
public:
    npc_tanaan_ganar() : CreatureScript("npc_tanaan_ganar") { }

    bool OnQuestAccept(Player* player, Creature* /*p_Creature*/, Quest const* p_Quest) override
    {
        if (p_Quest->GetQuestId() == TanaanQuests::QuestTheProdigalFrostwolf)
            player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneFreeGanar);

        return true;
    }
};

class npc_tanaan_overseer_gotrigg : public CreatureScript
{
public:
    npc_tanaan_overseer_gotrigg() : CreatureScript("npc_tanaan_overseer_gotrigg")
    {
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tanaan_overseer_gotriggAI(creature);
    }

    struct npc_tanaan_overseer_gotriggAI : public ScriptedAI
    {
        npc_tanaan_overseer_gotriggAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        enum eDatas
        {
            EventBurningBody = 1,
            SpellBurningBody = 166401,
            EventBurningRadiance = 2,
            SpellBurningRadiance = 166403
        };

        void Reset() override
        {
            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Target*/) override
        {
            /// TALK

            m_Events.ScheduleEvent(eDatas::EventBurningBody, 3000);
            m_Events.ScheduleEvent(eDatas::EventBurningBody, 10000);
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            m_Events.Update(p_Diff);

            if (m_Events.ExecuteEvent() == eDatas::EventBurningBody)
            {
                if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->AddAura(eDatas::SpellBurningBody, l_Target);
                m_Events.ScheduleEvent(eDatas::EventBurningBody, 20000);
            }
            else if (m_Events.ExecuteEvent() == eDatas::EventBurningRadiance)
            {
                me->AddAura(eDatas::SpellBurningRadiance, me);
                m_Events.ScheduleEvent(eDatas::EventBurningRadiance, 20000);
            }
        }
    };
};

enum eSpell
{
    SPELL_LEAKY_GUNPOWDER_BARREL = 162676,
};

/// 231119 - Powder Keg
class gob_powder_keg : public GameObjectScript
{
public:
    gob_powder_keg() : GameObjectScript("gob_powder_keg") { }

    bool OnGossipHello(Player* player, GameObject* /*p_Gameobject*/) override
    {
        if (player->GetQuestStatus(TanaanQuests::QuestTheGunpowderPlot) == QUEST_STATUS_INCOMPLETE)
        {
            if (player->GetQuestObjectiveCounter(273294))
                return true;

            player->AddAura(SPELL_LEAKY_GUNPOWDER_BARREL, player);
            player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneBuildingExplosion);
        }

        return false;
    }
};

/// 231066 - Makeshift Plunger
class gob_makeshift_plunger : public GameObjectScript
{
public:
    gob_makeshift_plunger() : GameObjectScript("gob_makeshift_plunger") { }

    bool OnGossipHello(Player* player, GameObject* /*p_Gameobject*/) override
    {
        if (player->GetQuestStatus(TanaanQuests::QuestTheGunpowderPlot) == QUEST_STATUS_INCOMPLETE)
        {
            if (!player->GetQuestObjectiveCounter(273294))
                return true;

            player->KilledMonsterCredit(TanaanKillCredits::CreditMakeShiftPlunger);

            if (player->HasAura(SPELL_LEAKY_GUNPOWDER_BARREL))
                player->RemoveAurasDueToSpell(SPELL_LEAKY_GUNPOWDER_BARREL);
            else
            {
                player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneBuildingExplosion);
                player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneBuildingExplosionFallBack);
            }
        }

        return true;
    }
};

void AddSC_tanaan_intro_blackrock()
{
    new npc_thrall_maladaar_blackrock();
    new npc_blackrock_follower();
    new npc_blackrock_grunt();
    new npc_ogron_warcrusher();
    new npc_tanaan_ganar();
    new npc_tanaan_overseer_gotrigg();
    new gob_powder_keg();
    new gob_makeshift_plunger();

    new playerScript_from_cave_to_ridge();
    new playerScript_blackhand_reveal();
    new playerScript_khadgar_goes_to_dam();
    new playerScript_dam_explosion();
    new playerScript_khadgar_water_portal();
    new playerScript_gunpowder_plot();
}