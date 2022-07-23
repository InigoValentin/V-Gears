#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <OgreSingleton.h>

#include "Background2D.h"
#include "Entity.h"
#include "EntityPoint.h"
#include "EntityTrigger.h"
#include "Event.h"
#include "Walkmesh.h"



class EntityManager : public Ogre::Singleton< EntityManager >
{
public:
    EntityManager();
    virtual ~EntityManager();

    void Input(const QGears::Event& event);
    void Update();
    void UpdateDebug();
    void OnResize();

    void Clear();

    void ScriptSetPaused( const bool paused );

    Walkmesh* GetWalkmesh();
    Background2D* GetBackground2D();
    void AddEntity( const Ogre::String& name, const Ogre::String& file_name, const Ogre::Vector3& position, const Ogre::Degree& direction );
    void AddEntity( const Ogre::String& name, const Ogre::String& file_name, const Ogre::Vector3& position, const Ogre::Degree& rotation, const Ogre::Vector3& scale, const Ogre::Quaternion& root_orientation );
    void ScriptAddEntity( const char* name, const char* file_name, const float x, const float y, const float z, const float direction );
    void AddEntityTrigger( const Ogre::String& name, const Ogre::Vector3& point1, const Ogre::Vector3& point2, const bool enabled );
    void AddEntityPoint( const Ogre::String& name, const Ogre::Vector3& position, const float rotation );
    void AddEntityScript( const Ogre::String& name );
    void ScriptAddEntityScript( const char* name );

    Entity* GetEntity( const Ogre::String& name ) const;
    Entity* ScriptGetEntity( const char* name ) const;
    EntityPoint* ScriptGetEntityPoint( const char* name ) const;

    void ScriptSetPlayerEntity( const char* name );
    void ScriptUnsetPlayerEntity();
    void ScriptPlayerLock( const bool lock );
    void SetPlayerMoveRotation( const Ogre::Radian rotation );

    /**
     * Checks if random battle encounters are active in the field.
     *
     * @return True random battles can occur, false otherwise.
     */
    bool GetRandomEncounters();

    /**
     * Enables or disables random encounters in the field.
     *
     * @param active[in] True to enable encounters, false to deactivate
     * them.
     */
    void SetRandomEncounters(bool active);

    /**
     * Get the encounter rate for the field.
     *
     * @return The encounter rate, between 0 and 1 (both included).
     */
    float GetEncounterRate();

    /**
     * Sets the battle encounter rate.
     *
     * @param rate[in] The encounter rate (max. 1).
     */
    void SetEncounterRate(float rate);

    /**
     * Starts a battle.
     *
     * @TODO
     * @param formation[in] The enemy formation to fight.
     * @return True if the battle victory conditions are met, false\
     * otherwise.
     */
    //bool StartBattle(unsigned int formation);
    void StartBattle(unsigned int formation);

private:
    // movement
    bool SetEntityOnWalkmesh( Entity* entity );
    bool PerformWalkmeshMove( Entity* entity, const float speed );
    bool WalkmeshBorderCross( Entity* entity, Ogre::Vector3& position, const Ogre::Vector2& move_vector );
    bool CheckSolidCollisions( Entity* entity, Ogre::Vector3& position );
    void SetEntityDirectionByVector( Entity* entity, const Ogre::Vector2& vector );
    void CheckTriggers( Entity* entity, Ogre::Vector3& position );
    void CheckEntityInteract();

    void SetNextOffsetStep( Entity* entity );
    void SetNextTurnStep( Entity* entity );
    void SetNextLinearStep( Entity* entity );
    void SetNextJumpStep( Entity* entity );
    void SetNextScrollStep();

private:
    bool                          m_Paused;

    Walkmesh                      m_Walkmesh;
    Background2D                  m_Background2D;

    Ogre::String                  m_EntityTableName;
    std::vector< Entity* >        m_Entity;
    Entity*                       m_PlayerEntity;
    Ogre::Vector3                 m_PlayerMove;
    Ogre::Radian                  m_PlayerMoveRotation;
    bool                          m_PlayerLock;
    bool                          m_PlayerRun;

    std::vector< EntityTrigger* > m_EntityTriggers;
    std::vector< EntityPoint* >   m_EntityPoints;
    std::vector< Ogre::String >   m_EntityScripts;

    Ogre::SceneNode*              m_SceneNode;

    Ogre::Entity*                 m_Grid;
    Ogre::Entity*                 m_Axis;

    // TODO battle formations
    bool random_encounters_;
    float encounter_rate_;
};



#endif // ENTITY_MANAGER_H
