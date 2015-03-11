import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.media.opengl.DefaultGLCapabilitiesChooser;
import javax.media.opengl.GL;
import javax.media.opengl.GLAutoDrawable;
import javax.media.opengl.GLCapabilities;
import javax.media.opengl.GLDrawableFactory;
import javax.media.opengl.GLEventListener;

public class ShaderTestCompilation {
	
	private GLAutoDrawable drawable;
	private GL gl;

	public ShaderTestCompilation() {
		//configure context
		GLCapabilities capabilities = new GLCapabilities();
		capabilities.setHardwareAccelerated(true);

		drawable = GLDrawableFactory.getFactory().createGLPbuffer( capabilities, new DefaultGLCapabilitiesChooser(), 1, 1, null );
	}
	
	private final int logSize = 2048;
	
	private boolean compileShader( int shader, String[] codes, List<String> messages ) {
		gl.glShaderSource(shader, codes.length, codes, null );
		gl.glCompileShader(shader);
		
		byte[] log = new byte[logSize];
		int[] logLength = new int[1];
		gl.glGetShaderInfoLog(shader, log.length, logLength, 0, log, 0);
		
		if( logLength[0] > 0 ) {
			messages.add( fixFilename( new String(log, 0, logLength[0] ) ) );
		}
		
		int[] compileStatus = new int[1];
		gl.glGetShaderiv(shader, GL.GL_COMPILE_STATUS, compileStatus, 0);
		return compileStatus[0] == GL.GL_TRUE;
	}
	
	private boolean compileShaderAndDumpMessages( int shader, String[] codes, String headerMessage ) {
		ArrayList<String> messages = new ArrayList<String>();
		boolean compiled = compileShader(shader, codes, messages);
		
		if( messages.size() > 0 ) {
			System.out.println( headerMessage );
			for(String message: messages) {
				System.out.println( message );
			}
		}
		
		return compiled;
	}
	
	// TODO: use this on every string message! right now its called in compileShader.. >_>
	private String fixFilename(String logMessage) {
		Pattern filenameFix = Pattern.compile("^0(\\(\\d+\\))", Pattern.MULTILINE);
		return filenameFix.matcher(logMessage).replaceAll( Matcher.quoteReplacement(filename) + "$1" );
	}
	
	public String vertexShaderInitCode = "";
	public String geometryShaderInitCode = "";
	public String fragmentShaderInitCode = "";
	
	public String filename = "";
	
	// ugly code but I have no idea how to access the GL interface reliably otherwise
	private boolean _testCompileReturnValue;
	public boolean testCompile(String vertexShaderCode, String geometryShaderCode, String fragmentShaderCode) {
		final String _vertexShaderCode = vertexShaderCode;
		final String _geometryShaderCode = geometryShaderCode;
		final String _fragmentShaderCode = fragmentShaderCode;
		
		_testCompileReturnValue = false;
		GLEventListener eventListener = new GLEventListener() {
			@Override
            public void display(GLAutoDrawable arg0) {
	            gl = arg0.getGL();
	            
	            _testCompileReturnValue = _testCompile(_vertexShaderCode, _geometryShaderCode, _fragmentShaderCode);
            }

			@Override
            public void displayChanged(GLAutoDrawable arg0, boolean arg1,
                    boolean arg2) {
	            // TODO Auto-generated method stub
	            
            }

			@Override
            public void init(GLAutoDrawable arg0) {
	            // TODO Auto-generated method stub
	            
            }

			@Override
            public void reshape(GLAutoDrawable arg0, int arg1, int arg2,
                    int arg3, int arg4) {
	            // TODO Auto-generated method stub
	            
            }
			
		};
		drawable.addGLEventListener(eventListener);
		drawable.display();
		drawable.removeGLEventListener(eventListener);
		return _testCompileReturnValue;
	}
	
	private boolean _testCompile(String vertexShaderCode, String geometryShaderCode, String fragmentShaderCode) {
		int program = gl.glCreateProgram();
		
		int vertexShader = 0;
		if( vertexShaderCode != null )
			vertexShader = gl.glCreateShader(GL.GL_VERTEX_SHADER);
		
		int geometryShader = 0;
		if( geometryShaderCode != null )
			geometryShader = gl.glCreateShader(GL.GL_GEOMETRY_SHADER_EXT);
			
		int fragmentShader = 0;
		if( fragmentShaderCode != null )
			fragmentShader = gl.glCreateShader(GL.GL_FRAGMENT_SHADER);
		
		boolean allCompiledAndLinked = true;
		
		// no early out, dump all the dirty details
		if( vertexShader != 0 ) {		
			allCompiledAndLinked = compileShaderAndDumpMessages(vertexShader, new String[] {vertexShaderInitCode, "", vertexShaderCode }, "Vertex Shader:") && allCompiledAndLinked;

			gl.glAttachShader(program, vertexShader);
		}
		if( geometryShader != 0 ) {
			allCompiledAndLinked = compileShaderAndDumpMessages(geometryShader, new String[] {geometryShaderInitCode, "", geometryShaderCode }, "Geometry Shader:") && allCompiledAndLinked;
			
			gl.glAttachShader(program, geometryShader);
		}
		if( fragmentShader != 0 ) {
			allCompiledAndLinked = compileShaderAndDumpMessages(fragmentShader, new String[] {fragmentShaderInitCode, "", fragmentShaderCode }, "Fragment Shader:") && allCompiledAndLinked;
			
			gl.glAttachShader(program, fragmentShader);
		}
		
		// NOTE: disable linking for now until all the geometry shader faff has been figured out
		// (the compiler atm simply stores the geometry shader parameters as strings and
		// this needs to be converted to usable enum values..
		// the effects also need to be checked to see if they don't do any arithmetics o.s there)
		if( false && allCompiledAndLinked ) {
			gl.glLinkProgram(program);
		
			byte[] log = new byte[logSize];
			int[] logLength = new int[1];
			gl.glGetProgramInfoLog(program, log.length, logLength, 0, log, 0);
			
			if( logLength[0] > 0 ) {
				System.out.println( "Linking:" );
				System.out.println( new String(log, 0, logLength[0] ) );
			}
			
			int[] linkStatus = new int[1];
			gl.glGetProgramiv(program, GL.GL_LINK_STATUS, linkStatus, 0);
			allCompiledAndLinked = (linkStatus[0] == GL.GL_TRUE) && allCompiledAndLinked;
		}
		
		if( vertexShader != 0 )
			gl.glDeleteShader(vertexShader);
		if( geometryShader != 0 )
			gl.glDeleteShader(geometryShader);
		if( fragmentShader != 0 )
			gl.glDeleteShader(fragmentShader);
		
		gl.glDeleteProgram(program);
		
		return allCompiledAndLinked;
	}
}
