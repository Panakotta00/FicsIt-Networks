#include "FINSignalListener.h"

IFINSignalListener::SignalListenerImpl::SignalListenerImpl(IFINSignalListener* parent) : parent(parent) {}

void IFINSignalListener::SignalListenerImpl::handleSignal(std::shared_ptr<FicsItKernel::Network::Signal> signal, FicsItKernel::Network::NetworkTrace sender) {
	parent->HandleSignal(FFINSignal(signal), sender);
}

IFINSignalListener::IFINSignalListener() : impl(new SignalListenerImpl(this)) {}

IFINSignalListener::operator std::shared_ptr<FicsItKernel::Network::SignalListener>() {
	return impl;
}
