// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ConstDeclMultiple extends ConstDeclMore {

    private ConstDeclMore ConstDeclMore;
    private ConstDeclCore ConstDeclCore;

    public ConstDeclMultiple (ConstDeclMore ConstDeclMore, ConstDeclCore ConstDeclCore) {
        this.ConstDeclMore=ConstDeclMore;
        if(ConstDeclMore!=null) ConstDeclMore.setParent(this);
        this.ConstDeclCore=ConstDeclCore;
        if(ConstDeclCore!=null) ConstDeclCore.setParent(this);
    }

    public ConstDeclMore getConstDeclMore() {
        return ConstDeclMore;
    }

    public void setConstDeclMore(ConstDeclMore ConstDeclMore) {
        this.ConstDeclMore=ConstDeclMore;
    }

    public ConstDeclCore getConstDeclCore() {
        return ConstDeclCore;
    }

    public void setConstDeclCore(ConstDeclCore ConstDeclCore) {
        this.ConstDeclCore=ConstDeclCore;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(ConstDeclMore!=null) ConstDeclMore.accept(visitor);
        if(ConstDeclCore!=null) ConstDeclCore.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(ConstDeclMore!=null) ConstDeclMore.traverseTopDown(visitor);
        if(ConstDeclCore!=null) ConstDeclCore.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(ConstDeclMore!=null) ConstDeclMore.traverseBottomUp(visitor);
        if(ConstDeclCore!=null) ConstDeclCore.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ConstDeclMultiple(\n");

        if(ConstDeclMore!=null)
            buffer.append(ConstDeclMore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ConstDeclCore!=null)
            buffer.append(ConstDeclCore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ConstDeclMultiple]");
        return buffer.toString();
    }
}
