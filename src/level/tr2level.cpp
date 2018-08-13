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

#include "tr2level.h"

using namespace level;

#define TR_AUDIO_MAP_SIZE_TR2  370

void TR2Level::loadFileData()
{
    // Version
    uint32_t file_version = m_reader.readU32();

    if( file_version != 0x0000002d )
        BOOST_THROW_EXCEPTION( std::runtime_error( "TR2 Level: Wrong level version" ) );

    m_palette = loader::Palette::readTr1( m_reader );
    /*Palette palette16 =*/ loader::Palette::readTr2( m_reader );

    auto numTextiles = m_reader.readU32();
    std::vector<loader::ByteTexture> texture8;
    m_reader.readVector( texture8, numTextiles, &loader::ByteTexture::read );
    std::vector<loader::WordTexture> texture16;
    m_reader.readVector( texture16, numTextiles, &loader::WordTexture::read );

    // Unused
    if( m_reader.readU32() != 0 )
        BOOST_LOG_TRIVIAL( warning ) << "TR2 Level: Bad value for 'unused'";

    m_reader.readVector( m_rooms, m_reader.readU16(), loader::Room::readTr2 );
    m_reader.readVector( m_floorData, m_reader.readU32() );

    readMeshData( m_reader );

    m_reader.readVector( m_animations, m_reader.readU32(), &loader::Animation::readTr1 );

    m_reader.readVector( m_transitions, m_reader.readU32(), &loader::Transitions::read );

    m_reader.readVector( m_transitionCases, m_reader.readU32(), &loader::TransitionCase::read );

    m_reader.readVector( m_animCommands, m_reader.readU32() );

    m_reader.readVector( m_boneTrees, m_reader.readU32() );

    m_reader.readVector( m_poseData, m_reader.readU32() );

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

    m_reader.readVector( m_staticMeshes, m_reader.readU32(), &loader::StaticMesh::read );

    m_reader.readVector( m_textureProxies, m_reader.readU32(), &loader::TextureLayoutProxy::readTr1 );

    m_reader.readVector( m_sprites, m_reader.readU32(), &loader::Sprite::readTr1 );

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

    if( m_demoOrUb )
        m_lightmap = loader::LightMap::read( m_reader );

    m_reader.readVector( m_cameras, m_reader.readU32(), &loader::Camera::read );

    m_reader.readVector( m_soundSources, m_reader.readU32(), &loader::SoundSource::read );

    m_reader.readVector( m_boxes, m_reader.readU32(), &loader::Box::readTr2 );

    m_reader.readVector( m_overlaps, m_reader.readU32() );

    m_baseZones.read( m_boxes.size(), m_reader );
    m_alternateZones.read( m_boxes.size(), m_reader );

    m_animatedTexturesUvCount = 0; // No UVRotate in TR2
    m_reader.readVector( m_animatedTextures, m_reader.readU32() );

    m_reader.readVector( m_items, m_reader.readU32(), &loader::Item::readTr2 );

    if( !m_demoOrUb )
        m_lightmap = loader::LightMap::read( m_reader );

    m_reader.readVector( m_cinematicFrames, m_reader.readU16(), &loader::CinematicFrame::read );

    m_reader.readVector( m_demoData, m_reader.readU16() );

    // Soundmap
    m_reader.readVector( m_soundmap, TR_AUDIO_MAP_SIZE_TR2 );

    m_reader.readVector( m_soundDetails, m_reader.readU32(), &loader::SoundDetails::readTr1 );

    m_reader.readVector( m_sampleIndices, m_reader.readU32() );

    // remap all sample indices here
    for( auto& soundDetail : m_soundDetails )
    {
        if( soundDetail.sample < m_sampleIndices.size() )
        {
            soundDetail.sample = m_sampleIndices[soundDetail.sample];
        }
    }

    // LOAD SAMPLES

    // In TR2, samples are stored in separate file called MAIN.SFX.
    // If there is no such files, no samples are loaded.

    loader::io::SDLReader newsrc( m_sfxPath );
    if( !newsrc.isOpen() )
    {
        BOOST_LOG_TRIVIAL( warning ) << "TR2 Level: failed to open '" << m_sfxPath << "', no samples loaded.";
    }
    else
    {
        m_samplesCount = 0;
        m_samplesData.resize( static_cast<size_t>(newsrc.size()) );
        for( size_t i = 0; i < m_samplesData.size(); i++ )
        {
            m_samplesData[i] = newsrc.readU8();
            if( i >= 4 && *reinterpret_cast<uint32_t*>(m_samplesData.data() + i - 4) == 0x46464952 )   /// RIFF
            {
                m_samplesCount++;
            }
        }
    }

    m_textures.resize( texture16.size() );
    for( size_t i = 0; i < texture16.size(); i++ )
        convertTexture( texture16[i], m_textures[i] );

    postProcessDataStructures();
}
