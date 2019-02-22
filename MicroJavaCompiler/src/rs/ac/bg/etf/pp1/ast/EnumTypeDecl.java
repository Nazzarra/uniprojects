// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class EnumTypeDecl implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    public rs.etf.pp1.symboltable.concepts.Obj obj = null;

    private String enumTypeName;

    public EnumTypeDecl (String enumTypeName) {
        this.enumTypeName=enumTypeName;
    }

    public String getEnumTypeName() {
        return enumTypeName;
    }

    public void setEnumTypeName(String enumTypeName) {
        this.enumTypeName=enumTypeName;
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
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("EnumTypeDecl(\n");

        buffer.append(" "+tab+enumTypeName);
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [EnumTypeDecl]");
        return buffer.toString();
    }
}
