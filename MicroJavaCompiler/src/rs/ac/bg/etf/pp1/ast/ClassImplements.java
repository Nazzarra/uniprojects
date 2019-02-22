// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ClassImplements extends ClassImplementsChoice {

    private Type Type;
    private ClassImplementsMultiple ClassImplementsMultiple;

    public ClassImplements (Type Type, ClassImplementsMultiple ClassImplementsMultiple) {
        this.Type=Type;
        if(Type!=null) Type.setParent(this);
        this.ClassImplementsMultiple=ClassImplementsMultiple;
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.setParent(this);
    }

    public Type getType() {
        return Type;
    }

    public void setType(Type Type) {
        this.Type=Type;
    }

    public ClassImplementsMultiple getClassImplementsMultiple() {
        return ClassImplementsMultiple;
    }

    public void setClassImplementsMultiple(ClassImplementsMultiple ClassImplementsMultiple) {
        this.ClassImplementsMultiple=ClassImplementsMultiple;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(Type!=null) Type.accept(visitor);
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(Type!=null) Type.traverseTopDown(visitor);
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(Type!=null) Type.traverseBottomUp(visitor);
        if(ClassImplementsMultiple!=null) ClassImplementsMultiple.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ClassImplements(\n");

        if(Type!=null)
            buffer.append(Type.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ClassImplementsMultiple!=null)
            buffer.append(ClassImplementsMultiple.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ClassImplements]");
        return buffer.toString();
    }
}
