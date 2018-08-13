/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include "tr4level.h"

using namespace level;

#define TR_AUDIO_MAP_SIZE_TR4  370

void TR4Level::loadFileData()
{
    // Version
    uint32_t file_version = m_reader.readU32();

    if( file_version != 0x00345254 /*&& file_version != 0x63345254*/)           // +TRLE
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: Wrong level version" ) );

    std::vector<loader::WordTexture> texture16;
    {
        auto numRoomTextiles = m_reader.readU16();
        auto numObjTextiles = m_reader.readU16();
        auto numBumpTextiles = m_reader.readU16();
        auto numMiscTextiles = 2;
        auto numTextiles = numRoomTextiles + numObjTextiles + numBumpTextiles + numMiscTextiles;

        uint32_t uncomp_size = m_reader.readU32();
        if( uncomp_size == 0 )
            BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: textiles32 is empty" ) );

        uint32_t comp_size = m_reader.readU32();
        if( comp_size > 0 )
        {
            std::vector<uint8_t> comp_buffer( comp_size );
            m_reader.readBytes( comp_buffer.data(), comp_size );

            loader::io::SDLReader newsrc = loader::io::SDLReader::decompress( comp_buffer, uncomp_size );
            newsrc.readVector( m_textures, numTextiles - numMiscTextiles, &loader::DWordTexture::read );
        }

        uncomp_size = m_reader.readU32();
        if( uncomp_size == 0 )
            BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: textiles16 is empty" ) );

        comp_size = m_reader.readU32();
        if( comp_size > 0 )
        {
            if( m_textures.empty() )
            {
                std::vector<uint8_t> comp_buffer( comp_size );
                m_reader.readBytes( comp_buffer.data(), comp_size );

                loader::io::SDLReader newsrc = loader::io::SDLReader::decompress( comp_buffer, uncomp_size );
                newsrc.readVector( texture16, numTextiles - numMiscTextiles, &loader::WordTexture::read );
            }
            else
            {
                m_reader.skip( comp_size );
            }
        }

        uncomp_size = m_reader.readU32();
        if( uncomp_size == 0 )
            BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: textiles32d is empty" ) );

        comp_size = m_reader.readU32();
        if( comp_size > 0 )
        {
            if( !m_textures.empty() )
            {
                m_reader.skip( comp_size );
            }
            else
            {
                if( uncomp_size / (256 * 256 * 4) > 2 )
                    BOOST_LOG_TRIVIAL( warning ) << "TR4 Level: number of misc textiles > 2";

                if( m_textures.empty() )
                {
                    m_textures.resize( numTextiles );
                }
                std::vector<uint8_t> comp_buffer( comp_size );

                m_reader.readBytes( comp_buffer.data(), comp_size );

                loader::io::SDLReader newsrc = loader::io::SDLReader::decompress( comp_buffer, uncomp_size );
                newsrc.appendVector( m_textures, numMiscTextiles, &loader::DWordTexture::read );
            }
        }
    }

    auto uncomp_size = m_reader.readU32();
    if( uncomp_size == 0 )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: packed geometry (decompressed) is empty" ) );

    auto comp_size = m_reader.readU32();

    if( comp_size == 0 )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: packed geometry (compressed) is empty" ) );

    std::vector<uint8_t> comp_buffer( comp_size );
    m_reader.readBytes( comp_buffer.data(), comp_size );

    loader::io::SDLReader newsrc = loader::io::SDLReader::decompress( comp_buffer, uncomp_size );
    if( !newsrc.isOpen() )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: packed geometry could not be decompressed" ) );

    // Unused
    if( newsrc.readU32() != 0 )
        BOOST_LOG_TRIVIAL( warning ) << "TR4 Level: Bad value for 'unused'";

    newsrc.readVector( m_rooms, newsrc.readU16(), &loader::Room::readTr4 );

    newsrc.readVector( m_floorData, newsrc.readU32() );

    readMeshData( newsrc );

    newsrc.readVector( m_animations, newsrc.readU32(), &loader::Animation::readTr4 );

    newsrc.readVector( m_transitions, newsrc.readU32(), &loader::Transitions::read );

    newsrc.readVector( m_transitionCases, newsrc.readU32(), loader::TransitionCase::read );

    newsrc.readVector( m_animCommands, newsrc.readU32() );

    newsrc.readVector( m_boneTrees, newsrc.readU32() );

    newsrc.readVector( m_poseData, newsrc.readU32() );

    {
        const auto n = m_reader.readU32();
        for( uint32_t i = 0; i < n; ++i )
        {
            auto m = loader::SkeletalModelType::readTr1( m_reader );
            // FIXME: this uses TR1 item IDs...
            if( m_animatedModels.find( m->typeId ) != m_animatedModels.end() )
                BOOST_THROW_EXCEPTION( std::runtime_error( "Duplicate type id" ) );

            m_animatedModels[m->typeId] = std::move( m );
        }
    }

    newsrc.readVector( m_staticMeshes, newsrc.readU32(), &loader::StaticMesh::read );

    if( newsrc.readI8() != 'S' )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: 'SPR' not found" ) );

    if( newsrc.readI8() != 'P' )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: 'SPR' not found" ) );

    if( newsrc.readI8() != 'R' )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: 'SPR' not found" ) );

    newsrc.readVector( m_sprites, newsrc.readU32(), &loader::Sprite::readTr4 );

    {
        const auto n = m_reader.readU32();
        for( uint32_t i = 0; i < n; ++i )
        {
            auto m = loader::SpriteSequence::read( m_reader );
            if( m_spriteSequences.find( m->type ) != m_spriteSequences.end() )
                BOOST_THROW_EXCEPTION( std::runtime_error( "Duplicate type id" ) );

            m_spriteSequences[m->type] = std::move( m );
        }
    }

    newsrc.readVector( m_cameras, newsrc.readU32(), &loader::Camera::read );
    //SDL_RWseek(newsrc, this->cameras.size() * 16, SEEK_CUR);

    newsrc.readVector( m_flybyCameras, newsrc.readU32(), &loader::FlybyCamera::read );
    //SDL_RWseek(newsrc, this->flyby_cameras.size() * 40, SEEK_CUR);

    newsrc.readVector( m_soundSources, newsrc.readU32(), &loader::SoundSource::read );

    newsrc.readVector( m_boxes, newsrc.readU32(), &loader::Box::readTr2 );

    newsrc.readVector( m_overlaps, newsrc.readU32() );

    m_baseZones.read( m_boxes.size(), m_reader );
    m_alternateZones.read( m_boxes.size(), m_reader );

    newsrc.readVector( m_animatedTextures, newsrc.readU32() );

    m_animatedTexturesUvCount = newsrc.readU8();

    if( newsrc.readI8() != 'T' )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: 'TEX' not found" ) );

    if( newsrc.readI8() != 'E' )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: 'TEX' not found" ) );

    if( newsrc.readI8() != 'X' )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR4 Level: 'TEX' not found" ) );

    newsrc.readVector( m_textureProxies, newsrc.readU32(), &loader::TextureLayoutProxy::readTr4 );

    newsrc.readVector( m_items, newsrc.readU32(), &loader::Item::readTr4 );

    newsrc.readVector( m_aiObjects, newsrc.readU32(), &loader::AIObject::read );

    newsrc.readVector( m_demoData, newsrc.readU16() );

    // Soundmap
    newsrc.readVector( m_soundmap, TR_AUDIO_MAP_SIZE_TR4 );

    newsrc.readVector( m_soundDetails, newsrc.readU32(), &loader::SoundDetails::readTr3 );

    // IMPORTANT NOTE: Sample indices ARE NOT USED in TR4 engine, but are parsed anyway.
    newsrc.readVector( m_sampleIndices, newsrc.readU32() );

    // LOAD SAMPLES

    if( auto i = m_reader.readU32() )
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_reader.readVector( m_samplesData, static_cast<size_t>(m_reader.size() - m_reader.tell()) );
    }

    if( !m_textures.empty() )
        return;

    m_textures.resize( texture16.size() );
    for( size_t i = 0; i < texture16.size(); i++ )
        convertTexture( texture16[i], m_textures[i] );

    postProcessDataStructures();
}
