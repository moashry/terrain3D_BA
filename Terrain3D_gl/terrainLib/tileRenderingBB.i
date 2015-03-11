#line 1 ".\\terrainLib\\tileRenderingBB.gfx"
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Bounding Boxes
version 150

uniform {
	// vertex shader uniforms
	mat4x4 mWorldView;
	mat4x4 mProjection;
	
	// fragment shader uniforms
	vec4 vColorBB;
}

inputlayout { position: FLOAT[3] }

pass {
	fragDataName vColor;

	vertex {
		in vec4 position;
		
		void main() {
			gl_Position = mProjection * mWorldView * position;
		}
	}
	fragment {
		out vec4 vColor;

		void main() {
			vColor = vColorBB;
		}
	}
}
