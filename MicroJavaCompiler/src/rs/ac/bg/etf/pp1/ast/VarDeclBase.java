// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class VarDeclBase extends VarDeclList {

    private Type Type;
    private VarDeclCore VarDeclCore;
    private VarDeclMore VarDeclMore;

    public VarDeclBase (Type Type, VarDeclCore VarDeclCore, VarDeclMore VarDeclMore) {
        this.Type=Type;
        if(Type!=null) Type.setParent(this);
        this.VarDeclCore=VarDeclCore;
        if(VarDeclCore!=null) VarDeclCore.setParent(this);
        this.VarDeclMore=VarDeclMore;
        if(VarDeclMore!=null) VarDeclMore.setParent(this);
    }

    public Type getType() {
        return Type;
    }

    public void setType(Type Type) {
        this.Type=Type;
    }

    public VarDeclCore getVarDeclCore() {
        return VarDeclCore;
    }

    public void setVarDeclCore(VarDeclCore VarDeclCore) {
        this.VarDeclCore=VarDeclCore;
    }

    public VarDeclMore getVarDeclMore() {
        return VarDeclMore;
    }

    public void setVarDeclMore(VarDeclMore VarDeclMore) {
        this.VarDeclMore=VarDeclMore;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(Type!=null) Type.accept(visitor);
        if(VarDeclCore!=null) VarDeclCore.accept(visitor);
        if(VarDeclMore!=null) VarDeclMore.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(Type!=null) Type.traverseTopDown(visitor);
        if(VarDeclCore!=null) VarDeclCore.traverseTopDown(visitor);
        if(VarDeclMore!=null) VarDeclMore.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(Type!=null) Type.traverseBottomUp(visitor);
        if(VarDeclCore!=null) VarDeclCore.traverseBottomUp(visitor);
        if(VarDeclMore!=null) VarDeclMore.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("VarDeclBase(\n");

        if(Type!=null)
            buffer.append(Type.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(VarDeclCore!=null)
            buffer.append(VarDeclCore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(VarDeclMore!=null)
            buffer.append(VarDeclMore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [VarDeclBase]");
        return buffer.toString();
    }
}
