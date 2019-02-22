// generated with ast extension for cup
// version 0.8
// 3/1/2019 17:32:21


package rs.ac.bg.etf.pp1.ast;

public class MethodDecl implements SyntaxNode {

    private SyntaxNode parent;
    private int line;
    private MethodType MethodType;
    private MethodName MethodName;
    private FormalParamsChoice FormalParamsChoice;
    private VarDeclChoice VarDeclChoice;
    private StatementList StatementList;

    public MethodDecl (MethodType MethodType, MethodName MethodName, FormalParamsChoice FormalParamsChoice, VarDeclChoice VarDeclChoice, StatementList StatementList) {
        this.MethodType=MethodType;
        if(MethodType!=null) MethodType.setParent(this);
        this.MethodName=MethodName;
        if(MethodName!=null) MethodName.setParent(this);
        this.FormalParamsChoice=FormalParamsChoice;
        if(FormalParamsChoice!=null) FormalParamsChoice.setParent(this);
        this.VarDeclChoice=VarDeclChoice;
        if(VarDeclChoice!=null) VarDeclChoice.setParent(this);
        this.StatementList=StatementList;
        if(StatementList!=null) StatementList.setParent(this);
    }

    public MethodType getMethodType() {
        return MethodType;
    }

    public void setMethodType(MethodType MethodType) {
        this.MethodType=MethodType;
    }

    public MethodName getMethodName() {
        return MethodName;
    }

    public void setMethodName(MethodName MethodName) {
        this.MethodName=MethodName;
    }

    public FormalParamsChoice getFormalParamsChoice() {
        return FormalParamsChoice;
    }

    public void setFormalParamsChoice(FormalParamsChoice FormalParamsChoice) {
        this.FormalParamsChoice=FormalParamsChoice;
    }

    public VarDeclChoice getVarDeclChoice() {
        return VarDeclChoice;
    }

    public void setVarDeclChoice(VarDeclChoice VarDeclChoice) {
        this.VarDeclChoice=VarDeclChoice;
    }

    public StatementList getStatementList() {
        return StatementList;
    }

    public void setStatementList(StatementList StatementList) {
        this.StatementList=StatementList;
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
        if(MethodName!=null) MethodName.accept(visitor);
        if(FormalParamsChoice!=null) FormalParamsChoice.accept(visitor);
        if(VarDeclChoice!=null) VarDeclChoice.accept(visitor);
        if(StatementList!=null) StatementList.accept(visitor);
    }

    public void traverseTopDown(Visitor visitor) {
        accept(visitor);
        if(MethodType!=null) MethodType.traverseTopDown(visitor);
        if(MethodName!=null) MethodName.traverseTopDown(visitor);
        if(FormalParamsChoice!=null) FormalParamsChoice.traverseTopDown(visitor);
        if(VarDeclChoice!=null) VarDeclChoice.traverseTopDown(visitor);
        if(StatementList!=null) StatementList.traverseTopDown(visitor);
    }

    public void traverseBottomUp(Visitor visitor) {
        if(MethodType!=null) MethodType.traverseBottomUp(visitor);
        if(MethodName!=null) MethodName.traverseBottomUp(visitor);
        if(FormalParamsChoice!=null) FormalParamsChoice.traverseBottomUp(visitor);
        if(VarDeclChoice!=null) VarDeclChoice.traverseBottomUp(visitor);
        if(StatementList!=null) StatementList.traverseBottomUp(visitor);
        accept(visitor);
    }

    public String toString(String tab) {
        StringBuffer buffer=new StringBuffer();
        buffer.append(tab);
        buffer.append("MethodDecl(\n");

        if(MethodType!=null)
            buffer.append(MethodType.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(MethodName!=null)
            buffer.append(MethodName.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(FormalParamsChoice!=null)
            buffer.append(FormalParamsChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(VarDeclChoice!=null)
            buffer.append(VarDeclChoice.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        if(StatementList!=null)
            buffer.append(StatementList.toString("  "+tab));
        else
            buffer.append(tab+"  null");
        buffer.append("\n");

        buffer.append(tab);
        buffer.append(") [MethodDecl]");
        return buffer.toString();
    }
}
