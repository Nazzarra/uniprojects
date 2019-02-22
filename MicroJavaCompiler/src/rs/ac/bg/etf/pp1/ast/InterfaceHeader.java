// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class InterfaceHeader implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    private String interfaceName;

    public InterfaceHeader (String interfaceName) {
        this.interfaceName=interfaceName;
    }

    public String getInterfaceName() {
        return interfaceName;
    }

    public void setInterfaceName(String interfaceName) {
        this.interfaceName=interfaceName;
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
        buffer.append("InterfaceHeader(\n");

        buffer.append(" "+tab+interfaceName);
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [InterfaceHeader]");
        return buffer.toString();
    }
}
