package rs.ac.bg.etf.pp1;

import java_cup.runtime.Symbol;

import java.io.*;
import java.lang.reflect.Field;

public class LexerTest {

    private static String symbolTypeToSymbolName(int type) {
        Field[] fields = sym.class.getFields();
        String ret = "UNKNOWN";
        for(Field f : fields){
            try {
                if(f.getInt(null) == type){
                    ret = f.getName();
                    break;
                }
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }

        return ret;
    }

    public static void main(String[] args) throws IOException {
        if(args.length == 0){
            System.out.println("Usage: LexerTest <input file>");
            return;
        }

        System.out.println("Opening file " + args[0]);

        File inputFile = new File(args[0]);
        if(!inputFile.exists()){
            System.out.println("Input file does not exist");
            return;
        }
        System.out.println("Opened input file " + inputFile.getAbsolutePath());

        Yylex lexer = new Yylex(new InputStreamReader(new FileInputStream(inputFile)));

        while(true){
            Symbol symbol = lexer.next_token();
            System.out.println("Sym " + symbolTypeToSymbolName(symbol.sym) + " Value: `" + symbol.value + "`");

            if(symbol.sym == sym.EOF)
                break;
        }

        System.out.println("Finished parsing input file.");
    }

}
