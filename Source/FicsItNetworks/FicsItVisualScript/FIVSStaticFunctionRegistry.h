#pragma once

class FFIVSStaticFunctionRegistry {
private:
	static FFIVSStaticFunctionRegistry SingletonInstance;
	FFIVSStaticFunctionRegistry();

	
public:
	static FFIVSStaticFunctionRegistry& Get() { return SingletonInstance; }

	
};


