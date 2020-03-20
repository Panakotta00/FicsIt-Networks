#include "FINSignal.h"

FFINSignal::FFINSignal() : signal(new FicsItKernel::Network::NoSignal()) {}

FFINSignal::FFINSignal(const std::shared_ptr<FicsItKernel::Network::Signal>& signal) : signal(signal) {
	
}

FFINSignal::operator std::shared_ptr<FicsItKernel::Network::Signal>() {
	return signal;
}
