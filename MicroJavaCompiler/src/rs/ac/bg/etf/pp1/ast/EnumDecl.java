// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class EnumDecl implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    private EnumTypeDecl EnumTypeDecl;
    private EnumDeclCore EnumDeclCore;
    private EnumDeclMore EnumDeclMore;

    public EnumDecl (EnumTypeDecl EnumTypeDecl, EnumDeclCore EnumDeclCore, EnumDeclMore EnumDeclMore) {
        this.EnumTypeDecl=EnumTypeDecl;
        if(EnumTypeDecl!=null) EnumTypeDecl.setParent(this);
        this.EnumDeclCore=EnumDeclCore;
        if(EnumDeclCore!=null) EnumDeclCore.setParent(this);
        this.EnumDeclMore=EnumDeclMore;
        if(EnumDeclMore!=null) EnumDeclMore.setParent(this);
    }

    public EnumTypeDecl getEnumTypeDecl() {
        return EnumTypeDecl;
    }

    public void setEnumTypeDecl(EnumTypeDecl EnumTypeDecl) {
        this.EnumTypeDecl=EnumTypeDecl;
    }

    public EnumDeclCore getEnumDeclCore() {
        return EnumDeclCore;
    }

    public void setEnumDeclCore(EnumDeclCore EnumDeclCore) {
        this.EnumDeclCore=EnumDeclCore;
    }

    public EnumDeclMore getEnumDeclMore() {
        return EnumDeclMore;
    }

    public void setEnumDeclMore(EnumDeclMore EnumDeclMore) {
        this.EnumDeclMore=EnumDeclMore;
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
        if(EnumTypeDecl!=null) EnumTypeDecl.accept(visitor);
        if(EnumDeclCore!=null) EnumDeclCore.accept(visitor);
        if(EnumDeclMore!=null) EnumDeclMore.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(EnumTypeDecl!=null) EnumTypeDecl.traverseTopDown(visitor);
        if(EnumDeclCore!=null) EnumDeclCore.traverseTopDown(visitor);
        if(EnumDeclMore!=null) EnumDeclMore.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(EnumTypeDecl!=null) EnumTypeDecl.traverseBottomUp(visitor);
        if(EnumDeclCore!=null) EnumDeclCore.traverseBottomUp(visitor);
        if(EnumDeclMore!=null) EnumDeclMore.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("EnumDecl(\n");

        if(EnumTypeDecl!=null)
            buffer.append(EnumTypeDecl.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(EnumDeclCore!=null)
            buffer.append(EnumDeclCore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(EnumDeclMore!=null)
            buffer.append(EnumDeclMore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [EnumDecl]");
        return buffer.toString();
    }
}
