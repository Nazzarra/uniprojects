package rs.ac.bg.etf.pp1;

import rs.ac.bg.etf.pp1.ast.*;
import rs.etf.pp1.mj.runtime.Code;
import rs.etf.pp1.symboltable.Tab;
import rs.etf.pp1.symboltable.concepts.Obj;
import rs.etf.pp1.symboltable.concepts.Struct;

import java.util.*;

public class CodeGenerator extends VisitorAdaptor {

    private Program root;
    private Obj currentClass;
    private boolean shouldReportInfo;

    private Stack<ArrayList<Integer>> breakFixAddr = new Stack<>();
    private Stack<ArrayList<Integer>> contiinueFixAddr = new Stack<>();

    private Map<Struct, Integer> vtableStartAddr = new HashMap<>();
    private Map<Struct, ArrayList<Integer>> vtableFixAddr = new HashMap<>();

    public void reportInfo(String message, SyntaxNode node){
        if(shouldReportInfo)
            System.out.println("(Line: " + node.getLine() + ") INFO: " + message);
    }

    private void fixVirtualCalls(){
        int currentFreeLocation = getProgramVarCount();
        int startFreeLocation = currentFreeLocation;
        Obj rootObj = Tab.find(root.getProgramName().getProgName());
        for(Obj obj : rootObj.getLocalSymbols()){
            if(obj.getKind() == Obj.Type){
                Struct type = obj.getType();
                if(type.getKind() == Struct.Class && type != Tab.noType){
                    Map<String, Integer> methMap = new HashMap<>();
                    System.out.println("Generate VTable for class " + obj.getName() + " Start address: " + currentFreeLocation);
                    vtableStartAddr.put(type, currentFreeLocation);
                    while(type != Tab.noType){
                        for(Obj localMeth : type.getMembers()){
                            if(localMeth.getKind() == Obj.Meth){
                                System.out.println("--- Method " + localMeth.getName() + " addr: " + currentFreeLocation);
                                String name = localMeth.getName();
                                int pc = localMeth.getAdr();
                                if(methMap.get(localMeth.getName()) != null)
                                    continue;

                                methMap.put(localMeth.getName(), 1);
                                for(int i = 0 ; i < name.length(); ++i){
                                    Code.loadConst(name.charAt(i));
                                    Code.put(Code.putstatic);
                                    Code.put2(currentFreeLocation);
                                    ++currentFreeLocation;
                                }
                                Code.loadConst(-1);
                                Code.put(Code.putstatic);
                                Code.put2(currentFreeLocation);
                                ++currentFreeLocation;

                                System.out.println("------ PC: " + pc);
                                Code.loadConst(pc);
                                Code.put(Code.putstatic);
                                Code.put2(currentFreeLocation);
                                ++currentFreeLocation;
                            }
                        }
                        type = type.getElemType();
                    }

                    Code.loadConst(-2);
                    Code.put(Code.putstatic);
                    Code.put2(currentFreeLocation);
                    ++currentFreeLocation;
                }
            }
        }
        Code.dataSize = getProgramVarCount() + currentFreeLocation - startFreeLocation;
    }

    private void patchVtableCallsForClass(Struct struct, int startAddress){
        ArrayList<Integer> addresses = vtableFixAddr.get(struct);
        for(Integer addr : addresses){
            Code.put2(addr, startAddress >> 16);
            Code.put2(addr + 2, startAddress & 0xFFFF);
        }
    }

    private void patchVtableCalls(){
        for(Map.Entry<Struct, Integer> entry : vtableStartAddr.entrySet())
            patchVtableCallsForClass(entry.getKey(), entry.getValue());
    }

    public CodeGenerator(Program root, boolean shouldReportInfo){
        this.shouldReportInfo = shouldReportInfo;
        this.root = root;
    }

    public void generateCode(){
        currentClass = Tab.noObj;
        root.traverseBottomUp(this);
        patchVtableCalls();
    }

    private int getProgramVarCount(){
        int vars = 0;
        Obj progObj =  Tab.find(root.getProgramName().getProgName());
        for(Obj obj : progObj.getLocalSymbols())
            if(obj.getKind() == Obj.Var)
                ++vars;

        return vars;
    }

    //todo help me
    public void visit(ClassDeclHeader classDeclHeader){
        Obj program = Tab.find(root.getProgramName().getProgName());
        Obj classType = Tab.noObj;
        for(Obj o : program.getLocalSymbols())
            if(o.getName().equals(classDeclHeader.getClassName())){
                classType = o;
                break;
            }
        Struct type = classType.getType();
        vtableFixAddr.put(type, new ArrayList<>());
    }

    public void visit(MethodName methodName){
        if(currentClass ==Tab.noObj && methodName.getMethodName().equals("main")) {
            Code.mainPc = Code.pc;
            reportInfo("Found main method", methodName);
        }

        Obj methodObj = methodName.obj;
        methodObj.setAdr(Code.pc);
        int formalParamCount = methodObj.getLevel();
        int totalVarCount = methodObj.getLocalSymbols().size();
        Code.put(Code.enter);
        Code.put(formalParamCount);
        Code.put(totalVarCount);
        if(currentClass ==Tab.noObj && methodName.getMethodName().equals("main")) {
            fixVirtualCalls();
        }
    }

    public void visit(MethodDecl methodDecl){
        Code.put(Code.exit);
        if(methodDecl.getMethodType() instanceof MethodTypeVoid)
            Code.put(Code.return_);
        else{
            Code.put(Code.trap);
            Code.put(1);
        }
    }

    public void visit(ExprList list){
        if(list.getAddop() instanceof AddopAdd)
            Code.put(Code.add);
        else
            Code.put(Code.sub);
    }

    public void visit(ExprNegatedTerm term){
        Code.put(Code.neg);
    }

    public void visit(TermList termList){
        if(termList.getMulop() instanceof MulopMul)
            Code.put(Code.mul);
        else if(termList.getMulop() instanceof MulopDiv)
            Code.put(Code.div);
        else if(termList.getMulop() instanceof MulopMod)
            Code.put(Code.rem);
    }

    public void visit(FactorNum factorNum){
        Code.loadConst(factorNum.getNumberFactor());
    }

    public void visit(FactorBool factorBool){
        if(factorBool.getBoolFactor())
            Code.loadConst(1);
        else
            Code.loadConst(0);
    }

    public void visit(FactorChar factorChar){
        Code.loadConst(factorChar.getCharFactor());
    }

    public void visit(PrintStatementOneArg printStatementOneArg){
        if(printStatementOneArg.getExpr().struct == Tab.charType) {
            Code.loadConst(1);
            Code.put(Code.bprint);
        }
        else {
            Code.loadConst(4);
            Code.put(Code.print);
        }
    }

    public void visit(PrintStatementTwoArg printStatementTwoArg){
        Code.loadConst(printStatementTwoArg.getNum());
        if(printStatementTwoArg.getExpr().struct == Tab.charType){
            Code.put(Code.bprint);
        }
        else{
            Code.put(Code.print);
        }
    }

    public void visit(FactorDesignator factorDesignator){
        Obj obj = factorDesignator.getDesignatorBase().obj;
        if(obj.getKind() == Obj.Type && obj.getType().getKind() == Struct.Enum){
            DesignatorDot designatorDot = (DesignatorDot)factorDesignator.getDesignatorBase().getDesignator();
            for(Obj tmp : designatorDot.getDesignator().obj.getType().getMembers())
                if(tmp.getName().equals(designatorDot.getDesignatorName()))
                    obj = tmp;
        }

        Code.load(obj);
    }

    public void visit(DesignatorIdent designatorIdent){
        if(designatorIdent.obj.getKind() == Obj.Fld || (designatorIdent.obj.getKind() == Obj.Meth && designatorIdent.obj.getFpPos() == 1))
            Code.put(Code.load_n);
    }

    public void visit(DesignatorDot designatorDot){
        if(designatorDot.getDesignator().obj.getType().getKind() != Struct.Enum)
            Code.load(designatorDot.getDesignator().obj);
    }

    public void visit(FactorAllocation factorAllocation){
        int fieldNum = factorAllocation.getType().struct.getNumberOfFields() + SemanticAnalyzer.getParentClassNumOfFields(factorAllocation.getType().struct);
        ArrayList<Integer> vtableFixAddresses = vtableFixAddr.get(factorAllocation.getType().struct);
        Code.put(Code.new_);
        Code.put2(fieldNum * 4);
        Code.put(Code.dup);
        Code.put(Code.const_);
        int fixAddr = Code.pc;
        vtableFixAddresses.add(fixAddr);
        Code.put4(0);
        Code.put(Code.putfield);
        Code.put2(0);
    }

    public void visit(FactorAllocationArray factorAllocationArray){
        Code.put(Code.newarray);
        Code.put(factorAllocationArray.getType().struct == Tab.charType ? 0 : 1);
    }

    public void visit(FactorDesignatorCall factorDesignatorCall){
        if(factorDesignatorCall.getDesignatorBase().obj.getFpPos() == 0) {
            int diff = factorDesignatorCall.getDesignatorBase().obj.getAdr() - Code.pc;
            Code.put(Code.call);
            Code.put2(diff);
        }
        else{
            Designator baseDesig = factorDesignatorCall.getDesignatorBase().getDesignator();
            if(baseDesig instanceof DesignatorDot){
                DesignatorDot desig = (DesignatorDot)baseDesig;
                baseDesig.traverseBottomUp(this); //:D
            }
            else{
                Code.put(Code.load_n);
            }
            Code.put(Code.getfield);
            Code.put2(0);
            String methName = factorDesignatorCall.getDesignatorBase().obj.getName();
            Code.put(Code.invokevirtual);
            for(int i = 0; i < methName.length(); ++i)
                Code.put4(methName.charAt(i));
            Code.put4(-1);
        }
    }

    public void visit(ArrayLoadMarker arrayLoadMarker){

        DesignatorArray parent = (DesignatorArray)arrayLoadMarker.getParent();
        Code.load(parent.getDesignator().obj);
        reportInfo("PC at " + Code.pc + " Added array loading!", arrayLoadMarker);
    }

    public void visit(DesignatorAssign designatorAssign){
        Code.store(designatorAssign.getDesignatorBase().obj);
    }

    //todo fix for arrays and fields
    public void visit(DesignatorInc designatorInc){
        designatorInc.getDesignatorBase().getDesignator().traverseBottomUp(this);
        Code.load(designatorInc.getDesignatorBase().obj);
        Code.loadConst(1);
        Code.put(Code.add);
        Code.store(designatorInc.getDesignatorBase().obj);
        reportInfo("Increment of " + designatorInc.getDesignatorBase().obj.getName(), designatorInc);
    }

    public void visit(DesignatorDec designatorDec){
        designatorDec.getDesignatorBase().getDesignator().traverseBottomUp(this);
        Code.load(designatorDec.getDesignatorBase().obj);
        Code.loadConst(1);
        Code.put(Code.sub);
        Code.store(designatorDec.getDesignatorBase().obj);
    }

    public void visit(DesignatorCall designatorCall){
        if(designatorCall.getDesignatorBase().obj.getFpPos() == 0) {
            int diff = designatorCall.getDesignatorBase().obj.getAdr() - Code.pc;
            Code.put(Code.call);
            Code.put2(diff);
        }
        else{
            Designator baseDesig = designatorCall.getDesignatorBase().getDesignator();
            if(baseDesig instanceof DesignatorDot){
                DesignatorDot desig = (DesignatorDot)baseDesig;
                baseDesig.traverseBottomUp(this); //:D
            }
            else{
                Code.put(Code.load_n);
            }
            Code.put(Code.getfield);
            Code.put2(0);
            String methName = designatorCall.getDesignatorBase().obj.getName();
            Code.put(Code.invokevirtual);
            for(int i = 0; i < methName.length(); ++i)
                Code.put4(methName.charAt(i));
            Code.put4(-1);
        }

        if(designatorCall.getDesignatorBase().obj.getType() != Tab.noType)
            Code.put(Code.pop);
    }

    public void visit(ReadStatement readStatement){
        Obj obj = readStatement.getDesignatorBase().obj;
        if(obj.getType().getKind() == Struct.Char)
            Code.put(Code.bread);
        else
            Code.put(Code.read);

        Code.store(obj);
    }

    public void visit(ReturnStatementVoid returnStatementVoid){
        Code.put(Code.exit);
        Code.put(Code.return_);
    }

    public void visit(ReturnStatement returnStatement){
        Code.put(Code.exit);
        Code.put(Code.return_);
    }

    public void visit(CondFactRel condFactRel){
        int cond = 0;
        Relop relop = condFactRel.getRelop();
        if(relop instanceof RelopEquals)
            cond = Code.eq;
        else if(relop instanceof RelopNotEquals)
            cond = Code.ne;
        else if(relop instanceof RelopGreaterEqual)
            cond = Code.ge;
        else if(relop instanceof RelopGreaterThan)
            cond = Code.gt;
        else if(relop instanceof RelopLessThan)
            cond = Code.lt;
        else if(relop instanceof RelopLessEqual)
            cond = Code.le;
        else
            reportInfo("Possible undeclared relational operator used", condFactRel);

        Code.put(Code.jcc + cond);
        int patchToFinishTrue = Code.pc;
        Code.put2(0);
        Code.loadConst(0);
        Code.put(Code.jmp);
        int patchToProgStart = Code.pc;
        Code.put2(0);
        Code.fixup(patchToFinishTrue);
        Code.loadConst(1);
        Code.fixup(patchToProgStart);
    }

    public void visit(CondTermList condTermList) { // cond1 && cond2
        Code.put(Code.add);
        Code.loadConst(2);
        Code.put(Code.jcc + Code.eq);
        int truePatchAddress = Code.pc;
        Code.put2(0);
        Code.loadConst(0);
        Code.put(Code.jmp);
        int finishPatchAddress = Code.pc;
        Code.put2(0);
        Code.fixup(truePatchAddress);
        Code.loadConst(1);
        Code.fixup(finishPatchAddress);
    }

    public void visit(ConditionList conditionList){ // cond1 || cond2
        Code.put(Code.add);
        Code.loadConst(1);
        Code.put(Code.jcc + Code.ge);
        int truePatchAddress = Code.pc;
        Code.put2(0);
        Code.loadConst(0);
        Code.put(Code.jmp);
        int finishPatchAddress = Code.pc;
        Code.put2(0);
        Code.fixup(truePatchAddress);
        Code.loadConst(1);
        Code.fixup(finishPatchAddress);
    }

    public void visit(ProgramName program){
        //Ord i chr ne treba nista da rade
        Obj chr = Tab.find("chr");
        chr.setAdr(Code.pc);
        Code.put(Code.return_);
        Obj ord = Tab.find("ord");
        ord.setAdr(Code.pc);
        Code.put(Code.return_);
        Obj len = Tab.find("len");
        len.setAdr(Code.pc);
        Code.put(Code.enter);
        Code.put(1);
        Code.put(1);
        Code.put(Code.load);
        Code.put(0);
        Code.loadConst(1);
        Code.put(Code.sub);
        Code.put(Code.getfield);
        Code.put2(0);
        Code.put(Code.exit);
        Code.put(Code.return_);
    }

    public void visit(IfStartMarkerCond ifStartMarker){
        Obj fixAddressHolder = ifStartMarker.obj = new Obj(0, "", null); //Hack :D
        Code.loadConst(1);
        Code.put(Code.jcc + Code.ne);
        fixAddressHolder.setAdr(Code.pc);
        Code.put2(0);
    }

    public void visit(IfStatement ifStatement){
        int jumpFixAddress = ifStatement.getIfStartMarker().obj.getAdr();
        Code.fixup(jumpFixAddress);
    }

    public void visit(IfElseMarker ifElseMarker){
        Obj fixAddressHolder = ifElseMarker.obj = new Obj(0, "", null); //More hacks :D
        Code.put(Code.jmp); //Unconditional jump to skip else
        fixAddressHolder.setAdr(Code.pc);
        Code.put2(0);

        IfElseStatement parent = (IfElseStatement)ifElseMarker.getParent();
        int jumpFixAddress = parent.getIfStartMarker().obj.getAdr();
        Code.fixup(jumpFixAddress); //Fix initial jump to else
    }

    public void visit(IfElseStatement ifElseStatement){
        int jumpFixAddress = ifElseStatement.getIfElseMarker().obj.getAdr();
        Code.fixup(jumpFixAddress); //Patch the jump to the end
    }

    public void visit(ForInit forInitPart){
        forInitPart.forcondaddr = new ForCondAddr();
        forInitPart.forcondaddr.addr = Code.pc;
    }

    public void visit(NoForInit noForInit){
        noForInit.forcondaddr = new ForCondAddr();
        noForInit.forcondaddr.addr = Code.pc;
    }

    public void visit(ForCond forCondPart){
        Code.loadConst(1);
        Code.put(Code.jcc + Code.eq);
        int iterationFixAddress = Code.pc;
        Code.put2(0);
        Code.put(Code.jmp);
        int exitFixAddress = Code.pc;
        Code.put2(0);
        ForCondFixAddr addr = forCondPart.forcondfixaddr = new ForCondFixAddr();
        addr.iterationFixAddr = iterationFixAddress;
        addr.exitFixAddr = exitFixAddress;
        addr.stepStartAddr = Code.pc;
    }

    public void visit(NoForCond noForCond){
        Code.put(Code.jmp);
        int iterationFixAddress = Code.pc;
        Code.put2(0);
        Code.put(Code.jmp);
        int exitFixAddress = Code.pc;
        Code.put2(0);
        ForCondFixAddr addr = noForCond.forcondfixaddr = new ForCondFixAddr();
        addr.iterationFixAddr = iterationFixAddress;
        addr.exitFixAddr = exitFixAddress;
        addr.stepStartAddr = Code.pc;
    }

    public void visit(ForStep forStepPart){
        ForStatement stmt = (ForStatement)forStepPart.getParent();
        int condAddr = stmt.getForInitPart().forcondaddr.addr;
        Code.putJump(condAddr);

        int iterationFixAddr = stmt.getForCondPart().forcondfixaddr.iterationFixAddr;
        Code.fixup(iterationFixAddr);
    }

    public void visit(NoForStep noForStep){
        ForStatement stmt = (ForStatement)noForStep.getParent();
        int condAddr = stmt.getForInitPart().forcondaddr.addr;
        Code.putJump(condAddr);

        int iterationFixAddr = stmt.getForCondPart().forcondfixaddr.iterationFixAddr;
        Code.fixup(iterationFixAddr);
    }

    public void visit(ForStatement forStatement){
        int stepAddr = forStatement.getForCondPart().forcondfixaddr.stepStartAddr;
        Code.putJump(forStatement.getForCondPart().forcondfixaddr.stepStartAddr);
        int exitFixAddr = forStatement.getForCondPart().forcondfixaddr.exitFixAddr;
        Code.fixup(exitFixAddr);

        ArrayList<Integer> breakList = breakFixAddr.pop();
        ArrayList<Integer> continueList = contiinueFixAddr.pop();

        for(Integer addr : breakList)
            Code.fixup(addr);

        for(Integer addr: continueList){
            int diff = stepAddr - addr + 1;
            Code.put2(addr, diff);
        }
    }

    public void visit(BreakStatement breakStatement){
        Code.put(Code.jmp);
        int fixAddr = Code.pc;
        Code.put2(0);
        breakFixAddr.peek().add(fixAddr);
    }

    public void visit(ContinueStatement continueStatement){
        Code.put(Code.jmp);
        int fixAddr = Code.pc;
        Code.put2(0);
        contiinueFixAddr.peek().add(fixAddr);
    }

    public void visit(ForStartMarker forStartMarker){
        ArrayList<Integer> breakList = new ArrayList<>();
        ArrayList<Integer> contList = new ArrayList<>();

        breakFixAddr.push(breakList);
        contiinueFixAddr.push(contList);
    }

}
