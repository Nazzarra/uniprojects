// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class IfElseStatement extends CondStatement {

    private IfStartMarker IfStartMarker;
    private Statement Statement;
    private IfElseMarker IfElseMarker;
    private Statement Statement1;

    public IfElseStatement (IfStartMarker IfStartMarker, Statement Statement, IfElseMarker IfElseMarker, Statement Statement1) {
        this.IfStartMarker=IfStartMarker;
        if(IfStartMarker!=null) IfStartMarker.setParent(this);
        this.Statement=Statement;
        if(Statement!=null) Statement.setParent(this);
        this.IfElseMarker=IfElseMarker;
        if(IfElseMarker!=null) IfElseMarker.setParent(this);
        this.Statement1=Statement1;
        if(Statement1!=null) Statement1.setParent(this);
    }

    public IfStartMarker getIfStartMarker() {
        return IfStartMarker;
    }

    public void setIfStartMarker(IfStartMarker IfStartMarker) {
        this.IfStartMarker=IfStartMarker;
    }

    public Statement getStatement() {
        return Statement;
    }

    public void setStatement(Statement Statement) {
        this.Statement=Statement;
    }

    public IfElseMarker getIfElseMarker() {
        return IfElseMarker;
    }

    public void setIfElseMarker(IfElseMarker IfElseMarker) {
        this.IfElseMarker=IfElseMarker;
    }

    public Statement getStatement1() {
        return Statement1;
    }

    public void setStatement1(Statement Statement1) {
        this.Statement1=Statement1;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(IfStartMarker!=null) IfStartMarker.accept(visitor);
        if(Statement!=null) Statement.accept(visitor);
        if(IfElseMarker!=null) IfElseMarker.accept(visitor);
        if(Statement1!=null) Statement1.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(IfStartMarker!=null) IfStartMarker.traverseTopDown(visitor);
        if(Statement!=null) Statement.traverseTopDown(visitor);
        if(IfElseMarker!=null) IfElseMarker.traverseTopDown(visitor);
        if(Statement1!=null) Statement1.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(IfStartMarker!=null) IfStartMarker.traverseBottomUp(visitor);
        if(Statement!=null) Statement.traverseBottomUp(visitor);
        if(IfElseMarker!=null) IfElseMarker.traverseBottomUp(visitor);
        if(Statement1!=null) Statement1.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("IfElseStatement(\n");

        if(IfStartMarker!=null)
            buffer.append(IfStartMarker.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(Statement!=null)
            buffer.append(Statement.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(IfElseMarker!=null)
            buffer.append(IfElseMarker.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(Statement1!=null)
            buffer.append(Statement1.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [IfElseStatement]");
        return buffer.toString();
    }
}
