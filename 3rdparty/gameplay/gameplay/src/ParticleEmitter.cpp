#include "Base.h"
#include "ParticleEmitter.h"
#include "Game.h"
#include "Node.h"
#include "Scene.h"

#include <glm/gtc/random.hpp>

#include <boost/log/trivial.hpp>

#define PARTICLE_COUNT_MAX                       100
#define PARTICLE_EMISSION_RATE                   10
#define PARTICLE_EMISSION_RATE_TIME_INTERVAL     1000.0f / (float)PARTICLE_EMISSION_RATE
#define PARTICLE_UPDATE_RATE_MAX                 8


namespace gameplay
{
    ParticleEmitter::ParticleEmitter(Game* game, unsigned int particleCountMax)
        : Drawable()
        , _particleCountMax(particleCountMax)
        , _particleCount(0)
        , _particles(nullptr)
        , _emissionRate(PARTICLE_EMISSION_RATE)
        , _started(false)
        , _ellipsoid(false)
        , _sizeStartMin(1.0f)
        , _sizeStartMax(1.0f)
        , _sizeEndMin(1.0f)
        , _sizeEndMax(1.0f)
        , _energyMin(1000L)
        , _energyMax(1000L)
        , _colorStart{0,0,0,0}
        , _colorStartVar{0,0,0,0}
        , _colorEnd{1,1,1,1}
        , _colorEndVar{0,0,0,0}
        , _position{0,0,0}
        , _positionVar{0,0,0}
        , _velocity{0,0,0}
        , _velocityVar{1,1,1}
        , _acceleration{0,0,0}
        , _accelerationVar{0,0,0}
        , _rotationPerParticleSpeedMin(0.0f)
        , _rotationPerParticleSpeedMax(0.0f)
        , _rotationSpeedMin(0.0f)
        , _rotationSpeedMax(0.0f)
        , _rotationAxis{0,0,0}
        , _rotation{}
        , _spriteBatch(nullptr)
        , _spriteBlendMode(BLEND_ALPHA)
        , _spriteTextureWidth(0)
        , _spriteTextureHeight(0)
        , _spriteTextureWidthRatio(0)
        , _spriteTextureHeightRatio(0)
        , _spriteTextureCoords(nullptr)
        , _spriteAnimated(false)
        , _spriteLooped(false)
        , _spriteFrameCount(1)
        , _spriteFrameRandomOffset(0)
        , _spriteFrameDuration(0L)
        , _spriteFrameDurationSecs(0.0f)
        , _spritePercentPerFrame(0.0f)
        , _orbitPosition(false)
        , _orbitVelocity(false)
        , _orbitAcceleration(false)
        , _timePerEmission(PARTICLE_EMISSION_RATE_TIME_INTERVAL)
        , _emitTime(0)
        , _lastUpdated(0)
        , _game{game}
    {
        BOOST_ASSERT(particleCountMax > 0);
        BOOST_ASSERT(game != nullptr);
        _particles = new Particle[particleCountMax];
    }


    ParticleEmitter::~ParticleEmitter()
    {
        SAFE_DELETE_ARRAY(_particles);
        SAFE_DELETE_ARRAY(_spriteTextureCoords);
    }


    ParticleEmitter* ParticleEmitter::create(Game* game, const std::shared_ptr<Texture>& texture, BlendMode blendMode, unsigned int particleCountMax)
    {
        ParticleEmitter* emitter = new ParticleEmitter(game, particleCountMax);
        BOOST_ASSERT(emitter);

        emitter->setTexture(texture, blendMode);

        return emitter;
    }


    void ParticleEmitter::setTexture(const std::shared_ptr<Texture>& texture, BlendMode blendMode)
    {
        // Create new batch before releasing old one, in case the same texture
        // is used for both (so it's not released before passing to the new batch).
        auto batch = SpriteBatch::create(_game, texture, nullptr);
        batch->getSampler()->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);

        _spriteBatch = batch;
        _spriteBatch->getStateBlock()->setDepthWrite(false);
        _spriteBatch->getStateBlock()->setDepthTest(true);

        setBlendMode(blendMode);
        _spriteTextureWidth = texture->getWidth();
        _spriteTextureHeight = texture->getHeight();
        _spriteTextureWidthRatio = 1.0f / (float)texture->getWidth();
        _spriteTextureHeightRatio = 1.0f / (float)texture->getHeight();

        // By default assume only one frame which uses the entire texture.
        Rectangle texCoord((float)texture->getWidth(), (float)texture->getHeight());
        setSpriteFrameCoords(1, &texCoord);
    }


    std::shared_ptr<Texture> ParticleEmitter::getTexture() const
    {
        auto sampler = _spriteBatch ? _spriteBatch->getSampler() : nullptr;
        return sampler ? sampler->getTexture() : nullptr;
    }


    void ParticleEmitter::setParticleCountMax(unsigned int max)
    {
        _particleCountMax = max;
    }


    unsigned int ParticleEmitter::getParticleCountMax() const
    {
        return _particleCountMax;
    }


    unsigned int ParticleEmitter::getEmissionRate() const
    {
        return _emissionRate;
    }


    void ParticleEmitter::setEmissionRate(unsigned int rate)
    {
        BOOST_ASSERT(rate);
        _emissionRate = rate;
        _timePerEmission = 1000.0f / (float)_emissionRate;
    }


    void ParticleEmitter::start()
    {
        _started = true;
        _lastUpdated = 0;
    }


    void ParticleEmitter::stop()
    {
        _started = false;
    }


    bool ParticleEmitter::isStarted() const
    {
        return _started;
    }


    bool ParticleEmitter::isActive() const
    {
        if( _started )
            return true;

        if( !_node )
            return false;

        return (_particleCount > 0);
    }


    void ParticleEmitter::emitOnce(unsigned int particleCount)
    {
        BOOST_ASSERT(_node);
        BOOST_ASSERT(_particles);

        // Limit particleCount so as not to go over _particleCountMax.
        if( particleCount + _particleCount > _particleCountMax )
        {
            particleCount = _particleCountMax - _particleCount;
        }

        glm::mat4 world = _node->getWorldMatrix();
        glm::vec3 translation{ world[3] };

        // Take translation out of world matrix so it can be used to rotate orbiting properties.
        world[3][0] = 0.0f;
        world[3][1] = 0.0f;
        world[3][2] = 0.0f;

        // Emit the new particles.
        for( unsigned int i = 0; i < particleCount; i++ )
        {
            Particle* p = &_particles[_particleCount];

            generateColor(_colorStart, _colorStartVar, &p->_colorStart);
            generateColor(_colorEnd, _colorEndVar, &p->_colorEnd);
            p->_color = p->_colorStart;

            p->_energy = p->_energyStart = generateScalar(_energyMin, _energyMax);
            p->_size = p->_sizeStart = generateScalar(_sizeStartMin, _sizeStartMax);
            p->_sizeEnd = generateScalar(_sizeEndMin, _sizeEndMax);
            p->_rotationPerParticleSpeed = generateScalar(_rotationPerParticleSpeedMin, _rotationPerParticleSpeedMax);
            p->_angle = generateScalar(0.0f, p->_rotationPerParticleSpeed);
            p->_rotationSpeed = generateScalar(_rotationSpeedMin, _rotationSpeedMax);

            // Only initial position can be generated within an ellipsoidal domain.
            generateVector(_position, _positionVar, &p->_position, _ellipsoid);
            generateVector(_velocity, _velocityVar, &p->_velocity, false);
            generateVector(_acceleration, _accelerationVar, &p->_acceleration, false);
            generateVector(_rotationAxis, _rotationAxisVar, &p->_rotationAxis, false);

            // Initial position, velocity and acceleration can all be relative to the emitter's transform.
            // Rotate specified properties by the node's rotation.
            if( _orbitPosition )
            {
                p->_position = glm::vec3(world * glm::vec4(p->_position, 1));
            }

            if( _orbitVelocity )
            {
                p->_velocity = glm::vec3( world * glm::vec4(p->_velocity, 1) );
            }

            if( _orbitAcceleration )
            {
                p->_acceleration = glm::vec3(world * glm::vec4(p->_acceleration, 1));
            }

            // The rotation axis always orbits the node.
            if( p->_rotationSpeed != 0.0f && glm::length2(p->_rotationAxis) > std::numeric_limits<float>::epsilon() )
            {
                p->_rotationAxis = glm::vec3(world * glm::vec4(p->_rotationAxis, 1));
            }

            // Translate position relative to the node's world space.
            p->_position += translation;

            // Initial sprite frame.
            if( _spriteFrameRandomOffset > 0 )
            {
                p->_frame = rand() % _spriteFrameRandomOffset;
            }
            else
            {
                p->_frame = 0;
            }
            p->_timeOnCurrentFrame = 0.0f;

            ++_particleCount;
        }
    }


    unsigned int ParticleEmitter::getParticlesCount() const
    {
        return _particleCount;
    }


    void ParticleEmitter::setEllipsoid(bool ellipsoid)
    {
        _ellipsoid = ellipsoid;
    }


    bool ParticleEmitter::isEllipsoid() const
    {
        return _ellipsoid;
    }


    void ParticleEmitter::setSize(float startMin, float startMax, float endMin, float endMax)
    {
        _sizeStartMin = startMin;
        _sizeStartMax = startMax;
        _sizeEndMin = endMin;
        _sizeEndMax = endMax;
    }


    float ParticleEmitter::getSizeStartMin() const
    {
        return _sizeStartMin;
    }


    float ParticleEmitter::getSizeStartMax() const
    {
        return _sizeStartMax;
    }


    float ParticleEmitter::getSizeEndMin() const
    {
        return _sizeEndMin;
    }


    float ParticleEmitter::getSizeEndMax() const
    {
        return _sizeEndMax;
    }


    void ParticleEmitter::setEnergy(long energyMin, long energyMax)
    {
        _energyMin = energyMin;
        _energyMax = energyMax;
    }


    long ParticleEmitter::getEnergyMin() const
    {
        return _energyMin;
    }


    long ParticleEmitter::getEnergyMax() const
    {
        return _energyMax;
    }


    void ParticleEmitter::setColor(const glm::vec4& startColor, const glm::vec4& startColorVar, const glm::vec4& endColor, const glm::vec4& endColorVar)
    {
        _colorStart = startColor;
        _colorStartVar = startColorVar;
        _colorEnd = endColor;
        _colorEndVar = endColorVar;
    }


    const glm::vec4& ParticleEmitter::getColorStart() const
    {
        return _colorStart;
    }


    const glm::vec4& ParticleEmitter::getColorStartVariance() const
    {
        return _colorStartVar;
    }


    const glm::vec4& ParticleEmitter::getColorEnd() const
    {
        return _colorEnd;
    }


    const glm::vec4& ParticleEmitter::getColorEndVariance() const
    {
        return _colorEndVar;
    }


    void ParticleEmitter::setPosition(const glm::vec3& position, const glm::vec3& positionVar)
    {
        _position = position;
        _positionVar = positionVar;
    }


    const glm::vec3& ParticleEmitter::getPosition() const
    {
        return _position;
    }


    const glm::vec3& ParticleEmitter::getPositionVariance() const
    {
        return _positionVar;
    }


    const glm::vec3& ParticleEmitter::getVelocity() const
    {
        return _velocity;
    }


    const glm::vec3& ParticleEmitter::getVelocityVariance() const
    {
        return _velocityVar;
    }


    void ParticleEmitter::setVelocity(const glm::vec3& velocity, const glm::vec3& velocityVar)
    {
        _velocity = velocity;
        _velocityVar = velocityVar;
    }


    const glm::vec3& ParticleEmitter::getAcceleration() const
    {
        return _acceleration;
    }


    const glm::vec3& ParticleEmitter::getAccelerationVariance() const
    {
        return _accelerationVar;
    }


    void ParticleEmitter::setAcceleration(const glm::vec3& acceleration, const glm::vec3& accelerationVar)
    {
        _acceleration = acceleration;
        _accelerationVar = accelerationVar;
    }


    void ParticleEmitter::setRotationPerParticle(float speedMin, float speedMax)
    {
        _rotationPerParticleSpeedMin = speedMin;
        _rotationPerParticleSpeedMax = speedMax;
    }


    float ParticleEmitter::getRotationPerParticleSpeedMin() const
    {
        return _rotationPerParticleSpeedMin;
    }


    float ParticleEmitter::getRotationPerParticleSpeedMax() const
    {
        return _rotationPerParticleSpeedMax;
    }


    void ParticleEmitter::setRotation(float speedMin, float speedMax, const glm::vec3& axis, const glm::vec3& axisVariance)
    {
        _rotationSpeedMin = speedMin;
        _rotationSpeedMax = speedMax;
        _rotationAxis = axis;
        _rotationAxisVar = axisVariance;
    }


    float ParticleEmitter::getRotationSpeedMin() const
    {
        return _rotationSpeedMin;
    }


    float ParticleEmitter::getRotationSpeedMax() const
    {
        return _rotationSpeedMax;
    }


    const glm::vec3& ParticleEmitter::getRotationAxis() const
    {
        return _rotationAxis;
    }


    const glm::vec3& ParticleEmitter::getRotationAxisVariance() const
    {
        return _rotationAxisVar;
    }


    void ParticleEmitter::setBlendMode(BlendMode blendMode)
    {
        BOOST_ASSERT(_spriteBatch);
        BOOST_ASSERT(_spriteBatch->getStateBlock());

        switch( blendMode )
        {
            case BLEND_NONE:
                _spriteBatch->getStateBlock()->setBlend(false);
                break;
            case BLEND_ALPHA:
                _spriteBatch->getStateBlock()->setBlend(true);
                _spriteBatch->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
                _spriteBatch->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);
                break;
            case BLEND_ADDITIVE:
                _spriteBatch->getStateBlock()->setBlend(true);
                _spriteBatch->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
                _spriteBatch->getStateBlock()->setBlendDst(RenderState::BLEND_ONE);
                break;
            case BLEND_MULTIPLIED:
                _spriteBatch->getStateBlock()->setBlend(true);
                _spriteBatch->getStateBlock()->setBlendSrc(RenderState::BLEND_ZERO);
                _spriteBatch->getStateBlock()->setBlendDst(RenderState::BLEND_SRC_COLOR);
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported blend mode (" << blendMode << ").";
                break;
        }

        _spriteBlendMode = blendMode;
    }


    ParticleEmitter::BlendMode ParticleEmitter::getBlendMode() const
    {
        return _spriteBlendMode;
    }


    void ParticleEmitter::setSpriteAnimated(bool animated)
    {
        _spriteAnimated = animated;
    }


    bool ParticleEmitter::isSpriteAnimated() const
    {
        return _spriteAnimated;
    }


    void ParticleEmitter::setSpriteLooped(bool looped)
    {
        _spriteLooped = looped;
    }


    bool ParticleEmitter::isSpriteLooped() const
    {
        return _spriteLooped;
    }


    void ParticleEmitter::setSpriteFrameRandomOffset(int maxOffset)
    {
        _spriteFrameRandomOffset = maxOffset;
    }


    int ParticleEmitter::getSpriteFrameRandomOffset() const
    {
        return _spriteFrameRandomOffset;
    }


    void ParticleEmitter::setSpriteFrameDuration(long duration)
    {
        _spriteFrameDuration = duration;
        _spriteFrameDurationSecs = (float)duration / 1000.0f;
    }


    long ParticleEmitter::getSpriteFrameDuration() const
    {
        return _spriteFrameDuration;
    }


    unsigned int ParticleEmitter::getSpriteWidth() const
    {
        return (unsigned int)fabs(_spriteTextureWidth * (_spriteTextureCoords[2] - _spriteTextureCoords[0]));
    }


    unsigned int ParticleEmitter::getSpriteHeight() const
    {
        return (unsigned int)fabs(_spriteTextureHeight * (_spriteTextureCoords[3] - _spriteTextureCoords[1]));
    }


    void ParticleEmitter::setSpriteTexCoords(unsigned int frameCount, float* texCoords)
    {
        BOOST_ASSERT(frameCount);
        BOOST_ASSERT(texCoords);

        _spriteFrameCount = frameCount;
        _spritePercentPerFrame = 1.0f / (float)frameCount;

        SAFE_DELETE_ARRAY(_spriteTextureCoords);
        _spriteTextureCoords = new float[frameCount * 4];
        memcpy(_spriteTextureCoords, texCoords, frameCount * 4 * sizeof(float));
    }


    void ParticleEmitter::setSpriteFrameCoords(unsigned int frameCount, Rectangle* frameCoords)
    {
        BOOST_ASSERT(frameCount);
        BOOST_ASSERT(frameCoords);

        _spriteFrameCount = frameCount;
        _spritePercentPerFrame = 1.0f / (float)frameCount;

        SAFE_DELETE_ARRAY(_spriteTextureCoords);
        _spriteTextureCoords = new float[frameCount * 4];

        // Pre-compute texture coordinates from rects.
        for( unsigned int i = 0; i < frameCount; i++ )
        {
            _spriteTextureCoords[i * 4] = _spriteTextureWidthRatio * frameCoords[i].x;
            _spriteTextureCoords[i * 4 + 1] = 1.0f - _spriteTextureHeightRatio * frameCoords[i].y;
            _spriteTextureCoords[i * 4 + 2] = _spriteTextureCoords[i * 4] + _spriteTextureWidthRatio * frameCoords[i].width;
            _spriteTextureCoords[i * 4 + 3] = _spriteTextureCoords[i * 4 + 1] - _spriteTextureHeightRatio * frameCoords[i].height;
        }
    }


    void ParticleEmitter::setSpriteFrameCoords(unsigned int frameCount, int width, int height)
    {
        BOOST_ASSERT(width);
        BOOST_ASSERT(height);

        Rectangle* frameCoords = new Rectangle[frameCount];
        unsigned int cols = _spriteTextureWidth / width;
        unsigned int rows = _spriteTextureHeight / height;

        unsigned int n = 0;
        for( unsigned int i = 0; i < rows; ++i )
        {
            int y = i * height;
            for( unsigned int j = 0; j < cols; ++j )
            {
                int x = j * width;
                frameCoords[i * cols + j] = Rectangle(x, y, width, height);
                if( ++n == frameCount )
                {
                    break;
                }
            }

            if( n == frameCount )
            {
                break;
            }
        }

        setSpriteFrameCoords(frameCount, frameCoords);

        SAFE_DELETE_ARRAY(frameCoords);
    }


    unsigned int ParticleEmitter::getSpriteFrameCount() const
    {
        return _spriteFrameCount;
    }


    void ParticleEmitter::setOrbit(bool orbitPosition, bool orbitVelocity, bool orbitAcceleration)
    {
        _orbitPosition = orbitPosition;
        _orbitVelocity = orbitVelocity;
        _orbitAcceleration = orbitAcceleration;
    }


    bool ParticleEmitter::getOrbitPosition() const
    {
        return _orbitPosition;
    }


    bool ParticleEmitter::getOrbitVelocity() const
    {
        return _orbitVelocity;
    }


    bool ParticleEmitter::getOrbitAcceleration() const
    {
        return _orbitAcceleration;
    }


    long ParticleEmitter::generateScalar(long min, long max)
    {
        // Note: this is not a very good RNG, but it should be suitable for our purposes.
        long r = 0;
        for( unsigned int i = 0; i < sizeof(long) / sizeof(int); i++ )
        {
            r = r << 8; // sizeof(int) * CHAR_BITS
            r |= rand();
        }

        // Now we have a random long between 0 and MAX_LONG.  We need to clamp it between min and max.
        r %= max - min;
        r += min;

        return r;
    }


    float ParticleEmitter::generateScalar(float min, float max)
    {
        return min + (max - min) * glm::linearRand(0.0f, 1.0f);
    }


    void ParticleEmitter::generateVectorInRect(const glm::vec3& base, const glm::vec3& variance, glm::vec3* dst)
    {
        BOOST_ASSERT(dst);

        // Scale each component of the variance vector by a random float
        // between -1 and 1, then add this to the corresponding base component.
        dst->x = base.x + variance.x * glm::linearRand(-1.0f, 1.0f);
        dst->y = base.y + variance.y * glm::linearRand(-1.0f, 1.0f);
        dst->z = base.z + variance.z * glm::linearRand(-1.0f, 1.0f);
    }


    void ParticleEmitter::generateVectorInEllipsoid(const glm::vec3& center, const glm::vec3& scale, glm::vec3* dst)
    {
        BOOST_ASSERT(dst);

        // Generate a point within a unit cube, then reject if the point is not in a unit sphere.
        do
        {
            dst->x = glm::linearRand(-1.0f, 1.0f);
            dst->y = glm::linearRand(-1.0f, 1.0f);
            dst->z = glm::linearRand(-1.0f, 1.0f);
        } while( dst->length() > 1.0f );

        // Scale this point by the scaling vector.
        dst->x *= scale.x;
        dst->y *= scale.y;
        dst->z *= scale.z;

        // Translate by the center point.
        *dst += center;
    }


    void ParticleEmitter::generateVector(const glm::vec3& base, const glm::vec3& variance, glm::vec3* dst, bool ellipsoid)
    {
        if( ellipsoid )
        {
            generateVectorInEllipsoid(base, variance, dst);
        }
        else
        {
            generateVectorInRect(base, variance, dst);
        }
    }


    void ParticleEmitter::generateColor(const glm::vec4& base, const glm::vec4& variance, glm::vec4* dst)
    {
        BOOST_ASSERT(dst);

        // Scale each component of the variance color by a random float
        // between -1 and 1, then add this to the corresponding base component.
        dst->x = base.x + variance.x * glm::linearRand(-1.0f, 1.0f);
        dst->y = base.y + variance.y * glm::linearRand(-1.0f, 1.0f);
        dst->z = base.z + variance.z * glm::linearRand(-1.0f, 1.0f);
        dst->w = base.w + variance.w * glm::linearRand(-1.0f, 1.0f);
    }


    ParticleEmitter::BlendMode ParticleEmitter::getBlendModeFromString(const char* str)
    {
        BOOST_ASSERT(str);

        if( strcmp(str, "BLEND_NONE") == 0 || strcmp(str, "NONE") == 0 )
        {
            return BLEND_NONE;
        }
        else if( strcmp(str, "BLEND_OPAQUE") == 0 || strcmp(str, "OPAQUE") == 0 )
        {
            return BLEND_NONE;
        }
        else if( strcmp(str, "BLEND_ALPHA") == 0 || strcmp(str, "ALPHA") == 0 )
        {
            return BLEND_ALPHA;
        }
        else if( strcmp(str, "BLEND_TRANSPARENT") == 0 || strcmp(str, "TRANSPARENT") == 0 )
        {
            return BLEND_ALPHA;
        }
        else if( strcmp(str, "BLEND_ADDITIVE") == 0 || strcmp(str, "ADDITIVE") == 0 )
        {
            return BLEND_ADDITIVE;
        }
        else if( strcmp(str, "BLEND_MULTIPLIED") == 0 || strcmp(str, "MULTIPLIED") == 0 )
        {
            return BLEND_MULTIPLIED;
        }
        else
        {
            return BLEND_ALPHA;
        }
    }


    void ParticleEmitter::update(float elapsedTime)
    {
        if( !isActive() )
            return;

        // Cap particle updates at a maximum rate. This saves processing
        // and also improves precision since updating with very small
        // time increments is more lossy.
        static double runningTime = 0;
        runningTime += elapsedTime;
        if( runningTime < PARTICLE_UPDATE_RATE_MAX )
            return;

        float elapsedMs = runningTime;
        runningTime = 0;

        float elapsedSecs = elapsedMs * 0.001f;

        if( _started && _emissionRate )
        {
            // Calculate how much time has passed since we last emitted particles.
            _emitTime += elapsedMs; //+= elapsedTime;

            // How many particles should we emit this frame?
            BOOST_ASSERT(_timePerEmission);
            unsigned int emitCount = (unsigned int)(_emitTime / _timePerEmission);

            if( emitCount )
            {
                if( (int)_timePerEmission > 0 )
                {
                    _emitTime = fmod((double)_emitTime, (double)_timePerEmission);
                }
                emitOnce(emitCount);
            }
        }

        // Now update all currently living particles.
        BOOST_ASSERT(_particles);
        for( unsigned int particlesIndex = 0; particlesIndex < _particleCount; ++particlesIndex )
        {
            Particle* p = &_particles[particlesIndex];
            p->_energy -= elapsedMs;

            if( p->_energy > 0L )
            {
                if( p->_rotationSpeed != 0.0f && glm::length2(p->_rotationAxis) > std::numeric_limits<float>::epsilon() )
                {
                    _rotation = glm::quat(p->_rotationSpeed*elapsedSecs, p->_rotationAxis);

                    p->_velocity = _rotation * p->_velocity;
                    p->_acceleration = _rotation * p->_acceleration;
                }

                // Particle is still alive.
                p->_velocity.x += p->_acceleration.x * elapsedSecs;
                p->_velocity.y += p->_acceleration.y * elapsedSecs;
                p->_velocity.z += p->_acceleration.z * elapsedSecs;

                p->_position.x += p->_velocity.x * elapsedSecs;
                p->_position.y += p->_velocity.y * elapsedSecs;
                p->_position.z += p->_velocity.z * elapsedSecs;

                p->_angle += p->_rotationPerParticleSpeed * elapsedSecs;

                // Simple linear interpolation of color and size.
                float percent = 1.0f - ((float)p->_energy / (float)p->_energyStart);

                p->_color.x = p->_colorStart.x + (p->_colorEnd.x - p->_colorStart.x) * percent;
                p->_color.y = p->_colorStart.y + (p->_colorEnd.y - p->_colorStart.y) * percent;
                p->_color.z = p->_colorStart.z + (p->_colorEnd.z - p->_colorStart.z) * percent;
                p->_color.w = p->_colorStart.w + (p->_colorEnd.w - p->_colorStart.w) * percent;

                p->_size = p->_sizeStart + (p->_sizeEnd - p->_sizeStart) * percent;

                // Handle sprite animations.
                if( _spriteAnimated )
                {
                    if( !_spriteLooped )
                    {
                        // The last frame should finish exactly when the particle dies.
                        float percentSpent = 0.0f;
                        for( unsigned int i = 0; i < p->_frame; i++ )
                        {
                            percentSpent += _spritePercentPerFrame;
                        }
                        p->_timeOnCurrentFrame = percent - percentSpent;
                        if( p->_frame < _spriteFrameCount - 1 &&
                            p->_timeOnCurrentFrame >= _spritePercentPerFrame )
                        {
                            ++p->_frame;
                        }
                    }
                    else
                    {
                        // _spriteFrameDurationSecs is an absolute time measured in seconds,
                        // and the animation repeats indefinitely.
                        p->_timeOnCurrentFrame += elapsedSecs;
                        if( p->_timeOnCurrentFrame >= _spriteFrameDurationSecs )
                        {
                            p->_timeOnCurrentFrame -= _spriteFrameDurationSecs;
                            ++p->_frame;
                            if( p->_frame == _spriteFrameCount )
                            {
                                p->_frame = 0;
                            }
                        }
                    }
                }
            }
            else
            {
                // Particle is dead.  Move the particle furthest from the start of the array
                // down to take its place, and re-use the slot at the end of the list of living particles.
                if( particlesIndex != _particleCount - 1 )
                {
                    _particles[particlesIndex] = _particles[_particleCount - 1];
                }
                --_particleCount;
            }
        }
    }


    size_t ParticleEmitter::draw(bool /*wireframe*/)
    {
        if( !isActive() )
            return 0;

        if( _particleCount > 0 )
        {
            BOOST_ASSERT(_spriteBatch);
            BOOST_ASSERT(_particles);
            BOOST_ASSERT(_spriteTextureCoords);

            // Set our node's view projection matrix to this emitter's effect.
            if( _node )
            {
                _spriteBatch->setProjectionMatrix(_node->getViewProjectionMatrix());
            }

            // Begin sprite batch drawing
            _spriteBatch->start();

            // 2D Rotation.
            static const glm::vec2 pivot(0.5f, 0.5f);

            // 3D Rotation so that particles always face the camera.
            BOOST_ASSERT(_node && _node->getScene() && _node->getScene()->getActiveCamera() && _node->getScene()->getActiveCamera()->getNode());
            const glm::mat4& cameraWorldMatrix = _node->getScene()->getActiveCamera()->getNode()->getWorldMatrix();

            glm::vec3 right{ cameraWorldMatrix * glm::vec4{1,0,0,1} };
            glm::vec3 up{ cameraWorldMatrix * glm::vec4{0,1,0,1} };

            for( unsigned int i = 0; i < _particleCount; i++ )
            {
                Particle* p = &_particles[i];

                _spriteBatch->draw(p->_position, right, up, p->_size, p->_size,
                                   _spriteTextureCoords[p->_frame * 4], _spriteTextureCoords[p->_frame * 4 + 1], _spriteTextureCoords[p->_frame * 4 + 2], _spriteTextureCoords[p->_frame * 4 + 3],
                                   p->_color, pivot, p->_angle);
            }

            // Render.
            _spriteBatch->finishAndDraw();
        }
        return 1;
    }
}
