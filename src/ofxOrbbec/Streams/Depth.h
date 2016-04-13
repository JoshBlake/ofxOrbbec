#pragma once

#include "Base.h"

namespace ofxOrbbec {
	namespace Streams {
		class Depth : public TemplateBaseImage<astra::DepthStream, astra::DepthFrame, unsigned short> {
		public:
			string getTypeName() const override {
				return "Depth";
			}

			const astra::CoordinateMapper & getCoordinateMapper() const;

			ofVec3f depthToWorld(const ofVec3f & depthMapPosition);
			ofVec3f depthToWorld(float x, float y, float depth);

			ofVec3f worldToDepth(const ofVec3f & worldPosition);
			ofVec3f worldToDepth(float x, float y, float z);
		protected:
			int getNumChannels() override {
				return 1;
			}
		};
	}
}