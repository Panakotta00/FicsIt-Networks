#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "Reflection/Source/Static/FIRSourceStaticHooks.h"

#include "FGFactoryConnectionComponent.h"
#include "FIRSubsystem.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturer.h"

BeginClass(AFGBuildable, "Buildable", "Buildable", "The base class of all buildables.")
	Hook(UFIRBuildableHook)
BeginSignal(ProductionChanged, "Production Changed", "Triggers when the production state of the buildable changes.")
	SignalParam(0, RInt, state, "State", "The new production state.")
EndSignal()
BeginProp(RInt, numPowerConnections, "Num Power Connection", "The count of available power connections this building has.") {
	FIRReturn (FIRInt)self->GetNumPowerConnections();
} EndProp()
BeginProp(RInt, numFactoryConnections, "Num Factory Connection", "The cound of available factory connections this building has.") {
	FIRReturn (FIRInt)self->GetNumFactoryConnections();
} EndProp()
BeginProp(RInt, numFactoryOutputConnections, "Num Factory Output Connection", "The count of available factory output connections this building has.") {
	FIRReturn (FIRInt)self->GetNumFactoryOuputConnections();
} EndProp()
EndClass()

BeginClass(AFGBuildableFactory, "Factory", "Factory", "The base class of most machines you can build.")
BeginProp(RFloat, progress, "Progress", "The current production progress of the current production cycle.") {
	FIRReturn self->GetProductionProgress();
} EndProp()
BeginProp(RFloat, powerConsumProducing,	"Producing Power Consumption", "The power consumption when producing.") {
	FIRReturn self->GetProducingPowerConsumption();
} EndProp()
BeginProp(RFloat, productivity,	"Productivity", "The productivity of this factory.") {
	FIRReturn self->GetProductivity();
} EndProp()
BeginProp(RFloat, cycleTime, "Cycle Time", "The time that passes till one production cycle is finsihed.") {
	FIRReturn self->GetProductionCycleTime();
} EndProp()
BeginProp(RBool, canChangePotential, "Can Change Potential", "True if the factory can change its potential.") {
	FIRReturn self->GetCanChangePotential();
} EndProp()
BeginProp(RFloat, maxPotential, "Max Potential", "The maximum potential this factory can be set to (depending on the number of crystals).") {
	FIRReturn self->GetCurrentMaxPotential();
} EndProp()
BeginProp(RFloat, minPotential, "Min Potential", "The minimum potential this factory needs to be set to.") {
	FIRReturn self->GetCurrentMinPotential();
} EndProp()
BeginProp(RFloat, maxDefaultPotential, "Max Default Potential", "The default maximum potential this factory can be set to.") {
	FIRReturn self->GetMaxPotential();
} EndProp()
BeginProp(RFloat, currentPotential, "Current Potential", "The potential this factory is currently using.") {
	FIRReturn self->GetCurrentPotential();
} EndProp()
BeginProp(RFloat, potential, "Potential", "The potential this factory is currently set to and 'should'  use. (the overclock value)\n 0 = 0%, 1 = 100%") {
	FIRReturn self->GetPendingPotential();
} PropSet() {
	float min = self->GetCurrentMinPotential();
	float max = self->GetCurrentMaxPotential();
	self->SetPendingPotential(FMath::Clamp((float)Val, min, max));
} EndProp()
BeginProp(RTrace<UFGInventoryComponent>, potentialInventory, "Potential Inventory", "The Inventory that holds the crystals used for potential.") {
	FIRReturn (Ctx.GetTrace() / self->GetPotentialInventory());
} EndProp()
BeginProp(RBool, canChangeProductionBoost, "Can Change Production Boost", "True if the factory can change its production boost.") {
	FIRReturn self->CanChangeProductionBoost();
} EndProp()
BeginProp(RFloat, maxProductionBoost, "Max Production Boost", "The maximum production boost this factory can be set to (depending on the number of shards).") {
	FIRReturn self->GetCurrentMaxProductionBoost();
} EndProp()
BeginProp(RFloat, maxDefaultProductionBoost, "Max Default Production Boost", "The maximum production boost this factory can be set to.") {
	FIRReturn self->GetMaxProductionBoost();
} EndProp()
BeginProp(RFloat, minDefaultProductionBoost, "Min Default Production Boost", "The minimum production boost this factory has to be set to.") {
	FIRReturn self->GetMinProductionBoost();
} EndProp()
BeginProp(RFloat, currentProductionBoost, "Current Production Boost", "The current production boost this factory uses.") {
	FIRReturn self->GetCurrentProductionBoost();
} EndProp()
BeginProp(RFloat, productionBoost, "Production Boost", "The current production boost this factory should use.") {
	FIRReturn self->GetPendingProductionBoost();
} PropSet() {
	float min = self->GetMinProductionBoost();
	float max = self->GetCurrentMaxProductionBoost();
	self->SetPendingProductionBoost(FMath::Clamp((float)Val, min, max));
} EndProp()
BeginProp(RBool, standby, "Standby", "True if the factory is in standby.") {
	FIRReturn self->IsProductionPaused();
} PropSet() {
	self->SetIsProductionPaused(Val);
} EndProp()
EndClass()

BeginClass(AFGBuildableManufacturer, "Manufacturer", "Manufacturer", "The base class of every machine that uses a recipe to produce something automatically.")
BeginFunc(getRecipe, "Get Recipe", "Returns the currently set recipe of the manufacturer.") {
	OutVal(0, RClass<UFGRecipe>, recipe, "Recipe", "The currently set recipe.")
	Body()
	recipe = (UClass*)self->GetCurrentRecipe();
} EndFunc()
BeginFunc(getRecipes, "Get Recipes", "Returns the list of recipes this manufacturer can get set to and process.") {
	OutVal(0, RArray<RClass<UFGRecipe>>, recipes, "Recipes", "The list of avalible recipes.")
	Body()
	TArray<FIRAny> OutRecipes;
	TArray<TSubclassOf<UFGRecipe>> Recipes;
	self->GetAvailableRecipes(Recipes);
	for (TSubclassOf<UFGRecipe> Recipe : Recipes) {
		OutRecipes.Add((FIRAny)(UClass*)Recipe);
	}
	recipes = OutRecipes;
} EndFunc()
BeginFunc(setRecipe, "Set Recipe", "Sets the currently producing recipe of this manufacturer.", 0) {
	InVal(0, RClass<UFGRecipe>, recipe, "Recipe", "The recipe this manufacturer should produce.")
	OutVal(1, RBool, gotSet, "Got Set", "True if the current recipe got successfully set to the new recipe.")
	Body()
	TArray<TSubclassOf<UFGRecipe>> recipes;
	self->GetAvailableRecipes(recipes);
	if (recipes.Contains(recipe)) {
		TArray<FInventoryStack> stacks;
		self->GetInputInventory()->GetInventoryStacks(stacks);
		self->GetInputInventory()->Empty();
		self->GetOutputInventory()->AddStacks(stacks);
		self->SetRecipe(recipe);
		gotSet = self->GetCurrentRecipe() == recipe;
	} else {
		gotSet = false;
	}
} EndFunc()
BeginFunc(getInputInv, "Get Input Inventory", "Returns the input inventory of this manufacturer.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The input inventory of this manufacturer")
	Body()
	inventory = Ctx.GetTrace() / self->GetInputInventory();
} EndFunc()
BeginFunc(getOutputInv, "Get Output Inventory", "Returns the output inventory of this manufacturer.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The output inventory of this manufacturer.")
	Body()
	inventory = Ctx.GetTrace() / self->GetOutputInventory();
} EndFunc()
EndClass()

BeginClass(UFGFactoryConnectionComponent, "FactoryConnection", "Factory Connection", "A actor component that is a connection point to which a conveyor or pipe can get attached to.")
Hook(UFIRFactoryConnectorHook)
BeginSignal(ItemTransfer, "Item Transfer", "Triggers when the factory connection component transfers an item.")
	SignalParam(0, RStruct<FInventoryItem>, item, "Item", "The transfered item")
EndSignal()
BeginProp(RInt, type, "Type", "Returns the type of the connection. 0 = Conveyor, 1 = Pipe") {
	FIRReturn (int64)self->GetConnector();
} EndProp()
BeginProp(RInt, direction, "Direction", "The direction in which the items/fluids flow. 0 = Input, 1 = Output, 2 = Any, 3 = Used just as snap point") {
	FIRReturn (int64)self->GetDirection();
} EndProp()
BeginProp(RBool, isConnected, "Is Connected", "True if something is connected to this connection.") {
	FIRReturn self->IsConnected();
} EndProp()
BeginProp(RClass<UFGItemDescriptor>, allowedItem, "Allowed Item", "This item type defines which items are the only ones this connector can transfer. Null allows all items to be transfered.") {
	FIRReturn (FIRClass)AFIRSubsystem::GetReflectionSubsystem(self)->GetFactoryConnectorAllowedItem(self);
} PropSet() {
	AFIRSubsystem::GetReflectionSubsystem(self)->SetFactoryConnectorAllowedItem(self, Val);
} EndProp()
BeginProp(RBool, blocked, "Blocked", "True if this connector doesn't transfer any items except the 'Unblocked Transfers'.") {
	FIRReturn AFIRSubsystem::GetReflectionSubsystem(self)->GetFactoryConnectorBlocked(self);
} PropSet() {
	AFIRSubsystem::GetReflectionSubsystem(self)->SetFactoryConnectorBlocked(self, Val);
} EndProp()
BeginProp(RInt, unblockedTransfers, "Unblocked Transfers", "The count of transfers that can still happen even if the connector is blocked. Use the 'AddUnblockedTransfers' function to change this. The count decreases by one when an item gets transfered.") {
	FIRReturn AFIRSubsystem::GetReflectionSubsystem(self)->GetFactoryConnectorUnblockedTransfers(self);
} EndProp()
BeginFunc(addUnblockedTransfers, "Add Unblocked Transfers", "Adds the given count to the unblocked transfers counter. The resulting value gets clamped to >= 0. Negative values allow to decrease the counter manually. The returning int is the now set count.") {
	InVal(0, RInt, unblockedTransfers, "Unblocked Transfers", "The count of unblocked transfers to add.")
	OutVal(1, RInt, newUnblockedTransfers, "New Unblocked Transfers", "The new count of unblocked transfers.")
	Body()
	newUnblockedTransfers = (FIRInt) AFIRSubsystem::GetReflectionSubsystem(self)->AddFactoryConnectorUnblockedTransfers(self, unblockedTransfers);
} EndFunc()
BeginFunc(getInventory, "Get Inventory", "Returns the internal inventory of the connection component.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The internal inventory of the connection component.")
	Body()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getConnected, "Get Connected", "Returns the connected factory connection component.") {
	OutVal(0, RTrace<UFGFactoryConnectionComponent>, connected, "Connected", "The connected factory connection component.")
	Body()
	connected = Ctx.GetTrace() / self->GetConnection();
} EndFunc()
EndClass()

BeginClass(UFGRecipe, "Recipe", "Recipe", "A struct that holds information about a recipe in its class. Means don't use it as object, use it as class type!")
BeginClassProp(RString, name, "Name", "The name of this recipe.") {
	FIRReturn (FIRStr)UFGRecipe::GetRecipeName(self).ToString();
} EndProp()
BeginClassProp(RFloat, duration, "Duration", "The duration how much time it takes to cycle the recipe once.") {
	FIRReturn UFGRecipe::GetManufacturingDuration(self);
} EndProp()
BeginClassFunc(getProducts, "Get Products", "Returns a array of item amounts, this recipe returns (outputs) when the recipe is processed once.", false) {
	OutVal(0, RArray<RStruct<FItemAmount>>, products, "Products", "The products of this recipe.")
	Body()
	TArray<FIRAny> Products;
	for (const FItemAmount& Product : UFGRecipe::GetProducts(self)) {
		Products.Add((FIRAny)Product);
	}
	products = Products;
} EndFunc()
BeginClassFunc(getIngredients, "Get Ingredients", "Returns a array of item amounts, this recipe needs (input) so the recipe can be processed.", false) {
	OutVal(0, RArray<RStruct<FItemAmount>>, ingredients, "Ingredients", "The ingredients of this recipe.")
	Body()
	TArray<FIRAny> Ingredients;
	for (const FItemAmount& Ingredient : UFGRecipe::GetIngredients(self)) {
		Ingredients.Add((FIRAny)Ingredient);
	}
	ingredients = Ingredients;
} EndFunc()
EndClass()
