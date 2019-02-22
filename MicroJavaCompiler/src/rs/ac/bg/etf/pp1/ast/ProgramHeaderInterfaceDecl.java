// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ProgramHeaderInterfaceDecl extends ProgramHeaderDeclList {

    private ProgramHeaderDeclList ProgramHeaderDeclList;
    private InterfaceDecl InterfaceDecl;

    public ProgramHeaderInterfaceDecl (ProgramHeaderDeclList ProgramHeaderDeclList, InterfaceDecl InterfaceDecl) {
        this.ProgramHeaderDeclList=ProgramHeaderDeclList;
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.setParent(this);
        this.InterfaceDecl=InterfaceDecl;
        if(InterfaceDecl!=null) InterfaceDecl.setParent(this);
    }

    public ProgramHeaderDeclList getProgramHeaderDeclList() {
        return ProgramHeaderDeclList;
    }

    public void setProgramHeaderDeclList(ProgramHeaderDeclList ProgramHeaderDeclList) {
        this.ProgramHeaderDeclList=ProgramHeaderDeclList;
    }

    public InterfaceDecl getInterfaceDecl() {
        return InterfaceDecl;
    }

    public void setInterfaceDecl(InterfaceDecl InterfaceDecl) {
        this.InterfaceDecl=InterfaceDecl;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.accept(visitor);
        if(InterfaceDecl!=null) InterfaceDecl.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.traverseTopDown(visitor);
        if(InterfaceDecl!=null) InterfaceDecl.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.traverseBottomUp(visitor);
        if(InterfaceDecl!=null) InterfaceDecl.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ProgramHeaderInterfaceDecl(\n");

        if(ProgramHeaderDeclList!=null)
            buffer.append(ProgramHeaderDeclList.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(InterfaceDecl!=null)
            buffer.append(InterfaceDecl.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ProgramHeaderInterfaceDecl]");
        return buffer.toString();
    }
}
