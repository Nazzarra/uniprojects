// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ClassDecl implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    private ClassDeclHeader ClassDeclHeader;
    private ClassExtendsChoice ClassExtendsChoice;
    private ClassImplementsChoice ClassImplementsChoice;
    private VarDeclChoice VarDeclChoice;
    private MethDeclChoice MethDeclChoice;

    public ClassDecl (ClassDeclHeader ClassDeclHeader, ClassExtendsChoice ClassExtendsChoice, ClassImplementsChoice ClassImplementsChoice, VarDeclChoice VarDeclChoice, MethDeclChoice MethDeclChoice) {
        this.ClassDeclHeader=ClassDeclHeader;
        if(ClassDeclHeader!=null) ClassDeclHeader.setParent(this);
        this.ClassExtendsChoice=ClassExtendsChoice;
        if(ClassExtendsChoice!=null) ClassExtendsChoice.setParent(this);
        this.ClassImplementsChoice=ClassImplementsChoice;
        if(ClassImplementsChoice!=null) ClassImplementsChoice.setParent(this);
        this.VarDeclChoice=VarDeclChoice;
        if(VarDeclChoice!=null) VarDeclChoice.setParent(this);
        this.MethDeclChoice=MethDeclChoice;
        if(MethDeclChoice!=null) MethDeclChoice.setParent(this);
    }

    public ClassDeclHeader getClassDeclHeader() {
        return ClassDeclHeader;
    }

    public void setClassDeclHeader(ClassDeclHeader ClassDeclHeader) {
        this.ClassDeclHeader=ClassDeclHeader;
    }

    public ClassExtendsChoice getClassExtendsChoice() {
        return ClassExtendsChoice;
    }

    public void setClassExtendsChoice(ClassExtendsChoice ClassExtendsChoice) {
        this.ClassExtendsChoice=ClassExtendsChoice;
    }

    public ClassImplementsChoice getClassImplementsChoice() {
        return ClassImplementsChoice;
    }

    public void setClassImplementsChoice(ClassImplementsChoice ClassImplementsChoice) {
        this.ClassImplementsChoice=ClassImplementsChoice;
    }

    public VarDeclChoice getVarDeclChoice() {
        return VarDeclChoice;
    }

    public void setVarDeclChoice(VarDeclChoice VarDeclChoice) {
        this.VarDeclChoice=VarDeclChoice;
    }

    public MethDeclChoice getMethDeclChoice() {
        return MethDeclChoice;
    }

    public void setMethDeclChoice(MethDeclChoice MethDeclChoice) {
        this.MethDeclChoice=MethDeclChoice;
    }

    public SyntaxNode getParent() {
        return parent;
    }

    public void setParent(SyntaxNode parent) {
        this.parent=parent;
    }

    public int getLine() {
        return line;
    }

    public void setLine(int line) {
        this.line=line;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(ClassDeclHeader!=null) ClassDeclHeader.accept(visitor);
        if(ClassExtendsChoice!=null) ClassExtendsChoice.accept(visitor);
        if(ClassImplementsChoice!=null) ClassImplementsChoice.accept(visitor);
        if(VarDeclChoice!=null) VarDeclChoice.accept(visitor);
        if(MethDeclChoice!=null) MethDeclChoice.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(ClassDeclHeader!=null) ClassDeclHeader.traverseTopDown(visitor);
        if(ClassExtendsChoice!=null) ClassExtendsChoice.traverseTopDown(visitor);
        if(ClassImplementsChoice!=null) ClassImplementsChoice.traverseTopDown(visitor);
        if(VarDeclChoice!=null) VarDeclChoice.traverseTopDown(visitor);
        if(MethDeclChoice!=null) MethDeclChoice.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(ClassDeclHeader!=null) ClassDeclHeader.traverseBottomUp(visitor);
        if(ClassExtendsChoice!=null) ClassExtendsChoice.traverseBottomUp(visitor);
        if(ClassImplementsChoice!=null) ClassImplementsChoice.traverseBottomUp(visitor);
        if(VarDeclChoice!=null) VarDeclChoice.traverseBottomUp(visitor);
        if(MethDeclChoice!=null) MethDeclChoice.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ClassDecl(\n");

        if(ClassDeclHeader!=null)
            buffer.append(ClassDeclHeader.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ClassExtendsChoice!=null)
            buffer.append(ClassExtendsChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ClassImplementsChoice!=null)
            buffer.append(ClassImplementsChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(VarDeclChoice!=null)
            buffer.append(VarDeclChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(MethDeclChoice!=null)
            buffer.append(MethDeclChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ClassDecl]");
        return buffer.toString();
    }
}
