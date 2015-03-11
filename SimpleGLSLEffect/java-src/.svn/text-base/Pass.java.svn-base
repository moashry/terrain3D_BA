import java.util.List;

public class Pass {
	public static class State {
		public boolean depthTest = true;
		public boolean stencilTest = false;
	};
	
	public static class GeometryShader {
		public String code;
		public String inputType;
		public String outputType;
		public String maxEmitVertices;
		
		public GeometryShader( String code, String inputType, String outputType, String maxEmitVertices ) {
			this.code = code;
			this.inputType = inputType;
			this.outputType = outputType;
			this.maxEmitVertices = maxEmitVertices;
		}
	};
	
	public State stateBlock;
	public String vertexShader, fragmentShader;
	public GeometryShader geometryShader;
	public List<String> transformFeedbackVaryings;
	public String fragDataName;
	
	public Pass( String vertexShader, GeometryShader geometryShader, String fragmentShader, List<String> transformFeedbackVaryings, State stateBlock, String fragDataName ) {
		this.vertexShader = vertexShader;
		this.geometryShader = geometryShader;
		this.transformFeedbackVaryings = transformFeedbackVaryings;
		this.fragmentShader = fragmentShader;
		this.stateBlock = stateBlock;
		this.fragDataName = fragDataName;
	}
}