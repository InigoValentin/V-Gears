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

#include <OgreSingleton.h>
#include <boost/thread.hpp>
#include <vorbis/vorbisfile.h>

// Include OpenAL
#if defined(__WIN32__) || defined(_WIN32)
    #include <al.h>
    #include <alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

/**
 * The audio manager.
 *
 * It handles all music and sounds in the application.
 */
class AudioManager : public Ogre::Singleton<AudioManager>{

    public:

        /**
         * Constructor.
         */
        AudioManager();

        /**
         * Destructor
         */
        virtual ~AudioManager();

        /**
         * Used by boost libraries.
         */
        void operator()();

        /**
         * Updates the music player.
         */
        void Update();

        /**
         * Pauses currently playing music.
         *
         * @todo How to resume it?
         */
        void MusicPause();

        /**
         * Plays a music track.
         *
         * To be called from Lua scripts
         *
         * @param[in] name Name of the track to play.
         */
        void ScriptPlayMusic(const char* name);

        /**
         * Plays a music track.
         *
         * @param[in] name Name of the track to play.
         */
        void MusicPlay(const Ogre::String& name);

        /**
         * Plays a sound.
         *
         * To be called from Lua scripts
         *
         * @param[in] name Name of the sound to play.
         */
        void ScriptPlaySound(const char* name);

        /**
         * Plays a sound in a channel.
         *
         * To be called from Lua scripts
         *
         * @param[in] name Name of the sound to play.
         * @param[in] channel Channel to play the sound in (1-5).
         */
        void ScriptPlaySound(const char* name, const int channel);

        /**
         * Plays up to 4 sounds, in 4 different channels.
         *
         * To be called from Lua scripts.
         *
         * @param[in] name1 Name of the first sound to play.
         * @param[in] name2 Name of the second sound to play.
         * @param[in] name3 Name of the third sound to play.
         * @param[in] name4 Name of the fourth sound to play.
         */
        void ScriptPlaySounds(
          const char* name1, const char* name2, const char* name3, const char* name4
        );

        /**
         * Plays a sound.
         *
         * @param[in] name Name of the sound to play.
         */
        void SoundPlay(const Ogre::String& name);

        /**
         * Stops the currently playing music.
         *
         * Playback can't be resumed.
         */
        void MusicStop();

        /**
         * Music structure.
         *
         * Defines a music entry in musics.xml.
         */
        struct Music{

            /**
             * The name of the track.
             */
            Ogre::String name;

            /**
             * Track filename
             */
            Ogre::String file;

            /**
             * Music loop location for continous playback.
             *
             * @todo How is a loop done with only one value? shouldn't it be
             * start and end of loop?
             */
            float loop;
        };

        /**
         * Music structure.
         *
         * Defines a sound entry in sound.xml.
         */
        struct Sound{

            /**
             * The name of the sound.
             */
            Ogre::String name;

            /**
             * Sound filename
             */
            Ogre::String file;
        };

        /**
         * Adds a music track to the audio manager.
         *
         * @param[in] music The track to add.
         */
        void AddMusic(const AudioManager::Music& music);

        /**
         * Retrieves a music track by name.
         *
         * @param[in] name The track name.
         * @return The music track, or nullptr if there is no track by that name.
         */
        AudioManager::Music* GetMusic(const Ogre::String& name);

        /**
         * Adds a sound to the audio manager.
         *
         * @param[in] sound The sound to add.
         */
        void AddSound(const AudioManager::Sound& sound);

        /**
         * Retrieves a sound by name.
         *
         * @param[in] name The sound name.
         * @return The sound track, or nullptr if there is no sound by that name.
         */
        AudioManager::Sound* GetSound(const Ogre::String& name);

    private:

        /**
         * Initializes the audio manager.
         *
         * @todo Verify this comment.
         */
        const bool Init();

        /**
         * Handles errors
         *
         * @return nullptr
         * @todo Implement and get error info.
         */
        const char* ALError();

        /**
         * Handles errors
         *
         * @return nullptr
         * @todo Implement and get error info.
         */
        const char* ALCError( const ALCdevice* device );

        /**
         * An audio player
         */
        class Player{

            public:
                /**
                 * Constructor.
                 *
                 * @param[in] mutex Mutex to handle concurrent updates.
                 */
                Player(boost::recursive_mutex* mutex);

                /**
                 * Destructor.
                 */
                ~Player();

                /**
                 * Pauses the player.
                 */
                void Pause();

                /**
                 * Plays an audio file.
                 *
                 * @param[in] file Path to the file to play
                 */
                void Play(const Ogre::String& file);

                /**
                 * Stops the audio player.
                 */
                void Stop();

                /**
                 * Sets the loop for the current track.
                 *
                 * @param[in] loop Loop point
                 */
                void SetLoop(const float loop);

                /**
                 * Updates the audio player.
                 *
                 * @todo Understand and document.
                 */
                void Update();

                /**
                 * Get the playing position of the current track.
                 *
                 * @return Playing position.
                 * @todo Is this in seconds?
                 */
                float GetPosition();

            private:

                /**
                 * Mutex to handle concurrent update on the player.
                 */
                boost::recursive_mutex* update_mutex_;

                /**
                 * Loop point for the current track.
                 */
                float loop_;

                /**
                 * Name of the file currently being played.
                 */
                Ogre::String file_;

                /**
                 * File played or to play.
                 */
                OggVorbis_File vorbis_file_;

                /**
                 * Info about the track.
                 */
                vorbis_info* vorbis_info_;

                /**
                 * Section of the track.
                 */
                int vorbis_section_;

                /**
                 * Indicates if the stream is finished.
                 */
                bool stream_finished_;

                /**
                 * Audio buffer.
                 */
                char* buffer_;

                /**
                 * Audio source.
                 */
                ALuint source_;

                /**
                 * Fills the audio buffer.
                 *
                 * @return Size of the data added to the buffer.
                 */
                ALsizei FillBuffer();
        };

    private:

        /**
         * Indicatesif the audio manager has been initialized.
         */
        bool initialized_;

        /**
         * Audio output device.
         */
        ALCdevice* al_device_;

        /**
         * Audio context.
         */
        ALCcontext* al_context_;

        /**
         * Audio buffer.
         *
         * TODO: Unused? Not there are buffers per player. Remove.
         */
        char* buffer_;

        /**
         * Mutex to handle concurrent operations.
         */
        boost::recursive_mutex update_mutex_;

        /**
         * Thread to handle concurrent operations.
         */
        boost::thread* update_thread_;

        /**
         * Flag to indicate if the playback must continue.
         */
        bool thread_continue_;

        /**
         * Music player.
         */
        AudioManager::Player music_;

        /**
         * Sound effect player.
         *
         * TODO: Unused? remove.
         */
        AudioManager::Player fx_;

        /**
         * List of music.
         */
        std::list<AudioManager::Music> music_list_;

        /**
         * List of music.
         */
        std::list<AudioManager::Sound> sound_list_;

        /**
         * Size of a channel buffer.
         *
         * 96Kb are allocated for each channel. ~0.5 seconds of 44100Hz stereo
         * 16-bit data can be allocated in each channel. This way, 250ms can
         * pass between buffer updates.
         */
        static ALsizei channel_buffer_size_;

        /**
         * Number of buffers.
         *
         * 2 channels.
         */
        static int channel_buffer_number_;
};
