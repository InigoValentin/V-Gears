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

#include <string>
#include <vector>

/**
 * A model.
 *
 * It can be a battle model or an spell model.
 */
struct Model{

    /**
     * Model identifier.
     *
     * For battle models, a two letters string. For spell models,
     * anything.
     */
    std::string id;

    /**
     * Name of the model .hrc file.
     *
     * For battle models, it will be the *aa file. For spell models,
     * the *.d file.
     */
    std::string hrc;

    /**
     * Name of the animation file.
     *
     * For battle models, it will be the *da file. For spell models,
     * the *.b file.
     */
    std::string anim;

    /**
     * List of .p polygon files associated to the model
     */
    std::vector<std::string> p;

    Model(): id(""), hrc(""), anim(""){
        p.clear();
    }

};

/**
 * Files to compose a battle model.
 */
struct BattleModel: Model{

    /**
     * Name of the "ab" scripts file.
     */
    std::string script;

    /**
     * List of .tex texture files associated to the model
     */
    std::vector<std::string> tex;

    /**
     * List of .a animation files associated to the model
     */
    std::vector<std::string> a;

    /**
     * Constructor, initializes default values.
     */
    BattleModel(): script(""){
        tex.clear();
        a.clear();
    }

};

/**
 * Files to compose a spell model.
 */
struct SpellModel: Model{

    /**
     * List of .a files associated to the model
     */
    std::vector<std::string> a;

    /**
     * List of .s files associated to the model
     */
    std::vector<std::string> s;

    /**
     * Constructor, initializes default values.
     */
    SpellModel(){
        a.clear();
        s.clear();
    }
};

/**
 * Retrieves the model with an identifier, adding it to the list if it's not there.
 *
 * @param[in,out] models The list of models.
 * @param[in] id Identifier of the model.
 * @return The model. It's only valid until another model is added.
 */
template<typename T> T& FindOrAddModel(std::vector<T>& models, const std::string& id){
    for (T& model : models) if (model.id == id) return model;
    models.emplace_back();
    models.back().id = id;
    return models.back();
}
