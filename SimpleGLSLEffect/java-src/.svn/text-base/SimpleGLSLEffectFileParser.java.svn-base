// $ANTLR 3.2 Sep 23, 2009 12:02:23 D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g 2010-01-10 16:05:15

import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
public class SimpleGLSLEffectFileParser extends Parser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "NUMBER", "ID", "LETTER", "WS", "COMMENT", "LINE_COMMENT", "LINE", "ANYTHING_BUT_BRACES_OR_PARENTHESES", "'version'", "'shared'", "'vertex'", "'fragment'", "'geometry'", "'uniform'", "'{'", "'}'", "'inputlayout'", "','", "':'", "'['", "']'", "';'", "'pass'", "'('", "')'", "'feedback'", "'fragDataName'", "'state'", "'depthTest'", "'stencilTest'", "'true'", "'false'"
    };
    public static final int T__29=29;
    public static final int T__28=28;
    public static final int ANYTHING_BUT_BRACES_OR_PARENTHESES=11;
    public static final int T__27=27;
    public static final int T__26=26;
    public static final int LINE_COMMENT=9;
    public static final int T__25=25;
    public static final int T__24=24;
    public static final int LETTER=6;
    public static final int T__23=23;
    public static final int T__22=22;
    public static final int T__21=21;
    public static final int T__20=20;
    public static final int NUMBER=4;
    public static final int ID=5;
    public static final int EOF=-1;
    public static final int LINE=10;
    public static final int T__30=30;
    public static final int T__19=19;
    public static final int T__31=31;
    public static final int T__32=32;
    public static final int WS=7;
    public static final int T__33=33;
    public static final int T__16=16;
    public static final int T__34=34;
    public static final int T__15=15;
    public static final int T__35=35;
    public static final int T__18=18;
    public static final int T__17=17;
    public static final int T__12=12;
    public static final int T__14=14;
    public static final int T__13=13;
    public static final int COMMENT=8;

    // delegates
    // delegators


        public SimpleGLSLEffectFileParser(TokenStream input) {
            this(input, new RecognizerSharedState());
        }
        public SimpleGLSLEffectFileParser(TokenStream input, RecognizerSharedState state) {
            super(input, state);
             
        }
        

    public String[] getTokenNames() { return SimpleGLSLEffectFileParser.tokenNames; }
    public String getGrammarFileName() { return "D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g"; }


    public static class effectFile_return extends ParserRuleReturnScope {
        public Integer version;
        public List<SharedCode> sharedCodes;
        public List<Pass> passes;
        public List<Uniform> uniforms;
        public List<Binding> bindings;
    };

    // $ANTLR start "effectFile"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:7:1: effectFile returns [Integer version, List<SharedCode> sharedCodes, List<Pass> passes, List<Uniform> uniforms, List<Binding> bindings] : ( 'version' versionNumber= NUMBER )? ( 'shared' commonCode= codeBlock | 'vertex' vertexCode= codeBlock | 'fragment' fragmentCode= codeBlock | 'geometry' geometryCode= codeBlock | uniformsDeclaration | {...}? inputLayoutDeclaration )* ( passDefinition )+ ;
    public final SimpleGLSLEffectFileParser.effectFile_return effectFile() throws RecognitionException {
        SimpleGLSLEffectFileParser.effectFile_return retval = new SimpleGLSLEffectFileParser.effectFile_return();
        retval.start = input.LT(1);

        Token versionNumber=null;
        String commonCode = null;

        String vertexCode = null;

        String fragmentCode = null;

        String geometryCode = null;

        List<Uniform> uniformsDeclaration1 = null;

        List<Binding> inputLayoutDeclaration2 = null;

        SimpleGLSLEffectFileParser.passDefinition_return passDefinition3 = null;



        	retval.passes = new ArrayList<Pass>();
        	retval.uniforms = new ArrayList<Uniform>();
        	retval.sharedCodes = new ArrayList<SharedCode>();

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:13:2: ( ( 'version' versionNumber= NUMBER )? ( 'shared' commonCode= codeBlock | 'vertex' vertexCode= codeBlock | 'fragment' fragmentCode= codeBlock | 'geometry' geometryCode= codeBlock | uniformsDeclaration | {...}? inputLayoutDeclaration )* ( passDefinition )+ )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:13:4: ( 'version' versionNumber= NUMBER )? ( 'shared' commonCode= codeBlock | 'vertex' vertexCode= codeBlock | 'fragment' fragmentCode= codeBlock | 'geometry' geometryCode= codeBlock | uniformsDeclaration | {...}? inputLayoutDeclaration )* ( passDefinition )+
            {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:13:4: ( 'version' versionNumber= NUMBER )?
            int alt1=2;
            int LA1_0 = input.LA(1);

            if ( (LA1_0==12) ) {
                alt1=1;
            }
            switch (alt1) {
                case 1 :
                    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:13:5: 'version' versionNumber= NUMBER
                    {
                    match(input,12,FOLLOW_12_in_effectFile41); if (state.failed) return retval;
                    versionNumber=(Token)match(input,NUMBER,FOLLOW_NUMBER_in_effectFile45); if (state.failed) return retval;
                    if ( state.backtracking==0 ) {
                       retval.version = new Integer( (versionNumber!=null?versionNumber.getText():null) ); 
                    }

                    }
                    break;

            }

            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:14:3: ( 'shared' commonCode= codeBlock | 'vertex' vertexCode= codeBlock | 'fragment' fragmentCode= codeBlock | 'geometry' geometryCode= codeBlock | uniformsDeclaration | {...}? inputLayoutDeclaration )*
            loop2:
            do {
                int alt2=7;
                switch ( input.LA(1) ) {
                case 13:
                    {
                    alt2=1;
                    }
                    break;
                case 14:
                    {
                    alt2=2;
                    }
                    break;
                case 15:
                    {
                    alt2=3;
                    }
                    break;
                case 16:
                    {
                    alt2=4;
                    }
                    break;
                case 17:
                    {
                    alt2=5;
                    }
                    break;
                case 20:
                    {
                    alt2=6;
                    }
                    break;

                }

                switch (alt2) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:15:4: 'shared' commonCode= codeBlock
            	    {
            	    match(input,13,FOLLOW_13_in_effectFile58); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_effectFile62);
            	    commonCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.sharedCodes.add( new SharedCode( SharedCode.Type.COMMON, commonCode ) ); 
            	    }

            	    }
            	    break;
            	case 2 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:16:5: 'vertex' vertexCode= codeBlock
            	    {
            	    match(input,14,FOLLOW_14_in_effectFile70); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_effectFile74);
            	    vertexCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.sharedCodes.add( new SharedCode( SharedCode.Type.VERTEX, vertexCode ) ); 
            	    }

            	    }
            	    break;
            	case 3 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:17:5: 'fragment' fragmentCode= codeBlock
            	    {
            	    match(input,15,FOLLOW_15_in_effectFile82); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_effectFile86);
            	    fragmentCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.sharedCodes.add( new SharedCode( SharedCode.Type.FRAGMENT, fragmentCode ) ); 
            	    }

            	    }
            	    break;
            	case 4 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:18:5: 'geometry' geometryCode= codeBlock
            	    {
            	    match(input,16,FOLLOW_16_in_effectFile94); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_effectFile98);
            	    geometryCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.sharedCodes.add( new SharedCode( SharedCode.Type.GEOMETRY, geometryCode ) ); 
            	    }

            	    }
            	    break;
            	case 5 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:19:5: uniformsDeclaration
            	    {
            	    pushFollow(FOLLOW_uniformsDeclaration_in_effectFile106);
            	    uniformsDeclaration1=uniformsDeclaration();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       
            	      				retval.uniforms.addAll( uniformsDeclaration1 );
            	      				retval.sharedCodes.add( new SharedCode( SharedCode.Type.UNIFORM_BLOCK, uniformsDeclaration1 ) ); 
            	      			
            	    }

            	    }
            	    break;
            	case 6 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:23:5: {...}? inputLayoutDeclaration
            	    {
            	    if ( !((retval.bindings == null)) ) {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        throw new FailedPredicateException(input, "effectFile", "$bindings == null");
            	    }
            	    pushFollow(FOLLOW_inputLayoutDeclaration_in_effectFile116);
            	    inputLayoutDeclaration2=inputLayoutDeclaration();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.bindings = inputLayoutDeclaration2; 
            	    }

            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:25:3: ( passDefinition )+
            int cnt3=0;
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0==26) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:25:4: passDefinition
            	    {
            	    pushFollow(FOLLOW_passDefinition_in_effectFile129);
            	    passDefinition3=passDefinition();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {

            	      				retval.passes.add( new Pass( (passDefinition3!=null?passDefinition3.vertexShader:null),
            	      									(passDefinition3!=null?passDefinition3.geometryShader:null),
            	      									(passDefinition3!=null?passDefinition3.fragmentShader:null),
            	      									(passDefinition3!=null?passDefinition3.transformFeedbackVaryings:null),
            	      									(passDefinition3!=null?passDefinition3.stateBlock:null),
            	      									(passDefinition3!=null?passDefinition3.fragDataName:null) ) );
            	      			
            	    }

            	    }
            	    break;

            	default :
            	    if ( cnt3 >= 1 ) break loop3;
            	    if (state.backtracking>0) {state.failed=true; return retval;}
                        EarlyExitException eee =
                            new EarlyExitException(3, input);
                        throw eee;
                }
                cnt3++;
            } while (true);


            }

            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "effectFile"


    // $ANTLR start "uniformsDeclaration"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:37:1: uniformsDeclaration returns [ List<Uniform> uniforms ] : 'uniform' '{' ( uniformDeclaration )* '}' ;
    public final List<Uniform> uniformsDeclaration() throws RecognitionException {
        List<Uniform> uniforms = null;

        Uniform uniformDeclaration4 = null;



        	uniforms = new ArrayList<Uniform>();

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:41:2: ( 'uniform' '{' ( uniformDeclaration )* '}' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:41:4: 'uniform' '{' ( uniformDeclaration )* '}'
            {
            match(input,17,FOLLOW_17_in_uniformsDeclaration161); if (state.failed) return uniforms;
            match(input,18,FOLLOW_18_in_uniformsDeclaration163); if (state.failed) return uniforms;
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:42:3: ( uniformDeclaration )*
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0==ID) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:42:5: uniformDeclaration
            	    {
            	    pushFollow(FOLLOW_uniformDeclaration_in_uniformsDeclaration169);
            	    uniformDeclaration4=uniformDeclaration();

            	    state._fsp--;
            	    if (state.failed) return uniforms;
            	    if ( state.backtracking==0 ) {
            	       uniforms.add( uniformDeclaration4 ); 
            	    }

            	    }
            	    break;

            	default :
            	    break loop4;
                }
            } while (true);

            match(input,19,FOLLOW_19_in_uniformsDeclaration178); if (state.failed) return uniforms;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return uniforms;
    }
    // $ANTLR end "uniformsDeclaration"


    // $ANTLR start "inputLayoutDeclaration"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:46:1: inputLayoutDeclaration returns [List<Binding> bindings] : 'inputlayout' '{' ( inputDeclaration[$bindings] ( ',' inputDeclaration[$bindings] )* )? '}' ;
    public final List<Binding> inputLayoutDeclaration() throws RecognitionException {
        List<Binding> bindings = null;


        	bindings = new ArrayList<Binding>();

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:50:2: ( 'inputlayout' '{' ( inputDeclaration[$bindings] ( ',' inputDeclaration[$bindings] )* )? '}' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:50:5: 'inputlayout' '{' ( inputDeclaration[$bindings] ( ',' inputDeclaration[$bindings] )* )? '}'
            {
            match(input,20,FOLLOW_20_in_inputLayoutDeclaration200); if (state.failed) return bindings;
            match(input,18,FOLLOW_18_in_inputLayoutDeclaration202); if (state.failed) return bindings;
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:51:4: ( inputDeclaration[$bindings] ( ',' inputDeclaration[$bindings] )* )?
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( (LA6_0==ID) ) {
                alt6=1;
            }
            switch (alt6) {
                case 1 :
                    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:51:5: inputDeclaration[$bindings] ( ',' inputDeclaration[$bindings] )*
                    {
                    pushFollow(FOLLOW_inputDeclaration_in_inputLayoutDeclaration208);
                    inputDeclaration(bindings);

                    state._fsp--;
                    if (state.failed) return bindings;
                    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:51:33: ( ',' inputDeclaration[$bindings] )*
                    loop5:
                    do {
                        int alt5=2;
                        int LA5_0 = input.LA(1);

                        if ( (LA5_0==21) ) {
                            alt5=1;
                        }


                        switch (alt5) {
                    	case 1 :
                    	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:51:34: ',' inputDeclaration[$bindings]
                    	    {
                    	    match(input,21,FOLLOW_21_in_inputLayoutDeclaration212); if (state.failed) return bindings;
                    	    pushFollow(FOLLOW_inputDeclaration_in_inputLayoutDeclaration214);
                    	    inputDeclaration(bindings);

                    	    state._fsp--;
                    	    if (state.failed) return bindings;

                    	    }
                    	    break;

                    	default :
                    	    break loop5;
                        }
                    } while (true);


                    }
                    break;

            }

            match(input,19,FOLLOW_19_in_inputLayoutDeclaration223); if (state.failed) return bindings;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return bindings;
    }
    // $ANTLR end "inputLayoutDeclaration"


    // $ANTLR start "inputDeclaration"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:55:1: inputDeclaration[List<Binding> bindings] : name= ID ':' type= ID ( '[' size= NUMBER ']' )? ;
    public final void inputDeclaration(List<Binding> bindings) throws RecognitionException {
        Token name=null;
        Token type=null;
        Token size=null;


        	int numComponents = 1;

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:59:2: (name= ID ':' type= ID ( '[' size= NUMBER ']' )? )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:59:4: name= ID ':' type= ID ( '[' size= NUMBER ']' )?
            {
            name=(Token)match(input,ID,FOLLOW_ID_in_inputDeclaration242); if (state.failed) return ;
            match(input,22,FOLLOW_22_in_inputDeclaration244); if (state.failed) return ;
            type=(Token)match(input,ID,FOLLOW_ID_in_inputDeclaration248); if (state.failed) return ;
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:59:24: ( '[' size= NUMBER ']' )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0==23) ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:59:25: '[' size= NUMBER ']'
                    {
                    match(input,23,FOLLOW_23_in_inputDeclaration251); if (state.failed) return ;
                    size=(Token)match(input,NUMBER,FOLLOW_NUMBER_in_inputDeclaration255); if (state.failed) return ;
                    if ( state.backtracking==0 ) {
                       numComponents = new Integer( (size!=null?size.getText():null) ); 
                    }
                    match(input,24,FOLLOW_24_in_inputDeclaration259); if (state.failed) return ;

                    }
                    break;

            }

            if ( state.backtracking==0 ) {
               bindings.add( new Binding( (name!=null?name.getText():null), numComponents, (type!=null?type.getText():null) ) ); 
            }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "inputDeclaration"


    // $ANTLR start "uniformDeclaration"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:62:1: uniformDeclaration returns [Uniform uniform] : type= ID name= ID ';' ;
    public final Uniform uniformDeclaration() throws RecognitionException {
        Uniform uniform = null;

        Token type=null;
        Token name=null;

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:63:2: (type= ID name= ID ';' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:63:4: type= ID name= ID ';'
            {
            type=(Token)match(input,ID,FOLLOW_ID_in_uniformDeclaration280); if (state.failed) return uniform;
            name=(Token)match(input,ID,FOLLOW_ID_in_uniformDeclaration284); if (state.failed) return uniform;
            match(input,25,FOLLOW_25_in_uniformDeclaration286); if (state.failed) return uniform;
            if ( state.backtracking==0 ) {
               uniform = new Uniform( (type!=null?type.getText():null), (name!=null?name.getText():null) ); 
            }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return uniform;
    }
    // $ANTLR end "uniformDeclaration"

    public static class passDefinition_return extends ParserRuleReturnScope {
        public String vertexShader;
        public Pass.GeometryShader geometryShader;
        public String fragmentShader;
        public List<String> transformFeedbackVaryings;
        public Pass.State stateBlock;
        public String fragDataName;
    };

    // $ANTLR start "passDefinition"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:66:1: passDefinition returns [String vertexShader, Pass.GeometryShader geometryShader, String fragmentShader, List<String> transformFeedbackVaryings, Pass.State stateBlock, String fragDataName] : 'pass' '{' ({...}? 'vertex' vertexShaderCode= codeBlock | {...}? 'fragment' fragmentShaderCode= codeBlock | {...}? 'geometry' '(' inputType= ID ',' outputType= ID ',' maxEmitVertices= parameter ')' geometryShaderCode= codeBlock | {...}? 'feedback' '{' ( transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )* )? '}' | {...}? 'fragDataName' fragDataNameID= ID | stateDefinition[$stateBlock] | ';' )* '}' ;
    public final SimpleGLSLEffectFileParser.passDefinition_return passDefinition() throws RecognitionException {
        SimpleGLSLEffectFileParser.passDefinition_return retval = new SimpleGLSLEffectFileParser.passDefinition_return();
        retval.start = input.LT(1);

        Token inputType=null;
        Token outputType=null;
        Token fragDataNameID=null;
        String vertexShaderCode = null;

        String fragmentShaderCode = null;

        SimpleGLSLEffectFileParser.parameter_return maxEmitVertices = null;

        String geometryShaderCode = null;



        	retval.stateBlock = new Pass.State();

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:69:3: ( 'pass' '{' ({...}? 'vertex' vertexShaderCode= codeBlock | {...}? 'fragment' fragmentShaderCode= codeBlock | {...}? 'geometry' '(' inputType= ID ',' outputType= ID ',' maxEmitVertices= parameter ')' geometryShaderCode= codeBlock | {...}? 'feedback' '{' ( transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )* )? '}' | {...}? 'fragDataName' fragDataNameID= ID | stateDefinition[$stateBlock] | ';' )* '}' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:69:5: 'pass' '{' ({...}? 'vertex' vertexShaderCode= codeBlock | {...}? 'fragment' fragmentShaderCode= codeBlock | {...}? 'geometry' '(' inputType= ID ',' outputType= ID ',' maxEmitVertices= parameter ')' geometryShaderCode= codeBlock | {...}? 'feedback' '{' ( transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )* )? '}' | {...}? 'fragDataName' fragDataNameID= ID | stateDefinition[$stateBlock] | ';' )* '}'
            {
            match(input,26,FOLLOW_26_in_passDefinition308); if (state.failed) return retval;
            match(input,18,FOLLOW_18_in_passDefinition310); if (state.failed) return retval;
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:70:4: ({...}? 'vertex' vertexShaderCode= codeBlock | {...}? 'fragment' fragmentShaderCode= codeBlock | {...}? 'geometry' '(' inputType= ID ',' outputType= ID ',' maxEmitVertices= parameter ')' geometryShaderCode= codeBlock | {...}? 'feedback' '{' ( transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )* )? '}' | {...}? 'fragDataName' fragDataNameID= ID | stateDefinition[$stateBlock] | ';' )*
            loop10:
            do {
                int alt10=8;
                switch ( input.LA(1) ) {
                case 14:
                    {
                    alt10=1;
                    }
                    break;
                case 15:
                    {
                    alt10=2;
                    }
                    break;
                case 16:
                    {
                    alt10=3;
                    }
                    break;
                case 29:
                    {
                    alt10=4;
                    }
                    break;
                case 30:
                    {
                    alt10=5;
                    }
                    break;
                case 31:
                    {
                    alt10=6;
                    }
                    break;
                case 25:
                    {
                    alt10=7;
                    }
                    break;

                }

                switch (alt10) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:70:5: {...}? 'vertex' vertexShaderCode= codeBlock
            	    {
            	    if ( !((retval.vertexShader == null)) ) {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        throw new FailedPredicateException(input, "passDefinition", "$vertexShader == null");
            	    }
            	    match(input,14,FOLLOW_14_in_passDefinition318); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_passDefinition322);
            	    vertexShaderCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.vertexShader = vertexShaderCode; 
            	    }

            	    }
            	    break;
            	case 2 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:71:5: {...}? 'fragment' fragmentShaderCode= codeBlock
            	    {
            	    if ( !((retval.fragmentShader == null)) ) {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        throw new FailedPredicateException(input, "passDefinition", "$fragmentShader == null");
            	    }
            	    match(input,15,FOLLOW_15_in_passDefinition333); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_passDefinition337);
            	    fragmentShaderCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.fragmentShader = fragmentShaderCode; 
            	    }

            	    }
            	    break;
            	case 3 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:72:5: {...}? 'geometry' '(' inputType= ID ',' outputType= ID ',' maxEmitVertices= parameter ')' geometryShaderCode= codeBlock
            	    {
            	    if ( !((retval.geometryShader == null)) ) {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        throw new FailedPredicateException(input, "passDefinition", "$geometryShader == null");
            	    }
            	    match(input,16,FOLLOW_16_in_passDefinition347); if (state.failed) return retval;
            	    match(input,27,FOLLOW_27_in_passDefinition349); if (state.failed) return retval;
            	    inputType=(Token)match(input,ID,FOLLOW_ID_in_passDefinition353); if (state.failed) return retval;
            	    match(input,21,FOLLOW_21_in_passDefinition355); if (state.failed) return retval;
            	    outputType=(Token)match(input,ID,FOLLOW_ID_in_passDefinition359); if (state.failed) return retval;
            	    match(input,21,FOLLOW_21_in_passDefinition361); if (state.failed) return retval;
            	    pushFollow(FOLLOW_parameter_in_passDefinition365);
            	    maxEmitVertices=parameter();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    match(input,28,FOLLOW_28_in_passDefinition367); if (state.failed) return retval;
            	    pushFollow(FOLLOW_codeBlock_in_passDefinition371);
            	    geometryShaderCode=codeBlock();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.geometryShader = new Pass.GeometryShader( geometryShaderCode, (inputType!=null?inputType.getText():null), (outputType!=null?outputType.getText():null), (maxEmitVertices!=null?input.toString(maxEmitVertices.start,maxEmitVertices.stop):null) ); 
            	    }

            	    }
            	    break;
            	case 4 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:75:5: {...}? 'feedback' '{' ( transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )* )? '}'
            	    {
            	    if ( !((retval.transformFeedbackVaryings == null)) ) {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        throw new FailedPredicateException(input, "passDefinition", "$transformFeedbackVaryings == null");
            	    }
            	    match(input,29,FOLLOW_29_in_passDefinition390); if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.transformFeedbackVaryings = new ArrayList<String>(); 
            	    }
            	    match(input,18,FOLLOW_18_in_passDefinition403); if (state.failed) return retval;
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:78:6: ( transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )* )?
            	    int alt9=2;
            	    int LA9_0 = input.LA(1);

            	    if ( (LA9_0==ID) ) {
            	        alt9=1;
            	    }
            	    switch (alt9) {
            	        case 1 :
            	            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:78:8: transformFeedbackVarying[ $transformFeedbackVaryings ] ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )*
            	            {
            	            pushFollow(FOLLOW_transformFeedbackVarying_in_passDefinition413);
            	            transformFeedbackVarying(retval.transformFeedbackVaryings);

            	            state._fsp--;
            	            if (state.failed) return retval;
            	            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:78:63: ( ',' transformFeedbackVarying[ $transformFeedbackVaryings ] )*
            	            loop8:
            	            do {
            	                int alt8=2;
            	                int LA8_0 = input.LA(1);

            	                if ( (LA8_0==21) ) {
            	                    alt8=1;
            	                }


            	                switch (alt8) {
            	            	case 1 :
            	            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:78:64: ',' transformFeedbackVarying[ $transformFeedbackVaryings ]
            	            	    {
            	            	    match(input,21,FOLLOW_21_in_passDefinition417); if (state.failed) return retval;
            	            	    pushFollow(FOLLOW_transformFeedbackVarying_in_passDefinition419);
            	            	    transformFeedbackVarying(retval.transformFeedbackVaryings);

            	            	    state._fsp--;
            	            	    if (state.failed) return retval;

            	            	    }
            	            	    break;

            	            	default :
            	            	    break loop8;
            	                }
            	            } while (true);


            	            }
            	            break;

            	    }

            	    match(input,19,FOLLOW_19_in_passDefinition431); if (state.failed) return retval;

            	    }
            	    break;
            	case 5 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:80:6: {...}? 'fragDataName' fragDataNameID= ID
            	    {
            	    if ( !((retval.fragDataName == null)) ) {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        throw new FailedPredicateException(input, "passDefinition", "$fragDataName == null");
            	    }
            	    match(input,30,FOLLOW_30_in_passDefinition440); if (state.failed) return retval;
            	    fragDataNameID=(Token)match(input,ID,FOLLOW_ID_in_passDefinition444); if (state.failed) return retval;
            	    if ( state.backtracking==0 ) {
            	       retval.fragDataName = (fragDataNameID!=null?fragDataNameID.getText():null); 
            	    }

            	    }
            	    break;
            	case 6 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:81:6: stateDefinition[$stateBlock]
            	    {
            	    pushFollow(FOLLOW_stateDefinition_in_passDefinition453);
            	    stateDefinition(retval.stateBlock);

            	    state._fsp--;
            	    if (state.failed) return retval;

            	    }
            	    break;
            	case 7 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:82:6: ';'
            	    {
            	    match(input,25,FOLLOW_25_in_passDefinition461); if (state.failed) return retval;

            	    }
            	    break;

            	default :
            	    break loop10;
                }
            } while (true);

            match(input,19,FOLLOW_19_in_passDefinition467); if (state.failed) return retval;

            }

            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "passDefinition"


    // $ANTLR start "transformFeedbackVarying"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:85:1: transformFeedbackVarying[List<String> transformFeedbackVaryings] : ID ;
    public final void transformFeedbackVarying(List<String> transformFeedbackVaryings) throws RecognitionException {
        Token ID5=null;

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:86:2: ( ID )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:86:4: ID
            {
            ID5=(Token)match(input,ID,FOLLOW_ID_in_transformFeedbackVarying479); if (state.failed) return ;
            if ( state.backtracking==0 ) {
               transformFeedbackVaryings.add( (ID5!=null?ID5.getText():null) ); 
            }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "transformFeedbackVarying"


    // $ANTLR start "stateDefinition"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:89:1: stateDefinition[Pass.State stateBlock] : 'state' '{' ( ( 'depthTest' ':' toggle= bool ) | ( 'stencilTest' ':' toggle= bool ) )* '}' ;
    public final void stateDefinition(Pass.State stateBlock) throws RecognitionException {
        boolean toggle = false;


        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:90:2: ( 'state' '{' ( ( 'depthTest' ':' toggle= bool ) | ( 'stencilTest' ':' toggle= bool ) )* '}' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:90:4: 'state' '{' ( ( 'depthTest' ':' toggle= bool ) | ( 'stencilTest' ':' toggle= bool ) )* '}'
            {
            match(input,31,FOLLOW_31_in_stateDefinition493); if (state.failed) return ;
            match(input,18,FOLLOW_18_in_stateDefinition495); if (state.failed) return ;
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:91:3: ( ( 'depthTest' ':' toggle= bool ) | ( 'stencilTest' ':' toggle= bool ) )*
            loop11:
            do {
                int alt11=3;
                int LA11_0 = input.LA(1);

                if ( (LA11_0==32) ) {
                    alt11=1;
                }
                else if ( (LA11_0==33) ) {
                    alt11=2;
                }


                switch (alt11) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:91:5: ( 'depthTest' ':' toggle= bool )
            	    {
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:91:5: ( 'depthTest' ':' toggle= bool )
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:91:6: 'depthTest' ':' toggle= bool
            	    {
            	    match(input,32,FOLLOW_32_in_stateDefinition502); if (state.failed) return ;
            	    match(input,22,FOLLOW_22_in_stateDefinition504); if (state.failed) return ;
            	    pushFollow(FOLLOW_bool_in_stateDefinition508);
            	    toggle=bool();

            	    state._fsp--;
            	    if (state.failed) return ;
            	    if ( state.backtracking==0 ) {
            	       stateBlock.depthTest = toggle; 
            	    }

            	    }


            	    }
            	    break;
            	case 2 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:92:5: ( 'stencilTest' ':' toggle= bool )
            	    {
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:92:5: ( 'stencilTest' ':' toggle= bool )
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:92:6: 'stencilTest' ':' toggle= bool
            	    {
            	    match(input,33,FOLLOW_33_in_stateDefinition518); if (state.failed) return ;
            	    match(input,22,FOLLOW_22_in_stateDefinition520); if (state.failed) return ;
            	    pushFollow(FOLLOW_bool_in_stateDefinition524);
            	    toggle=bool();

            	    state._fsp--;
            	    if (state.failed) return ;
            	    if ( state.backtracking==0 ) {
            	       stateBlock.stencilTest = toggle; 
            	    }

            	    }


            	    }
            	    break;

            	default :
            	    break loop11;
                }
            } while (true);

            match(input,19,FOLLOW_19_in_stateDefinition536); if (state.failed) return ;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "stateDefinition"


    // $ANTLR start "bool"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:96:1: bool returns [boolean value] : ( 'true' | 'false' );
    public final boolean bool() throws RecognitionException {
        boolean value = false;

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:97:2: ( 'true' | 'false' )
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==34) ) {
                alt12=1;
            }
            else if ( (LA12_0==35) ) {
                alt12=2;
            }
            else {
                if (state.backtracking>0) {state.failed=true; return value;}
                NoViableAltException nvae =
                    new NoViableAltException("", 12, 0, input);

                throw nvae;
            }
            switch (alt12) {
                case 1 :
                    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:97:4: 'true'
                    {
                    match(input,34,FOLLOW_34_in_bool551); if (state.failed) return value;
                    if ( state.backtracking==0 ) {
                       value = true; 
                    }

                    }
                    break;
                case 2 :
                    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:97:32: 'false'
                    {
                    match(input,35,FOLLOW_35_in_bool557); if (state.failed) return value;
                    if ( state.backtracking==0 ) {
                       value = false; 
                    }

                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "bool"

    public static class parameter_return extends ParserRuleReturnScope {
    };

    // $ANTLR start "parameter"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:100:1: parameter : ( '(' parameter ')' | '{' | '}' | ~ ( '(' | ')' ) )* ;
    public final SimpleGLSLEffectFileParser.parameter_return parameter() throws RecognitionException {
        SimpleGLSLEffectFileParser.parameter_return retval = new SimpleGLSLEffectFileParser.parameter_return();
        retval.start = input.LT(1);

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:2: ( ( '(' parameter ')' | '{' | '}' | ~ ( '(' | ')' ) )* )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:4: ( '(' parameter ')' | '{' | '}' | ~ ( '(' | ')' ) )*
            {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:4: ( '(' parameter ')' | '{' | '}' | ~ ( '(' | ')' ) )*
            loop13:
            do {
                int alt13=5;
                switch ( input.LA(1) ) {
                case 27:
                    {
                    alt13=1;
                    }
                    break;
                case 18:
                    {
                    int LA13_3 = input.LA(2);

                    if ( (synpred26_SimpleGLSLEffectFile()) ) {
                        alt13=2;
                    }
                    else if ( (synpred29_SimpleGLSLEffectFile()) ) {
                        alt13=4;
                    }


                    }
                    break;
                case 19:
                    {
                    int LA13_4 = input.LA(2);

                    if ( (synpred27_SimpleGLSLEffectFile()) ) {
                        alt13=3;
                    }
                    else if ( (synpred29_SimpleGLSLEffectFile()) ) {
                        alt13=4;
                    }


                    }
                    break;
                case NUMBER:
                case ID:
                case LETTER:
                case WS:
                case COMMENT:
                case LINE_COMMENT:
                case LINE:
                case ANYTHING_BUT_BRACES_OR_PARENTHESES:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 34:
                case 35:
                    {
                    alt13=4;
                    }
                    break;

                }

                switch (alt13) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:6: '(' parameter ')'
            	    {
            	    match(input,27,FOLLOW_27_in_parameter573); if (state.failed) return retval;
            	    pushFollow(FOLLOW_parameter_in_parameter575);
            	    parameter();

            	    state._fsp--;
            	    if (state.failed) return retval;
            	    match(input,28,FOLLOW_28_in_parameter577); if (state.failed) return retval;

            	    }
            	    break;
            	case 2 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:26: '{'
            	    {
            	    match(input,18,FOLLOW_18_in_parameter581); if (state.failed) return retval;

            	    }
            	    break;
            	case 3 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:32: '}'
            	    {
            	    match(input,19,FOLLOW_19_in_parameter585); if (state.failed) return retval;

            	    }
            	    break;
            	case 4 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:38: ~ ( '(' | ')' )
            	    {
            	    if ( (input.LA(1)>=NUMBER && input.LA(1)<=26)||(input.LA(1)>=29 && input.LA(1)<=35) ) {
            	        input.consume();
            	        state.errorRecovery=false;state.failed=false;
            	    }
            	    else {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop13;
                }
            } while (true);


            }

            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "parameter"


    // $ANTLR start "codeBlock"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:105:1: codeBlock returns [String innerCode] : '{' code '}' ;
    public final String codeBlock() throws RecognitionException {
        String innerCode = null;

        SimpleGLSLEffectFileParser.code_return code6 = null;


        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:106:2: ( '{' code '}' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:106:4: '{' code '}'
            {
            match(input,18,FOLLOW_18_in_codeBlock617); if (state.failed) return innerCode;
            pushFollow(FOLLOW_code_in_codeBlock619);
            code6=code();

            state._fsp--;
            if (state.failed) return innerCode;
            match(input,19,FOLLOW_19_in_codeBlock621); if (state.failed) return innerCode;
            if ( state.backtracking==0 ) {
               innerCode = "#line " + ((code6!=null?((Token)code6.start):null).getLine() - 1) + "\r\n" + (code6!=null?input.toString(code6.start,code6.stop):null) + "\r\n"; 
            }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return innerCode;
    }
    // $ANTLR end "codeBlock"

    public static class code_return extends ParserRuleReturnScope {
    };

    // $ANTLR start "code"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:109:1: code : ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )* ;
    public final SimpleGLSLEffectFileParser.code_return code() throws RecognitionException {
        SimpleGLSLEffectFileParser.code_return retval = new SimpleGLSLEffectFileParser.code_return();
        retval.start = input.LT(1);

        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:2: ( ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )* )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:5: ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )*
            {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:5: ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )*
            loop14:
            do {
                int alt14=5;
                switch ( input.LA(1) ) {
                case 18:
                    {
                    alt14=1;
                    }
                    break;
                case 27:
                    {
                    int LA14_3 = input.LA(2);

                    if ( (synpred31_SimpleGLSLEffectFile()) ) {
                        alt14=2;
                    }
                    else if ( (synpred34_SimpleGLSLEffectFile()) ) {
                        alt14=4;
                    }


                    }
                    break;
                case 28:
                    {
                    int LA14_4 = input.LA(2);

                    if ( (synpred32_SimpleGLSLEffectFile()) ) {
                        alt14=3;
                    }
                    else if ( (synpred34_SimpleGLSLEffectFile()) ) {
                        alt14=4;
                    }


                    }
                    break;
                case NUMBER:
                case ID:
                case LETTER:
                case WS:
                case COMMENT:
                case LINE_COMMENT:
                case LINE:
                case ANYTHING_BUT_BRACES_OR_PARENTHESES:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 34:
                case 35:
                    {
                    alt14=4;
                    }
                    break;

                }

                switch (alt14) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:7: simpleCode
            	    {
            	    pushFollow(FOLLOW_simpleCode_in_code638);
            	    simpleCode();

            	    state._fsp--;
            	    if (state.failed) return retval;

            	    }
            	    break;
            	case 2 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:20: '('
            	    {
            	    match(input,27,FOLLOW_27_in_code642); if (state.failed) return retval;

            	    }
            	    break;
            	case 3 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:26: ')'
            	    {
            	    match(input,28,FOLLOW_28_in_code646); if (state.failed) return retval;

            	    }
            	    break;
            	case 4 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:32: ~ ( '{' | '}' )
            	    {
            	    if ( (input.LA(1)>=NUMBER && input.LA(1)<=17)||(input.LA(1)>=20 && input.LA(1)<=35) ) {
            	        input.consume();
            	        state.errorRecovery=false;state.failed=false;
            	    }
            	    else {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);


            }

            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "code"


    // $ANTLR start "simpleCode"
    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:113:1: simpleCode : '{' ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )* '}' ;
    public final void simpleCode() throws RecognitionException {
        try {
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:2: ( '{' ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )* '}' )
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:4: '{' ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )* '}'
            {
            match(input,18,FOLLOW_18_in_simpleCode671); if (state.failed) return ;
            // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:8: ( simpleCode | '(' | ')' | ~ ( '{' | '}' ) )*
            loop15:
            do {
                int alt15=5;
                switch ( input.LA(1) ) {
                case 18:
                    {
                    alt15=1;
                    }
                    break;
                case 27:
                    {
                    int LA15_3 = input.LA(2);

                    if ( (synpred36_SimpleGLSLEffectFile()) ) {
                        alt15=2;
                    }
                    else if ( (synpred39_SimpleGLSLEffectFile()) ) {
                        alt15=4;
                    }


                    }
                    break;
                case 28:
                    {
                    int LA15_4 = input.LA(2);

                    if ( (synpred37_SimpleGLSLEffectFile()) ) {
                        alt15=3;
                    }
                    else if ( (synpred39_SimpleGLSLEffectFile()) ) {
                        alt15=4;
                    }


                    }
                    break;
                case NUMBER:
                case ID:
                case LETTER:
                case WS:
                case COMMENT:
                case LINE_COMMENT:
                case LINE:
                case ANYTHING_BUT_BRACES_OR_PARENTHESES:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 34:
                case 35:
                    {
                    alt15=4;
                    }
                    break;

                }

                switch (alt15) {
            	case 1 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:10: simpleCode
            	    {
            	    pushFollow(FOLLOW_simpleCode_in_simpleCode675);
            	    simpleCode();

            	    state._fsp--;
            	    if (state.failed) return ;

            	    }
            	    break;
            	case 2 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:23: '('
            	    {
            	    match(input,27,FOLLOW_27_in_simpleCode679); if (state.failed) return ;

            	    }
            	    break;
            	case 3 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:29: ')'
            	    {
            	    match(input,28,FOLLOW_28_in_simpleCode683); if (state.failed) return ;

            	    }
            	    break;
            	case 4 :
            	    // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:35: ~ ( '{' | '}' )
            	    {
            	    if ( (input.LA(1)>=NUMBER && input.LA(1)<=17)||(input.LA(1)>=20 && input.LA(1)<=35) ) {
            	        input.consume();
            	        state.errorRecovery=false;state.failed=false;
            	    }
            	    else {
            	        if (state.backtracking>0) {state.failed=true; return ;}
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop15;
                }
            } while (true);

            match(input,19,FOLLOW_19_in_simpleCode698); if (state.failed) return ;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "simpleCode"

    // $ANTLR start synpred26_SimpleGLSLEffectFile
    public final void synpred26_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:26: ( '{' )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:26: '{'
        {
        match(input,18,FOLLOW_18_in_synpred26_SimpleGLSLEffectFile581); if (state.failed) return ;

        }
    }
    // $ANTLR end synpred26_SimpleGLSLEffectFile

    // $ANTLR start synpred27_SimpleGLSLEffectFile
    public final void synpred27_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:32: ( '}' )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:32: '}'
        {
        match(input,19,FOLLOW_19_in_synpred27_SimpleGLSLEffectFile585); if (state.failed) return ;

        }
    }
    // $ANTLR end synpred27_SimpleGLSLEffectFile

    // $ANTLR start synpred29_SimpleGLSLEffectFile
    public final void synpred29_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:38: (~ ( '(' | ')' ) )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:101:38: ~ ( '(' | ')' )
        {
        if ( (input.LA(1)>=NUMBER && input.LA(1)<=26)||(input.LA(1)>=29 && input.LA(1)<=35) ) {
            input.consume();
            state.errorRecovery=false;state.failed=false;
        }
        else {
            if (state.backtracking>0) {state.failed=true; return ;}
            MismatchedSetException mse = new MismatchedSetException(null,input);
            throw mse;
        }


        }
    }
    // $ANTLR end synpred29_SimpleGLSLEffectFile

    // $ANTLR start synpred31_SimpleGLSLEffectFile
    public final void synpred31_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:20: ( '(' )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:20: '('
        {
        match(input,27,FOLLOW_27_in_synpred31_SimpleGLSLEffectFile642); if (state.failed) return ;

        }
    }
    // $ANTLR end synpred31_SimpleGLSLEffectFile

    // $ANTLR start synpred32_SimpleGLSLEffectFile
    public final void synpred32_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:26: ( ')' )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:26: ')'
        {
        match(input,28,FOLLOW_28_in_synpred32_SimpleGLSLEffectFile646); if (state.failed) return ;

        }
    }
    // $ANTLR end synpred32_SimpleGLSLEffectFile

    // $ANTLR start synpred34_SimpleGLSLEffectFile
    public final void synpred34_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:32: (~ ( '{' | '}' ) )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:110:32: ~ ( '{' | '}' )
        {
        if ( (input.LA(1)>=NUMBER && input.LA(1)<=17)||(input.LA(1)>=20 && input.LA(1)<=35) ) {
            input.consume();
            state.errorRecovery=false;state.failed=false;
        }
        else {
            if (state.backtracking>0) {state.failed=true; return ;}
            MismatchedSetException mse = new MismatchedSetException(null,input);
            throw mse;
        }


        }
    }
    // $ANTLR end synpred34_SimpleGLSLEffectFile

    // $ANTLR start synpred36_SimpleGLSLEffectFile
    public final void synpred36_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:23: ( '(' )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:23: '('
        {
        match(input,27,FOLLOW_27_in_synpred36_SimpleGLSLEffectFile679); if (state.failed) return ;

        }
    }
    // $ANTLR end synpred36_SimpleGLSLEffectFile

    // $ANTLR start synpred37_SimpleGLSLEffectFile
    public final void synpred37_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:29: ( ')' )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:29: ')'
        {
        match(input,28,FOLLOW_28_in_synpred37_SimpleGLSLEffectFile683); if (state.failed) return ;

        }
    }
    // $ANTLR end synpred37_SimpleGLSLEffectFile

    // $ANTLR start synpred39_SimpleGLSLEffectFile
    public final void synpred39_SimpleGLSLEffectFile_fragment() throws RecognitionException {   
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:35: (~ ( '{' | '}' ) )
        // D:\\CGHiwi\\SimpleGLSLEffect\\SimpleGLSLEffectFile.g:114:35: ~ ( '{' | '}' )
        {
        if ( (input.LA(1)>=NUMBER && input.LA(1)<=17)||(input.LA(1)>=20 && input.LA(1)<=35) ) {
            input.consume();
            state.errorRecovery=false;state.failed=false;
        }
        else {
            if (state.backtracking>0) {state.failed=true; return ;}
            MismatchedSetException mse = new MismatchedSetException(null,input);
            throw mse;
        }


        }
    }
    // $ANTLR end synpred39_SimpleGLSLEffectFile

    // Delegated rules

    public final boolean synpred37_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred37_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred31_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred31_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred36_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred36_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred29_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred29_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred26_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred26_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred39_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred39_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred27_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred27_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred34_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred34_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }
    public final boolean synpred32_SimpleGLSLEffectFile() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred32_SimpleGLSLEffectFile_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }


 

    public static final BitSet FOLLOW_12_in_effectFile41 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_NUMBER_in_effectFile45 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_13_in_effectFile58 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_effectFile62 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_14_in_effectFile70 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_effectFile74 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_15_in_effectFile82 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_effectFile86 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_16_in_effectFile94 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_effectFile98 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_uniformsDeclaration_in_effectFile106 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_inputLayoutDeclaration_in_effectFile116 = new BitSet(new long[]{0x000000000413E000L});
    public static final BitSet FOLLOW_passDefinition_in_effectFile129 = new BitSet(new long[]{0x000000000413E002L});
    public static final BitSet FOLLOW_17_in_uniformsDeclaration161 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_uniformsDeclaration163 = new BitSet(new long[]{0x0000000000080020L});
    public static final BitSet FOLLOW_uniformDeclaration_in_uniformsDeclaration169 = new BitSet(new long[]{0x0000000000080020L});
    public static final BitSet FOLLOW_19_in_uniformsDeclaration178 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_inputLayoutDeclaration200 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_inputLayoutDeclaration202 = new BitSet(new long[]{0x0000000000080020L});
    public static final BitSet FOLLOW_inputDeclaration_in_inputLayoutDeclaration208 = new BitSet(new long[]{0x0000000000280000L});
    public static final BitSet FOLLOW_21_in_inputLayoutDeclaration212 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_inputDeclaration_in_inputLayoutDeclaration214 = new BitSet(new long[]{0x0000000000280000L});
    public static final BitSet FOLLOW_19_in_inputLayoutDeclaration223 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ID_in_inputDeclaration242 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_inputDeclaration244 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_ID_in_inputDeclaration248 = new BitSet(new long[]{0x0000000000800002L});
    public static final BitSet FOLLOW_23_in_inputDeclaration251 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_NUMBER_in_inputDeclaration255 = new BitSet(new long[]{0x0000000001000000L});
    public static final BitSet FOLLOW_24_in_inputDeclaration259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ID_in_uniformDeclaration280 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_ID_in_uniformDeclaration284 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_25_in_uniformDeclaration286 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_passDefinition308 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_passDefinition310 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_14_in_passDefinition318 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_passDefinition322 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_15_in_passDefinition333 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_passDefinition337 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_16_in_passDefinition347 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_27_in_passDefinition349 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_ID_in_passDefinition353 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_21_in_passDefinition355 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_ID_in_passDefinition359 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_21_in_passDefinition361 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_parameter_in_passDefinition365 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_28_in_passDefinition367 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_codeBlock_in_passDefinition371 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_29_in_passDefinition390 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_passDefinition403 = new BitSet(new long[]{0x0000000000080020L});
    public static final BitSet FOLLOW_transformFeedbackVarying_in_passDefinition413 = new BitSet(new long[]{0x0000000000280000L});
    public static final BitSet FOLLOW_21_in_passDefinition417 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_transformFeedbackVarying_in_passDefinition419 = new BitSet(new long[]{0x0000000000280000L});
    public static final BitSet FOLLOW_19_in_passDefinition431 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_30_in_passDefinition440 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_ID_in_passDefinition444 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_stateDefinition_in_passDefinition453 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_25_in_passDefinition461 = new BitSet(new long[]{0x00000000E209C000L});
    public static final BitSet FOLLOW_19_in_passDefinition467 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ID_in_transformFeedbackVarying479 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_stateDefinition493 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_stateDefinition495 = new BitSet(new long[]{0x0000000300080000L});
    public static final BitSet FOLLOW_32_in_stateDefinition502 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_stateDefinition504 = new BitSet(new long[]{0x0000000C00000000L});
    public static final BitSet FOLLOW_bool_in_stateDefinition508 = new BitSet(new long[]{0x0000000300080000L});
    public static final BitSet FOLLOW_33_in_stateDefinition518 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_stateDefinition520 = new BitSet(new long[]{0x0000000C00000000L});
    public static final BitSet FOLLOW_bool_in_stateDefinition524 = new BitSet(new long[]{0x0000000300080000L});
    public static final BitSet FOLLOW_19_in_stateDefinition536 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_bool551 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_35_in_bool557 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_parameter573 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_parameter_in_parameter575 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_28_in_parameter577 = new BitSet(new long[]{0x0000000FEFFFFFF2L});
    public static final BitSet FOLLOW_18_in_parameter581 = new BitSet(new long[]{0x0000000FEFFFFFF2L});
    public static final BitSet FOLLOW_19_in_parameter585 = new BitSet(new long[]{0x0000000FEFFFFFF2L});
    public static final BitSet FOLLOW_set_in_parameter589 = new BitSet(new long[]{0x0000000FEFFFFFF2L});
    public static final BitSet FOLLOW_18_in_codeBlock617 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_code_in_codeBlock619 = new BitSet(new long[]{0x0000000000080000L});
    public static final BitSet FOLLOW_19_in_codeBlock621 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_simpleCode_in_code638 = new BitSet(new long[]{0x0000000FFFF7FFF2L});
    public static final BitSet FOLLOW_27_in_code642 = new BitSet(new long[]{0x0000000FFFF7FFF2L});
    public static final BitSet FOLLOW_28_in_code646 = new BitSet(new long[]{0x0000000FFFF7FFF2L});
    public static final BitSet FOLLOW_set_in_code650 = new BitSet(new long[]{0x0000000FFFF7FFF2L});
    public static final BitSet FOLLOW_18_in_simpleCode671 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_simpleCode_in_simpleCode675 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_27_in_simpleCode679 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_28_in_simpleCode683 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_set_in_simpleCode687 = new BitSet(new long[]{0x0000000FFFFFFFF0L});
    public static final BitSet FOLLOW_19_in_simpleCode698 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_synpred26_SimpleGLSLEffectFile581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_synpred27_SimpleGLSLEffectFile585 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_set_in_synpred29_SimpleGLSLEffectFile589 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_synpred31_SimpleGLSLEffectFile642 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_synpred32_SimpleGLSLEffectFile646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_set_in_synpred34_SimpleGLSLEffectFile650 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_synpred36_SimpleGLSLEffectFile679 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_synpred37_SimpleGLSLEffectFile683 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_set_in_synpred39_SimpleGLSLEffectFile687 = new BitSet(new long[]{0x0000000000000002L});

}