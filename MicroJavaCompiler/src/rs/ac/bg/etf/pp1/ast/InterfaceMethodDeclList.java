// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class InterfaceMethodDeclList extends InterfaceMethodList {

    private InterfaceMethodList InterfaceMethodList;
    private InterfaceMethodDecl InterfaceMethodDecl;

    public InterfaceMethodDeclList (InterfaceMethodList InterfaceMethodList, InterfaceMethodDecl InterfaceMethodDecl) {
        this.InterfaceMethodList=InterfaceMethodList;
        if(InterfaceMethodList!=null) InterfaceMethodList.setParent(this);
        this.InterfaceMethodDecl=InterfaceMethodDecl;
        if(InterfaceMethodDecl!=null) InterfaceMethodDecl.setParent(this);
    }

    public InterfaceMethodList getInterfaceMethodList() {
        return InterfaceMethodList;
    }

    public void setInterfaceMethodList(InterfaceMethodList InterfaceMethodList) {
        this.InterfaceMethodList=InterfaceMethodList;
    }

    public InterfaceMethodDecl getInterfaceMethodDecl() {
        return InterfaceMethodDecl;
    }

    public void setInterfaceMethodDecl(InterfaceMethodDecl InterfaceMethodDecl) {
        this.InterfaceMethodDecl=InterfaceMethodDecl;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(InterfaceMethodList!=null) InterfaceMethodList.accept(visitor);
        if(InterfaceMethodDecl!=null) InterfaceMethodDecl.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(InterfaceMethodList!=null) InterfaceMethodList.traverseTopDown(visitor);
        if(InterfaceMethodDecl!=null) InterfaceMethodDecl.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(InterfaceMethodList!=null) InterfaceMethodList.traverseBottomUp(visitor);
        if(InterfaceMethodDecl!=null) InterfaceMethodDecl.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("InterfaceMethodDeclList(\n");

        if(InterfaceMethodList!=null)
            buffer.append(InterfaceMethodList.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(InterfaceMethodDecl!=null)
            buffer.append(InterfaceMethodDecl.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [InterfaceMethodDeclList]");
        return buffer.toString();
    }
}
