#pragma once

#include "CoreMinimal.h"

class UFIVSGraph;
struct FFIVSScript;
class UFIVSScriptNode;

class FFIVSCompiler {
public:
	static TMap<UFIVSScriptNode*, FFIVSScript> CompileGraph(UFIVSGraph* Graph);
	static FFIVSScript CompileScript(UFIVSScriptNode* Node);
};
