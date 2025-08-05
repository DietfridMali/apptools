#pragma once

#include "std_defines.h"
#include "conversions.hpp"
#include "SDL.h"
#include "SDL_mixer.h"
#include "string.hpp"
#include "vector.hpp"
#include "array.hpp"
#include "list.hpp"
#include "dictionary.hpp"
#include "singletonbase.hpp"

#include <math.h>

// =================================================================================================

class SoundObject 
{
    public:
        int         m_id;
        String      m_name;
        Mix_Chunk*  m_sound;
        int         m_channel;
        float       m_volume;
        Vector3f    m_position;
        void*       m_owner;
        size_t      m_startTime;
        size_t      m_endTime;

        SoundObject(int id = -1, String name = String(""), int channel = -1, Mix_Chunk * sound = nullptr, Vector4f position = {0, 0, 0}, float volume = 1.0f)
            : m_id(id), m_channel(channel), m_sound(sound), m_position(position), m_owner (nullptr), m_volume(volume), m_startTime (0), m_endTime(0)
        {}

        ~SoundObject () {
            if (Busy ())
                Stop ();
        }

        void Play (int loops = 1);

        void FadeOut(int fadeTime);

        void Stop (void);

        void SetPanning (float left, float right);

        void SetVolume (float volume);

        bool Busy (void) const;

        bool IsSilent(void) const;
    };

    // =================================================================================================
// The sound handler class handles sound creation and sound channel management
// It tries to provide 128 sound channels. They are preinitialized and are kept in m_idleChannels
// (list of available channels) and busyChannels (list of channels currently used for playing back sound)
// When a new sound is to played, a channel is picked from the idleChannels list. If there are no idle
// channels available, the oldest playing channel from busyChannels will be reused. Since channels are 
// append to busyChannels in the temporal sequence they are deployed, the first channel in busyChannels
// will always be the oldest one.

class BaseSoundHandler 
    : public PolymorphSingleton<BaseSoundHandler>
{
    public:
        Dictionary<String, Mix_Chunk*>  m_sounds;
        List<SoundObject>               m_idleChannels;
        List<SoundObject>               m_busyChannels;
        int                             m_soundLevel;
        float                           m_masterVolume;
        float                           m_maxAudibleDistance;
        int                             m_channelCount;

        struct SoundParams {
            float volume = 1.0f;
            int loops = 0;
            int level = 1;
        };

        BaseSoundHandler();

        ~BaseSoundHandler() = default;

        static BaseSoundHandler& Instance(void) { return static_cast<BaseSoundHandler&>(SingletonInstance()); }

        // preload sound data. Sound data is kept in a dictionary. The sound name is the key to it.
        void LoadSounds(String soundFolder, List<String> soundNames);

        SoundObject* FindSoundByOwner(const void* owner, const String& soundName);

        inline SoundObject* FindSoundByOwner(const void* owner, String&& soundName) {
            return FindSoundByOwner(owner, static_cast<const String&>(soundName));
        }

        // update all sound volumes depending on application specific cirumstances (e.g. listener or sound source have been moving)
        virtual void UpdateSound(SoundObject& soundObject) { }


        // play back the sound with the name 'name'. Position, viewer and DistFunc serve for computing the sound volume
        // depending on the distance of the viewer to the sound position
        SoundObject* Start(const String& soundName, const SoundParams& params, size_t startTime, const Vector3f position = Vector3f::NONE, const void* owner = nullptr);

        template <typename T>
        inline int Play(T&& soundName, const SoundParams& params, size_t startTime, const Vector3f position = Vector3f::NONE, const void* owner = nullptr) {
            SoundObject* activeSound = Start(std::forward<T>(soundName), params, startTime, position, owner);
            return (activeSound == nullptr) ? -1 : activeSound->m_id;
        }

        void FadeOut(int id, int fadeTime);
            
        void Stop(int id);

        void StopSoundsByOwner(void* owner);

        // move all channels that are not playing back sound anymore from the busyChannels to the idleChannels list
        void Cleanup(void);

        // cleanup expired channels and update sound volumes
        void Update(void);

    private:

        // compute stereo panning from the angle between the viewer direction and the vector from the viewer to the sound source
        virtual float Pan(Vector3f& position) { return 0.0f; }

        void UpdateVolume(SoundObject& soundObject, float distance);

        // get a channel for playing back a new sound
        // if all channels are busy, pick the oldest busy one
        SoundObject& GetChannel(void);

        template <typename Predicate>
        void ConditionalStop(Predicate condition)
        {
            for (auto it = m_busyChannels.begin(); it != m_busyChannels.end(); )
            {
                SoundObject& c = *it;
                if (not condition(c)) {
                    ++it;
                }
                else {
                    c.Stop();
                    m_idleChannels.Append(c);
                    // Achtung: erase mit reverse_iterator!
                    it = m_busyChannels.Discard(it);
                }
            }
        }
};

extern BaseSoundHandler* baseSoundHandlerInstance;

// =================================================================================================
