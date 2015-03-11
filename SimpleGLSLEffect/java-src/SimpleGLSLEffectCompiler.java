// Andreas Kirsch 2009
import org.antlr.runtime.*;
import org.antlr.stringtemplate.*;
import java.io.*;

public class SimpleGLSLEffectCompiler {
	public static class StringFormatRenderer implements AttributeRenderer {
		public String toString(Object o) {
			return o.toString();
		}
		
		static private String toCString( String text ) {
			return "\"" + text.replaceAll("\\\\", "\\\\\\\\").replaceAll("\r", "\\\\r").replaceAll(
	                "\t", "\\\\t").replaceAll("\"", "\\\\\"")
	                .replaceAll("\n", "\\\\n\"\n\"") + "\"";
		}

		public String toString(Object o, String formatName) {
			if (formatName.equals("cstring")) {
				return toCString( o.toString() );
			} else if (formatName.equals("glslcstring")) {
				return toCString( o.toString().replaceAll( "\\$", "\\#" ) );
			} else if (formatName.equals("codedump")) {
				return o.toString().replaceAll( "\\$", "\\#" );
			} else if (formatName.equals("commentdump")) {
				return o.toString().replaceAll( "/\\*", "/ *" ).replaceAll( "\\*/", "* /" );
			} else {
				return o.toString();
			}
		}
	}

	public static void printHelp() {
		System.out.println(
			"SimpleGLSLEffectCompiler [--forceOutput] [--noCompile] [--originalFilename <filename>] <effect.*>\n" +
			"creates <effect>Gfx.h and <effect>Gfx.cpp based on the effect file\n");
	}

	public static void main(String[] args) throws Exception {	
		// process --options
		boolean testCompile = true;
		boolean forceOutput = false;
		int filenameArg = 0;
		
		String originalFilename = "";

		for( ; args[filenameArg].startsWith("--") && filenameArg < args.length ; filenameArg++ ) {
			if( args[filenameArg].equalsIgnoreCase("--noCompile") ) {
				testCompile = false;
			}
			if( args[filenameArg].equalsIgnoreCase("--forceOutput") ) {
				forceOutput = true;
			}
			if( args[filenameArg].equalsIgnoreCase("--originalFilename") ) {
				if( ++filenameArg < args.length ) {
					originalFilename = args[filenameArg];
				}
			}
		}
		
		if (filenameArg >= args.length) {
			printHelp();
			System.exit(1);
		}

		String filename = args[filenameArg];
		// remove path
		String name = filename;
		String path = "";
		int lastPathSeparator = filename.lastIndexOf("\\");
		if (lastPathSeparator != -1) {
			name = filename.substring(lastPathSeparator + 1);
			path = filename.substring(0, lastPathSeparator + 1);
		}
		int lastDot = name.lastIndexOf(".");
		if (lastDot != -1) {
			name = name.substring(0, lastDot);
		}

		// Create an input character stream from standard in
		ANTLRFileStream input = new ANTLRFileStream(filename);
		// Create an ExprLexer that feeds from that stream
		SimpleGLSLEffectFileLexer lexer = new SimpleGLSLEffectFileLexer(input);
		// Create a stream of tokens fed by the lexer
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		// Create a parser that feeds off the token stream
		SimpleGLSLEffectFileParser parser = new SimpleGLSLEffectFileParser(
		        tokens);
		// Begin parsing at rule prog
		SimpleGLSLEffectFileParser.effectFile_return parsedEffectFile = parser
		        .effectFile();
				
		StringTemplateGroup templates = new StringTemplateGroup(
		        new InputStreamReader(SimpleGLSLEffectCompiler.class
		                .getResourceAsStream("effectTemplates.stg")));

		templates.registerRenderer(String.class, new StringFormatRenderer());
			
		boolean allCompiled = true;
		if( testCompile ) {			
			// test compile each pass
			ShaderTestCompilation shaderTestCompilation = new ShaderTestCompilation();
			
			shaderTestCompilation.filename = (originalFilename != "") ? originalFilename : filename;
			
			// NOTE: this code should match the output of the string template
			String versionInfo = "";
			if( parsedEffectFile.version != null ) {
				versionInfo = "#version " + parsedEffectFile.version + "\r\n" /*+ "#line 1 \"" + shaderTestCompilation.filename + "\"\r\n"*/;
			}
			
			shaderTestCompilation.vertexShaderInitCode = versionInfo + "#define VERTEX_SHADER\r\n";
			shaderTestCompilation.geometryShaderInitCode = versionInfo + "#define GEOMETRY_SHADER\r\n" +
										"#extension GL_EXT_geometry_shader4 : require\r\n";
			shaderTestCompilation.fragmentShaderInitCode = versionInfo + "#define FRAGMENT_SHADER\r\n";
	
			for(int i = 0 ; i < parsedEffectFile.passes.size() ; i++ ) {
				Pass pass = parsedEffectFile.passes.get( i );
		
				// getting weird exceptions (non-terminating for some reason)
				// if the templates are instantiated outside the loop
				// and the shared attributes are kept
				// maybe StringTemplate tries to update the output text on every attribute change?
				StringTemplate vertexShaderCodeRule = templates.getInstanceOf("dumpCode");
				StringTemplate fragmentShaderCodeRule = templates.getInstanceOf("dumpCode");
				StringTemplate geometryShaderCodeRule = templates.getInstanceOf("dumpCode");

				vertexShaderCodeRule.setAttribute("type", "vertex");
				fragmentShaderCodeRule.setAttribute("type", "fragment");
				geometryShaderCodeRule.setAttribute("type", "geometry");
				
				vertexShaderCodeRule.setAttribute("result", parsedEffectFile);
				fragmentShaderCodeRule.setAttribute("result", parsedEffectFile);
				geometryShaderCodeRule.setAttribute("result", parsedEffectFile);
				
				vertexShaderCodeRule.setAttribute("pass", pass);
				fragmentShaderCodeRule.setAttribute("pass", pass);
				geometryShaderCodeRule.setAttribute("pass", pass);
				
				String vertexShaderCode = null;
				if( pass.vertexShader != null ) {
					vertexShaderCode = vertexShaderCodeRule.toString();
				}
				String geometryShaderCode = null;
				if( pass.geometryShader != null ) {
					geometryShaderCode = geometryShaderCodeRule.toString();
				}
				String fragmentShaderCode = null;
				if( pass.fragmentShader != null ) {
					fragmentShaderCode = fragmentShaderCodeRule.toString();
				}
				
				System.out.println( "Pass " + i + ":" );
				//System.out.println(vertexShaderCode);
				//System.out.println(geometryShaderShared);
				//System.out.println(fragmentShaderShared);
				allCompiled = shaderTestCompilation.testCompile(vertexShaderCode, geometryShaderCode, fragmentShaderCode) && allCompiled;
			}
		}
		
		if( forceOutput || allCompiled ) {
			StringTemplate header = templates.getInstanceOf("headerContent");
			header.setAttribute("name", name);
			header.setAttribute("result", parsedEffectFile);
	
			// System.out.println( "Header:\n" + header );
	
			final String headerFilename = path + name + ".h";
			PrintWriter headerFile = new PrintWriter(new FileWriter(headerFilename));
			headerFile.println(header);
			headerFile.close();
	
			StringTemplate source = templates.getInstanceOf("sourceContent");
			source.setAttribute("name", name);
			source.setAttribute("result", parsedEffectFile);
	
			// System.out.println( "Source:\n" + source );
			final String sourceFilename = path + name + ".cpp";
			PrintWriter sourceFile = new PrintWriter(new FileWriter(sourceFilename));
			sourceFile.println(source);
			sourceFile.close();
			
			System.out.println( "Output written to " + headerFilename + " and " + sourceFilename + "." );
		}
		else {
			System.out.println( "No output was written." );
			System.exit(1);
		}
	}
}