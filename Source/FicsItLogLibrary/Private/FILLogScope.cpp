#include "FILLogScope.h"

UFILLogContainer*& FFILLogScope::GetCurrentLog() {
	thread_local UFILLogContainer* CurrentLog = nullptr;
	return CurrentLog;
}

FFILLogScope::FWhereFunction& FFILLogScope::GetCurrentWhereFunction() {
	thread_local FWhereFunction WhereFunction;
	return WhereFunction;
}

FFILLogScope::FStackFunction& FFILLogScope::GetCurrentStackFunction() {
	thread_local FStackFunction StackFunction;
	return StackFunction;
}
