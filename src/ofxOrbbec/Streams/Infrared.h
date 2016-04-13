#pragma once

#include "Base.h"

namespace ofxOrbbec {
	namespace Streams {
		class Infrared : public TemplateBaseImage<astra::InfraredStream, astra::InfraredFrame16, unsigned short> {
		public:
			string getTypeName() const override {
				return "Infrared";
			}
			void init(astra::StreamReader & streamReader) override;

		protected:
			int getNumChannels() override {
				return 1;
			}
		};
	}
}