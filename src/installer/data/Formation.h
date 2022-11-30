/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include "common/TypeDefine.h"
#include "data/GameData.h"

/**
 * An enemy formation
 */
struct Formation{

    /**
     * Battle layouts.
     */
    enum class Layout{

        /**
         * Normal fight.
         */
        NORMAL = 0,

        /**
         * Preemptive attack.
         *
         * Enemies are on their back, they bars are empty. When attacked, or after a certain time,
         * they turn and their bars start to fill.
         */
        PREEMPTIVE,

        /**
         * Back attack.
         *
         * Players are on their back, they bars are empty. When attacked, or after a certain time,
         * they turn and their bars start to fill. Player rows are reversed.
         */
        BACK_ATTACK,

        /**
         * Side attack.
         *
         * Players in both sides, enemies in center, facing one or other direction. Change command
         * disabled.
         */
        SIDE_ATTACK,

        /**
         * Enemies in both sides, player in center, front row, facing one or the other direction.
         * Change command disabled. Unescapable until one of the sides is cleared.
         */
        PINCER_ATTACK,

        /**
         * Locked position fight.
         *
         * Like a normal one, but all players in the front row. Change command disabled.
         */
        LOCKED
    };

    /**
     * Each enemy in the formation.
     */
    struct Enemy{

        /**
         * Enemy ID.
         */
        unsigned int id;

        /**
         * Enemy X position.
         */
        int x;

        /**
         * Enemy Y position.
         */
        int y;

        /**
         * Enemy Z position.
         */
        int z;

        /**
         * Enemy row.
         */
        unsigned int row;

        /**
         * Cover flags.
         *
         * These flags are used in conjunction with row to determine if a target can be selected
         * as the target of a short-range attack. The determination of this is worked out in this
         * way: An enemy exists in row 1 and another in row 2. If the enemy in row 1 shares a cover
         * flag with the enemy in row 2 then the enemy in row 2 cannot be targeted until all
         * enemies in row 1 that share a cover flag with the row 2 enemy is defeated. It works like
         * this. Two active enemies exist, A and B.
         *
         * If ((B's row > A's row) and (B's cover flags AND A's cover flags) > 0)
         * then enemy B cannot be targeted by short-range attacks.
         *
         * For any enemies A and B.
         *
         * Example:
         * Consider the Battery Cap x6 battle in the forest between Nibelheim and Rocket Town.
         * Their cover flags (in binary) are:
         *
         * Row 1:       00100
         * Row 2:    00110 01100
         * Row 3: 00011 00100 11000
         *
         * The battery caps in row 2 cannot be targeted by a short-range attack until the one in
         * row 1 has been defeated because they share the 0x4 cover flag. Once row 1 has been
         * cleared:
         *
         * Row 2:    00110 01100
         * Row 3: 00011 00100 11000
         *
         * The battery cap on left in row 2 covers the left two in row 3 because it shares flag 0x4
         * with the one in the middle and flag 0x2 with the one on the far left. As long as it is
         * active these in row 3 cannot be targeted. Similarly, the battery cap on the right in row
         * 2 shares the 0x4 flag with the middle of row 3 and the 0x8 flag with the far right of
         * row 3 so these cannot be targeted until the right side of row 2 is defeated. It is also
         * necessary to note that because row 1 does not share any flags with the extreme right and
         * left of row 3, they can be targeted if the corresponding enemy in row 2 is defeated even
         * if the row 1 enemy is still active. Also of note is that enemies in the same row that
         * share cover flags are not considered.
         */
        char cover[5];

        /**
         * Indicates if the enemy is initially visible.
         */
        bool visible;

        /**
         * Indicates initial direction facing if players get a side attack.
         */
        int direction;

        /**
         * Indicates if the enemy is targeteable.
         */
        bool targeteable;

        /**
         * @todo Understand and document.
         */
        bool main_script_active;

        /**
         * Constructor.
         *
         * Initializes to default values.
         */
        Enemy():
          id(0), x(0), y(0), z(0), row(0), visible(true),
          direction(0), targeteable(true), main_script_active(true)
        {
            for (int c = 0; c < 5; c ++) cover[c] = '0';
        }
    };

    /**
     * Position for a camera.
     */
    struct Camera{

        /**
         * X position.
         */
        int x;

        /**
         * Y position.
         */
        int y;

        /**
         * z position.
         */
        int z;

        /**
         * X direction.
         */
        int d_x;

        /**
         * Y direction.
         */
        int d_y;

        /**
         * z direction.
         */
        int d_z;

        /**
         * Constructor.
         *
         * Initializes to default values.
         */
        Camera(): x(0), y(0), z(0), d_x(0), d_y(0), d_z(0){}
    };

    /**
     * Formation ID.
     */
    unsigned int id;

    /**
     * When all enemies are defeated, begin battle with this ID without ending battle scene.
     */
    int next_formation;

    /**
     * Difficulty to escape.
     */
    float escape_counter;

    /**
     * Indicates if the battle can be escaped.
     */
    bool escapable;

    /**
     * If true, the victory pose will be played after the battle.
     */
    bool skip_victory_pose;

    /**
     * If true, spoils (item, ap, money...) will not be obtained after the battle.
     *
     * The spoil screens will not be displayed.
     */
    bool skip_spoils;

    /**
     * If false, this battle can never be preemtied.
     */
    bool preemptive_disabled;

    /**
     * Battle layout.
     */
    Layout layout;

    /**
     * The enemies in the formation.
     */
    std::vector<Enemy> enemies;

    /**
     * Location ID.
     *
     * Must be set, except for formations referenced by other formation's {@see next_formation} or
     * {@see next_arena_formation_candidates}. In those cases, it can be -1.
     */
    int location;

    /**
     * Location name.
     *
     * Never used, it's to have descriptive XML files.
     */
    std::string location_name;

    /**
     * Camera positions.
     */
    std::vector<Camera> camera_positions;

    /**
     * Index of the initial camera position.
     */
    int initial_camera_position;

    /**
     * Indicates if the formation is an arena battle.
     */
    bool is_arena_battle;

    /**
     * FOr arena battles, candidates fro the next formation.
     */
    std::vector<int> next_arena_formation_candidates;

    /**
     * Constructor.
     *
     * Initializes to default values.
     */
    Formation():
      id(0), next_formation(-1), escape_counter(0.0f), escapable(true), skip_victory_pose(false),
      skip_spoils(false), preemptive_disabled(false), layout(Layout::NORMAL), location(-1),
      location_name(""), initial_camera_position(0),is_arena_battle(false)
    {
        enemies.clear();
        camera_positions.clear();
        next_arena_formation_candidates.clear();
    }

};
