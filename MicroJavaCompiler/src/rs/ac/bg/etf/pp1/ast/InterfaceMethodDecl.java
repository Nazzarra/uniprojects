// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class InterfaceMethodDecl implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    private MethodType MethodType;
    private InterfaceMethodName InterfaceMethodName;
    private FormalParamsChoice FormalParamsChoice;

    public InterfaceMethodDecl (MethodType MethodType, InterfaceMethodName InterfaceMethodName, FormalParamsChoice FormalParamsChoice) {
        this.MethodType=MethodType;
        if(MethodType!=null) MethodType.setParent(this);
        this.InterfaceMethodName=InterfaceMethodName;
        if(InterfaceMethodName!=null) InterfaceMethodName.setParent(this);
        this.FormalParamsChoice=FormalParamsChoice;
        if(FormalParamsChoice!=null) FormalParamsChoice.setParent(this);
    }

    public MethodType getMethodType() {
        return MethodType;
    }

    public void setMethodType(MethodType MethodType) {
        this.MethodType=MethodType;
    }

    public InterfaceMethodName getInterfaceMethodName() {
        return InterfaceMethodName;
    }

    public void setInterfaceMethodName(InterfaceMethodName InterfaceMethodName) {
        this.InterfaceMethodName=InterfaceMethodName;
    }

    public FormalParamsChoice getFormalParamsChoice() {
        return FormalParamsChoice;
    }

    public void setFormalParamsChoice(FormalParamsChoice FormalParamsChoice) {
        this.FormalParamsChoice=FormalParamsChoice;
    }

    public SyntaxNode getParent() {
        return parent;
    }

    public void setParent(SyntaxNode parent) {
        this.parent=parent;
    }

    public int getLine() {
        return line;
    }

    public void setLine(int line) {
        this.line=line;
    }

    public void accept(Visitor visitor) {
        visitor.visit(this);
    }

    public void childrenAccept(Visitor visitor) {
        if(MethodType!=null) MethodType.accept(visitor);
        if(InterfaceMethodName!=null) InterfaceMethodName.accept(visitor);
        if(FormalParamsChoice!=null) FormalParamsChoice.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(MethodType!=null) MethodType.traverseTopDown(visitor);
        if(InterfaceMethodName!=null) InterfaceMethodName.traverseTopDown(visitor);
        if(FormalParamsChoice!=null) FormalParamsChoice.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(MethodType!=null) MethodType.traverseBottomUp(visitor);
        if(InterfaceMethodName!=null) InterfaceMethodName.traverseBottomUp(visitor);
        if(FormalParamsChoice!=null) FormalParamsChoice.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("InterfaceMethodDecl(\n");

        if(MethodType!=null)
            buffer.append(MethodType.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(InterfaceMethodName!=null)
            buffer.append(InterfaceMethodName.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(FormalParamsChoice!=null)
            buffer.append(FormalParamsChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [InterfaceMethodDecl]");
        return buffer.toString();
    }
}
