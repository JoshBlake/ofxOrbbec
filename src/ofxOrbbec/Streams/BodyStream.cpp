#include "BodyStream.h"

#include "ofGraphics.h"

namespace ofxOrbbec {
	namespace Streams {

		//----------
		BodyStream::BodyStream() {
			
		}

		//----------
		BodyStream::~BodyStream() {
			this->close();
		}

		//----------
		string BodyStream::getTypeName() const {
			return "Skeleton";
		}

		//----------
		void BodyStream::init(astra::StreamReader & streamReader) {
			if (!ofFile::doesFileExist("../orbbec.classifier")) {
				ofLogError("ofxOrbbec::Streams::Skeleton") << "orbbec.classifier file is missing";
				return;
			}

			//start the body tracker at recommended resolution
			//this function also creates the body tracker
			this->setBodyTrackerResolution(160, 120);
		}

		//----------
		void BodyStream::close() {
			this->bodyTracker.reset();
		}

		//----------
		void BodyStream::update() {
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
		shared_ptr<os::BodyTracker> BodyStream::getBodyTracker() {
			return this->bodyTracker;
		}

		//----------
		void BodyStream::setBodyTrackerResolution(int width, int height, bool upscaleOutput) {
            auto outputSize = upscaleOutput ? os::OutputSize::InputSized : os::OutputSize::ProcessingSized;
			this->bodyTracker = make_shared<os::BodyTracker>(58.59f, 45.64f, width, height, outputSize);
            //depth/IR:               58.59f, 45.64f
            //color/registered depth: 62.70f, 49.00f

			this->bodyTracker->set_classifier(os::load_classifier(ofToDataPath("../orbbec.classifier").c_str()));
		}


        //----------
        ofPixels BodyStream::getUserMask(bool copy) const {
            return toOf(this->bodyTracker->user_field());
        }

        //----------
        ofPixels BodyStream::getLabelsImage(bool copy) const {
            return toOf(this->bodyTracker->labels());
        }

        //----------
        ofPixels BodyStream::getProbabilityMap(uint8_t labelIndex, float scaleOutput) {
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
        void BodyStream::drawSkeleton2D(const os::Body& body) {
            const auto & joints = body.joints();
            const auto & bonesAtlas = getBonesAtlas();
            ofPushMatrix();
            {
                ofScale(1, 1, 0); // flatten depth away
                for (auto bone : bonesAtlas) {
                    const auto & firstJoint = joints.find(bone.first);
                    const auto & secondJoint = joints.find(bone.second);
                    if (firstJoint != joints.end() &&
                        secondJoint != joints.end() &&
                        firstJoint->second.status() == os::TrackingStatus::Tracked &&
                        secondJoint->second.status() == os::TrackingStatus::Tracked) {
                        ofDrawLine(toOf(firstJoint->second.depth_position()), toOf(secondJoint->second.depth_position()));
                    }
                }
            }
            ofPopMatrix();
        }

        //----------
        void BodyStream::drawSkeleton3D(const os::Body& body) {
            const auto & joints = body.joints();
            const auto & bonesAtlas = getBonesAtlas();
            for (auto bone : bonesAtlas) {
                const auto & firstJoint = joints.find(bone.first);
                const auto & secondJoint = joints.find(bone.second);
                if (firstJoint != joints.end() &&
                    secondJoint != joints.end() &&
                    firstJoint->second.status() == os::TrackingStatus::Tracked &&
                    secondJoint->second.status() == os::TrackingStatus::Tracked) {
                    ofDrawLine(toOf(firstJoint->second.world_position()), toOf(secondJoint->second.world_position()));
                }
            }
        }

        const vector<os::Body> BodyStream::bodies()
        {
            //make a copy
            const vector<os::Body> bodies = this->bodyTracker->bodies();
            return bodies;
        }

		//----------
		const vector<pair<os::JointType, os::JointType>> & BodyStream::getBonesAtlas() {
			static vector<pair<os::JointType, os::JointType>> bonesAtlas;
			if (bonesAtlas.empty()) {
				bonesAtlas.emplace_back(os::JointType::Head, os::JointType::Neck);

				bonesAtlas.emplace_back(os::JointType::Neck, os::JointType::LeftShoulder);
				bonesAtlas.emplace_back(os::JointType::Neck, os::JointType::RightShoulder);

				bonesAtlas.emplace_back(os::JointType::LeftShoulder, os::JointType::LeftElbow);
				bonesAtlas.emplace_back(os::JointType::LeftElbow, os::JointType::LeftHand);

				bonesAtlas.emplace_back(os::JointType::RightShoulder, os::JointType::RightElbow);
				bonesAtlas.emplace_back(os::JointType::RightElbow, os::JointType::RightHand);

				bonesAtlas.emplace_back(os::JointType::Neck, os::JointType::Chest);
				bonesAtlas.emplace_back(os::JointType::Chest, os::JointType::CenterHips);

				bonesAtlas.emplace_back(os::JointType::CenterHips, os::JointType::LeftHips);
				bonesAtlas.emplace_back(os::JointType::LeftHips, os::JointType::LeftKnee);
				bonesAtlas.emplace_back(os::JointType::LeftKnee, os::JointType::LeftFoot);

				bonesAtlas.emplace_back(os::JointType::CenterHips, os::JointType::RightHips);
				bonesAtlas.emplace_back(os::JointType::RightHips, os::JointType::RightKnee);
				bonesAtlas.emplace_back(os::JointType::RightKnee, os::JointType::RightFoot);
			}
			return bonesAtlas;
		}

		//----------
		void BodyStream::newFrameArrived(astra::Frame & frame) {
			auto pointFrame = frame.get<astra::PointFrame>();
			if (pointFrame.is_valid()) {
				this->pointFrameLock.lock();
				{
					this->pointFrame.recreate(pointFrame.width(), pointFrame.height());
					memcpy(this->pointFrame.data(), pointFrame.data(), this->pointFrame.byteLength());
					this->pointFrameNew = true;
				}
				this->pointFrameLock.unlock();
			}
		}
	}
}