#include "FIVSStateEEPROM.h"

#include "Script/FIVSNodeSerialization.h"

AFIVSStateEEPROM::AFIVSStateEEPROM() {
	Graph = CreateDefaultSubobject<UFIVSGraph>("Graph");
}

void AFIVSStateEEPROM::Serialize(FStructuredArchive::FRecord StructuredArchiveRecord) {
	Super::Serialize(StructuredArchiveRecord);

	if (StructuredArchiveRecord.GetUnderlyingArchive().IsSaving()) {
		FString Str = UFIVSSerailizationUtils::FIVS_SerailizeGraph(Graph);
		StructuredArchiveRecord.EnterField(SA_FIELD_NAME(TEXT("Graph"))) << Str;
	} else if (StructuredArchiveRecord.GetUnderlyingArchive().IsLoading()) {
		FString Str;
		StructuredArchiveRecord.EnterField(SA_FIELD_NAME(TEXT("Graph"))) << Str;
		for (UFIVSNode* Node : Graph->GetNodes()) Graph->RemoveNode(Node);
		UFIVSSerailizationUtils::FIVS_DeserializeGraph(Graph, Str);
	}
}
