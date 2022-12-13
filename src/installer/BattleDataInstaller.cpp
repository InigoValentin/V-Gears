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

#include <iostream>
#include <fstream>
#include <zlib.h>
#include <tinyxml.h>
#include <boost/filesystem.hpp>
#include <OgreMesh.h>
#include "BattleDataInstaller.h"
#include "TexFile.h"
#include "data/VGearsHRCFileManager.h"
#include "data/VGearsAFileManager.h"
#include "data/DaFile.h"
#include "common/VGearsStringUtil.h"
#include "common/FinalFantasy7/FF7NameLookup.h"

BattleDataInstaller::BattleDataInstaller(const std::string input_dir, const std::string output_dir):
  input_dir_(input_dir), output_dir_(output_dir),
  scene_bin_(input_dir + "data/battle/scene.bin"), total_scenes_(0), next_scene_(0)
{}

BattleDataInstaller::~BattleDataInstaller(){}

unsigned int BattleDataInstaller::InitializeScenes(){
    next_scene_ = 0;
    scenes_.clear();
    enemies_.clear();
    attacks_.clear();
    // Get all scenes from scene.bin.
    for (int i = 0; i < 32; i ++){
        File scene_bin_block(&scene_bin_, i * 0x2000, 0x2000);
        for (int o = 0; o < 10; o ++){
            scene_bin_block.SetOffset(4 * o);
            u32 start_offset = scene_bin_block.readU32LE() * 4;
            u32 end_offset = scene_bin_block.readU32LE() * 4;
            if (end_offset > start_offset + 0x2000) end_offset = start_offset + 0x2000;
            if (start_offset >= 0 && start_offset < 0x2000){
                File scene_bin_fragment(
                  &scene_bin_block, start_offset, end_offset - start_offset - 1
                );
                File* extracted = ExtractGZipScene(scene_bin_fragment);
                if (extracted != NULL){
                    scenes_uncompressed_.push_back(extracted);
                    total_scenes_ ++;
                }
            }
            else break;
        }
    }
    return total_scenes_;
}

unsigned int BattleDataInstaller::InitializeModels(){
    next_model_to_process_ = 0;
    next_model_to_convert_ = 0;
    battle_lgp_files_.clear();
    battle_lgp_file_names_.clear();
    models_.clear();
    // Open battle.lgp
    File battle_lgp_file(input_dir_ + "data/battle/battle.lgp");
    // Also, open it as a LGP archive.
    VGears::LGPArchive battle_lgp(input_dir_ + "data/battle/battle.lgp", "LGP");
    battle_lgp.open(input_dir_ + "data/battle/battle.lgp", true);
    battle_lgp.load();
    VGears::LGPArchive::FileList file_list = battle_lgp.GetFiles();
    for (int i = 0; i < file_list.size(); i ++){
        VGears::LGPArchive::FileEntry f = file_list.at(i);
        if (f.data_offset + f.data_size <= battle_lgp_file.GetFileSize()){
            File b_lgp_file(&battle_lgp_file, f.data_offset, f.data_size);
            battle_lgp_files_.push_back(b_lgp_file);
            battle_lgp_file_names_.push_back("btl_" + f.file_name);
        }
    }
    return battle_lgp_files_.size();
}

unsigned int BattleDataInstaller::ProcessScene(){
    if (next_scene_ >= scenes_uncompressed_.size()) return total_scenes_;
    BattleSceneFile scene(next_scene_, scenes_uncompressed_[next_scene_]);
    scenes_.push_back(scene);
    // Add new enemies to the list.
    std::vector<Enemy> enemies = scene.GetEnemies();
    for (int e = 0; e < enemies.size(); e ++){
        bool already_added = false;
        for (int l = 0; l < enemies_.size(); l ++){
            if (enemies_[l].id == enemies[e].id){
                already_added = true;
                break;
            }
        }
        if (already_added) continue;
        enemies_.push_back(enemies[e]);
    }
    // Add new attacks to the list.
    std::vector<Attack> attacks = scene.GetAttacks();
    for (int a = 0; a < attacks.size(); a ++){
        bool already_added = false;
        for (int l = 0; l < attacks_.size(); l ++){
            if (attacks_[l].id == attacks[a].id){
                already_added = true;
                break;
            }
        }
        if (already_added) continue;
        attacks_.push_back(attacks[a]);
    }
    // Add new formations to the list.
    std::vector<Formation> formations = scene.GetFormations();
    for (Formation f : formations){
        f.id = formations_.size(); // Assign the next ID.
        formations_.push_back(f);
    }
    next_scene_ ++;
    return next_scene_;
}

unsigned int BattleDataInstaller::ProcessModel(){
    if (next_model_to_process_ >= battle_lgp_files_.size()) return battle_lgp_files_.size();
    // TODO: Do something with the files.
    // A little explanation. Each file has a 4 letter name, for example: 1234
    // 12 is the model identifier.
    // 34 if the type of file:
    //  - aa: Skeleton file (.hrc)
    //  - ab: Unknown
    //  - ac - al: Textures (.tex)
    //  - am - cz: Polygon files (.p)
    //  - da: Animations (.anim)
    // Note that the names in battle_lgp_file_names_ are prefixed with "btl_"
    std::string id = battle_lgp_file_names_[next_model_to_process_].substr(4, 2);
    if (id != "at"){next_model_to_process_ ++; return next_model_to_process_;} // TODO: DEBUG
    std::string type = battle_lgp_file_names_[next_model_to_process_].substr(6, 2);
    if (type == "aa"){
        // HRC file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".hrcbin"
        );
        bool found = false;
        for (int m = 0; m < models_.size(); m ++){
            if (models_[m].id == id){
                models_[m].hrc = battle_lgp_file_names_[next_model_to_process_] + ".hrc";
                found = true;
                break;
            }
        }
        if (found == false){
            Model model;
            model.id = id;
            model.hrc = battle_lgp_file_names_[next_model_to_process_] + ".hrc";
            models_.push_back(model);
        }
    }
    else if (type == "ab"){
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".unknown"
        );
    } // Unknown and not needed.
    else if (type == "da"){
        // .a Animation file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".anim"
        );
        bool found = false;
        for (int m = 0; m < models_.size(); m ++){
            if (models_[m].id == id){
                models_[m].anim = battle_lgp_file_names_[next_model_to_process_] + ".anim";
                found = true;
                break;
            }
        }
        if (found == false){
            Model model;
            model.id = id;
            model.anim = battle_lgp_file_names_[next_model_to_process_] + ".anim";
            models_.push_back(model);
        }
    }
    else if (
      type == "ac" || type == "ad" || type == "ae" || type == "af" || type == "ag"
      || type == "ah"|| type == "ai" || type == "aj" || type == "ak" || type == "al"
    ){
        // .tex texture files. Save directly to their directory.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".tex"
        );
        TexFile tex(battle_lgp_files_[next_model_to_process_]);
        tex.SavePng(
          output_dir_ + "models/battle/entities/"
            + battle_lgp_file_names_[next_model_to_process_] + ".png",
          0
          );
        bool found = false;
        for (int m = 0; m < models_.size(); m ++){
            if (models_[m].id == id){
                models_[m].tex.push_back(battle_lgp_file_names_[next_model_to_process_] + ".tex");
                found = true;
                break;
            }
        }
        if (found == false){
            Model model;
            model.id = id;
            model.tex.push_back(battle_lgp_file_names_[next_model_to_process_] + ".tex");
            models_.push_back(model);
        }
    }
    else{
        // .p polygon file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".p"
        );
        bool found = false;
        for (int m = 0; m < models_.size(); m ++){
            if (models_[m].id == id){
                models_[m].p.push_back(battle_lgp_file_names_[next_model_to_process_] + ".p");
                found = true;
                break;
            }
        }
        if (found == false){
            Model model;
            model.id = id;
            model.p.push_back(battle_lgp_file_names_[next_model_to_process_] + ".p");
            models_.push_back(model);
        }
    }
    next_model_to_process_ ++;
    return next_model_to_process_;
}

unsigned int BattleDataInstaller::ConvertModelsInit(){
    next_model_to_convert_ = 0;
    return models_.size();
}

unsigned int BattleDataInstaller::ConvertModel(){
    if (next_model_to_convert_ >= models_.size()) return models_.size();
    Model model = models_[next_model_to_convert_];
    try{
        DecompileHrc(
          File(output_dir_ + "temp/battle_models/" + model.hrc + "bin"),
          model,
          output_dir_ + "temp/battle_models/" + model.hrc
        );
        GenerateRsdFiles(model, output_dir_ + "temp/battle_models/");
        //ExtractAFilesFromDAFile(
        //  File(output_dir_ + "temp/battle_models/" + model.anim),
        //  &model,
        //  output_dir_ + "temp/battle_models/"
        //);

        DaFile da(File(output_dir_ + "temp/battle_models/" + model.anim));
        std::vector<std::string> a_files = da.GenerateAFiles(
          model.id, output_dir_ + "temp/battle_models/"
        );
        for (std::string file_name : a_files) model.a.push_back(file_name);

        Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(model.hrc, "FFVII");
        Ogre::String base_name;
        VGears::StringUtil::splitBase(model.hrc, base_name);
        auto mesh_name = VGears::NameLookup::model(base_name) + ".mesh";
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
        Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
        for (std::string anim : model.a){
        //if (model.anim != ""){
            VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
            Ogre::String a_base_name;
            VGears::StringUtil::splitBase(anim, a_base_name);
            VGears::AFilePtr a = afl_mgr.load(a_base_name + ".a", "FFVII")
              .staticCast<VGears::AFile>();
            // Convert the FF7 name to a more readable name set in the meta data.
            VGears::StringUtil::splitBase(anim, base_name);
            a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
        }
        ExportMesh(output_dir_ + "models/battle/entities/", mesh);
    }
    catch (const Ogre::Exception& ex){
        std::cerr << "[ERROR] Ogre exception converting battle model "
          << model.hrc <<": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception converting battle model "
          << model.hrc << ": " << ex.what() << std::endl;
    }
    next_model_to_convert_ ++;
    return next_model_to_convert_;
}

void BattleDataInstaller::WriteEnemies(){
    for (Enemy enemy : enemies_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("Enemy"));
        container->SetAttribute("id", enemy.id);
        container->SetAttribute("name", enemy.name);
        container->SetAttribute("level", enemy.level);
        container->SetAttribute("exp", enemy.exp);
        container->SetAttribute("ap", enemy.ap);
        container->SetAttribute("money", enemy.money);
        container->SetAttribute("morph", enemy.morph);
        container->SetDoubleAttribute("back_damage", enemy.back_damage);
        std::unique_ptr<TiXmlElement> stats(new TiXmlElement("Stats"));
        std::unique_ptr<TiXmlElement> stat_str(new TiXmlElement("Stat"));
        stat_str->SetAttribute("id", "str");
        stat_str->SetAttribute("value", enemy.str);
        stats->LinkEndChild(stat_str.release());
        std::unique_ptr<TiXmlElement> stat_mag(new TiXmlElement("Stat"));
        stat_mag->SetAttribute("id", "mag");
        stat_mag->SetAttribute("value", enemy.mag);
        stats->LinkEndChild(stat_mag.release());
        std::unique_ptr<TiXmlElement> stat_def(new TiXmlElement("Stat"));
        stat_def->SetAttribute("id", "def");
        stat_def->SetAttribute("value", enemy.def);
        stats->LinkEndChild(stat_def.release());
        std::unique_ptr<TiXmlElement> stat_mdef(new TiXmlElement("Stat"));
        stat_mdef->SetAttribute("id", "mdef");
        stat_mdef->SetAttribute("value", enemy.mdef);
        stats->LinkEndChild(stat_mdef.release());
        std::unique_ptr<TiXmlElement> stat_spd(new TiXmlElement("Stat"));
        stat_spd->SetAttribute("id", "spd");
        stat_spd->SetAttribute("value", enemy.spd);
        stats->LinkEndChild(stat_spd.release());
        std::unique_ptr<TiXmlElement> stat_lck(new TiXmlElement("Stat"));
        stat_lck->SetAttribute("id", "lck");
        stat_lck->SetAttribute("value", enemy.lck);
        stats->LinkEndChild(stat_lck.release());
        std::unique_ptr<TiXmlElement> stat_eva(new TiXmlElement("Stat"));
        stat_eva->SetAttribute("id", "eva");
        stat_eva->SetAttribute("value", enemy.eva);
        stats->LinkEndChild(stat_eva.release());
        container->LinkEndChild(stats.release());
        std::unique_ptr<TiXmlElement> elements(new TiXmlElement("Elements"));
        for (Enemy::Element element : enemy.elements){
            std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("Element"));
            xml_element->SetAttribute("id", element.id);
            xml_element->SetDoubleAttribute("factor", element.factor);
            elements->LinkEndChild(xml_element.release());
        }
        container->LinkEndChild(elements.release());
        std::unique_ptr<TiXmlElement> immunities(new TiXmlElement("Immunities"));
        for (Enemy::Immunity immunity : enemy.immunities){
            std::unique_ptr<TiXmlElement> xml_immunity(new TiXmlElement("Immunity"));
            xml_immunity->SetAttribute("status", immunity.status);
            xml_immunity->SetDoubleAttribute("rate", immunity.rate);
            immunities->LinkEndChild(xml_immunity.release());
        }
        container->LinkEndChild(immunities.release());
        std::unique_ptr<TiXmlElement> attacks(new TiXmlElement("Attacks"));
        for (Enemy::Attack attack : enemy.attacks){
            std::unique_ptr<TiXmlElement> xml_attack(new TiXmlElement("Attack"));
            xml_attack->SetAttribute("status", attack.id);
            xml_attack->SetAttribute("camera", attack.camera);
            attacks->LinkEndChild(xml_attack.release());
        }
        container->LinkEndChild(attacks.release());
        std::unique_ptr<TiXmlElement> manip_attacks(new TiXmlElement("ManipulateAttacks"));
        for (u16 attack : enemy.manipulate_attacks){
            std::unique_ptr<TiXmlElement> xml_manip_attack(new TiXmlElement("Attack"));
            xml_manip_attack->SetAttribute("id", attack);
            manip_attacks->LinkEndChild(xml_manip_attack.release());
        }
        container->LinkEndChild(manip_attacks.release());
        std::unique_ptr<TiXmlElement> steals(new TiXmlElement("Steal"));
        for (Enemy::Item item : enemy.steal){
            std::unique_ptr<TiXmlElement> xml_steal(new TiXmlElement("Item"));
            xml_steal->SetAttribute("id", item.id);
            xml_steal->SetDoubleAttribute("rate", item.rate);
            steals->LinkEndChild(xml_steal.release());
        }
        container->LinkEndChild(steals.release());
        std::unique_ptr<TiXmlElement> drops(new TiXmlElement("Drops"));
        for (Enemy::Item item : enemy.drop){
            std::unique_ptr<TiXmlElement> xml_drop(new TiXmlElement("Item"));
            xml_drop->SetAttribute("id", item.id);
            xml_drop->SetDoubleAttribute("rate", item.rate);
            drops->LinkEndChild(xml_drop.release());
        }
        container->LinkEndChild(drops.release());
        std::unique_ptr<TiXmlElement> animations(new TiXmlElement("Animations"));
        for (unsigned int animation : enemy.animations){
            std::unique_ptr<TiXmlElement> xml_animation(new TiXmlElement("Animation"));
            xml_animation->SetAttribute("id", animation);
            animations->LinkEndChild(xml_animation.release());
        }
        container->LinkEndChild(animations.release());
        xml.LinkEndChild(container.release());
        std::string file_name = BuildEnemyFileName(enemy);
        xml.SaveFile(output_dir_ + file_name);
        enemy_map_[enemy.id] = file_name;

    }
}

void BattleDataInstaller::WriteAttacks(){
    for (Attack attack : attacks_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("Attack"));
        container->SetAttribute("id", attack.id);
        container->SetAttribute("name", attack.name);
        container->SetAttribute("mp", attack.mp);
        container->SetAttribute("reflectable", attack.reflectable);
        std::unique_ptr<TiXmlElement> damage(new TiXmlElement("Damage"));
        std::unique_ptr<TiXmlElement> damage_power(new TiXmlElement("Parameter"));
        damage_power->SetAttribute("id", "power");
        damage_power->SetAttribute("value", attack.damage.power);
        damage->LinkEndChild(damage_power.release());
        std::unique_ptr<TiXmlElement> damage_formula(new TiXmlElement("Parameter"));
        damage_formula->SetAttribute("id", "formula");
        damage_formula->SetAttribute("value", attack.damage.formula);
        damage->LinkEndChild(damage_formula.release());
        std::unique_ptr<TiXmlElement> damage_modifier(new TiXmlElement("Parameter"));
        damage_modifier->SetAttribute("id", "modifier");
        damage_modifier->SetAttribute("value", attack.damage.modifier);
        damage->LinkEndChild(damage_modifier.release());
        std::unique_ptr<TiXmlElement> damage_acc(new TiXmlElement("Parameter"));
        damage_acc->SetAttribute("id", "accuracy");
        damage_acc->SetAttribute("value", attack.damage.accuracy);
        damage->LinkEndChild(damage_acc.release());
        std::unique_ptr<TiXmlElement> damage_darkness(new TiXmlElement("Parameter"));
        damage_darkness->SetAttribute("id", "affected_by_darkness");
        damage_darkness->SetAttribute("value", attack.damage.affected_by_darkness);
        damage->LinkEndChild(damage_darkness.release());
        std::unique_ptr<TiXmlElement> damage_restore(new TiXmlElement("Parameter"));
        damage_restore->SetAttribute("id", "restore");
        damage_restore->SetAttribute("value", static_cast<int>(attack.damage.restore));
        damage->LinkEndChild(damage_restore.release());
        std::unique_ptr<TiXmlElement> damage_mp(new TiXmlElement("Parameter"));
        damage_mp->SetAttribute("id", "damage_mp");
        damage_mp->SetAttribute("value", attack.damage.damage_mp);
        damage->LinkEndChild(damage_mp.release());
        std::unique_ptr<TiXmlElement> damage_drain_hp(new TiXmlElement("Parameter"));
        damage_drain_hp->SetAttribute("id", "drain_hp");
        damage_drain_hp->SetAttribute("value", attack.damage.drain_hp);
        damage->LinkEndChild(damage_drain_hp.release());
        std::unique_ptr<TiXmlElement> damage_drain_mp(new TiXmlElement("Parameter"));
        damage_drain_mp->SetAttribute("id", "drain_mp");
        damage_drain_mp->SetAttribute("value", attack.damage.drain_mp);
        damage->LinkEndChild(damage_drain_mp.release());
        std::unique_ptr<TiXmlElement> damage_ignore(new TiXmlElement("Parameter"));
        damage_ignore->SetAttribute("id", "ignore_status_defense");
        damage_ignore->SetAttribute("value", attack.damage.ignore_status_defense);
        damage->LinkEndChild(damage_ignore.release());
        std::unique_ptr<TiXmlElement> damage_critical(new TiXmlElement("Parameter"));
        damage_critical->SetAttribute("id", "always_critical");
        damage_critical->SetAttribute("value", attack.damage.critical);
        damage->LinkEndChild(damage_critical.release());
        container->LinkEndChild(damage.release());
        std::unique_ptr<TiXmlElement> target_opts(new TiXmlElement("Target"));
        std::unique_ptr<TiXmlElement> target_se(new TiXmlElement("Parameter"));
        target_se->SetAttribute("id", "selection_enabled");
        target_se->SetAttribute("value", attack.target.selection_enabled);
        target_opts->LinkEndChild(target_se.release());
        std::unique_ptr<TiXmlElement> target_de(new TiXmlElement("Parameter"));
        target_de->SetAttribute("id", "default_enemy");
        target_de->SetAttribute("value", attack.target.default_enemy);
        target_opts->LinkEndChild(target_de.release());
        std::unique_ptr<TiXmlElement> target_dm(new TiXmlElement("Parameter"));
        target_dm->SetAttribute("id", "default_multiple");
        target_dm->SetAttribute("value", attack.target.default_multiple);
        target_opts->LinkEndChild(target_dm.release());
        std::unique_ptr<TiXmlElement> target_tm(new TiXmlElement("Parameter"));
        target_tm->SetAttribute("id", "toggle_multiple");
        target_tm->SetAttribute("value", attack.target.toggle_multiple);
        target_opts->LinkEndChild(target_tm.release());
        std::unique_ptr<TiXmlElement> target_fr(new TiXmlElement("Parameter"));
        target_fr->SetAttribute("id", "fixed_row");
        target_fr->SetAttribute("value", attack.target.fixed_row);
        target_opts->LinkEndChild(target_fr.release());
        std::unique_ptr<TiXmlElement> target_sr(new TiXmlElement("Parameter"));
        target_sr->SetAttribute("id", "short_range");
        target_sr->SetAttribute("value", attack.target.short_range);
        target_opts->LinkEndChild(target_sr.release());
        std::unique_ptr<TiXmlElement> target_ar(new TiXmlElement("Parameter"));
        target_ar->SetAttribute("id", "all_rows");
        target_ar->SetAttribute("value", attack.target.all_rows);
        target_opts->LinkEndChild(target_ar.release());
        std::unique_ptr<TiXmlElement> target_rd(new TiXmlElement("Parameter"));
        target_rd->SetAttribute("id", "random");
        target_rd->SetAttribute("value", attack.target.random);
        target_opts->LinkEndChild(target_rd.release());
        std::unique_ptr<TiXmlElement> target_od(new TiXmlElement("Parameter"));
        target_od->SetAttribute("id", "only_for_dead");
        target_od->SetAttribute("value", attack.target.only_for_dead);
        target_opts->LinkEndChild(target_od.release());
        std::unique_ptr<TiXmlElement> target_nr(new TiXmlElement("Parameter"));
        target_nr->SetAttribute("id", "no_retarget");
        target_nr->SetAttribute("value", attack.target.no_re_target);
        target_opts->LinkEndChild(target_nr.release());
        container->LinkEndChild(target_opts.release());
        std::unique_ptr<TiXmlElement> effects(new TiXmlElement("Effects"));
        for (Attack::StatusEffect effect : attack.status_effects){
            std::unique_ptr<TiXmlElement> xml_effect(new TiXmlElement("Effect"));
            xml_effect->SetAttribute("id", effect.id);
            xml_effect->SetDoubleAttribute("chance", effect.chance);
            xml_effect->SetAttribute("mode", GameData::STATUS_EFFECT_MODE::to_string(effect.type));
            effects->LinkEndChild(xml_effect.release());
        }
        container->LinkEndChild(effects.release());
        std::unique_ptr<TiXmlElement> elements(new TiXmlElement("Elements"));
        for (unsigned int element : attack.elements){
            std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("Element"));
            xml_element->SetAttribute("id", element);
            elements->LinkEndChild(xml_element.release());
        }
        container->LinkEndChild(elements.release());
        std::unique_ptr<TiXmlElement> anim(new TiXmlElement("Animation"));
        std::unique_ptr<TiXmlElement> anim_e(new TiXmlElement("Parameter"));
        anim_e->SetAttribute("id", "effect");
        anim_e->SetAttribute("value", attack.animation.effect);
        anim->LinkEndChild(anim_e.release());
        std::unique_ptr<TiXmlElement> anim_ie(new TiXmlElement("Parameter"));
        anim_ie->SetAttribute("id", "impact_effect");
        anim_ie->SetAttribute("value", attack.animation.impact_effect);
        anim->LinkEndChild(anim_ie.release());
        std::unique_ptr<TiXmlElement> anim_s(new TiXmlElement("Parameter"));
        anim_s->SetAttribute("id", "sound");
        anim_s->SetAttribute("value", attack.animation.sound);
        anim->LinkEndChild(anim_s.release());
        std::unique_ptr<TiXmlElement> anim_ha(new TiXmlElement("Parameter"));
        anim_ha->SetAttribute("id", "hurt_anim");
        anim_ha->SetAttribute("value", attack.animation.hurt_anim);
        anim->LinkEndChild(anim_ha.release());
        std::unique_ptr<TiXmlElement> anim_c(new TiXmlElement("Parameter"));
        anim_c->SetAttribute("id", "camera_single");
        anim_c->SetAttribute("value", attack.animation.camera_single);
        anim->LinkEndChild(anim_c.release());
        std::unique_ptr<TiXmlElement> anim_cm(new TiXmlElement("Parameter"));
        anim_cm->SetAttribute("id", "camera_multiple");
        anim_cm->SetAttribute("value", attack.animation.camera_multiple);
        anim->LinkEndChild(anim_cm.release());
        container->LinkEndChild(anim.release());
        xml.LinkEndChild(container.release());
        std::string file_name = BuildAttackFileName(attack);
        xml.SaveFile(output_dir_ + file_name);
        attack_map_[attack.id] = file_name;
    }
}

void BattleDataInstaller::WriteFormations(){
    for (Formation formation : formations_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("Formation"));
        container->SetAttribute("id", formation.id);
        container->SetAttribute("next", formation.next_formation);
        container->SetDoubleAttribute("escape_difficulty", formation.escape_counter);
        container->SetAttribute("escapable", formation.escapable);
        container->SetAttribute("skip_victory_pose", formation.skip_victory_pose);
        container->SetAttribute("skip_spoils", formation.skip_spoils);
        container->SetAttribute("skip_victory_pose", formation.skip_victory_pose);
        container->SetAttribute("preemptive_disabled", formation.preemptive_disabled);
        container->SetAttribute("layout", static_cast<int>(formation.layout));
        std::unique_ptr<TiXmlElement> location(new TiXmlElement("Location"));
        location->SetAttribute("id", formation.location);
        location->SetAttribute("name", formation.location_name);
        container->LinkEndChild(location.release());
        std::unique_ptr<TiXmlElement> enemies(new TiXmlElement("Enemies"));
        for (Formation::Enemy enemy : formation.enemies){
            std::unique_ptr<TiXmlElement> enemy_xml(new TiXmlElement("Enemy"));
            enemy_xml->SetAttribute("id", enemy.id);
            enemy_xml->SetAttribute("x", enemy.x);
            enemy_xml->SetAttribute("y", enemy.y);
            enemy_xml->SetAttribute("z", enemy.z);
            enemy_xml->SetAttribute("row", enemy.row);
            std::string cover = "";
            for (int c = 0; c < 5; c ++) cover += enemy.cover[c];
            enemy_xml->SetAttribute("cover", cover);
            enemy_xml->SetAttribute("visible", enemy.visible);
            enemy_xml->SetAttribute("direction", enemy.direction);
            enemy_xml->SetAttribute("targeteable", enemy.targeteable);
            enemy_xml->SetAttribute("main_script_active", enemy.main_script_active);
            enemies->LinkEndChild(enemy_xml.release());
        }
        container->LinkEndChild(enemies.release());
        std::unique_ptr<TiXmlElement> cameras(new TiXmlElement("Camera"));
        cameras->SetAttribute("initial", formation.initial_camera_position);
        int c = 0;
        for (Formation::Camera camera : formation.camera_positions){
            std::unique_ptr<TiXmlElement> camera_xml(new TiXmlElement("Position"));
            camera_xml->SetAttribute("id", c);
            camera_xml->SetAttribute("x", camera.x);
            camera_xml->SetAttribute("y", camera.y);
            camera_xml->SetAttribute("z", camera.z);
            camera_xml->SetAttribute("direction_x", camera.d_x);
            camera_xml->SetAttribute("direction_y", camera.d_y);
            camera_xml->SetAttribute("direction_z", camera.d_z);
            cameras->LinkEndChild(camera_xml.release());
            c ++;
        }
        container->LinkEndChild(cameras.release());
        std::unique_ptr<TiXmlElement> arena(new TiXmlElement("Arena"));
        arena->SetAttribute("is_arena", formation.is_arena_battle);
        for (int next : formation.next_arena_formation_candidates){
            std::unique_ptr<TiXmlElement> next_xml(new TiXmlElement("NextCandidate"));
            next_xml->SetAttribute("id", next);
            arena->LinkEndChild(next_xml.release());
        }
        container->LinkEndChild(arena.release());
        xml.LinkEndChild(container.release());
        std::string file_name = std::to_string(formation.id);
        while (file_name.length() < 4) file_name = "0" + file_name;
        file_name = "game/formation/" + file_name + ".xml";
        xml.SaveFile(output_dir_ + file_name);
        formation_map_[formation.id] = file_name;
    }
}

std::string BattleDataInstaller::BuildEnemyFileName(Enemy enemy){
    std::string file_name = "game/enemy/";
    std::string id = std::to_string(enemy.id);
    while (id.size() < 4) id = "0" + id;
    file_name += (id + "_");
    for (int n = 0; n < enemy.name.size(); n ++){
        if (
          (enemy.name[n] >= '0' && enemy.name[n] <= '9')
          || (enemy.name[n] >= 'a' && enemy.name[n] <= 'z')
          || (enemy.name[n] >= 'A' && enemy.name[n] <= 'Z')
        ){
            file_name += enemy.name[n];
        }
    }
    file_name += ".xml";
    return file_name;
}

std::string BattleDataInstaller::BuildAttackFileName(Attack attack){
    std::string file_name = "game/attack/";
    std::string id = std::to_string(attack.id);
    while (id.size() < 4) id = "0" + id;
    file_name += (id + "_");
    for (int n = 0; n < attack.name.size(); n ++){
        if (
          (attack.name[n] >= '0' && attack.name[n] <= '9')
          || (attack.name[n] >= 'a' && attack.name[n] <= 'z')
          || (attack.name[n] >= 'A' && attack.name[n] <= 'Z')
        ){
            file_name += attack.name[n];
        }
    }
    file_name += ".xml";
    return file_name;
}

File* BattleDataInstaller::ExtractGZipScene(File file){
    u32 offset = 0;
    u8* file_buffer = static_cast<u8*>(malloc(file.GetFileSize()));
    file.GetFileBuffer(file_buffer, 0, file.GetFileSize());
    u32 extract_size = 256 * 1024;
    u8* extract_buffer = static_cast<u8*>(malloc(extract_size));
    int ret;
    z_stream strm;
    strm.zalloc = Z_NULL; // Used to allocate the internal state.
    strm.zfree = Z_NULL; // Used to free the internal state.
    strm.opaque = Z_NULL; // Private data object passed to zalloc and zfree.
    strm.next_in = file_buffer + offset; //Next input byte.
    strm.avail_in = file.GetFileSize() - offset; // Number of bytes available at next_in.
    strm.next_out = extract_buffer; // Next output byte should be put there.
    strm.avail_out = extract_size; // Remaining free space at next_out.
    ret = inflateInit2(&strm, 15 + 32);
    if (ret != Z_OK){
        switch (ret){
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflateInit2 - Z_MEM_ERROR in file " << std::endl;
                break;
            case Z_STREAM_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflateInit2 - Z_STREAM_ERROR in file " << std::endl;
                break;
            default:
                inflateEnd(&strm);
                std::cout << "Warning: inflateInit2 - unknown error in file " << std::endl;
        }
        return NULL;
    }
    do{
        if (strm.next_out == NULL){
            inflateEnd(&strm);
            std::cout << "Warning: strm.next_out == NULL in file " << std::endl;
            return NULL;
        }
        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);
        switch (ret){
            case Z_NEED_DICT:
                inflateEnd(&strm);
                std::cout << "Warning: inflate - Z_NEED_DICT in file " << std::endl;
                return NULL;
            case Z_DATA_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflate - Z_DATA_ERROR in file " << std::endl;
                return NULL;
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflate - Z_MEM_ERROR in file " << std::endl;
                return NULL;
        }
        if (ret != Z_STREAM_END){
            extract_buffer = static_cast<u8*>(realloc(extract_buffer, extract_size * 2));
            if (extract_buffer == NULL){
                inflateEnd(&strm);
                std::cout << "Warning: extract_buffer == NULL in file " << std::endl;
                return NULL;
            }
            strm.next_out = static_cast<u8*>(extract_buffer + extract_size);
            strm.avail_out = extract_size;
            extract_size *= 2;
        }
    } while (ret != Z_STREAM_END);
    extract_size = extract_size - strm.avail_out;
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END){
        std::cout << "Warning: ret != Z_STREAM_END in file " << std::endl;
        return NULL;
    }
    File* out_file = new File(extract_buffer, 0, extract_size);
    return out_file;
}

void BattleDataInstaller::ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.getPointer(), outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName("models/battle/entities/" + base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(mesh.getPointer(), outdir + mesh->getName());
    Ogre::Mesh::SubMeshIterator it(mesh->getSubMeshIterator());
    Ogre::MaterialSerializer mat_ser;
    size_t i(0);
    std::set<std::string> textures;
    while (it.hasMoreElements()){
        Ogre::SubMesh *sub_mesh(it.getNext());
        Ogre::MaterialPtr mat(Ogre::MaterialManager::getSingleton().getByName(
          sub_mesh->getMaterialName())
        );
        if (mat != nullptr){
            for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
                Ogre::Technique* tech = mat->getTechnique(techs);
                if (tech){
                    for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                        Ogre::Pass* pass = tech->getPass(pass_num);
                        if (pass){
                            for (
                              size_t texture_unit_num = 0;
                              texture_unit_num < pass->getNumTextureUnitStates();
                              texture_unit_num ++
                            ){
                                Ogre::TextureUnitState* unit  = pass->getTextureUnitState(
                                  texture_unit_num
                                );
                                if (unit && unit->getTextureName().empty() == false){
                                     // Ensure the output material script references png files
                                    // rather than tex files.
                                    Ogre::String base_name;
                                    VGears::StringUtil::splitBase(
                                      unit->getTextureName(), base_name
                                    );
                                    unit->setTextureName(
                                      "models/battle/entities/" + base_name + ".png"
                                    );
                                    textures.insert(unit->getTextureName());
                                }
                            }
                        }
                    }
                }
            }
            if (std::count(materials_.begin(), materials_.end(), sub_mesh->getMaterialName()) == 0){
                mat_ser.queueForExport(mat);
                materials_.push_back(sub_mesh->getMaterialName());
            }
        }
        ++ i;
    }
    mat_ser.exportQueued(outdir + base_mesh_name + VGears::EXT_MATERIAL);
    for (auto& texture_name : textures){
        std::string tex_name = texture_name.c_str();
        Ogre::String base_name;
        VGears::StringUtil::splitBase(
          tex_name, base_name
        );
        base_name += ".png";
        try{
            Ogre::TexturePtr texture_ptr
              = Ogre::TextureManager::getSingleton().load(base_name, "FFVIITextures");
            Ogre::Image image;
            texture_ptr->convertToImage(image);
            Ogre::String base_name;
            VGears::StringUtil::splitBase(texture_name, base_name);
            image.save(outdir + base_mesh_name + "_" + base_name + ".png");
        }
        catch (std::exception const& ex){
            std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
        }
    }
}

void BattleDataInstaller::GenerateRsdFiles(Model model, std::string path){
    std::sort(model.tex.begin(), model.tex.end());
    // For each p file, generate a rsd file.
    for (std::string p : model.p){
        std::string base_name = p.substr(0, 8);
        std::string content = "@RSD940102\n";

        content += "PLY=" + base_name + ".PLY\n";
        content += "MAT=" + base_name + ".MAT\n";
        content += "GRP=" + base_name + ".GRP\n";
        content += "NTEX=" + std::to_string(model.tex.size()) + "\n";
        for (int t = 0; t < model.tex.size(); t ++)
            content += "TEX[" + std::to_string(t) + "]=" + model.tex[t] + "\n";
        content += "\n";
        std::ofstream out(path + base_name + ".rsd");
        out << content;
        out.close();
    }
}

void BattleDataInstaller::DecompileHrc(File compiled, Model model, std::string path){
    compiled.SetOffset(0);
    compiled.readU32LE();
    compiled.readU32LE();
    compiled.readU32LE();
    std::sort(model.p.begin(), model.p.end());
    std::string decompiled(":HEADER_BLOCK 2\n:SKELETON " + model.id + "aa\n");
    int bones = compiled.readU32LE();
    compiled.SetOffset(compiled.GetFileSize() - (bones * 12));
    decompiled += ":BONES " + std::to_string(bones) + "\n\n";
    int p_file_index = 0;
    for (int b = 0; b < bones; b ++){
        decompiled += "bone" + std::to_string(b) + "\n";
        int parent = compiled.readU32LE();
        if (parent == 0xFFFFFFFF) decompiled += "root\n";
        else decompiled += "bone" + std::to_string(parent) + "\n";
        union {
            float f;
            unsigned long ul;
            unsigned char b[4];
        } u;
        u.b[0] = compiled.readU8();
        u.b[1] = compiled.readU8();
        u.b[2] = compiled.readU8();
        u.b[3] = compiled.readU8();
        decompiled += std::to_string(u.f * -1) + "\n";
        int offset = compiled.readU32LE();
        if (offset == 0) decompiled += "0\n\n";
        else {
            decompiled += std::to_string(offset) + " ";
            if (p_file_index < model.p.size()){
                decompiled += model.p[p_file_index].substr(0, 8);
            }
            p_file_index ++;
            decompiled += "\n\n";
        }
    }
    std::ofstream out(path);
    out << decompiled;
    out.close();
}

void BattleDataInstaller::ExtractAFilesFromDAFile(File da, Model* model, std::string path){

    struct Bone{
        int raw[3]; // Z, Y, X.
        float angle[3]; // Z, Y, X.
    };

    da.SetOffset(0);
    int num_animations = da.readU32LE();
    for (int anim = 0; anim < num_animations; anim ++){
        std::string file_index_name = std::to_string(anim);
        while (file_index_name.size() < 2) file_index_name = "0" + file_index_name;
        std::ofstream a(
          path + model->id + "_" + file_index_name + ".a", std::ios::out | std::ios::binary
        );
        model->a.push_back(path + model->id + "_" + file_index_name + ".a");
        u32 zero = 0x00000000;
        u32 version = 0x00000001;
        u32 rotation_order = 0x00020001;
        u32 bone_count = da.readU32LE() - 1;
        u32 frames = da.readU32LE();
        Bone bones[frames][bone_count];
        a.write(reinterpret_cast<const char*>(&version), 4);
        a.write(reinterpret_cast<const char*>(&frames), 4);
        a.write(reinterpret_cast<const char*>(&bone_count), 4);
        a.write(reinterpret_cast<const char*>(&rotation_order), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        int next_offset = da.GetCurrentOffset() + da.readU32LE() + 4;
        da.readU16LE(); // Frames, again.
        da.readU16LE(); // Size, again.
        int key = da.readU8(); // Scale decoding key.
        da.readU8(); // ???
        da.readU32LE(); // ???
        da.readU32LE(); // ???
        int frame = 0;
        while (frame < frames){
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            int bone = 0;
            while (bone < bone_count){
                if (bone == 0){
                    // Uncompressed, 3 bytes per bone rotations. Read one bone.
                    for (int d = 0; d < 3; d ++){ // Read Z, Y, X.
                        int delta = da.readU16LE();
                        delta = delta << key;
                        bones[frame][bone].raw[d] = delta;
                        bones[frame][bone].angle[d] = delta;
                        if (bones[frame][bone].angle[d] < 0) bones[frame][bone].angle[d] += 0x1000;
                        bones[frame][bone].angle[d]
                          = bones[frame][bone].angle[d] / 4096.0f * 360.0f;
                        a.write(reinterpret_cast<const char*>(&bones[frame][bone].angle[d]), 4);
                    }
                    bone ++;
                }
                else if (key == 0){
                    // 12 bits per bone rotation, 36 bits per bone. Read two bones (9 bytes).
                    u8 data[9];
                    for (int d = 0; d < 9; d ++) data[d] = da.readU8();
                    // Read two bones.
                    float rot[6]; // Z, Y, X, Z, Y, X
                    bones[frame][bone].raw[0] = data[0] * 16 + (data[1] >> 4);
                    bones[frame][bone].raw[1] = (data[1] & 15) * 256 + data[2];
                    bones[frame][bone].raw[2] = data[3] * 16 + (data[4] >> 4);
                    bones[frame][bone + 1].raw[0] = (data[4] & 15) * 256 + data[5];
                    bones[frame][bone + 1].raw[1] = data[6] * 16 + (data[7] >> 4);
                    bones[frame][bone + 1].raw[2] = (data[7] & 15) * 256 + data[8];
                    if (frame > 0){ // Frames 1 and up, sum to the previous frame.
                        bones[frame][bone].raw[0] += bones[frame - 1][bone].raw[0];
                        bones[frame][bone].raw[1] += bones[frame - 1][bone].raw[1];
                        bones[frame][bone].raw[2] += bones[frame - 1][bone].raw[2];
                        bones[frame][bone + 1].raw[0] += bones[frame - 1][bone + 1].raw[0];
                        bones[frame][bone + 1].raw[1] += bones[frame - 1][bone + 1].raw[1];
                        bones[frame][bone + 1].raw[2] += bones[frame - 1][bone + 1].raw[2];
                    }

                    // Quaternion to degree.
                    for (int b = bone; b < bone + 2; b ++){
                        for (int r = 0; r < 3; r ++){
                            bones[frame][b].angle[r] = bones[frame][b].raw[r];
                            if (bones[frame][b].angle[r] < 0) bones[frame][b].angle[r] += 0x1000;
                            bones[frame][b].angle[r] = bones[frame][b].angle[r] / 4096.0f * 360.0f;
                            a.write(reinterpret_cast<const char*>(&bones[frame][b].angle[r]), 4);
                        }
                    }
                    bone += 2;

                }
                else if (key == 2){
                    // 10 bits per bone rotation, 30 bits per bone. Read four bones (15 bytes).
                    std::cout << "TODO: Decode bones with key 2.\n";
                    // TODO
                    bone += 4;
                }
                else if (key == 4){
                    // 8 bits per bone rotation, 24 bits per bone. Read one bone (3 bytes).
                    std::cout << "TODO: Decode bones with key 4.\n";
                    // TODO: Fix this and do the same as key 0.
                    u8 data[3];
                    for (int d = 0; d < 3; d ++){
                        if (bone + d < bone_count) data[d] = da.readU8();
                        else data[d] = 0;
                    }
                    // Read two bones.
                    float rot[3]; // Z, Y, X
                    rot[0] = data[0];
                    rot[1] = data[1];
                    rot[2] = data[3];
                    // Quaternion to degree.
                    for (int d = 0; d < 3; d ++) rot[d] = rot[d] * 360 / 4096;
                    // Write.
                    if (bone < bone_count){
                        a.write(reinterpret_cast<const char*>(&rot[0]), 4);
                        a.write(reinterpret_cast<const char*>(&rot[1]), 4);
                        a.write(reinterpret_cast<const char*>(&rot[2]), 4);
                        bone ++;
                    }
                }
                else{
                    // This should not happen.
                    bone ++;
                }
            }
            frame ++;
        }
        a.close();
        da.SetOffset(next_offset);
    }
}
