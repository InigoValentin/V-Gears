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

#include <Ogre.h>
#include <vector>

/**
 * A triangle of a walkmesh.
 */
struct WalkmeshTriangle{

    /**
     * Constructor.
     */
    WalkmeshTriangle(): a(0, 0, 0), b(0, 0, 0), c(0, 0, 0), locked(false){
        access_side[0] = -1;
        access_side[1] = -1;
        access_side[2] = -1;
    }

    /**
     * A side of the triangle.
     */
    Ogre::Vector3 a;

    /**
     * A side of the triangle.
     */
    Ogre::Vector3 b;

    /**
     * A side of the triangle.
     */
    Ogre::Vector3 c;

    /**
     * @todo Understand and document.
     */
    int access_side[3];

    /**
     * Indicates if the triangle is locked (if it's walkable).
     */
    bool locked;
};

namespace VGears{
    typedef Ogre::SharedPtr<class WalkmeshFile> WalkmeshFilePtr;
}

class Walkmesh{

    public:

        /**
         * Constructor.
         */
        Walkmesh();

        /**
         * Destructor.
         */
        virtual ~Walkmesh();

        /**
         * Updates the walkmesh with debug information.
         */
        void UpdateDebug();

        /**
         * Deletes all the triangles in the walkmesh.
         */
        void Clear();

        /**
         * Adds a triangle to the walkmesh.
         *
         * @param[in] triangle Triangle to add.
         */
        void AddTriangle(const WalkmeshTriangle& triangle);

        /**
         * Checks which other triangle is accessed from one side of a triangle.
         *
         * @param[in] triangle_id Triangle.
         * @param[in] side The side index in the triangle.
         * @return The id of the triangle accessed from the indicated triangle
         * and side, or -1 if no triangle can be accessed.
         */
        int GetAccessSide(unsigned int triangle_id, unsigned char side) const;

        /**
         * Retrieves the first side of a triangle.
         *
         * @param[in] triangle_id ID of the triangle.
         * @return The side of the triangle. Ogre::Vector3::ZERO if the
         * triangle doesn't exist.
         */
        const Ogre::Vector3& GetA(unsigned int triangle_id) const;

        /**
         * Retrieves the second side of a triangle.
         *
         * @param[in] triangle_id ID of the triangle.
         * @return The side of the triangle. Ogre::Vector3::ZERO if the
         * triangle doesn't exist.
         */
        const Ogre::Vector3& GetB(unsigned int triangle_id) const;

        /**
         * Retrieves the third side of a triangle.
         *
         * @param[in] triangle_id ID of the triangle.
         * @return The side of the triangle. Ogre::Vector3::ZERO if the
         * triangle doesn't exist.
         */
        const Ogre::Vector3& GetC(unsigned int triangle_id) const;

        /**
         * Counts the triangles in the walkmesh.
         *
         * @return The number of triangles in the walkmesh.
         */
        int GetNumberOfTriangles() const;

        /**
         * Locks or unlocks a triangle.
         *
         * @param[in] triangle_id ID of the triangle.
         * @param[in] lock True to lock, false to unlock.
         */
        void LockWalkmesh(unsigned int triangle_id, bool lock);

        /**
         * Checks if a triangle is locked.
         *
         * @param[in] triangle_id ID of the trinagle.
         * @return True if the triangle is locked, false otherwise.
         */
        bool IsLocked(unsigned int triangle_id) const;

        /**
         * Loads a walkmesh from a file.
         *
         * @param[in] walkmesh Walkmesh file.
         */
        virtual void load(const VGears::WalkmeshFilePtr &walkmesh);

    private:

        /**
         * The list of triangles.
         */
        std::vector<WalkmeshTriangle> triangles_;
};

