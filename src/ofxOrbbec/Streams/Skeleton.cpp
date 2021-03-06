#include "Skeleton.h"

#include "ofGraphics.h"

namespace ofxOrbbec {
	namespace Streams {
		//----------
		Skeleton::Skeleton() {
			
		}

		//----------
		Skeleton::~Skeleton() {
			this->close();
		}

		//----------
		string Skeleton::getTypeName() const {
			return "Skeleton";
		}

		//----------
		void Skeleton::init(astra::stream_reader & streamReader) {
			if (!ofFile::doesFileExist("../orbbec.classifier")) {
				ofLogError("ofxOrbbec::Streams::Skeleton") << "orbbec.classifier file is missing";
				return;
			}

			//start the body tracker at recommended resolution
			//this function also creates the body tracker
			this->setBodyTrackerResolution(160, 120);
		}

		//----------
		void Skeleton::close() {
			this->bodyTracker.reset();
		}

		//----------
		void Skeleton::update() {
			this->frameNew = false;
			if (!this->bodyTracker) {
				ofLogError("ofxOrbbec::Streams::Skeleton") << "You must successfully init the skeleton tracker before calling update";
				return;
			}

			if (this->pointFrameNew) {
				this->pointFrameLock.lock();
				{
					this->bodyTracker->update(this->pointFrame);
					this->pointFrameNew = false;
				}
				this->pointFrameLock.unlock();
				this->frameNew = true;
			}
		}

		//----------
		shared_ptr<os::body_tracker> Skeleton::getBodyTracker() {
			return this->bodyTracker;
		}

		//----------
		void Skeleton::setBodyTrackerResolution(int width, int height) {
			this->bodyTracker = make_shared<os::body_tracker>(60.0f, 49.5, width, height);

			this->bodyTracker->set_classifier(os::load_classifier(ofToDataPath("../orbbec.classifier").c_str()));
		}

		//----------
		void Skeleton::enableUpscaling(bool enabled) {
			this->bodyTracker->set_upscale_outputs(enabled);
		}

		//----------
		bool Skeleton::getUpscalingEnabled() const {
			return this->bodyTracker->upscale_outputs_enabled();
		}

		//----------
		ofPixels Skeleton::getUserMask(bool copy) const {
			return toOf(this->bodyTracker->user_mask());
		}

		//----------
		ofPixels Skeleton::getLabelsImage(bool copy) const {
			return toOf(this->bodyTracker->labels());
		}

		//----------
		ofPixels Skeleton::getProbabilityMap(uint8_t labelIndex, float scaleOutput) {
			const auto & labelProbabilities = this->bodyTracker->label_probabilities();
			
			ofPixels pixels;
			pixels.allocate(this->bodyTracker->width(), this->bodyTracker->height(), 1);
			auto output = pixels.getData();

			for (int i = 0; i < pixels.size(); i++) {
				const auto & pixel = labelProbabilities[i];
				*output++ = pixel.probability_for_label(labelIndex) * scaleOutput;
			}

			return pixels;
		}

		//----------
		os::joint_map_type Skeleton::getJointsRaw() const {
#ifdef _DEBUG
			ofLogWarning("ofxOrbbec") << "getJointsRaw is only supported in Release builds. Really sorry! It just crashes otherwise... :(";
			return os::joint_map_type();
#endif
			return this->bodyTracker->joints();
		}

		//----------
		void Skeleton::drawSkeleton2D() const {
			const auto & joints = this->getJointsRaw();
			const auto & bonesAtlas = getBonesAtlas();
			ofPushMatrix();
			{
				ofScale(1, 1, 0); // flatten depth away
				for (auto bone : bonesAtlas) {
					const auto & firstJoint = joints.find(bone.first);
					const auto & secondJoint = joints.find(bone.second);
					if (firstJoint != joints.end() && secondJoint != joints.end()) {
						ofLine(toOf(firstJoint->second.depth_position), toOf(secondJoint->second.depth_position));
					}
				}
			}
			ofPopMatrix();
		}

		//----------
		void Skeleton::drawSkeleton3D() const {
			const auto & joints = this->getJointsRaw();
			const auto & bonesAtlas = getBonesAtlas();
			for (auto bone : bonesAtlas) {
				const auto & firstJoint = joints.find(bone.first);
				const auto & secondJoint = joints.find(bone.second);
				if (firstJoint != joints.end() && secondJoint != joints.end()) {
					ofLine(toOf(firstJoint->second.world_position), toOf(secondJoint->second.world_position));
				}
			}
		}

		//----------
		const vector<pair<os::joint_type, os::joint_type>> & Skeleton::getBonesAtlas() {
			static vector<pair<os::joint_type, os::joint_type>> bonesAtlas;
			if (bonesAtlas.empty()) {
				bonesAtlas.emplace_back(os::joint_type::head, os::joint_type::neck);

				bonesAtlas.emplace_back(os::joint_type::neck, os::joint_type::left_shoulder);
				bonesAtlas.emplace_back(os::joint_type::neck, os::joint_type::right_shoulder);

				bonesAtlas.emplace_back(os::joint_type::left_shoulder, os::joint_type::left_elbow);
				bonesAtlas.emplace_back(os::joint_type::left_elbow, os::joint_type::left_hand);

				bonesAtlas.emplace_back(os::joint_type::right_shoulder, os::joint_type::right_elbow);
				bonesAtlas.emplace_back(os::joint_type::right_elbow, os::joint_type::right_hand);

				bonesAtlas.emplace_back(os::joint_type::neck, os::joint_type::chest);
				bonesAtlas.emplace_back(os::joint_type::chest, os::joint_type::center_hips);

				bonesAtlas.emplace_back(os::joint_type::center_hips, os::joint_type::left_hips);
				bonesAtlas.emplace_back(os::joint_type::left_hips, os::joint_type::left_knee);
				bonesAtlas.emplace_back(os::joint_type::left_knee, os::joint_type::left_foot);

				bonesAtlas.emplace_back(os::joint_type::center_hips, os::joint_type::right_hips);
				bonesAtlas.emplace_back(os::joint_type::right_hips, os::joint_type::right_knee);
				bonesAtlas.emplace_back(os::joint_type::right_knee, os::joint_type::right_foot);
			}
			return bonesAtlas;
		}

		//----------
		void Skeleton::newFrameArrived(astra::frame & frame) {
			auto pointFrame = frame.get<astra::pointframe>();
			if (pointFrame.is_valid()) {
				this->pointFrameLock.lock();
				{
					this->pointFrame.recreate(pointFrame.resolutionX(), pointFrame.resolutionY());
					memcpy(this->pointFrame.data(), pointFrame.data(), pointFrame.byteLength());
					this->pointFrameNew = true;
				}
				this->pointFrameLock.unlock();
			}
		}
	}
}