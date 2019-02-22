// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class EnumDeclMultiple extends EnumDeclMore {

    private EnumDeclMore EnumDeclMore;
    private EnumDeclCore EnumDeclCore;

    public EnumDeclMultiple (EnumDeclMore EnumDeclMore, EnumDeclCore EnumDeclCore) {
        this.EnumDeclMore=EnumDeclMore;
        if(EnumDeclMore!=null) EnumDeclMore.setParent(this);
        this.EnumDeclCore=EnumDeclCore;
        if(EnumDeclCore!=null) EnumDeclCore.setParent(this);
    }

    public EnumDeclMore getEnumDeclMore() {
        return EnumDeclMore;
    }

    public void setEnumDeclMore(EnumDeclMore EnumDeclMore) {
        this.EnumDeclMore=EnumDeclMore;
    }

    public EnumDeclCore getEnumDeclCore() {
        return EnumDeclCore;
    }

    public void setEnumDeclCore(EnumDeclCore EnumDeclCore) {
        this.EnumDeclCore=EnumDeclCore;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(EnumDeclMore!=null) EnumDeclMore.accept(visitor);
        if(EnumDeclCore!=null) EnumDeclCore.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(EnumDeclMore!=null) EnumDeclMore.traverseTopDown(visitor);
        if(EnumDeclCore!=null) EnumDeclCore.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(EnumDeclMore!=null) EnumDeclMore.traverseBottomUp(visitor);
        if(EnumDeclCore!=null) EnumDeclCore.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("EnumDeclMultiple(\n");

        if(EnumDeclMore!=null)
            buffer.append(EnumDeclMore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(EnumDeclCore!=null)
            buffer.append(EnumDeclCore.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [EnumDeclMultiple]");
        return buffer.toString();
    }
}
