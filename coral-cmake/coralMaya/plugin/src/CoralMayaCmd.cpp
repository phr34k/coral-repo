// <license>
// Copyright (C) 2011 Andrea Interguglielmi, All rights reserved.
// This file is part of the coral repository downloaded from http://code.google.com/p/coral-repo.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
// 
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// </license>
#include "CoralMayaCmd.h"

MStatus CoralMayaCmd::doIt(const MArgList &args){
	MString action = args.asString(0);
	
	if(action == "updateCoralAttributeMap"){
		MString coralNetworkNodeName = args.asString(1);
		CoralNetworkNode *coralNetworkNode = getCoralNetworkNode(coralNetworkNodeName);
		
		coralNetworkNode->updateCoralAttributeMap();
	}
	
	return MS::kSuccess;
}

CoralNetworkNode *CoralMayaCmd::getCoralNetworkNode(const MString &nodeName){
	MSelectionList selList;
	MGlobal::getSelectionListByName(nodeName, selList);
	MObject coralNetworkNodeObj;
	selList.getDependNode(0, coralNetworkNodeObj);

	MFnDependencyNode coralNetworkNodeFn(coralNetworkNodeObj);
	CoralNetworkNode *coralNetworkNode = (CoralNetworkNode*)coralNetworkNodeFn.userNode();
	
	return coralNetworkNode;
}
