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
#include <zlib.h>
#include <tinyxml.h>
#include "BattleDataInstaller.h"

BattleDataInstaller::BattleDataInstaller(const std::string input_dir, const std::string output_dir):
  input_dir_(input_dir), output_dir_(output_dir),
  scene_bin_(input_dir + "data/battle/scene.bin"), total_scenes_(0), next_scene_(0)
{}

BattleDataInstaller::~BattleDataInstaller(){}

unsigned int BattleDataInstaller::Initialize(){
    next_scene_ = 0;
    scenes_.clear();
    enemies_.clear();
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

unsigned int BattleDataInstaller::ProcessScene(){
    if (next_scene_ >= scenes_uncompressed_.size()) return false;
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

void BattleDataInstaller::WriteEnemies(){
    for (Enemy enemy : enemies_){
        //std::cout << "Writting enemy " << enemy.id << ": " << (output_dir_ + "game/enemy/" + std::to_string(enemy.id) + ".xml") << "\n";
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
    //std::cout << "WRITE ATTACKS TOTAL: " << attacks_.size() << "\n";
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
        //std::cout << "Writting enemy " << enemy.id << ": " << (output_dir_ + "game/enemy/" + std::to_string(enemy.id) + ".xml") << "\n";
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
