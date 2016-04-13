#pragma once

#include "Base.h"
#include "Points.h"
#include "../Converters.h"

#include <skeleton/skeleton.hpp>

namespace ofxOrbbec {
	namespace Streams {
		namespace os = orbbec::skeleton;

		class BodyStream : public Base {
		public:
			BodyStream();
			~BodyStream();

			string getTypeName() const override;
			void init(astra::StreamReader & streamReader) override;
			void close() override;
			void update() override;

			shared_ptr<os::BodyTracker> getBodyTracker();
			void setBodyTrackerResolution(int width, int height, bool upscaleOutput = true);

            ofPixels getUserMask(bool copy = false) const;
            ofPixels getLabelsImage(bool copy = false) const;
            ofPixels getProbabilityMap(uint8_t labelIndex, float scaleOutput = 255.0f);

            const vector<os::Body> bodies();

            static void drawSkeleton2D(const os::Body& body);
            static void drawSkeleton3D(const os::Body& body);
            
			static const vector<pair<os::JointType, os::JointType>> & getBonesAtlas();
			
		protected:
			void newFrameArrived(astra::Frame &) override;

			mutex pointFrameLock;

			os::BitmapVector3f pointFrame;
			bool pointFrameNew = true;
			shared_ptr<os::BodyTracker> bodyTracker;
            
		};
	}
}