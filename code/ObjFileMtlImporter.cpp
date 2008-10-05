/*
---------------------------------------------------------------------------
Open Asset Import Library (ASSIMP)
---------------------------------------------------------------------------

Copyright (c) 2006-2008, ASSIMP Development Team

All rights reserved.

Redistribution and use of this software in source and binary forms, 
with or without modification, are permitted provided that the following 
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/
#include "ObjFileMtlImporter.h"
#include "../include/aiTypes.h"
#include "../include/aiAssert.h"
#include "ObjTools.h"
#include "ObjFileData.h"
#include "fast_atof.h"


namespace Assimp
{

// -------------------------------------------------------------------
//	Constructor
ObjFileMtlImporter::ObjFileMtlImporter( std::vector<char> &buffer, 
									   const std::string &strAbsPath,
									   ObjFile::Model *pModel ) :
	m_DataIt( buffer.begin() ),
	m_DataItEnd( buffer.end() ),
	m_pModel( pModel ),
	m_uiLine( 0 )
{
	ai_assert( NULL != m_pModel );
	if ( NULL == m_pModel->m_pDefaultMaterial )
	{
		m_pModel->m_pDefaultMaterial = new ObjFile::Material();
		m_pModel->m_pDefaultMaterial->MaterialName.Set( "default" );
	}
	load();
}

// -------------------------------------------------------------------
//	Destructor
ObjFileMtlImporter::~ObjFileMtlImporter()
{
	// empty
}

// -------------------------------------------------------------------
//	Private copy constructor
ObjFileMtlImporter::ObjFileMtlImporter(const ObjFileMtlImporter &rOther)
{
	// empty
}
	
// -------------------------------------------------------------------
//	Private copy constructor
ObjFileMtlImporter &ObjFileMtlImporter::operator = (
	const ObjFileMtlImporter &rOther)
{
	return *this;
}

// -------------------------------------------------------------------
//	Loads the material description
void ObjFileMtlImporter::load()
{
	if ( m_DataIt == m_DataItEnd )
		return;

	while ( m_DataIt != m_DataItEnd )
	{
		switch (*m_DataIt)
		{
		case 'K':
			{
				++m_DataIt;
				if (*m_DataIt == 'a') // Ambient color
				{
					++m_DataIt;
					getColorRGBA( &m_pModel->m_pCurrentMaterial->ambient );
				}
				else if (*m_DataIt == 'd')	// Diffuse color
				{
					++m_DataIt;
					getColorRGBA( &m_pModel->m_pCurrentMaterial->diffuse );
				}
				else if (*m_DataIt == 's')
				{
					++m_DataIt;
					getColorRGBA( &m_pModel->m_pCurrentMaterial->specular );
				}
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
			}
			break;

		case 'd':	// Alpha value
			{
				++m_DataIt;
				getFloatValue( m_pModel->m_pCurrentMaterial->alpha );
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
			}
			break;

		case 'N':	// Shineness
			{
				++m_DataIt;
				switch(*m_DataIt) 
				{
				case 's':
					++m_DataIt;
					getFloatValue(m_pModel->m_pCurrentMaterial->shineness);
					break;
				case 'i': //Index Of refraction 
					//TODO
					break;
				}
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
				break;
			}
			break;
		

		case 'm':	// Texture
			{
				getTexture();
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
			}
			break;

		case 'n':	// New material name
			{
				createMaterial();
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
			}
			break;

		case 'i':	// Illumination model
			{
				m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
				getIlluminationModel( m_pModel->m_pCurrentMaterial->illumination_model );
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
			}
			break;

		default:
			{
				m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
			}
			break;
		}
	}
}

// -------------------------------------------------------------------
//	Loads a color definition
void ObjFileMtlImporter::getColorRGBA( aiColor3D *pColor )
{
	ai_assert( NULL != pColor );
	
	float r, g, b;
	m_DataIt = CopyNextWord<DataArrayIt>( m_DataIt, m_DataItEnd, m_buffer, BUFFERSIZE );
	r = (float) fast_atof(m_buffer);

	m_DataIt = CopyNextWord<DataArrayIt>( m_DataIt, m_DataItEnd, m_buffer, BUFFERSIZE );
	g = (float) fast_atof(m_buffer);

	m_DataIt = CopyNextWord<DataArrayIt>( m_DataIt, m_DataItEnd, m_buffer, BUFFERSIZE );
	b = (float) fast_atof(m_buffer);

	pColor->r = r;
	pColor->g = g;
	pColor->b = b;
}

// -------------------------------------------------------------------
//	Loads the kind of illumination model.
void ObjFileMtlImporter::getIlluminationModel( int &illum_model )
{
	m_DataIt = CopyNextWord<DataArrayIt>( m_DataIt, m_DataItEnd, m_buffer, BUFFERSIZE );
	illum_model = atoi(m_buffer);
}

// -------------------------------------------------------------------
//	Loads a single float value. 
void ObjFileMtlImporter::getFloatValue( float &value )
{
	m_DataIt = CopyNextWord<DataArrayIt>( m_DataIt, m_DataItEnd, m_buffer, BUFFERSIZE );
	value = (float) fast_atof(m_buffer);
}

// -------------------------------------------------------------------
//	Creates a material from loaded data.
void ObjFileMtlImporter::createMaterial()
{	
	std::string strName;
	m_DataIt = getName<DataArrayIt>( m_DataIt, m_DataItEnd, strName );
	if ( m_DataItEnd == m_DataIt )
		return;

	std::map<std::string, ObjFile::Material*>::iterator it = m_pModel->m_MaterialMap.find( strName );
	if ( m_pModel->m_MaterialMap.end() == it)
	{
		// New Material created
		m_pModel->m_pCurrentMaterial = new ObjFile::Material();	
		m_pModel->m_pCurrentMaterial->MaterialName.Set( strName );
		m_pModel->m_MaterialLib.push_back( strName );
		m_pModel->m_MaterialMap[ strName ] = m_pModel->m_pCurrentMaterial;
	}
	else
	{
		// Use older material
		m_pModel->m_pCurrentMaterial = (*it).second;
	}
}

// -------------------------------------------------------------------
//	Gets a texture name from data.
void ObjFileMtlImporter::getTexture()
{
	std::string strTexture;
	m_DataIt = getName<DataArrayIt>( m_DataIt, m_DataItEnd, strTexture );
	if ( m_DataItEnd == m_DataIt )
		return;

	m_pModel->m_pCurrentMaterial->texture.Set( strTexture );
}

// -------------------------------------------------------------------

} // Namespace Assimp
