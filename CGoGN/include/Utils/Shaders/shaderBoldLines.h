/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009-2012, IGG Team, LSIIT, University of Strasbourg           *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#ifndef __CGOGN_SHADER_BOLDLINES__
#define __CGOGN_SHADER_BOLDLINES__

#include "Utils/GLSLShader.h"
#include "Utils/Shaders/shaderPointsLines.h"
#include "Geometry/vector_gen.h"

#include "Utils/dll.h"

namespace CGoGN
{

namespace Utils
{

class CGoGN_UTILS_API ShaderBoldLines : public ShaderPointsLines
{
protected:
	// shader sources
    static std::string vertexShaderText;
    static std::string geometryShaderText;
    static std::string fragmentShaderText;

    CGoGNGLuint m_uniform_lineWidth;
    CGoGNGLuint m_uniform_color;

	Geom::Vec2f m_lineWidth;

	Geom::Vec4f m_color;

    VBO* m_vboPos;

	/// clipping
	CGoGNGLuint m_unif_planeClip;
	Geom::Vec4f m_planeClip;

	void getLocations();

	void sendParams();

	void restoreUniformsAttribs();

public:
	ShaderBoldLines();

	void setLineWidth(float pix);

	void setColor(const Geom::Vec4f& color);

	unsigned int setAttributePosition(VBO* vbo);

	void setClippingPlane(const Geom::Vec4f& plane);
	inline void setNoClippingPlane() { setClippingPlane(Geom::Vec4f(0.0f,0.0f,0.0f,0.0f)); }

};

} // namespace Utils

} // namespace CGoGN

#endif