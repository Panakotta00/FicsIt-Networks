#pragma once

#include "FicsItKernel/Processor/Processor.h"

namespace FicsItKernel {
	namespace FIVS {
		class FIVSProcessor : public Processor {
			public:
			// Begin Processor
			virtual void tick(float delta) override;
			virtual int64 getMemoryUsage(bool recalc) override;
			virtual void reset() override;
			virtual void setEEPROM(AFINStateEEPROM* eeprom) override;
			virtual void PreSerialize(UProcessorStateStorage* Storage, bool bLoading) override;
			virtual void Serialize(UProcessorStateStorage* Storage, bool bLoading) override;
			virtual void PostSerialize(UProcessorStateStorage* Storage, bool bLoading) override;
			virtual UProcessorStateStorage* CreateSerializationStorage() override;
			// End Processor
		};
	}
}
