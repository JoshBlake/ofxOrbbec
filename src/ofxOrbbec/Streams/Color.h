#pragma once

#include "Base.h"

namespace ofxOrbbec {
	namespace Streams {
		class Color : public TemplateBaseImage<astra::ColorStream, astra::ColorFrame, unsigned char> {
		public:
			string getTypeName() const override {
				return "Color";
			}
		protected:
			int getNumChannels() override {
				return 3;
			}
		};
	}
}