package rs.ac.bg.etf.pp1;

import java_cup.runtime.Symbol;
import rs.ac.bg.etf.pp1.ast.Program;
import rs.etf.pp1.mj.runtime.Code;
import rs.etf.pp1.symboltable.Tab;

import java.io.*;

public class Compiler {

    public static void main(String[] args){
        if(args.length != 2){
            System.out.println("Usage: <input_file_name> <output_file_name>");
            return;
        }

        File inputFile = new File(args[0]);
        if(!inputFile.exists()){
            System.out.println("File " + inputFile.getAbsolutePath() + " does not exist!");
            return;
        }

        Yylex lexer;
        try {
            lexer = new Yylex(new InputStreamReader(new FileInputStream(inputFile)));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            System.out.println("You probably do not have the required permission to open the file: " + inputFile.getAbsolutePath());
            return;
        }

        MJParser parser = new MJParser(lexer);

        Symbol root;
        try{
            root = parser.parse();
        }
        catch (Exception e){
            e.printStackTrace();
            return;
        }
        if(parser.errorWhileParsing){
            System.out.println("Parsing aborted!");
            if(root != null){
                Program prog = null;
                if(root.value instanceof Program)
                    prog = (Program)root.value;
                else {
                    return;
                }
                //System.out.println(prog.toString(""));
            }
            return;
        }
        Program prog = (Program)root.value;
        //System.out.println(prog.toString(""));
        System.out.println("Parsing successful!");

        System.out.println("");
        SemanticAnalyzer analyzer = new SemanticAnalyzer(prog, true);
        analyzer.analyze();

        //Tab.dump();
        if(analyzer.isProgramSemanticsOk()){
            System.out.println("Semantic analysis ok!");
            System.out.println("Compiling...");

            File outputFile = new File(args[1]);
            if(outputFile.exists()){
                System.out.println("Output file already exists, deleting");
                outputFile.delete();
            }

            FileOutputStream outputStream;
            try {
                outputStream = new FileOutputStream(outputFile);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                System.out.println("You probably do not have permission to make a file there");
                System.out.println("Compiling failed.");
                return;
            }

            CodeGenerator codeGenerator = new CodeGenerator(prog, false);
            codeGenerator.generateCode();
            Code.write(outputStream);
            System.out.println("Compiling finished!");
        }
        else{
            System.out.println("Semantic analysis encountered errors!");
        }
    }

}
