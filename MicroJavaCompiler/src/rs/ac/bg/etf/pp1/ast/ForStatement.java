// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ForStatement extends Statement {

    private ForStartMarker ForStartMarker;
    private ForInitPart ForInitPart;
    private ForCondPart ForCondPart;
    private ForStepPart ForStepPart;
    private ForStatementMarker ForStatementMarker;
    private Statement Statement;

    public ForStatement (ForStartMarker ForStartMarker, ForInitPart ForInitPart, ForCondPart ForCondPart, ForStepPart ForStepPart, ForStatementMarker ForStatementMarker, Statement Statement) {
        this.ForStartMarker=ForStartMarker;
        if(ForStartMarker!=null) ForStartMarker.setParent(this);
        this.ForInitPart=ForInitPart;
        if(ForInitPart!=null) ForInitPart.setParent(this);
        this.ForCondPart=ForCondPart;
        if(ForCondPart!=null) ForCondPart.setParent(this);
        this.ForStepPart=ForStepPart;
        if(ForStepPart!=null) ForStepPart.setParent(this);
        this.ForStatementMarker=ForStatementMarker;
        if(ForStatementMarker!=null) ForStatementMarker.setParent(this);
        this.Statement=Statement;
        if(Statement!=null) Statement.setParent(this);
    }

    public ForStartMarker getForStartMarker() {
        return ForStartMarker;
    }

    public void setForStartMarker(ForStartMarker ForStartMarker) {
        this.ForStartMarker=ForStartMarker;
    }

    public ForInitPart getForInitPart() {
        return ForInitPart;
    }

    public void setForInitPart(ForInitPart ForInitPart) {
        this.ForInitPart=ForInitPart;
    }

    public ForCondPart getForCondPart() {
        return ForCondPart;
    }

    public void setForCondPart(ForCondPart ForCondPart) {
        this.ForCondPart=ForCondPart;
    }

    public ForStepPart getForStepPart() {
        return ForStepPart;
    }

    public void setForStepPart(ForStepPart ForStepPart) {
        this.ForStepPart=ForStepPart;
    }

    public ForStatementMarker getForStatementMarker() {
        return ForStatementMarker;
    }

    public void setForStatementMarker(ForStatementMarker ForStatementMarker) {
        this.ForStatementMarker=ForStatementMarker;
    }

    public Statement getStatement() {
        return Statement;
    }

    public void setStatement(Statement Statement) {
        this.Statement=Statement;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(ForStartMarker!=null) ForStartMarker.accept(visitor);
        if(ForInitPart!=null) ForInitPart.accept(visitor);
        if(ForCondPart!=null) ForCondPart.accept(visitor);
        if(ForStepPart!=null) ForStepPart.accept(visitor);
        if(ForStatementMarker!=null) ForStatementMarker.accept(visitor);
        if(Statement!=null) Statement.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(ForStartMarker!=null) ForStartMarker.traverseTopDown(visitor);
        if(ForInitPart!=null) ForInitPart.traverseTopDown(visitor);
        if(ForCondPart!=null) ForCondPart.traverseTopDown(visitor);
        if(ForStepPart!=null) ForStepPart.traverseTopDown(visitor);
        if(ForStatementMarker!=null) ForStatementMarker.traverseTopDown(visitor);
        if(Statement!=null) Statement.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(ForStartMarker!=null) ForStartMarker.traverseBottomUp(visitor);
        if(ForInitPart!=null) ForInitPart.traverseBottomUp(visitor);
        if(ForCondPart!=null) ForCondPart.traverseBottomUp(visitor);
        if(ForStepPart!=null) ForStepPart.traverseBottomUp(visitor);
        if(ForStatementMarker!=null) ForStatementMarker.traverseBottomUp(visitor);
        if(Statement!=null) Statement.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ForStatement(\n");

        if(ForStartMarker!=null)
            buffer.append(ForStartMarker.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ForInitPart!=null)
            buffer.append(ForInitPart.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ForCondPart!=null)
            buffer.append(ForCondPart.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ForStepPart!=null)
            buffer.append(ForStepPart.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ForStatementMarker!=null)
            buffer.append(ForStatementMarker.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(Statement!=null)
            buffer.append(Statement.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ForStatement]");
        return buffer.toString();
    }
}
