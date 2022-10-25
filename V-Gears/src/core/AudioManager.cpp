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

#include <list>
#include <boost/thread.hpp>
#include "core/AudioManager.h"
#include "core/XmlMusicsFile.h"
#include "core/Logger.h"

template<>AudioManager *Ogre::Singleton<AudioManager>::msSingleton = nullptr;

ALsizei AudioManager::channel_buffer_number_ = 2;
int AudioManager::channel_buffer_size_ = 96 * 1024;

AudioManager::AudioManager():
  initialized_(false), thread_continue_(true), update_mutex_(), music_(&update_mutex_)
{
    al_device_ = alcOpenDevice(nullptr);
    if (al_device_ != nullptr){
        al_context_ = alcCreateContext(al_device_, nullptr);
        if (al_context_ != nullptr){
            alcMakeContextCurrent(al_context_);
            // Listeners
            ALfloat position[3] = {0.0f, 0.0f, 0.0f};
            ALfloat velocity[3] = {0.0f, 0.0f, 0.0f};
            ALfloat orientation[6] = {0.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f};
            alListenerfv(AL_POSITION, position);
            alListenerfv(AL_VELOCITY, velocity);
            alListenerfv(AL_ORIENTATION, orientation);
            initialized_ = true;
            buffer_       = new char[channel_buffer_size_];
            update_thread_ = new boost::thread(boost::ref(*this));
            LOG_TRIVIAL("AudioManager initialised.");
        }
        else{
            LOG_ERROR(
              "AudioManager failed to initialised. Could not create context for sound device."
            );
        }
    }
    else LOG_ERROR("AudioManager failed to initialised. There's no default sound device.");

    // Load musics
    XmlMusicsFile musics("./data/musics.xml");
    musics.LoadMusics();
}

AudioManager::~AudioManager(){
    MusicStop();
    if (initialized_){
        thread_continue_ = false;
        update_thread_->join();
        delete update_thread_;
        delete[] buffer_;
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(al_context_);
        alcCloseDevice(al_device_);
        LOG_TRIVIAL("AudioManager destroyed.");
    }
}

void AudioManager::operator()(){
    while(thread_continue_){
        Update();
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC_);
        xt.nsec += 250000000; // sleep for 250 ms
        boost::thread::sleep(xt);
        update_thread_->yield();
    }
}

void AudioManager::Update(){
    boost::recursive_mutex::scoped_lock lock(update_mutex_);
    music_.Update();
}

void AudioManager::MusicPause(){
    boost::recursive_mutex::scoped_lock lock(update_mutex_);
    music_.Pause();
}

void AudioManager::MusicPlay(const Ogre::String& name){
    if (initialized_){
        boost::recursive_mutex::scoped_lock lock(update_mutex_);
        AudioManager::Music* music = GetMusic(name);
        if (music == nullptr){
            LOG_ERROR("No music found with name \"" + name + "\".");
            return;
        }
        music_.SetLoop(music->loop);
        music_.Play(music->file);
    }
}

void AudioManager::MusicStop(){
    boost::recursive_mutex::scoped_lock lock(update_mutex_);
    music_.Stop();
}


void AudioManager::AddMusic(const AudioManager::Music& music){
    boost::recursive_mutex::scoped_lock lock(update_mutex_);
    for (
      std::list<AudioManager::Music>::iterator it = music_list_.begin();
      it != music_list_.end();
      ++ it
    ){
        if (it->name == music.name){
            LOG_ERROR("Music with name '" + music.name + "' already exists.");
            return;
        }
    }
    music_list_.push_back(music);
}

AudioManager::Music* AudioManager::GetMusic(const Ogre::String& name){
    boost::recursive_mutex::scoped_lock lock(update_mutex_);
    for (auto it = music_list_.begin(); it != music_list_.end(); ++ it){
        if (it->name == name) return &(*it);
    }
    return nullptr;
}

const char* AudioManager::ALError(){
    //ALenum error_code = alGetError();
    //if (error_code == AL_NO_ERROR)
        return nullptr;
    //else return alGetString(error_code);
}

const char* AudioManager::ALCError(const ALCdevice* device){
    //const ALCdevice *alc_device =
    //  ((device == nullptr) ? const_cast< ALCdevice* >(al_device_) :
    //    const_cast< ALCdevice* >(device));
    //ALCenum error_code = alcGetError(const_cast< ALCdevice* >(alc_device));
    //if (error_code == ALC_NO_ERROR)
        return nullptr;
    //else return alcGetString(const_cast< ALCdevice* >(device), error_code);
}

AudioManager::Player::Player(boost::recursive_mutex* mutex):
  loop_(-1.0), vorbis_info_(nullptr), vorbis_section_(0),
  stream_finished_(false), update_mutex_(mutex)
{}

AudioManager::Player::~Player(){Stop();}

void AudioManager::Player::Pause(){
    boost::recursive_mutex::scoped_lock lock(*update_mutex_);
    alSourcePause(source_);
}

void AudioManager::Player::Play(const Ogre::String &file){
    boost::recursive_mutex::scoped_lock lock(*update_mutex_);
    // Open vorbis file.
    if (ov_fopen(const_cast<char*>(file.c_str()), &vorbis_file_)){
        LOG_ERROR("Can't play file \"" + file + "\".");
        return;
    }
    // Get file info.
    vorbis_info_ = ov_info(&vorbis_file_, -1.0f);
    // Create sound source
    alGenSources(1, &source_);
    // Create buffers
    ALuint buffers[channel_buffer_number_];
    alGenBuffers(channel_buffer_number_, buffers);
    // Set source parameters
    alSourcei(source_, AL_LOOPING, AL_FALSE);
    // Fill buffers
    for (int i = 0; i < channel_buffer_number_; ++ i){
        ALsizei buffer_size = FillBuffer();
        if (buffer_size){
            alBufferData(
              buffers[i], vorbis_info_->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
              static_cast<const ALvoid*>(AudioManager::getSingleton().buffer_),
              static_cast<ALsizei>(buffer_size), static_cast<ALsizei>(vorbis_info_->rate)
            );
            alSourceQueueBuffers(source_, 1, &buffers[i]);
        }
        else{
            stream_finished_ = true;
            alDeleteBuffers(1, &buffers[i]);
        }
    }
    // Start playback
    alSourcePlay(source_);
}

void AudioManager::Player::Stop(){
    boost::recursive_mutex::scoped_lock lock(*update_mutex_);
    // Stop source
    alSourceStop(source_);
    // Get source buffers
    int queued_count = 0;
    alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued_count);
    // Dequeue and delete buffers
    for(int i = 0; i < queued_count; ++ i){
        ALuint buffer_id;
        alSourceUnqueueBuffers(source_, 1, &buffer_id);
        alDeleteBuffers(1, &buffer_id);
    }
    // Delete source
    alDeleteSources(1, &source_);
    // Cleanup
    ov_clear(&vorbis_file_);
}

void AudioManager::Player::SetLoop(const float loop){
    boost::recursive_mutex::scoped_lock lock(*update_mutex_);
    loop_ = loop;
}

void AudioManager::Player::Update(){
    // Try to fill processed buffers
    int processed;
    alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);
    for(int i = 0; i < processed; ++i){
        // Try to fill buffer
        ALsizei buffer_size = FillBuffer();
        if (buffer_size){
            ALuint buffer_id;
            alSourceUnqueueBuffers(source_, 1, &buffer_id);
            alBufferData(
              buffer_id, vorbis_info_->channels == 1 ?
                AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
              static_cast<const ALvoid*>(AudioManager::getSingleton().buffer_),
              static_cast<ALsizei>(buffer_size), static_cast<ALsizei>(vorbis_info_->rate)
            );
            alSourceQueueBuffers(source_, 1, &buffer_id);
        }
        // Finished reading stream
        else{
            stream_finished_ = true;
            break;
        }
    }

    // Manage source state
    alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);
    int queued;
    alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);
    if (stream_finished_ && processed == queued) Stop();
    else{
        ALenum source_state;
        alGetSourcei(source_, AL_SOURCE_STATE, &source_state);
        if (source_state == AL_STOPPED) alSourcePlay(source_);
    }
}

ALsizei AudioManager::Player::FillBuffer(){
    ALsizei read = 0;
    if (stream_finished_) return read;
    char *&buffer = AudioManager::getSingleton().buffer_;
    bool finished = false;
    do{
        long result = ov_read(
          &vorbis_file_, buffer + read, channel_buffer_size_ - read, 0, 2, 1, &vorbis_section_
        );

        switch(result){
            // Error
            case OV_HOLE:
            case OV_EBADLINK:
            case OV_EINVAL:
                finished = true;
                break;

            // End of stream
            case 0:
                // If there isn't loop point or can't seek
                if (loop_ < 0.0f || ov_time_seek(&vorbis_file_, loop_)) finished = true;
            break;
            // Readed "result" bytes
            default:
                read += result;
                if (read == channel_buffer_size_) finished = true;
        }
    }
    while (finished == false);
    return read;
}

float AudioManager::Player::GetPosition(){
    boost::recursive_mutex::scoped_lock lock(*update_mutex_);
    int play_offset;
    alGetSourcei(source_, AL_SAMPLE_OFFSET, &play_offset);
    return (
      ov_pcm_tell(&vorbis_file_)
      - (
        channel_buffer_number_ * channel_buffer_size_ / vorbis_info_->channels / sizeof(int16_t)
        - play_offset
      )
    ) * 1000 / vorbis_info_->rate;
}
