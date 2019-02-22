// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class InterfaceDecl implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    private InterfaceHeader InterfaceHeader;
    private InterfaceMethodList InterfaceMethodList;

    public InterfaceDecl (InterfaceHeader InterfaceHeader, InterfaceMethodList InterfaceMethodList) {
        this.InterfaceHeader=InterfaceHeader;
        if(InterfaceHeader!=null) InterfaceHeader.setParent(this);
        this.InterfaceMethodList=InterfaceMethodList;
        if(InterfaceMethodList!=null) InterfaceMethodList.setParent(this);
    }

    public InterfaceHeader getInterfaceHeader() {
        return InterfaceHeader;
    }

    public void setInterfaceHeader(InterfaceHeader InterfaceHeader) {
        this.InterfaceHeader=InterfaceHeader;
    }

    public InterfaceMethodList getInterfaceMethodList() {
        return InterfaceMethodList;
    }

    public void setInterfaceMethodList(InterfaceMethodList InterfaceMethodList) {
        this.InterfaceMethodList=InterfaceMethodList;
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
        if(InterfaceHeader!=null) InterfaceHeader.accept(visitor);
        if(InterfaceMethodList!=null) InterfaceMethodList.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(InterfaceHeader!=null) InterfaceHeader.traverseTopDown(visitor);
        if(InterfaceMethodList!=null) InterfaceMethodList.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(InterfaceHeader!=null) InterfaceHeader.traverseBottomUp(visitor);
        if(InterfaceMethodList!=null) InterfaceMethodList.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("InterfaceDecl(\n");

        if(InterfaceHeader!=null)
            buffer.append(InterfaceHeader.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(InterfaceMethodList!=null)
            buffer.append(InterfaceMethodList.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [InterfaceDecl]");
        return buffer.toString();
    }
}
