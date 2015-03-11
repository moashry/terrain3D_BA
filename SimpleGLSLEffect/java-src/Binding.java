
public class Binding {
	public String name;
	public int size;
	public String type;
	
	public boolean isInteger() {
		return type.indexOf( "INT" ) != -1;
	}
	
	public Binding( String name, int size, String type ) {
		this.name = name;
		this.size = size;
		this.type = type;
	}
}
