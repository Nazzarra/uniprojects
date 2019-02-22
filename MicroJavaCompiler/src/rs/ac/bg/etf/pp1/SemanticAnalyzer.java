package rs.ac.bg.etf.pp1;

import rs.ac.bg.etf.pp1.ast.*;
import rs.etf.pp1.symboltable.concepts.*;
import rs.etf.pp1.symboltable.Tab;
import rs.etf.pp1.symboltable.visitors.DumpSymbolTableVisitor;

import java.util.Collection;

public class SemanticAnalyzer extends VisitorAdaptor{

    private Program root;
    private boolean programSemanticsOk;
    private boolean shouldReportInfo;

    private Obj currentClass = Tab.noObj;
    private Obj currentMethod = Tab.noObj;
    private Obj currentInterface = Tab.noObj;

    private DumpSymbolTableVisitor objPrinter;
    private int loopNestDepth = 0;
    private Struct boolType;

    private static Obj findInCurrentScope(String name){
        Obj ret;
        if(Tab.currentScope().getLocals() == null)
            ret = Tab.noObj;
        else {
            ret = Tab.currentScope().getLocals().searchKey(name);
            if (ret == null)
                ret = Tab.noObj;
        }
        return ret;
    }

    public boolean isProgramSemanticsOk(){
        return programSemanticsOk;
    }

    public void reportError(String message, SyntaxNode node){
        System.err.println("(Line: " + node.getLine() + ") ERROR: " + message);
        programSemanticsOk = false;
    }

    public void reportInfo(String message, SyntaxNode node){
        if(shouldReportInfo)
            System.out.println("(Line: " + node.getLine() + ") INFO: " + message);
    }

    public SemanticAnalyzer(Program root, boolean shouldReportInfo){
        Tab.init();
        this.root = root;
        this.shouldReportInfo = shouldReportInfo;
        boolType = Tab.insert(Obj.Type, "bool", new Struct(Struct.Bool)).getType();
    }

    public void analyze(){
        programSemanticsOk = true;
        root.traverseBottomUp(this);
        if(programSemanticsOk){
            Obj prog = Tab.find(root.getProgramName().getProgName());
            Obj mainMeth = Tab.noObj;

            for(Obj local : prog.getLocalSymbols())
                if(local.getKind() == Obj.Meth && local.getName().equals("main")){
                    mainMeth = local;
                    break;
                }

            if(mainMeth == Tab.noObj){
                reportError("Program missing main method!", root);
            }
            else if(mainMeth.getLevel() != 0){
                reportError("Main must take no parameters!", root);
            }
            else if(mainMeth.getType() != Tab.noType){
                reportError("Main must not return a value!", root);
            }
        }
    }

    public void visit(ProgramName programName){
        programName.obj = Tab.insert(Obj.Prog, programName.getProgName(), Tab.noType);
        Tab.openScope();
    }

    public void visit(Program program){
        Tab.chainLocalSymbols(program.getProgramName().obj);
        Tab.closeScope();
    }

    public void visit(Type type){
        Obj typeObj = Tab.find(type.getTypeName());
        if(typeObj == Tab.noObj){
            reportError("Cannot find type: " + type.getTypeName(), type);
        }
        else{
            if(typeObj.getKind() != Obj.Type){
                reportError("Type name " + type.getTypeName() + " is not a type.", type);
            }
            else{
                type.struct = typeObj.getType();
            }
        }
    }

    public void visit(ClassDeclHeader classDeclHeader)
    {
        String className = classDeclHeader.getClassName();
        if(findInCurrentScope(className) != Tab.noObj){
            reportError("Class declaration error, name redeclared: " + className, classDeclHeader);
            return;
        }
        currentClass = Tab.insert(Obj.Type, className, new Struct(Struct.Class));
        currentClass.getType().setElementType(Tab.noType);
        Tab.openScope();
    }

    public void visit(ClassDecl classDecl)
    {
        Tab.chainLocalSymbols(currentClass.getType());
        Struct type = currentClass.getType();
        currentClass = Tab.noObj;
        Tab.closeScope();

        for(Struct inter : type.getImplementedInterfaces()){
            for(Obj method : inter.getMembers()){
                Obj actualMethod = getMethodWithName(type, method.getName());
                if(actualMethod == Tab.noObj){
                    reportError("Interface method " + method.getName() + " not implemented!", classDecl);
                }
                else if(!doMethodsMatch(method, actualMethod)){
                    reportError("Interface method " + method.getName() + " does not match with implemented method", classDecl);
                }
            }
        }
    }

    public void visit(ClassExtends classExtends){
        Struct type = classExtends.getType().struct;
        if(type.getKind() != Struct.Class && type != Tab.nullType){
            reportError("Attempt to use non-class type as base type", classExtends);
        }
        else {
            currentClass.getType().setElementType(type);
        }
    }

    public void visit(InterfaceHeader interfaceHeader){
        if(findInCurrentScope(interfaceHeader.getInterfaceName()) != Tab.noObj){
            reportError("Interface declaration error, name redeclared", interfaceHeader);
        }
        else{
            currentInterface = Tab.insert(Obj.Type, interfaceHeader.getInterfaceName(), new Struct(Struct.Interface));
            Tab.openScope();
        }
    }

    public void visit(InterfaceDecl interfaceDecl){
        Tab.chainLocalSymbols(currentInterface.getType());
        currentInterface = Tab.noObj;
        Tab.closeScope();
    }

    public void visit(InterfaceMethodName interfaceMethodName){
        Struct retType;
        InterfaceMethodDecl parent = (InterfaceMethodDecl)interfaceMethodName.getParent();
        if(parent.getMethodType() instanceof  MethodTypeVoid)
            retType = Tab.noType;
        else
            retType = ((MethodTypeActual)parent.getMethodType()).getType().struct;

        if(findInCurrentScope(interfaceMethodName.getMethodName()) != Tab.noObj){
            reportError("Interfcae method declaration error - name already defined.", interfaceMethodName);
        }
        else{
            currentMethod = Tab.insert(Obj.Meth, interfaceMethodName.getMethodName(), retType);
            Tab.openScope();
            currentMethod.setLevel(1);
            currentMethod.setFpPos(1);
            Tab.insert(Obj.Var, "this", currentClass.getType());
        }
    }

    public void visit(InterfaceMethodDecl interfaceMethodDecl){
        Tab.chainLocalSymbols(currentMethod);
        Tab.closeScope();
        currentMethod = Tab.noObj;
    }


    public void visit(NoClassExtends noClassExtends){
        Tab.insert(Obj.Fld, "__vtable", Tab.noType);
    }

    private static Obj getMethodWithName(Struct classStruct, String name) {
        Obj foundObj = Tab.noObj;

        while(classStruct != Tab.noType){
            for(Obj obj : classStruct.getMembers())
                if(obj.getKind() == Obj.Meth && obj.getName().equals(name)){
                    foundObj = obj;
                    break;
                }

            if(foundObj != Tab.noObj)
                break;

            classStruct = classStruct.getElemType();
        }

        return foundObj;
    }

    //Assume equal method names
    private boolean doMethodsMatch(Obj methodOne, Obj methodTwo){
        //diff param cnt
        if(methodOne.getLevel() != methodTwo.getLevel())
            return false;

        int paramCount = methodOne.getLevel();
        for(int i = 1; i < paramCount; ++i)
            if(!isAssignmentCompatible(getFpAtPos(i, methodOne).getType(), getFpAtPos(i, methodTwo).getType()))
                return false;

        return true;
    }

    public void visit(ClassImplements classImplements){
        Struct type = classImplements.getType().struct;
        if(type.getKind() != Struct.Interface){
            reportError("Attempt to use non-interface type for implementing an interface", classImplements);
        }
        else {
            if(doesClassImplementInterface(currentClass.getType(), type))
                reportError("Attempt to implement an already implemented interface", classImplements);
            else
                currentClass.getType().addImplementedInterface(type);
        }
    }

    public void visit(ImplementsMultiple classImplementsMultiple){
        Struct type = classImplementsMultiple.getType().struct;
        if(type.getKind() != Struct.Interface){
            reportError("Attempt to use non-interface type for implementing an interface", classImplementsMultiple);
        }
        else {
            if(doesClassImplementInterface(currentClass.getType(), type))
                reportError("Attempt to implement an already implemented interface", classImplementsMultiple);
            else
                currentClass.getType().addImplementedInterface(type);
        }
    }

    private Type getVarDeclCoreType(VarDeclCore core){
        VarDeclBase list;
        SyntaxNode parentIter = core.getParent();
        while(!(parentIter instanceof VarDeclBase))
            parentIter = parentIter.getParent();
        list = (VarDeclBase) parentIter;
        return list.getType();
    }

    public static int getParentClassNumOfFields(Struct base){
        if(base.getElemType() == Tab.noType)
            return 0;
        return base.getElemType().getNumberOfFields() + getParentClassNumOfFields(base.getElemType());
    }

    public void visit(VarDecl varDecl){
        Obj varObj = findInCurrentScope(varDecl.getVarName());
        if(varObj != Tab.noObj){
            reportError("Variable declaration error, Name redeclared: " + varDecl.getVarName(), varDecl);
        }
        else{
            Type myType = getVarDeclCoreType(varDecl);
            if(currentClass == Tab.noObj || currentMethod != Tab.noObj)
                varObj = Tab.insert(Obj.Var, varDecl.getVarName(), myType.struct);
            else {
                if(getMethodWithName(currentClass.getType(), varDecl.getVarName()) != Tab.noObj){
                    reportError("Attempt to shadow method with field", varDecl);
                }
                varObj = Tab.insert(Obj.Fld, varDecl.getVarName(), myType.struct);
                varObj.setAdr(varObj.getAdr() + getParentClassNumOfFields(currentClass.getType()));
            }
            //objPrinter = new DumpSymbolTableVisitor();
            //objPrinter.visitObjNode(varObj);
            //reportInfo("Variable declaration - " + objPrinter.getOutput(), varDecl);
        }
    }

    public void visit(VarDeclArray varDecl) {
        Obj varObj = findInCurrentScope(varDecl.getVarName());
        if (varObj != Tab.noObj) {
            reportError("Variable declaration error, Name redeclared: " + varDecl.getVarName().toString(), varDecl);
        } else {
            Type myTypeBase = getVarDeclCoreType(varDecl);
            Struct myType = new Struct(Struct.Array, myTypeBase.struct);
            if (currentClass == Tab.noObj || currentMethod != Tab.noObj)
                varObj = Tab.insert(Obj.Var, varDecl.getVarName(), myType);
            else {
                if(getMethodWithName(currentClass.getType(), varDecl.getVarName()) != Tab.noObj){
                    reportError("Attempt to shadow method with field", varDecl);
                }
                varObj = Tab.insert(Obj.Fld, varDecl.getVarName(), myType);
                varObj.setAdr(varObj.getAdr() + getParentClassNumOfFields(currentClass.getType()));
            }
            //objPrinter = new DumpSymbolTableVisitor();
            //objPrinter.visitObjNode(varObj);
            //reportInfo("Variable declaration - " + objPrinter.getOutput(), varDecl);
        }
    }

    public void visit(EnumTypeDecl enumTypeDecl){
        enumTypeDecl.obj = Tab.find(enumTypeDecl.getEnumTypeName());
        if(enumTypeDecl.obj != Tab.noObj){
            reportError("Enum declaration error, Name redeclared: " + enumTypeDecl.getEnumTypeName(), enumTypeDecl);
        }
        else {
            enumTypeDecl.obj = Tab.insert(Obj.Type, enumTypeDecl.getEnumTypeName(), new Struct(Struct.Enum));
            Tab.openScope();
        }
    }

    public void visit(EnumDecl enumDecl){
        Struct enumType = enumDecl.getEnumTypeDecl().obj.getType();
        Tab.chainLocalSymbols(enumType);
        Tab.closeScope();
        //objPrinter = new DumpSymbolTableVisitor();
        //objPrinter.visitObjNode(enumDecl.getEnumTypeDecl().obj);
        //String enumObjOutput = objPrinter.getOutput();
        String enumFields = "";
        for(Obj obj : enumType.getMembers())
            enumFields = enumFields + " " + obj.getName() + "(" + obj.getAdr() + ")";
        //reportInfo("Enum declaration: " + objPrinter.getOutput() + " Enum values: " + enumFields, enumDecl );
    }

    public void visit(EnumDeclValAuto auto){
        int enumVal = 0;
        if(findInCurrentScope(auto.getEnumVal()) != Tab.noObj){
            reportError("Enum value redeclared: " + auto.getEnumVal(), auto);
        }
        else {
            Scope currentScope = Tab.currentScope();
            if(currentScope.getLocals() != null) {
                for (Obj obj : currentScope.getLocals().symbols())
                    enumVal = obj.getAdr() + 1; // :)

                //reportInfo("Val: " + enumVal, auto);
                for (Obj obj : currentScope.getLocals().symbols()) {
                    //reportInfo("Test " + obj.getName() + " " + obj.getAdr(), auto);
                    if (obj.getAdr() == enumVal) {
                        reportError("Enum value " + auto.getEnumVal() + " has the same value as " + obj.getName(), auto);
                        return;
                    }
                }
            }

            Obj insertedVal = Tab.insert(Obj.Con, auto.getEnumVal(), Tab.intType);
            insertedVal.setAdr(enumVal);
        }
    }

    public void visit(EnumDeclValAssigned assigned){
        if(findInCurrentScope(assigned.getEnumVal()) != Tab.noObj){
            reportError("Enum value redeclared: " + assigned.getEnumVal(), assigned);
        }
        else {
            Scope currentScope = Tab.currentScope();
            if(currentScope.getLocals() != null){
                for(Obj obj : currentScope.getLocals().symbols())
                    if(obj.getAdr() == assigned.getAssignedVal()){
                        reportError("Enum value "  + assigned.getEnumVal() + " has the same value as " + obj.getName(), assigned);
                        return;
                    }
            }

            Obj insertedVal = Tab.insert(Obj.Con, assigned.getEnumVal(), Tab.intType);
            insertedVal.setAdr(assigned.getAssignedVal());
        }
    }

    private static Type getConstDeclCoreType(ConstDeclCore core){
        ConstDeclBase list;
        SyntaxNode parentIter = core.getParent();
        while(!(parentIter instanceof ConstDeclBase))
            parentIter = parentIter.getParent();
        list = (ConstDeclBase) parentIter;
        return list.getType();
    }

    public void visit(ConstDeclNum constDecl){
        Type myType = getConstDeclCoreType(constDecl);
        if(myType.struct != Tab.intType){
            reportError("Attempt to assign number to non-numeric constant", constDecl);
        }
        else{
            if(findInCurrentScope(constDecl.getConstIdentifier()) != Tab.noObj){
                reportError("Const declaration error - name redeclared: " + constDecl.getConstIdentifier(), constDecl);
            }
            else {
                Obj newConst = Tab.insert(Obj.Con, constDecl.getConstIdentifier(), Tab.intType);
                newConst.setAdr(constDecl.getConstVal());
                //objPrinter = new DumpSymbolTableVisitor();
                //objPrinter.visitObjNode(newConst);
                //reportInfo("Const declaration: " + objPrinter.getOutput(), constDecl);
            }
        }
    }

    public void visit(ConstDeclChar constDecl){
        Type myType = getConstDeclCoreType(constDecl);
        if(myType.struct != Tab.charType){
            reportError("Attempt to assign char to non-char constant", constDecl);
        }
        else{
            if(findInCurrentScope(constDecl.getConstIdentifier()) != Tab.noObj){
                reportError("Const declaration error - name redeclared: " + constDecl.getConstIdentifier(), constDecl);
            }
            else {
                Obj newConst = Tab.insert(Obj.Con, constDecl.getConstIdentifier(), Tab.charType);
                newConst.setAdr(constDecl.getConstVal());
                //objPrinter = new DumpSymbolTableVisitor();
                //objPrinter.visitObjNode(newConst);
                //reportInfo("Const declaration: " + objPrinter.getOutput(), constDecl);
            }
        }
    }

    public void visit(ConstDeclBool constDecl){
        Type myType = getConstDeclCoreType(constDecl);
        Struct boolType = Tab.find("bool").getType();
        if(myType.struct != boolType){
            reportError("Attempt to assign bool to non-bool constant", constDecl);
        }
        else{
            if(findInCurrentScope(constDecl.getConstIdentifier()) != Tab.noObj){
                reportError("Const declaration error - name redeclared: " + constDecl.getConstIdentifier(), constDecl);
            }
            else {
                Obj newConst = Tab.insert(Obj.Con, constDecl.getConstIdentifier(), boolType);
                newConst.setAdr(constDecl.getConstVal() == true ? 1 : 0);
                //objPrinter = new DumpSymbolTableVisitor();
                //objPrinter.visitObjNode(newConst);
                //reportInfo("Const declaration: " + objPrinter.getOutput(), constDecl);
            }
        }
    }

    public Obj getFpAtPos(int pos, Obj methodObj){
        Obj obj = Tab.noObj;
        Collection<Obj> collection = methodObj.getLocalSymbols();
        if(methodObj == currentMethod){
            if(Tab.currentScope().getLocals() == null)
                return obj;

            collection = Tab.currentScope().getLocals().symbols();
        }
        for(Obj currentObj : collection)
            if(currentObj.getFpPos() == pos){
                obj = currentObj;
                break;
            }
        return obj;
    }

    public void visit(HasActualParams hasActualParams){
        Obj methodObj;
        if(hasActualParams.getParent() instanceof FactorDesignatorCall){
            methodObj = ((FactorDesignatorCall) hasActualParams.getParent()).getDesignatorBase().obj;
        }
        else
            methodObj = ((DesignatorCall) hasActualParams.getParent()).getDesignatorBase().obj;

        ActualParams params = hasActualParams.getActualParams();
        while(params instanceof ActualParamsMultiple)
            params = ((ActualParamsMultiple) params).getActualParams();

        Struct currentParamType = ((ActualParamsSingle)params).getExpr().struct;
        int currentPos = methodObj.getFpPos();
        while(true){
            Obj paramObj = getFpAtPos(currentPos, methodObj);
            currentPos++;
            if(paramObj == Tab.noObj){
                reportError("Attempt to call method with too many parameters.(1)", hasActualParams);
                return;
            }
            if(!isAssignmentCompatible(paramObj.getType(), currentParamType)){
                if(!(methodObj.getName().equals("len") && paramObj.getType().getKind() == Struct.Array)) {
                    reportError("Attempt to call method with incompatible parameter types.", hasActualParams);
                    return;
                }
            }
            if(!(params.getParent() instanceof ActualParamsMultiple))
                break;

            params = (ActualParamsMultiple)params.getParent();
            currentParamType = ((ActualParamsMultiple)params).getExpr().struct;
        }
        if(methodObj.getLevel() != currentPos ){
            reportError("Attempt to call method with too little parameters.(2)", hasActualParams);
        }
    }

    public void visit(NoActualParams noActualParams){
        Obj methodObj;
        if(noActualParams.getParent() instanceof FactorDesignatorCall){
            methodObj = ((FactorDesignatorCall) noActualParams.getParent()).getDesignatorBase().obj;
        }
        else
            methodObj = ((DesignatorCall) noActualParams.getParent()).getDesignatorBase().obj;
        int expectedParamCount = methodObj.getLevel();
        if(expectedParamCount != methodObj.getFpPos())
            reportError("Attempt to call method with too little parameters.(1)", noActualParams);
    }

    public void visit(MethodName methodName){
        Struct retType;
        MethodDecl parent = (MethodDecl)methodName.getParent();
        methodName.obj = Tab.noObj;
        if(parent.getMethodType() instanceof  MethodTypeVoid)
            retType = Tab.noType;
        else
            retType = ((MethodTypeActual)parent.getMethodType()).getType().struct;

        if(findInCurrentScope(methodName.getMethodName()) != Tab.noObj){
            reportError("Method declaration error - name already defined.", methodName);
        }
        else{
            methodName.obj = Tab.insert(Obj.Meth, methodName.getMethodName(), retType);
            Tab.openScope();
            currentMethod = methodName.obj;
            currentMethod.setLevel(0);
            currentMethod.setFpPos(0);
            if(currentClass != Tab.noObj){
                Tab.insert(Obj.Var, "this", currentClass.getType());
                currentMethod.setLevel(1);
                currentMethod.setFpPos(1); //Method is in class
            }
        }
    }

    public void visit(ReturnStatementVoid returnStatementVoid){
        if(currentMethod.getType() != Tab.noType){
            reportError("Method must return a value.", returnStatementVoid);
        }
    }

    public void visit(ReturnStatement returnStatement){
        if(currentMethod.getType() == Tab.noType){
            reportError("Method declared void cannot return an expression", returnStatement);
        }
        else if(!isAssignmentCompatible(currentMethod.getType(), returnStatement.getExpr().struct)){
            reportError("Attempt to return incompatible type", returnStatement);
        }
    }

    public void visit(DesignatorCall designatorCall){
        if(designatorCall.getDesignatorBase().obj.getKind() != Obj.Meth){
            reportError("Attempt to call non-method object", designatorCall);
        }
    }

    public void visit(FormalParamSingle param){
        if(findInCurrentScope(param.getParamName()) != Tab.noObj){
            reportError("Redeclared formal param: " + param.getParamName(), param);
        }
        else{
            int formalParamPos = 0;
            Obj paramObj = Tab.insert(Obj.Var, param.getParamName(), param.getType().struct);
            if(Tab.currentScope().getLocals() != null)
                formalParamPos = Tab.currentScope().getLocals().symbols().size();
            paramObj.setFpPos(formalParamPos - 1);
            reportInfo("Declared formal method argument: " + param.getParamName() + " pos " + formalParamPos, param);
            currentMethod.setLevel(currentMethod.getLevel() + 1);
        }
    }

    public void visit(FormalParamArray param){
        if(findInCurrentScope(param.getParamName()) != Tab.noObj){
            reportError("Redeclared formal param: " + param.getParamName(), param);
        }
        else{
            int formalParamPos = 0;
            Obj paramObj = Tab.insert(Obj.Var, param.getParamName(), new Struct(Struct.Array, param.getType().struct));
            if(Tab.currentScope().getLocals() != null)
                formalParamPos = Tab.currentScope().getLocals().symbols().size();
            paramObj.setFpPos(formalParamPos - 1);
            reportInfo("Declared formal method argument: " + param.getParamName() + " pos " + formalParamPos, param);
            currentMethod.setLevel(currentMethod.getLevel() + 1);
        }
    }

    public void visit(MethodDecl methodDecl){
        Tab.chainLocalSymbols(methodDecl.getMethodName().obj);
        Tab.closeScope();
        MethodName methodName = methodDecl.getMethodName();

        Obj method = currentMethod; //:D
        //objPrinter = new DumpSymbolTableVisitor();
        //objPrinter.visitObjNode(currentMethod);
        //reportInfo("Declared method " + objPrinter.getOutput() , methodDecl);
        currentMethod = Tab.noObj;

        if(currentClass != Tab.noObj){
            Obj meth = getMethodWithName(currentClass.getType(), methodName.getMethodName());
            if( meth != Tab.noObj){
                //Check params
                if(!doMethodsMatch(meth, method)){
                    reportError("Attempt to overload base method " + methodDecl.getMethodName(), methodDecl);
                }
            }
            else if(findFieldInClassAncestors(currentClass.getType(), methodName.getMethodName()) != Tab.noObj){
                reportError("Attempt to shadow field declaration with method declaration: " + methodName.getMethodName(), methodName);
            }
        }

    }

    public void visit(ForStartMarker forStartMarker){
        ++loopNestDepth;
    }

    public void visit(ForStatement forStatement){
        --loopNestDepth;
    }

    public void visit(BreakStatement breakStatement){
        if(loopNestDepth == 0) {
            reportError("Usage of break statement outside of a loop", breakStatement);
        }
    }

    public void visit(ContinueStatement continueStatement){
        if(loopNestDepth == 0){
            reportError("Usage of continue statement outside of a loop", continueStatement);
        }
    }

    public void visit(FactorNum factorNum){
        factorNum.struct = Tab.intType;
    }

    public void visit(FactorBool factorBool){
        factorBool.struct = Tab.find("bool").getType();
    }

    public void visit(FactorChar factorChar){
        factorChar.struct = Tab.charType;
    }

    private static boolean isIntegerType(Struct struct){
        return struct == Tab.intType || struct.getKind() == Struct.Enum;
    }

    public void visit(FactorAllocation factorAllocation){
        if(factorAllocation.getType().struct.getKind() != Struct.Class){
            reportError("Attempt to allocalte a non-class type", factorAllocation);
        }
        else{
            reportInfo("Detektovanje alociranje objekta klase: " + factorAllocation.getType().getTypeName(), factorAllocation);
        }
        factorAllocation.struct = factorAllocation.getType().struct;
    }

    public void visit(FactorAllocationArray factorAllocationArray){
        if(factorAllocationArray.getExpr().struct != Tab.intType){
            reportError("Attempt to allocate an array with non-int length", factorAllocationArray);
        }
        Struct struct = new Struct(Struct.Array, factorAllocationArray.getType().struct);;
        factorAllocationArray.struct = struct;
        reportInfo("Factor allocation array: " + factorAllocationArray.struct.getKind(), factorAllocationArray);
    }

    public void visit(FactorGrouped grouped){
        grouped.struct = grouped.getExpr().struct;
    }

    public void visit(FactorDesignator designator){
        designator.struct = designator.getDesignatorBase().obj.getType();
    }

    public void visit(FactorDesignatorCall factorDesignatorCall){
        if(factorDesignatorCall.getDesignatorBase().obj.getKind() != Obj.Meth){
            reportError("Attempt to call non-method object.", factorDesignatorCall);
        }
        factorDesignatorCall.struct = factorDesignatorCall.getDesignatorBase().obj.getType();
    }

    public void visit(DesignatorBase base){
        base.obj = base.getDesignator().obj;
    }

    public void visit(TermBase termBase){
        termBase.struct = termBase.getFactor().struct;
    }

    public void visit(TermList term){
        if(!isIntegerType(term.getTerm().struct) || !isIntegerType(term.getFactor().struct)){
            reportError("Attempt to apply an arithmetic operation on non-int types", term);
        }
        term.struct = Tab.intType;
    }

    public void visit(ExprTerm exprTerm){
        exprTerm.struct = exprTerm.getTerm().struct;
    }

    public void visit(ExprNegatedTerm exprNegatedTerm){
        if(exprNegatedTerm.getTerm().struct != Tab.intType){
            reportError("Attempt to negate non-int type.", exprNegatedTerm);
        }
        exprNegatedTerm.struct = exprNegatedTerm.getTerm().struct;
    }

    public void visit(ExprList exprList){
        if(!isIntegerType(exprList.getExpr().struct) || !isIntegerType(exprList.getTerm().struct)){
            reportError("Attempt to apply an arithmetic operation on non-int types", exprList);
        }
        exprList.struct = exprList.getExpr().struct;
    }

    public void visit(DesignatorIdent designatorIdent){
        Obj desObj = findInCurrentScope(designatorIdent.getDesignatorName());
        if(desObj == Tab.noObj && currentClass != Tab.noObj)
            desObj = findFieldInClassAncestors(currentClass.getType(), designatorIdent.getDesignatorName());

        if(desObj == Tab.noObj)
            desObj = Tab.find(designatorIdent.getDesignatorName());

        if(desObj == Tab.noObj){
            reportError("Attempt to reference a non-existing object: " + designatorIdent.getDesignatorName(), designatorIdent);
        }

        objPrinter = new DumpSymbolTableVisitor();
        objPrinter.visitObjNode(desObj);

        if(desObj.getKind() == Obj.Con){
            reportInfo("Detektovana upotreba simbolicke konstante: " + objPrinter.getOutput(), designatorIdent);
        }
        else if(desObj.getKind() == Obj.Var){
            if(desObj.getLevel() == 0){
                reportInfo("Detektovana upotreba globalne promenljive: " + objPrinter.getOutput(), designatorIdent);
            }
            else if(currentMethod != null){
                if(desObj.getFpPos() < currentMethod.getLevel()){
                    reportInfo("Detektovana upotreba formalnog parametra funkcije: " + objPrinter.getOutput(), designatorIdent);
                }
                else{
                    reportInfo("Detektovana upotreba lokalne promenljive: " + objPrinter.getOutput(), designatorIdent);
                }
            }
        }
        else if(desObj.getKind() == Obj.Fld){
            reportInfo("Detektovan pristup polju objekta: " + objPrinter.getOutput(), designatorIdent);
        }
        else if(desObj.getKind() == Obj.Meth && desObj.getFpPos() == 0){
            reportInfo("Detektovan poziv globalne funkcije: " + objPrinter.getOutput(), designatorIdent);
        }
        else if(desObj.getKind() == Obj.Meth){
            reportInfo("Detektovan poziv metode klasse: " + objPrinter.getOutput(), designatorIdent);
        }
        designatorIdent.obj = desObj;
    }

    private static Obj findFieldInClassAncestors(Struct classType, String name){
        Obj foundObj = Tab.noObj;
        Struct iter = classType;
        while(iter != Tab.noType){
            Obj curr = iter.getMembersTable().searchKey(name);
            if(curr != null){
                foundObj = curr;
                break;
            }
            iter = iter.getElemType();
        }

        return foundObj;
    }

    public void visit(DesignatorDot designatorDot){
        //Hef to fix for classes
        if(designatorDot.getDesignator().obj.getType().getKind() == Struct.Enum) {
            Obj foundObj = Tab.noObj;
            for(Obj obj : designatorDot.getDesignator().obj.getType().getMembers())
                if(obj.getName().equals(designatorDot.getDesignatorName()))
                    foundObj = obj;
            if(foundObj == Tab.noObj){
                reportError("Attempt to access non-existant enum value: " + designatorDot.getDesignatorName(), designatorDot);
            }
            designatorDot.obj = designatorDot.getDesignator().obj;
        }
        else if(designatorDot.getDesignator().obj.getType().getKind() == Struct.Class){
            Obj foundObj = Tab.noObj;
            Struct accessedClass = designatorDot.getDesignator().obj.getType();
            String name = designatorDot.getDesignatorName();
            if(currentClass != Tab.noObj && currentClass.getType() == accessedClass) {
                Scope currScope = Tab.currentScope();
                currScope = currScope.getOuter();
                foundObj = currScope.findSymbol(name);
                if(foundObj == null)
                    foundObj = Tab.noObj;
            }

            if(foundObj == Tab.noObj)
                foundObj = findFieldInClassAncestors(accessedClass, name);

            designatorDot.obj = foundObj;
            if(foundObj == Tab.noObj){
                reportError("Attempt to access non-existant class field: " + name, designatorDot);
            }
            else{
                objPrinter = new DumpSymbolTableVisitor();
                objPrinter.visitObjNode(foundObj);
                if(foundObj.getKind() == Obj.Meth){
                    reportInfo("Detektovan poziv metode klase: " + objPrinter.getOutput(), designatorDot );
                }
                else{
                    reportInfo("Detektovana upotreba polja klase: " + objPrinter.getOutput(), designatorDot);
                }
            }
        }
        else if(designatorDot.getDesignator().obj.getType().getKind() == Struct.Interface){
            Obj foundObj;
            Struct accessedInterface = designatorDot.getDesignator().obj.getType();
            foundObj = accessedInterface.getMembersTable().searchKey(designatorDot.getDesignatorName());
            if(foundObj == null){
                foundObj = Tab.noObj;
                reportError("Attempt to call non-existant interface method: " + designatorDot.getDesignatorName(), designatorDot);
            }
            designatorDot.obj = foundObj;
        }
        else {
            reportError("Attempt to access non-enum and non-class type using dot", designatorDot);
            designatorDot.obj = Tab.noObj;
        }

    }


    public void visit(DesignatorArray designatorArray){
        Obj parentObj = designatorArray.getDesignator().obj;
        designatorArray.obj = Tab.noObj;
        if(!isIntegerType(designatorArray.getExpr().struct)){
            reportError("Attempt to index an array with non-int value", designatorArray);
        }

        if(parentObj.getType().getKind() != Struct.Array){
            reportError("Attempt to index a non-array value ", designatorArray);
        }
        else{
            Obj obj = new Obj(Obj.Elem, "", parentObj.getType().getElemType());
            designatorArray.obj = obj;
            objPrinter = new DumpSymbolTableVisitor();
            objPrinter.visitObjNode(obj);
            reportInfo("Detektovan pristup elementu niza: " + objPrinter.getOutput(), designatorArray);
        }
    }

    private static boolean isPrimitiveType(Struct type){
        Struct boolType = Tab.find("bool").getType();
        return type == Tab.intType || type == Tab.charType || type == boolType;
    }

    private static boolean isExtendedPrimitiveType(Struct type){
        return isPrimitiveType(type) || type.getKind() == Struct.Enum;
    }

    private static boolean isMutable(Obj obj){
        return obj.getKind() == Obj.Var || obj.getKind() == Obj.Fld || obj.getKind() == Obj.Elem;
    }

    public void visit(PrintStatementOneArg printStatementOneArg){
        Struct exprType = printStatementOneArg.getExpr().struct;
        if(!isExtendedPrimitiveType(exprType)){
            reportError("Attempt to print a non-primitive type", printStatementOneArg);
        }
    }

    public void visit(PrintStatementTwoArg printStatementTwoArg){
        Struct exprType = printStatementTwoArg.getExpr().struct;
        if(!isExtendedPrimitiveType(    exprType)){
            reportError("Attempt to print a non-primitive type", printStatementTwoArg);
        }
    }

    public void visit(ReadStatement readStatement){
        Struct designatorType = readStatement.getDesignatorBase().obj.getType();
        if(!isExtendedPrimitiveType(designatorType)){
            reportError("Attempt to read into a non-primitive designator " + readStatement.getDesignatorBase().obj.getName(), readStatement);
        }
    }

    public void visit(DesignatorInc designatorInc){
        if(designatorInc.getDesignatorBase().obj.getType() != Tab.intType){
            reportError("Attempt to increment non-int designator", designatorInc);
        }
        if(!isMutable(designatorInc.getDesignatorBase().obj)){
            reportError("Attempt to increment non-mutable designator", designatorInc);
        }
    }

    public void visit(DesignatorDec designatorDec){
        if(designatorDec.getDesignatorBase().obj.getType() != Tab.intType){
            reportError("Attempt to decrement non-int designator", designatorDec);
        }
        if(!isMutable(designatorDec.getDesignatorBase().obj)){
            reportError("Attempt to decrement non-mutable designator", designatorDec);
        }
    }

    private static boolean isEquivalent(Struct left, Struct right){
        if(left == right)
            return true;
        if(left.getKind() == Struct.Array && right.getKind() == Struct.Array)
            return isEquivalent(left.getElemType(), right.getElemType());

        return false;
    }

    private static boolean isRefrence(Struct struct){
        return struct.getKind() == Struct.Array || struct.getKind() == Struct.Class;
    }

    public static boolean isCompatible(Struct left, Struct right){
        if(isEquivalent(left, right))
            return true;

        if(left == Tab.nullType && isRefrence(right))
            return true;

        return right == Tab.nullType && isRefrence(left);
    }

    private static boolean isClassAncestor(Struct child, Struct parent){
        Struct iter = child.getElemType();
        if(child.getKind() != Struct.Class)
            return false;

        while(iter != Tab.noType){
            if(iter == parent)
                return true;

            iter = iter.getElemType();
        }

        return false;
    }

    private static boolean doesClassImplementInterface(Struct classStruct, Struct interfaceStruct){
        if(classStruct.getKind() != Struct.Class)
            return false;
        if(interfaceStruct.getKind() != Struct.Interface)
            return false;

        while(classStruct != Tab.noType) {
            for (Struct iter : classStruct.getImplementedInterfaces()) {
                if (iter == interfaceStruct)
                    return true;
            }
            classStruct = classStruct.getElemType();
        }
        return false;
    }

    public static boolean isAssignmentCompatible(Struct dst, Struct src){
        if(isEquivalent(src, dst))
            return true;
        if(isRefrence(dst) && src == Tab.nullType)
            return true;
        if(isClassAncestor(src, dst))
            return true;
        if(doesClassImplementInterface(src, dst))
            return true;
        if(dst == Tab.intType && src.getKind() == Struct.Enum)
            return true;
        return false;
    }

    public void visit(DesignatorAssign designatorAssign){
        Obj obj = designatorAssign.getDesignatorBase().obj;
        if(!isMutable(obj)){
            reportError("Attempt to assign value to a non-mutable object", designatorAssign);
        }
        //Todo feex for classes
        /*
        if(obj.getType() != designatorAssign.getExpr().struct && !(obj.getType().getKind() == Struct.Array && obj.getType().getElemType().getKind() != designatorAssign.getExpr().struct.getKind())){
            reportError("Attempt to assign to a non-compatible value. Type left: " + obj.getType().getKind() + " Type right: " + designatorAssign.getExpr().struct.getKind(), designatorAssign);
        }
        */
        if(!isAssignmentCompatible(obj.getType(), designatorAssign.getExpr().struct)){
            reportError("Attempt to assign to a non-compatible value. Kind left: " + obj.getType().getKind() + " Right: " + designatorAssign.getExpr().struct.getKind(), designatorAssign);
        }
    }

    public void visit(CondFactBase condFactBase){
        if(condFactBase.getExpr().struct != boolType){
            reportError("Conditional does not have boolean type", condFactBase);
        }
    }

    public void visit(CondFactRel condFactRel){
        if(!isCompatible(condFactRel.getExpr().struct, condFactRel.getExpr().struct)){
            reportError("Attempt to compare non compatible values", condFactRel);
        }
        else{
            Struct exprType = condFactRel.getExpr().struct;
            if(exprType.getKind() == Struct.Array || exprType.getKind() == Struct.Class || exprType.getKind() == Struct.Interface)
                if(!(condFactRel.getRelop() instanceof RelopEquals || condFactRel.getRelop() instanceof RelopNotEquals))
                    reportError("Illegal comparsion operator used", condFactRel);
        }
    }

}
