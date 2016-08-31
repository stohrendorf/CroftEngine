#pragma once

#include "Ref.h"
#include "Properties.h"
#include "Curve.h"


namespace gameplay
{
    class AnimationController;
    class AnimationClip;


    /**
     * Defines a generic property animation.
     *
     * To run an animation you must play an AnimationClip.
     * Every Animation has the default clip which will run from begin-end time.
     * You can create additional clips to run only parts of an animation and control
     * various runtime characteristics, such as repeat count, etc.
     *
     * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Animation
     */
    class Animation : public Ref
    {
        friend class AnimationClip;

    public:

        /**
         * Gets the Animation's ID.
         *
         * @return The Animation's ID.
         */
        const std::string& getId() const;

        /**
         * Gets the Animation's duration.
         *
         * @return The Animation's duration (in milliseconds).
         */
        std::chrono::microseconds getDuration() const;

        /**
         * Creates an AnimationClip from the Animation.
         *
         * @param id The ID to the give the AnimationClip.
         * @param begin The begin time (in milliseconds) or keyframe(for keyframe animations).
         * @param end The end time (in milliseconds) or keyframe (for keyframe animations).
         *
         * @return The newly created AnimationClip; nullptr if an AnimationClip already exists with the same ID.
         * @script{create}
         */
        AnimationClip* createClip(const std::string& id, const std::chrono::microseconds& begin, const std::chrono::microseconds& end);

        /**
         * Finds the AnimationClip with the specified name. If nullptr, gets the default clip.
         *
         * @param clipId The ID of the AnimationClip to get.
         *
         * @return The AnimationClip with the specified ID; nullptr if an AnimationClip with the given ID is not found.
         */
        AnimationClip* getClip(const std::string& clipId);

        /**
         * Returns the AnimationClip at the given index.
         *
         * @param index Index of the clip to return.
         */
        AnimationClip* getClip(unsigned int index) const;

        /**
         * Returns the number of animation clips in this animation.
         */
        unsigned int getClipCount() const;

        /**
         * Plays the AnimationClip with the specified name.
         *
         * @param clipId The ID of the AnimationClip to play. If nullptr, plays the default clip.
         */
        void play(const std::string& clipId = nullptr);

        /**
         * Stops the AnimationClip with the specified name.
         *
         * @param clipId The ID of the AnimationClip to stop. If nullptr, stops the default clip.
         */
        void stop(const std::string& clipId = nullptr);

        /**
         * Pauses the AnimationClip with the specified name.
         *
         * @param clipId The ID of the AnimationClip to pause. If nullptr, pauses the default clip.
         */
        void pause(const std::string& clipId = nullptr);

    private:

        /**
         * Hidden copy constructor.
         */
        Animation(const Animation&) = delete;

        /**
         * Constructor.
         */
        explicit Animation(const char* id);

        /**
         * Destructor.
         */
        ~Animation();

        /**
         * Hidden copy assignment operator.
         */
        Animation& operator=(const Animation&) = delete;

        /**
         * Adds a clip to this Animation.
         */
        void addClip(AnimationClip* clip);

        /**
         * Finds the clip with the given ID.
         */
        AnimationClip* findClip(const std::string& id) const;

        AnimationController* _controller; // The AnimationController that this Animation will run on.
        std::string _id; // The Animation's ID.
        std::chrono::microseconds _duration; // the length of the animation (in milliseconds).
        std::vector<AnimationClip*>* _clips; // All the clips created from this Animation.
    };
}
