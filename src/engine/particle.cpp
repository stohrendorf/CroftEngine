#include "particle.h"

#include "engine/laranode.h"
#include "render/scene/Sprite.h"

namespace engine
{
void Particle::initDrawables(const Engine& engine, const float scale)
{
    if(const auto& modelType = engine.findAnimatedModelForType(object_number))
    {
        for(const auto& model : modelType->models)
        {
            m_drawables.emplace_back(model.get());
        }
    }
    else if(const auto& spriteSequence = engine.findSpriteSequenceForType(object_number))
    {
        shade = 4096;

        for(const loader::file::Sprite& spr : spriteSequence->sprites)
        {
            auto sprite = std::make_shared<render::scene::Sprite>(spr.x0 * scale,
                                                                  -spr.y0 * scale,
                                                                  spr.x1 * scale,
                                                                  -spr.y1 * scale,
                                                                  spr.t0,
                                                                  spr.t1,
                                                                  engine.getSpriteMaterial(),
                                                                  render::scene::Sprite::Axis::Y);
            m_drawables.emplace_back(sprite);
            m_spriteTextures.emplace_back(spr.texture);
        }

        addMaterialParameterSetter("u_diffuseTexture",
                                   [this](const Node& /*node*/, render::gl::Program::ActiveUniform& uniform) {
                                       uniform.set(*m_spriteTextures.front());
                                   });
    }
    else
    {
        BOOST_LOG_TRIVIAL(warning) << "Missing sprite/model referenced by particle: "
                                   << toString(static_cast<TR1ItemId>(object_number.get()));
        return;
    }

    setDrawable(m_drawables.front());
    m_lighting.bind(*this);
}

glm::vec3 Particle::getPosition() const
{
    return pos.position.toRenderSystem();
}

Particle::Particle(const std::string& id,
                   const core::TypeId objectNumber,
                   const gsl::not_null<const loader::file::Room*>& room,
                   Engine& engine,
                   float scale)
    : Node{id}
    , Emitter{&engine.getSoundEngine()}
    , pos{room}
    , object_number{objectNumber}
{
    initDrawables(engine, scale);
}

Particle::Particle(const std::string& id,
                   const core::TypeId objectNumber,
                   const core::RoomBoundPosition& pos,
                   Engine& engine,
                   float scale)
    : Node{id}
    , Emitter{&engine.getSoundEngine()}
    , pos{pos}
    , object_number{objectNumber}
{
    initDrawables(engine, scale);
}

bool BloodSplatterParticle::update(Engine& engine)
{
    pos.position += util::pitch(speed * 1_frame, angle.Y);
    ++timePerSpriteFrame;
    if(timePerSpriteFrame != 4)
        return true;

    timePerSpriteFrame = 0;
    nextFrame();
    if(negSpriteFrameId <= engine.findSpriteSequenceForType(object_number)->length)
    {
        return false;
    }

    applyTransform();
    return true;
}

bool SplashParticle::update(Engine& engine)
{
    nextFrame();

    if(negSpriteFrameId <= engine.findSpriteSequenceForType(object_number)->length)
    {
        return false;
    }

    pos.position += util::pitch(speed * 1_frame, angle.Y);

    applyTransform();
    return true;
}

bool BubbleParticle::update(Engine& engine)
{
    angle.X += 13_deg;
    angle.Y += 9_deg;
    pos.position.X += util::sin(11_len, angle.Y);
    pos.position.Y -= 1_frame * speed;
    pos.position.Z += util::cos(8_len, angle.X);
    auto sector = findRealFloorSector(pos.position, &pos.room);
    if(sector == nullptr || !pos.room->isWaterRoom())
    {
        return false;
    }

    const auto ceiling = HeightInfo::fromCeiling(sector, pos.position, engine.getItemNodes()).y;
    if(ceiling == -core::HeightLimit || pos.position.Y <= ceiling)
    {
        return false;
    }

    applyTransform();
    return true;
}

bool FlameParticle::update(Engine& engine)
{
    nextFrame();
    if(negSpriteFrameId <= engine.findSpriteSequenceForType(object_number)->length)
        negSpriteFrameId = 0;

    if(timePerSpriteFrame >= 0)
    {
        engine.getAudioEngine().playSound(TR1SoundId::Burning, this);
        if(timePerSpriteFrame != 0)
        {
            --timePerSpriteFrame;
            applyTransform();
            return true;
        }

        if(engine.getLara().isNear(*this, 600_len))
        {
            // it's hot here, isn't it?
            engine.getLara().m_state.health -= 3_hp;
            engine.getLara().m_state.is_hit = true;

            const auto distSq = util::square(engine.getLara().m_state.position.position.X - pos.position.X)
                                + util::square(engine.getLara().m_state.position.position.Z - pos.position.Z);
            if(distSq < util::square(300_len))
            {
                timePerSpriteFrame = 100;

                const auto particle = std::make_shared<FlameParticle>(pos, engine);
                particle->timePerSpriteFrame = -1;
                setParent(particle, pos.room->node);
                engine.getParticles().emplace_back(particle);
            }
        }
    }
    else
    {
        // burn baby burn

        pos.position = {0_len, 0_len, 0_len};
        if(timePerSpriteFrame == -1)
        {
            pos.position.Y = -100_len;
        }
        else
        {
            pos.position.Y = 0_len;
        }

        const auto itemSpheres = engine.getLara().getSkeleton()->getBoneCollisionSpheres(
            engine.getLara().m_state,
            *engine.getLara().getSkeleton()->getInterpolationInfo(engine.getLara().m_state).getNearestFrame(),
            nullptr);

        pos.position = core::TRVec{
            glm::vec3{translate(itemSpheres.at(-timePerSpriteFrame - 1).m, pos.position.toRenderSystem())[3]}};

        const auto waterHeight = pos.room->getWaterSurfaceHeight(pos);
        if(!waterHeight.is_initialized() || waterHeight.get() >= pos.position.Y)
        {
            engine.getAudioEngine().playSound(TR1SoundId::Burning, this);
            engine.getLara().m_state.health -= 3_hp;
            engine.getLara().m_state.is_hit = true;
        }
        else
        {
            timePerSpriteFrame = 0;
            engine.getAudioEngine().stopSound(TR1SoundId::Burning, this);
            return false;
        }
    }

    applyTransform();
    return true;
}
} // namespace engine
