/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

#include <OgreString.h>
#include "ScriptManager.h"

/**
 * Any enemy in a battle.
 */
class Enemy{

    public:

        struct Element{

            /**
             * The element ID.
             */
            unsigned int id;

            /**
             * Damage modification factor when attacked by the element.
             *
             * 1 means normal damage taken.
             * 0 means no damage taken.
             * [0-1] means reduced damage.
             * [1-10) means extended damage.
             * (-10-0) means recovery
             * >10 means death.
             * <-10 means full recovery.
             */
            float factor;
        };

        /**
         * Enemy attack data.
         */
        struct Attack{

            /**
             * Attack ID.
             */
            unsigned int id;

            /**
             * Camera ID to use during the attack.
             *
             * -1 to not move the camera.
             */
            int camera;
        };

        /**
         * Data for drop and steal items.
         */
        struct Item{

            /**
             * Item ID.
             */
            unsigned int id;

            /**
             * Steal or drop rate.
             */
            float rate;
        };

        /**
         * Status immunity.
         */
        struct Immunity{

            /**
             * Status ID.
             */
            unsigned int status;

            /**
             * Immunity rate.
             *
             * 0 or lower means no immunity, 1 or higher completely immune. Values between 0 and 1
             * indicate resistance.
             */
            float rate;
        };

        /**
         * Constructor.
         *
         * @param[in] id Enemy ID.
         */
        Enemy(const unsigned int id);

        /**
         * Constructor.
         *
         * @param[in] id Enemy ID.
         * @param[in] pos Enemy position (x, y, z).
         * @param[in] front True to set the enemy in the front row, false for back row.
         * @param[in] visible Indicates enemy visibility.
         * @param[in] targeteable Indicates if the enemy can be targeted.
         * @param[in] active Indicates whether the enemy main script is active or not.
         * @param[in] cover Cover binary flags string.
         */
        Enemy(
          const unsigned int id, const Ogre::Vector3 pos, const bool front, const bool visible,
          const bool targeteable, const bool active, const std::string cover
        );

        /**
         * Destructor.
         */
        virtual ~Enemy();

        /**
         * Retrieves the enemy ID.
         *
         * @return The enemy ID, or -1 if it's not loaded.
         */
        const int GetId() const;

        /**
         * Sets the enemy ID.
         *
         * @param[in] id The enemy ID.
         */
        void SetId(const int id);

        /**
         * Retrieves the enemy model.
         *
         * The model name is the filename of the .mesh file, without extension.
         *
         * @return The enemy model.
         */
        const std::string GetModel() const;

        /**
         * Sets the enemy model.
         *
         * The model name must be the filename of the .mesh file, without extension.
         *
         * @param[in] The enemy model name.
         */
        void SetModel(const std::string model);

        /**
         * Retrieves the enemy name.
         *
         * @return The enemy name.
         */
        const Ogre::String& GetName() const;

        /**
         * Sets the enemy name.
         *
         * @param[in] name The enemy name.
         */
        void SetName(const Ogre::String& name);

        /**
         * Retrieves the enemy level.
         *
         * @return The enemy level.
         */
        unsigned int GetLevel() const;

        /**
         * Sets the enemy level.
         *
         * @param[in] level The enemy level.
         */
        void SetLevel(const unsigned int level);

        /**
         * Retrieves the AP gain upon defeating the enemy.
         *
         * @return The AP gain.
         */
        unsigned int GetAp() const;

        /**
         * Sets the AP gain upon defeating the enemy.
         *
         * @param[in] ap The AP gain.
         */
        void SetAp(unsigned int ap);

        /**
         * Retrieves the EXP gain upon defeating the enemy.
         *
         * @return The EXP gain.
         */
        unsigned int GetExp() const;

        /**
         * Sets the EXP gain upon defeating the enemy.
         *
         * @param[in] exp The EXP gain.
         */
        void SetExp(const unsigned int exp);

        /**
         * Retrieves the money gain upon defeating the enemy.
         *
         * @return The money gain.
         */
        unsigned int GetMoney() const;

        /**
         * Sets the money gain upon defeating the enemy.
         *
         * @param[in] money The money gain.
         */
        void SetMoney(const unsigned int money);

        /**
         * Retrieves the enemy animation IDs.
         *
         * @return The list of animation IDs
         */
        std::vector<unsigned int> GetAnimations() const;

        /**
         * Adds an animation for the enemy.
         *
         * @param[in] Animation ID.
         */
        void AddAnimation(const unsigned int animation);

        /**
         * Retrieves the enemy attacks.
         *
         * @return The list of attacks.
         */
        std::vector<Attack> GetAttacks() const;

        /**
         * Adds an attack for the enemy.
         *
         * @param[in] Attack to add.
         */
        void AddAttack(const Attack attack);

        /**
         * Retrieves the enemy possible item drops.
         *
         * @return List of items that can be droped by the enemy.
         */
        std::vector<Item> GetDrop() const;

        /**
         * Adds an item drop for the enemy.
         *
         * @param[in] item The dropable item.
         */
        void AddDrop(const Item item);

        /**
         * Retrieves the list of the enemy elemental affinities.
         *
         * @return The list of elemental affinities.
         */
        std::vector<Element> GetElements() const;

        /**
         * Adds an elemental affinity to the monster.
         *
         * @param[in] element Elemental affinity.
         */
        void AddElement(const Element element);

        /**
         * Retrieves the list of status immunities.
         *
         * @return The list of status immunities.
         */
        std::vector<Immunity> GetImmunities() const;

        /**
         * Adds an immunity to the monster.
         *
         * @param[in] immunity The immunity to add.
         */
        void AddImmunity(const Immunity immunity);

        /**
         * Retrieves the list of attacks that can be used during the manipulated state.
         *
         * The first attack of the list is also the attack the enemy will use when in berserkr
         * state. If the list is empty, the enemy can't be manipulated or berserkred.
         *
         * @return List of IDs of attacks usable during manipulation
         */
        std::vector<unsigned int> GetManipulateAttacks() const;

        /**
         * Adds an attack usable during manipulation.
         *
         * @param[in] attack ID of the attack.
         */
        void AddManipulateAttack(const unsigned int attack);

        /**
         * Retrieves the enemy possible item stelas.
         *
         * @return List of items that can be stolen from the enemy.
         */
        std::vector<Item> GetSteal() const;

        /**
         * Adds an item steal to the enemy.
         *
         * @param[in] item The stealableitem.
         */
        void AddSteal(const Item item);

        /**
         * Retrieves the ID of the item the monster can be morphed into.
         *
         * @return ID of the item the enemy can be morphed into, -1 if none.
         */
        unsigned int GetMorph() const;

        /**
         * Sets the ID of the item the monster can be morphed into.
         *
         * @param[in] morph ID of the item the enemy can be morphed into, -1 if none.
         */
        void SetMorph(const int morph);

        /**
         * Retrieves the multiplier for the damage the enemy receives when attacked from the back.
         *
         * @return Back attack damage multiplier.
         */
        float GetBackDamage() const;

        /**
         * Sets the multiplier for the damage the enemy receives when attacked from the back.
         *
         * @param[in] back_damage Back attack damage multiplier, 0 or positive.
         */
        void SetBackDamage(const float back_damage);

        /**
         * Retrieves the enemy's strength stat.
         *
         * @return The strength stat.
         */
        unsigned int GetStr() const;

        /**
         * Sets the enemy's strength stat.
         *
         * @param[in] str The strength stat.
         */
        void SetStr(const unsigned int str);

        /**
         * Retrieves the enemy's defense stat.
         *
         * @return The defense stat.
         */
        unsigned int GetDef() const;

        /**
         * Sets the enemy's defense stat.
         *
         * @param[in] def The defense stat.
         */
        void SetDef(const unsigned int def);

        /**
         * Retrieves the enemy's magic stat.
         *
         * @return The magic stat.
         */
        unsigned int GetMag() const;

        /**
         * Sets the enemy's magic stat.
         *
         * @param[in] mag The magic stat.
         */
        void SetMag(const unsigned int mag);

        /**
         * Retrieves the enemy's spirit stat.
         *
         * @return The spirit stat.
         */
        unsigned int GetSpr() const;

        /**
         * Sets the enemy's spirit stat.
         *
         * @param[in] spr The spirit stat.
         */
        void SetSpr(const unsigned int spr);

        /**
         * Retrieves the enemy's dexterity stat.
         *
         * @return The dexterity stat.
         */
        unsigned int GetDex() const;

        /**
         * Sets the enemy's dexterity stat.
         *
         * @param[in] dex The dexterity stat.
         */
        void SetDex(const unsigned int dex);

        /**
         * Retrieves the enemy's luck stat.
         *
         * @return The luck stat.
         */
        unsigned int GetLck() const;

        /**
         * Sets the enemy's luck stat.
         *
         * @param[in] lck The luck stat.
         */
        void SetLck(const unsigned int lck);

        /**
         * Retrieves the enemy's evasion stat.
         *
         * @return The evasion stat.
         */
        unsigned int GetEva() const;

        /**
         * Sets the enemy's evasion stat.
         *
         * @param[in] eva The evasion stat.
         */
        void SetEva(const unsigned int eva);

        /**
         * Retrieves the enemy's magic evasion stat.
         *
         * @return The magic evasion stat.
         */
        unsigned int GetMeva() const;

        /**
         * Sets the enemy's magic evasion stat.
         *
         * @param[in] meva The magic evasion stat.
         */
        void SetMeva(const unsigned int meva);

        /**
         * Retrieves the enemy's current HP.
         *
         * @return The current HP.
         */
        unsigned int GetHp() const;

        /**
         * Sets the enemy's current HP.
         *
         * If set to higher then max HP reported by {@see GetHpMax()}, it will be capped to that
         * value.
         *
         * @param[in] hp The current HP.
         */
        void SetHp(const unsigned int hp);

        /**
         * Retrieves the enemy's max HP.
         *
         * @return The max HP.
         */
        unsigned int GetHpMax() const;

        /**
         * Sets the enemy's max HP.
         *
         * If the new max HP is higher than the current HP, the current HP will be set to the new
         * max HP.
         *
         * @param[in] str The strength stat.
         */
        void SetHpMax(const unsigned int hp_max);

        /**
         * Retrieves the enemy's current MP.
         *
         * @return The current MP.
         */
        unsigned int GetMp() const;

        /**
         * Sets the enemy's current MP.
         *
         * If set to higher then max MP reported by {@see GetMpMax()}, it will be capped to that
         * value.
         *
         * @param[in] mp The current MP.
         */
        void SetMp(const unsigned int mp);

        /**
         * Retrieves the enemy's max MP.
         *
         * @return The max MP.
         */
        unsigned int GetMpMax() const;

        /**
         * Sets the enemy's max MP.
         *
         * If the new max MP is higher than the current MP, the current MP will be set to the new
         * max MP.
         *
         * @param[in] str The strength stat.
         */
        void SetMpMax(const unsigned int mp_max);

        /**
         * Retrieves the enemy position in the battlefield.
         *
         * @return The enemy position (x, y, z).
         */
        Ogre::Vector3& GetPos();

        /**
         * Sets the enemy position in the battlefield.
         *
         * @param[in] pos The enemy position (x, y, z).
         */
        void SetPos(const Ogre::Vector3 &pos);

        /**
         * Checks if the enemy is in the front row.
         *
         * @return True if the enemy is in the front row. false if not.
         */
        bool IsFront() const;

        /**
         * Sets the enemy in or out the front row.
         *
         * @param[in] front True to set the enemy in the front row, false for the back row.
         */
        void SetFront(bool front);

        /**
         * Checks the enemy visibility.
         *
         * @return True if the enemy is visible, false if not.
         */
        bool IsVisible() const;

        /**
         * Toggles the enemy visibility.
         *
         * @param[in] visible True to make the enemy visible, false to make it invisible.
         */
        void SetVisible(bool visible);

        /**
         * Checks whether the enemy can be targeted.
         *
         * @return True if the enemy can be targeted, false if not.
         */
        bool IsTargeteable() const;

        /**
         * Determines whether the enemy can be targeted.
         *
         * @param[in] targeteable True to allow targeting the enemy false to prevent it.
         */
        void SetTargeteable(bool targeteable);

        /**
         * Checks whether the enemy's main script is active.
         *
         * @return True if the script is active, false if not.
         */
        bool IsActive() const;

        /**
         * Activates or deactivates the enemy main script.
         *
         * @param[in] active True to activate the script, false to deactivate it.
         */
        void SetActive(bool active);

        /**
         * Retrieves the flags for enemy covering.
         *
         * {@see https://wiki.ffrtt.ru/index.php/FF7/Battle/Battle_Scenes#Binary_.22Cover_Flags.22}
         * for more info about cover flags.
         *
         * @return A string with five 0s or 1s indicating the cover flags.
         */
        std::string GetCover() const;

        /**
         * Sets the flags for enemy covering.
         *
         * {@see https://wiki.ffrtt.ru/index.php/FF7/Battle/Battle_Scenes#Binary_.22Cover_Flags.22}
         * for more info about cover flags.
         *
         * @param[in] A string with five 0s or 1s indicating the cover flags.
         */
        void SetCover(std::string cover);

    private:

        /**
         * Reads enemy data from the xml file.
         */
        void ReadFromXml();

        /**
         * The enemy ID.
         */
        int id_;

        /**
         * NAme of the enemy model file.
         */
        std::string model_;

        /**
         * The name of the enemy.
         */
        Ogre::String name_;

        /**
         * The enemy level.
         */
        unsigned int level_;

        /**
         * EXP given upon defeating the enemy.
         */
        unsigned int exp_;

        /**
         * AP given upon defeating the enemy.
         */
        unsigned int ap_;

        /**
         * Money given upon defeating the enemy.
         */
        unsigned int money_;

        /**
         * List of animations.
         */
        std::vector<unsigned int> animations_;

        /**
         * Elemental affinities.
         */
        std::vector<Element> elements_;

        /**
         * Status immunities and resistances.
         */
        std::vector<Immunity> immunities_;

        /**
         * Enemy attacks.
         */
        std::vector<Attack> attacks_;

        /**
         * List of attacks that can be used while manipulated.
         *
         * The first one is also the one used in berserkr. If empty, it means the enemy can't be
         * manipulated or berserkd.
         */
        std::vector<unsigned int> manipulate_attacks_;

        /**
         * List of the items that can be stolen from the enemy.
         */
        std::vector<Item> steal_;

        /**
         * List of the items that can be dropped from the enemy.
         */
        std::vector<Item> drop_;

        /**
         * ID of the item the enemy can be morphed into. -1 if none..
         */
        int morph_;

        /**
         * Multiplier for back damage.
         */
        float back_damage_;

        /**
         * Enemy strength stat.
         */
        unsigned int str_;

        /**
         * Enemy defense stat.
         */
        unsigned int def_;

        /**
         * Enemy magic stat.
         */
        unsigned int mag_;

        /**
         * Enemy spirit stat.
         */
        unsigned int spr_;

        /**
         * Enemy dexterity stat.
         */
        unsigned int dex_;

        /**
         * Enemy luck stat.
         */
        unsigned int lck_;

        /**
         * Enemy evasion stat.
         */
        unsigned int eva_;

        /**
         * Enemy magic evasion stat.
         */
        unsigned int meva_;

        /**
         * Enemy's current HP.
         */
        unsigned int hp_;

        /**
         * Enemy's current MP.
         */
        unsigned int mp_;

        /**
         * Enemy's max HP.
         */
        unsigned int hp_max_;

        /**
         * Enemy's max MP.
         */
        unsigned int mp_max_;

        /**
         * Enemy position in the battlefield (x, y, z).
         */
        Ogre::Vector3 pos_;

        /**
         * Indicates if the enmy is in the frontline.
         */
        bool front_;

        /**
         * Indicates if the enemy is visible.
         */
        bool visible_;

        /**
         * Indicates if the enemy can be targeted.
         */
        bool targeteable_;

        /**
         * Indicates if the enemy's main script is active.
         */
        bool active_;

        /**
         * Cover binary flags.
         */
        std::string cover_;

};

