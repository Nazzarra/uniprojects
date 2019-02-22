// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ConstDeclBase extends ConstDeclList {

    private Type Type;
    private ConstDeclCore ConstDeclCore;
    private ConstDeclMore ConstDeclMore;

    public ConstDeclBase (Type Type, ConstDeclCore ConstDeclCore, ConstDeclMore ConstDeclMore) {
        this.Type=Type;
        if(Type!=null) Type.setParent(this);
        this.ConstDeclCore=ConstDeclCore;
        if(ConstDeclCore!=null) ConstDeclCore.setParent(this);
        this.ConstDeclMore=ConstDeclMore;
        if(ConstDeclMore!=null) ConstDeclMore.setParent(this);
    }

    public Type getType() {
        return Type;
    }

    public void setType(Type Type) {
        this.Type=Type;
    }

    public ConstDeclCore getConstDeclCore() {
        return ConstDeclCore;
    }

    public void setConstDeclCore(ConstDeclCore ConstDeclCore) {
        this.ConstDeclCore=ConstDeclCore;
    }

    public ConstDeclMore getConstDeclMore() {
        return ConstDeclMore;
    }

    public void setConstDeclMore(ConstDeclMore ConstDeclMore) {
        this.ConstDeclMore=ConstDeclMore;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(Type!=null) Type.accept(visitor);
        if(ConstDeclCore!=null) ConstDeclCore.accept(visitor);
        if(ConstDeclMore!=null) ConstDeclMore.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(Type!=null) Type.traverseTopDown(visitor);
        if(ConstDeclCore!=null) ConstDeclCore.traverseTopDown(visitor);
        if(ConstDeclMore!=null) ConstDeclMore.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(Type!=null) Type.traverseBottomUp(visitor);
        if(ConstDeclCore!=null) ConstDeclCore.traverseBottomUp(visitor);
        if(ConstDeclMore!=null) ConstDeclMore.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ConstDeclBase(\n");

        if(Type!=null)
            buffer.append(Type.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ConstDeclCore!=null)
            buffer.append(ConstDeclCore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ConstDeclMore!=null)
            buffer.append(ConstDeclMore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ConstDeclBase]");
        return buffer.toString();
    }
}
