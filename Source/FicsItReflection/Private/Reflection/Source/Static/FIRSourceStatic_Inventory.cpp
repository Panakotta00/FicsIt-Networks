#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "FGInventoryComponent.h"
#include "FGInventoryLibrary.h"
#include "FGItemCategory.h"
#include "FicsItLogLibrary.h"
#include "ItemAmount.h"

BeginClass(UFGItemDescriptor, "ItemType", "Item Type", "The type of an item (iron plate, iron rod, leaves)")
BeginClassProp(RInt, form, "Form", "The matter state of this resource.\n1: Solid\n2: Liquid\n3: Gas\n4: Heat") {
	FIRReturn (FIRInt)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RFloat, energy, "Energy", "How much energy this resource provides if used as fuel.") {
	FIRReturn (FIRFloat)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RFloat, radioactiveDecay, "Radioactive Decay", "The amount of radiation this item radiates.") {
	FIRReturn (FIRFloat)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RString, name, "Name", "The name of the item.") {
	FIRReturn (FIRStr)UFGItemDescriptor::GetItemName(self).ToString();
} EndProp()
BeginClassProp(RString, description, "Description", "The description of this item.") {
	FIRReturn (FIRStr)UFGItemDescriptor::GetItemDescription(self).ToString();
} EndProp()
BeginClassProp(RInt, max, "Max", "The maximum stack size of this item.") {
	FIRReturn (FIRInt)UFGItemDescriptor::GetStackSize(self);
} EndProp()
BeginClassProp(RBool, canBeDiscarded, "Can be Discarded", "True if this item can be discarded.") {
	FIRReturn (FIRBool)UFGItemDescriptor::CanBeDiscarded(self);
} EndProp()
BeginClassProp(RClass<UFGItemCategory>, category, "Category", "The category in which this item is in.") {
	FIRReturn (FIRClass)UFGItemDescriptor::GetCategory(self);
} EndProp()
BeginClassProp(RStruct<FLinearColor>, fluidColor, "Fluid Color", "The color of this fluid.") {
	FIRReturn (FIRStruct)(FLinearColor)UFGItemDescriptor::GetFluidColor(self);
} EndProp()
EndClass()

BeginClass(UFGItemCategory, "ItemCategory", "Item Category", "The category of some items.")
BeginClassProp(RString, name, "Name", "The name of the category.") {
	FIRReturn (FIRStr)UFGItemCategory::GetCategoryName(self).ToString();
} EndProp()
EndClass()

BeginClass(UFGInventoryComponent, "Inventory", "Inventory", "A actor component that can hold multiple item stacks.\nWARNING! Be aware of container inventories, and never open their UI, otherwise these function will not work as expected.")
BeginFuncVA(getStack, "Get Stack", "Returns the item stack at the given index.\nTakes integers as input and returns the corresponding stacks.") {
	Body()
	int ArgNum = Params.Num();
	for (int i = 0; i < ArgNum; ++i) {
		const FIRAny& Any = Params[i];
		FInventoryStack Stack;
		if (Any.GetType() == FIR_INT && self->GetStackFromIndex(Any.GetInt(), Stack)) { // GetInt realy correct?
			Params.Add(FIRAny(Stack));
		} else {
			Params.Add(FIRAny());
		}
	}
} EndFunc()
BeginProp(RInt, itemCount, "Item Count", "The absolute amount of items in the whole inventory.") {
	FIRReturn (int64)self->GetNumItems(nullptr);
} EndProp()
BeginProp(RInt, size, "Size", "The count of available item stack slots this inventory has.") {
	FIRReturn (int64)self->GetSizeLinear();
} EndProp()
BeginFunc(sort, "Sort", "Sorts the whole inventory. (like the middle mouse click into a inventory)") {
	Body()
	UFILogLibrary::Log(FIL_Verbosity_Warning, TEXT("It is currently Unsafe/Buggy to call sort!"));
	if (!self->IsLocked() && self->GetCanBeRearranged()) self->SortInventory();
} EndFunc()
BeginFunc(swapStacks, "Swap Stacks", "Swaps two given stacks inside the inventory.", 1) {
	InVal(0, RInt, index1, "Index 1", "The index of the first stack in the inventory.")
	InVal(1, RInt, index2, "Index 2", "The index of the second stack in the inventory.")
	OutVal(2, RBool, successful, "Successful", "True if the swap was successful.")
	Body()
	UFILogLibrary::Log(FIL_Verbosity_Warning, TEXT("It is currently Unsafe/Buggy to call swapStacks!"));
	successful = UFGInventoryLibrary::MoveInventoryItem(self, index1, self, index2);
} EndFunc()
BeginFunc(flush, "Flush", "Removes all discardable items from the inventory completely. They will be gone! No way to get them back!", 0) {
	Body()
	TArray<FInventoryStack> stacks;
	self->GetInventoryStacks(stacks);
	self->Empty();
	for (const FInventoryStack& stack : stacks) {
		if (stack.HasItems() && stack.Item.IsValid() && !UFGItemDescriptor::CanBeDiscarded(stack.Item.GetItemClass())) {
			self->AddStack(stack);
		}
	}
} EndFunc()
BeginFunc(canSplitAtIndex, "Can Split at Index", "Returns true if the item stack at the given index can be split.") {
	InVal(0, RInt, index, "Index", "The slot index of which you want to check if the stack can be split.")
	OutVal(1, RBool, canSplit, "Can Split", "True if the stack at the given index can be split.")
	Body()
	canSplit = self->CanSplitStackAtIdx(index);
} EndFunc()
BeginFunc(splitAtIndex, "Split At Index", "Tries to split the stack at the given index and puts the given amount of items into a free slot.") {
	InVal(0, RInt, index, "Index", "The index of the stack you want to split.")
	InVal(1, RInt, num, "Num", "The number of items you want to split off the stack at the given index.")
	Body()
	self->SplitStackAtIdx(index, num);
} EndFunc()
EndClass()

BeginStructConstructable(FItemAmount, "ItemAmount", "Item Amount", "A struct that holds a pair of amount and item type.")
BeginProp(RInt, amount, "Amount", "The amount of items.") {
	FIRReturn (int64) self->Amount;
} PropSet() {
	self->Amount = Val;
} EndProp()
BeginProp(RClass<UFGItemDescriptor>, type, "Type", "The type of the items.") {
	FIRReturn (UClass*)self->ItemClass;
} PropSet() {
	self->ItemClass = Val;
} EndProp()
EndStruct()

BeginStructConstructable(FInventoryStack, "ItemStack", "Item Stack", "A structure that holds item information and item amount to represent an item stack.")
BeginProp(RInt, count, "Count", "The count of items.") {
	FIRReturn (int64) self->NumItems;
} PropSet() {
	self->NumItems = Val;
} EndProp()
BeginProp(RStruct<FInventoryItem>, item, "Item", "The item information of this stack.") {
	FIRReturn self->Item;
} PropSet() {
	self->Item = Val;
} EndProp()
EndStruct()

BeginStructConstructable(FInventoryItem, "Item", "Item", "A structure that holds item information.")
BeginProp(RClass<UFGItemDescriptor>, type, "Type", "The type of the item.") {
	FIRReturn (UClass*)self->GetItemClass();
} PropSet() {
	self->SetItemClass(Val);
} EndProp()
EndStruct()
