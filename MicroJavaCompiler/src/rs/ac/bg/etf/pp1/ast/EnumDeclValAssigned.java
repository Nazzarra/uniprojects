// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class EnumDeclValAssigned extends EnumDeclCore {

    private String enumVal;
    private Integer assignedVal;

    public EnumDeclValAssigned (String enumVal, Integer assignedVal) {
        this.enumVal=enumVal;
        this.assignedVal=assignedVal;
    }

    public String getEnumVal() {
        return enumVal;
    }

    public void setEnumVal(String enumVal) {
        this.enumVal=enumVal;
    }

    public Integer getAssignedVal() {
        return assignedVal;
    }

    public void setAssignedVal(Integer assignedVal) {
        this.assignedVal=assignedVal;
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
        buffer.append("EnumDeclValAssigned(\n");

        buffer.append(" "+tab+enumVal);
        buffer.append("\n");

        buffer.append(" "+tab+assignedVal);
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [EnumDeclValAssigned]");
        return buffer.toString();
    }
}
