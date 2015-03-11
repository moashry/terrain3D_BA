import java.io.*;
import org.antlr.runtime.*;
import org.antlr.runtime.debug.DebugEventSocketProxy;


public class __Test__ {

    public static void main(String args[]) throws Exception {
        SamSolTestLexer lex = new SamSolTestLexer(new ANTLRFileStream("D:\\CGHiwi\\SimpleGLSLEffect\\__Test___input.txt"));
        CommonTokenStream tokens = new CommonTokenStream(lex);

        SamSolTestParser g = new SamSolTestParser(tokens, 49100, null);
        try {
            g.pass();
        } catch (RecognitionException e) {
            e.printStackTrace();
        }
    }
}