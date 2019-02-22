// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class ProgramHeaderClassDecl extends ProgramHeaderDeclList {

    private ProgramHeaderDeclList ProgramHeaderDeclList;
    private ClassDecl ClassDecl;

    public ProgramHeaderClassDecl (ProgramHeaderDeclList ProgramHeaderDeclList, ClassDecl ClassDecl) {
        this.ProgramHeaderDeclList=ProgramHeaderDeclList;
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.setParent(this);
        this.ClassDecl=ClassDecl;
        if(ClassDecl!=null) ClassDecl.setParent(this);
    }

    public ProgramHeaderDeclList getProgramHeaderDeclList() {
        return ProgramHeaderDeclList;
    }

    public void setProgramHeaderDeclList(ProgramHeaderDeclList ProgramHeaderDeclList) {
        this.ProgramHeaderDeclList=ProgramHeaderDeclList;
    }

    public ClassDecl getClassDecl() {
        return ClassDecl;
    }

    public void setClassDecl(ClassDecl ClassDecl) {
        this.ClassDecl=ClassDecl;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.accept(visitor);
        if(ClassDecl!=null) ClassDecl.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.traverseTopDown(visitor);
        if(ClassDecl!=null) ClassDecl.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(ProgramHeaderDeclList!=null) ProgramHeaderDeclList.traverseBottomUp(visitor);
        if(ClassDecl!=null) ClassDecl.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("ProgramHeaderClassDecl(\n");

        if(ProgramHeaderDeclList!=null)
            buffer.append(ProgramHeaderDeclList.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(ClassDecl!=null)
            buffer.append(ClassDecl.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [ProgramHeaderClassDecl]");
        return buffer.toString();
    }
}
