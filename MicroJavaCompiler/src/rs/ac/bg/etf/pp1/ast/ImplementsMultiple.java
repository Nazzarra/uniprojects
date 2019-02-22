// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ImplementsMultiple extends ClassImplementsMultiple {

    private ClassImplementsMultiple ClassImplementsMultiple;
    private Type Type;

    public ImplementsMultiple (ClassImplementsMultiple ClassImplementsMultiple, Type Type) {
        this.ClassImplementsMultiple=ClassImplementsMultiple;
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.setParent(this);
        this.Type=Type;
        if(Type!=null) Type.setParent(this);
    }

    public ClassImplementsMultiple getClassImplementsMultiple() {
        return ClassImplementsMultiple;
    }

    public void setClassImplementsMultiple(ClassImplementsMultiple ClassImplementsMultiple) {
        this.ClassImplementsMultiple=ClassImplementsMultiple;
    }

    public Type getType() {
        return Type;
    }

    public void setType(Type Type) {
        this.Type=Type;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.accept(visitor);
        if(Type!=null) Type.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.traverseTopDown(visitor);
        if(Type!=null) Type.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.traverseBottomUp(visitor);
        if(Type!=null) Type.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ImplementsMultiple(\n");

        if(ClassImplementsMultiple!=null)
            buffer.append(ClassImplementsMultiple.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(Type!=null)
            buffer.append(Type.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ImplementsMultiple]");
        return buffer.toString();
    }
}
