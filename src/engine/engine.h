#pragma once

#include "cameracontroller.h"
#include "floordata/floordata.h"
#include "loader/file/item.h"
#include "loader/file/animationid.h"
#include "util/cimgwrapper.h"

#include <boost/filesystem/path.hpp>

#include <memory>

namespace loader
{
namespace file
{
namespace level
{
class Level;
}

struct TextureKey;
struct Room;
struct Sector;
struct Mesh;
struct SkeletalModelType;
struct Box;
struct StaticMesh;
struct SpriteSequence;
struct AnimFrame;
struct Animation;
struct CinematicFrame;
}
}

namespace gameplay
{
namespace gl
{
class Font;
}
}

namespace render
{
class FullScreenFX;
}

namespace engine
{
namespace items
{
class ItemNode;


class PickupItem;
}

enum class TR1TrackId;


class Particle;


class InputHandler;


class Engine
{
public:
    explicit Engine();

    ~Engine();

    void run();

    std::shared_ptr<loader::file::level::Level> m_level;
    std::unique_ptr<CameraController> m_cameraController = nullptr;

    core::Frame m_effectTimer = 0_frame;
    boost::optional<size_t> m_activeEffect{};

    std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>> m_itemNodes;

    std::set<gsl::not_null<std::shared_ptr<items::ItemNode>>> m_dynamicItems;

    std::set<items::ItemNode*> m_scheduledDeletions;

    std::map<TR1TrackId, engine::floordata::ActivationState> m_cdTrackActivationStates;

    int m_cdTrack50time = 0;

    std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>> m_models;

    std::map<engine::TR1ItemId, size_t> m_inventory;

    int m_uvAnimTime{0};

    std::shared_ptr<gameplay::ShaderProgram> m_lightningShader;

    std::weak_ptr<audio::SourceHandle> m_underwaterAmbience;

    sol::state m_scriptEngine;

    std::map<loader::file::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>
    createMaterials(const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader);

    std::shared_ptr<LaraNode> createItems();

    void setUpRendering();

    const std::unique_ptr<loader::file::SkeletalModelType>& findAnimatedModelForType(TR1ItemId type) const;

    template<typename T>
    std::shared_ptr<T> createItem(const TR1ItemId type,
                                  const gsl::not_null<const loader::file::Room*>& room,
                                  const core::Angle& angle,
                                  const core::TRVec& position,
                                  const uint16_t activationState)
    {
        const auto& model = findAnimatedModelForType( type );
        if( model == nullptr )
            return nullptr;

        loader::file::Item item;
        item.type = type;
        item.room = uint16_t( -1 );
        item.position = position;
        item.rotation = angle;
        item.darkness = 0;
        item.activationState = activationState;

        auto node = std::make_shared<T>( this, room, item, *model );

        m_dynamicItems.emplace( node );
        addChild( room->node, node->getNode() );

        return node;
    }

    std::shared_ptr<items::PickupItem> createPickup(const TR1ItemId type,
                                                    const gsl::not_null<const loader::file::Room*>& room,
                                                    const core::TRVec& position);

    gsl::not_null<const loader::file::Room*> findRoomForPosition(const core::TRVec& position,
                                                                 gsl::not_null<const loader::file::Room*> room) const;

    std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                                 const core::TRVec& position) const;

    std::shared_ptr<LaraNode> m_lara = nullptr;

    std::shared_ptr<render::TextureAnimator> m_textureAnimator;

    std::shared_ptr<items::ItemNode> getItem(uint16_t id) const;

    void drawBars(const gsl::not_null<gameplay::Game*>& game,
                  const gsl::not_null<std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>>>& image) const;

    std::unique_ptr<InputHandler> m_inputHandler;

    audio::SoundEngine m_soundEngine;

    std::shared_ptr<audio::SourceHandle> playSound(const TR1SoundId id, audio::Emitter* emitter);

    std::shared_ptr<audio::SourceHandle> playSound(const TR1SoundId id, const glm::vec3& pos)
    {
        const auto handle = playSound( id, nullptr );
        handle->setPosition( pos );
        return handle;
    }

    gsl::not_null<std::shared_ptr<audio::Stream>> playStream(size_t trackId);

    void playStopCdTrack(TR1TrackId trackId, bool stop);

    void triggerNormalCdTrack(TR1TrackId trackId,
                              const floordata::ActivationState& activationRequest,
                              floordata::SequenceCondition triggerType);

    void triggerCdTrack(TR1TrackId trackId,
                        const floordata::ActivationState& activationRequest,
                        floordata::SequenceCondition triggerType);

    void stopSound(const TR1SoundId soundId, audio::Emitter* emitter);

    std::weak_ptr<audio::Stream> m_ambientStream;
    std::weak_ptr<audio::Stream> m_interceptStream;
    boost::optional<TR1TrackId> m_currentTrack;
    boost::optional<TR1SoundId> m_currentLaraTalk;

    void useAlternativeLaraAppearance(bool withHead = false);

    const gsl::not_null<std::shared_ptr<gameplay::Model>>& getModel(const size_t idx) const
    {
        Expects( idx < m_models.size() );

        return m_models[idx];
    }

    void scheduleDeletion(items::ItemNode* item)
    {
        m_scheduledDeletions.insert( item );
    }

    void applyScheduledDeletions()
    {
        if( m_scheduledDeletions.empty() )
            return;

        for( const auto& del : m_scheduledDeletions )
        {
            auto it = std::find_if( m_dynamicItems.begin(), m_dynamicItems.end(),
                                    [del](const std::shared_ptr<items::ItemNode>& i) {
                                        return i.get() == del;
                                    } );
            if( it == m_dynamicItems.end() )
                continue;
            m_dynamicItems.erase( it );
        }

        m_scheduledDeletions.clear();
    }

    bool roomsAreSwapped = false;

    std::vector<gsl::not_null<std::shared_ptr<Particle>>> m_particles;

    // list of meshes and models, resolved through m_meshIndices
    std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>> m_modelsDirect;
    std::vector<gsl::not_null<const loader::file::Mesh*>> m_meshesDirect;

    std::shared_ptr<gameplay::Material> m_spriteMaterial{nullptr};

    void turn180Effect(items::ItemNode& node);

    void dinoStompEffect(items::ItemNode& node);

    void laraNormalEffect();

    void laraBubblesEffect(items::ItemNode& node);

    void finishLevelEffect();

    void earthquakeEffect();

    void floodEffect();

    void chandelierEffect();

    void raisingBlockEffect();

    void stairsToSlopeEffect();

    void sandEffect();

    void explosionEffect();

    void laraHandsFreeEffect();

    void flipMapEffect();

    void unholsterRightGunEffect(items::ItemNode& node);

    void chainBlockEffect();

    void flickerEffect();

    void swapAllRooms();

    void setGlobalEffect(size_t fx)
    {
        m_activeEffect = fx;
        m_effectTimer = 0_frame;
    }

    void doGlobalEffect();

    void runEffect(const size_t id, items::ItemNode* node)
    {
        switch( id )
        {
            case 0:
                Expects( node != nullptr );
                return turn180Effect( *node );
            case 1:
                Expects( node != nullptr );
                return dinoStompEffect( *node );
            case 2:
                return laraNormalEffect();
            case 3:
                Expects( node != nullptr );
                return laraBubblesEffect( *node );
            case 4:
                return finishLevelEffect();
            case 5:
                return earthquakeEffect();
            case 6:
                return floodEffect();
            case 7:
                return chandelierEffect();
            case 8:
                return raisingBlockEffect();
            case 9:
                return stairsToSlopeEffect();
            case 10:
                return sandEffect();
            case 11:
                return explosionEffect();
            case 12:
                return laraHandsFreeEffect();
            case 13:
                return flipMapEffect();
            case 14:
                Expects( node != nullptr );
                return unholsterRightGunEffect( *node );
            case 15:
                return chainBlockEffect();
            case 16:
                return flickerEffect();
            default:
                BOOST_LOG_TRIVIAL( warning ) << "Unhandled effect: " << id;
        }
    }

    bool m_levelFinished = false;

    void swapWithAlternate(loader::file::Room& orig, loader::file::Room& alternate);

    void addInventoryItem(TR1ItemId id, size_t quantity = 1);

    bool takeInventoryItem(const TR1ItemId id, const size_t quantity = 1)
    {
        BOOST_LOG_TRIVIAL( debug ) << "Taking item " << toString( id ) << " from inventory";

        const auto it = m_inventory.find( id );
        if( it == m_inventory.end() )
            return false;

        if( it->second < quantity )
            return false;

        if( it->second == quantity )
            m_inventory.erase( it );
        else
            m_inventory[id] -= quantity;

        return true;
    }

    bool tryUseInventoryItem(TR1ItemId id);

    size_t countInventoryItem(const TR1ItemId id) const
    {
        const auto it = m_inventory.find( id );
        if( it == m_inventory.end() )
            return 0;

        return it->second;
    }

    void animateUV();

    YAML::Node save() const;

    void load(const YAML::Node& node);

    boost::optional<size_t> indexOfModel(const std::shared_ptr<gameplay::Drawable>& m) const
    {
        if( m == nullptr )
            return boost::none;

        for( size_t i = 0; i < m_models.size(); ++i )
            if( m_models[i].get() == m )
                return i;

        return boost::none;
    }

    std::array<floordata::ActivationState, 10> mapFlipActivationStates;

    items::ItemNode* m_pierre = nullptr;

    const std::vector<loader::file::Box>& getBoxes() const;

    const std::vector<loader::file::Room>& getRooms() const;

    const loader::file::StaticMesh* findStaticMeshById(core::StaticMeshId meshId) const;

    const std::unique_ptr<loader::file::SpriteSequence>& findSpriteSequenceForType(TR1ItemId type) const;

    bool isValid(const loader::file::AnimFrame* frame) const;

    const loader::file::Animation& getAnimation(loader::file::AnimationId id) const;

    const std::vector<loader::file::Animation>& getAnimations() const;

    const std::vector<loader::file::CinematicFrame>& getCinematicFrames() const;

    const std::vector<loader::file::Camera>& getCameras() const;

    const std::vector<int16_t>& getAnimCommands() const;

    const std::vector<uint16_t>& getOverlaps() const;

    std::shared_ptr<render::FullScreenFX> depthDarknessFx;
    std::shared_ptr<render::FullScreenFX> depthDarknessWaterFx;
    std::shared_ptr<gameplay::ScreenOverlay> screenOverlay;
    std::unique_ptr<gameplay::Game> game;
    sol::table levelInfo;

    void update(const bool godMode);

    static void drawText(const gsl::not_null<std::shared_ptr<gameplay::gl::Font>>& font, const int x, const int y,
                         const std::string& txt,
                         const gameplay::gl::RGBA8& col = {255, 255, 255, 255});

    void drawDebugInfo(const gsl::not_null<std::shared_ptr<gameplay::gl::Font>>& font, const int fps);

    const util::CImgWrapper splashImage;
    util::CImgWrapper splashImageScaled;
    std::shared_ptr<gameplay::gl::Font> abibasFont;

    void scaleSplashImage();

    void drawLoadingScreen(const std::string& state);;

    const std::vector<int16_t>& getPoseFrames() const;
};
}
