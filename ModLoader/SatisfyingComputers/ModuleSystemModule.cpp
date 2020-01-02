#include "stdafx.h"
#include "ModuleSystemModule.h"

void IModuleSystemModule::getModuleSize(int & width, int & height) const {}
void IModuleSystemModule::setPanel(UModuleSystemPanel* panel, int x, int y, int rot) {}
SML::Objects::FName IModuleSystemModule::getName() const { return SML::Objects::FName(); }

void UModuleSystemModule::execGetModuleSize(IModuleSystemModule * self, SML::Objects::FFrame & stack, void * ret) {
	int width_t = 0;
	int height_t = 0;

	auto& width = stack.stepCompInRef<int>(&width_t);
	auto& height = stack.stepCompInRef<int>(&height_t);
	
	stack.code += !!stack.code;

	self->getModuleSize(width, height);
}

void UModuleSystemModule::execSetPanel(IModuleSystemModule * self, SML::Objects::FFrame & stack, void * ret) {
	UModuleSystemPanel* panel;
	int x;
	int y;
	int rot;

	stack.stepCompIn(&panel);
	stack.stepCompIn(&x);
	stack.stepCompIn(&y);
	stack.stepCompIn(&rot);

	stack.code += !!stack.code;

	self->setPanel(panel, x, y, rot);
}

void UModuleSystemModule::execGetName(IModuleSystemModule * self, SML::Objects::FFrame & stack, void * ret) {
	stack.code += !!stack.code;

	*((SML::Objects::FName*)ret) = self->getName();
}

SML::Objects::UClass * UModuleSystemModule::staticClass() {
	return SML::Paks::ClassBuilder<UModuleSystemModule>::staticClass();
}
