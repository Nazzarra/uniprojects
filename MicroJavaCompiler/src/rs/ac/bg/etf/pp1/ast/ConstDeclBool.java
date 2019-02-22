// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ConstDeclBool extends ConstDeclCore {

    private String constIdentifier;
    private Boolean constVal;

    public ConstDeclBool (String constIdentifier, Boolean constVal) {
        this.constIdentifier=constIdentifier;
        this.constVal=constVal;
    }

    public String getConstIdentifier() {
        return constIdentifier;
    }

    public void setConstIdentifier(String constIdentifier) {
        this.constIdentifier=constIdentifier;
    }

    public Boolean getConstVal() {
        return constVal;
    }

    public void setConstVal(Boolean constVal) {
        this.constVal=constVal;
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
        buffer.append("ConstDeclBool(\n");

        buffer.append(" "+tab+constIdentifier);
        buffer.append("\n");

        buffer.append(" "+tab+constVal);
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ConstDeclBool]");
        return buffer.toString();
    }
}
