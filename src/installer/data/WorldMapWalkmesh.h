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

#pragma once

#include "OGRE/Ogre.h"
#include "common/TypeDefine.h"

/**
 * Represents a walkmesh for a world map.
 */
class WorldMapWalkmesh{

    public:

        class Triangle{

            public:

                /**
                 * Triangle reached when exiting this triangle from one of its borders.
                 */
                struct Target{

                    /**
                     * Target triangle ID.
                     *
                     * If -1, there is no way out by this side.
                     */
                    int id;

                    /**
                     * False if the target triangle is truly adjacent, true if it warps to other
                     * side of the world.
                     */
                    bool warp;
                };

                /**
                 * Creates a triangle
                 *
                 * @param[in] id Triangle ID.
                 * @param[in] a X, Y, Z coordinates of the first vertex.
                 * @param[in] b X, Y, Z coordinates of the second vertex.
                 * @param[in] c X, Y, Z coordinates of the third vertex.
                 * @param[in] walkability Walkability mode ID.
                 * @param[in] event Event triggered upon entering the triangle. -1 for no event.
                 */
                Triangle(
                  const unsigned int id, Ogre::Vector3 a, const Ogre::Vector3 b,
                  const Ogre::Vector3 c, const unsigned int walkability, const int event
                );

                /**
                 * Retrieves the coordinates of the first vertex
                 *
                 * @return Vertex coordinates (X, Y, Z).
                 */
                Ogre::Vector3 getA();

                /**
                 * Retrieves the coordinates of the second vertex
                 *
                 * @return Vertex coordinates (X, Y, Z).
                 */
                Ogre::Vector3 getB();

                /**
                 * Retrieves the coordinates of the third vertex
                 *
                 * @return Vertex coordinates (X, Y, Z).
                 */
                Ogre::Vector3 getC();

                /**
                 * Retrieves the triangle reached when exiting the triangle by the side between the
                 * vertices A and B.
                 *
                 * @return Target triangle.
                 */
                Target getAB();

                /**
                 * Retrieves the triangle reached when exiting the triangle by the side between the
                 * vertices B and C.
                 *
                 * @return Target triangle.
                 */
                Target getBC();

                /**
                 * Retrieves the triangle reached when exiting the triangle by the side between the
                 * vertices C and A.
                 *
                 * @return Target triangle.
                 */
                Target getCA();

                /**
                 * Sets the triangle reached when exiting the triangle by the side between the
                 * vertices A and B.
                 *
                 * @param[in] id Target triangle ID. -1 to indicate no way out by the edge.
                 * @param[in] warp True to indicate world warping (ie, if walking into an edge of
                 * the world gets you at the other end).
                 */
                void setAB(const int id, const bool warp);

                /**
                 * Sets the triangle reached when exiting the triangle by the side between the
                 * vertices B and C.
                 *
                 * @param[in] id Target triangle ID. -1 to indicate no way out by the edge.
                 * @param[in] warp True to indicate world warping (ie, if walking into an edge of
                 * the world gets you at the other end).
                 */
                void setBC(const int id, const bool warp);

                /**
                 * Sets the triangle reached when exiting the triangle by the side between the
                 * vertices C and A.
                 *
                 * @param[in] id Target triangle ID. -1 to indicate no way out by the edge.
                 * @param[in] warp True to indicate world warping (ie, if walking into an edge of
                 * the world gets you at the other end).
                 */
                void setCA(const int id, const bool warp);

            private:

                /**
                 * Triangle ID.
                 */
                unsigned int id_;

                /**
                 * First vertex, X, Y, Z coordinates.
                 */
                Ogre::Vector3 a_;

                /**
                 * Second vertex, X, Y, Z coordinates.
                 */
                Ogre::Vector3 b_;

                /**
                 * Third vertex, X, Y, Z coordinates.
                 */
                Ogre::Vector3 c_;

                /**
                 * Target reached when exiting by the side formed by {@see a_} and {@see b_}.
                 */
                Target ab_;

                /**
                 * Target reached when exiting by the side formed by {@see b_} and {@see c_}.
                 */
                Target bc_;

                /**
                 * Target reached when exiting by the side formed by {@see c_} and {@see a_}.
                 */
                Target ca_;

                /**
                 * Walkability mode.
                 */
                unsigned int walkability_;

                /**
                 * Id of the event triggered upon entering the triangle.
                 */
                int event_;
        };

        /**
         * Constructor.
         *
         * @param[in] id World map ID.
         * @param[in] warp Indicates if the world map is warpable (ie, if walking into an edge of
         * the world gets you at the other end).
         */
        WorldMapWalkmesh(const unsigned int id, const bool warp);

        /**
         * Adds a triangle.
         *
         * @param[in] a X, Y, Z coordinates of the first vertex.
         * @param[in] b X, Y, Z coordinates of the second vertex.
         * @param[in] c X, Y, Z coordinates of the third vertex.
         * @param[in] walkability Walkability mode ID.
         * @param[in] event Event triggered upon entering the triangle. -1 for no event.
         */
        void addTriangle(
          const Ogre::Vector3 a, const Ogre::Vector3 b, const Ogre::Vector3 c,
          const unsigned int walkability, const int event
        );

        /**
         * Generates a world map walkmesh file.
         *
         * @param[in] path Path where the file will be saved (directory must exist).
         */
        void generate(const std::string path);

    private:

        /**
         * Worldmap ID
         */
        unsigned int id_;

        /**
         * Indicates if the world is a warpable one.
         */
        bool warp_;

        /**
         * The list of triangles.
         */
        std::vector<Triangle> triangles_;

        /**
         * Indicates if the relations between triangles has been calculated since the last triangle
         * was added.
         */
        bool calculated_;

        /**
         * Calculates the relations between triangles.
         */
        void calculate();
};
