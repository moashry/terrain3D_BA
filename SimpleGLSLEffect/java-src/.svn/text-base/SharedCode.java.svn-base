import java.util.List;


public class SharedCode {
	public static enum Type {
		COMMON,
		VERTEX,
		FRAGMENT,
		GEOMETRY,
		UNIFORM_BLOCK
	}
	
	public Type type;
	public String code;
	public List<Uniform> uniforms;
	
	public SharedCode( Type type, String code ) {
		this.type = type;
		this.code = code;
		this.uniforms = null;
	}
	
	public SharedCode( Type type, List<Uniform> uniforms ) {
		this.type = type;
		this.code = "";
		this.uniforms = uniforms;
	}
	
	public boolean isUniformBlock() {
		return type == Type.UNIFORM_BLOCK;
	}
	
	public boolean isForAllShaders() {
		return type == Type.COMMON || type == Type.UNIFORM_BLOCK;
	}
	
	public boolean isForVertexShaders() {
		return isForAllShaders() || type == Type.VERTEX;
	}
	
	public boolean isForFragmentShaders() {
		return isForAllShaders() || type == Type.FRAGMENT;
	}
	
	public boolean isForGeometryShaders() {
		return isForAllShaders() || type == Type.GEOMETRY;
	}
}
